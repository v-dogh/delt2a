#include "check.hpp"
#include <core/mods/inference/d2_std.hpp>
#include <core/io/d2_context.hpp>
#include <thread>

using namespace d2;
using namespace d2::ai;

struct Provider : sys::SystemProvider, sys::ModuleImpl<Provider, sys::Load::Immediate>
{
    using SystemProvider::SystemProvider;
    std::vector<Request> requests;
    Response answer;
    bool immediate{true};
    mt::future<Response> pending;
    std::stop_token token;
    ModelInfo model_info(std::string_view model) const override
    {
        ModelInfo info;
        info.context_window = 100;
        for (auto f : {Feature::Tools, Feature::StrictTools, Feature::Images, Feature::Temperature,
                       Feature::ParallelTools, Feature::ForcedTools, Feature::JsonSchema})
            info.features[f] = Support::Supported;
        if (model == "small") info.features[Feature::Temperature] = Support::Unsupported;
        return info;
    }
    mt::future<Response> _generate_impl(Request request, EventHandler events, std::stop_token stop) override
    {
        requests.push_back(std::move(request)); token = stop;
        pending = mt::future<Response>::make();
        if (events) events(TextDelta{0,"stream"});
        if (immediate) pending.block()->set_value(answer);
        return pending;
    }
    void finish() { pending.block()->set_value(answer); }
};
void expect(ErrorCode code, auto action)
{
    try { action(); }
    catch (const Error& error) { CHECK(error.code == code); return; }
    throw std::runtime_error("Expected inference error");
}
mt::future<Json> add(Json args, std::stop_token) { co_return args.at("a").get<int>() + args.at("b").get<int>(); }
mt::future<std::vector<Message>> summary(CompactionInput input, std::stop_token)
{
    std::vector<Message> messages;
    for (const auto& message : input.request.messages) if (message.pinned) messages.push_back(message);
    messages.push_back(Message::text(Role::User,"summary"));
    co_return messages;
}
Response reply(std::string text)
{
    Response response; response.message = Message::text(Role::Assistant,std::move(text));
    response.usage.output_tokens = 7;
    return response;
}
int main()
{
    auto ctx = std::make_shared<IOContext>(mt::ConcurrentPool::make<>(), nullptr);
    ctx->load<sys::SystemConversations>();
    auto module = ctx->sys<sys::SystemConversations>();
    auto first = std::shared_ptr<Provider>(sys::SystemModule::make<Provider>({}));
    auto second = std::shared_ptr<Provider>(sys::SystemModule::make<Provider>({}));
    first->answer = reply("first"); second->answer = reply("second");
    module->bind("local", first); module->bind("remote", second);
    ConversationConfig config; config.model = {"local","large"};
    config.settings.extensions["local"] = {{"local_setting",1}};
    config.settings.extensions["remote"] = {{"remote_setting",2}};
    auto conversation = module->create("chat", config);
    conversation->append(Message::text(Role::System,"instructions",true));
    conversation->append(Message::text(Role::User,"hello"));
    bool streamed = false;
    const auto response = conversation->step({},[&](const Event& event) { streamed = std::holds_alternative<TextDelta>(event); }).result.value();
    CHECK(streamed && response.message.origin == config.model && response.message.id == 3);
    CHECK(first->requests.back().settings.extensions.size() == 1);
    conversation->switch_model({"remote","large"});
    conversation->append(Message::text(Role::User,"continue"));
    conversation->step().result.value();
    CHECK(second->requests.back().messages.size() == 4);
    CHECK(second->requests.back().messages[2].origin == config.model);
    CHECK(second->requests.back().settings.extensions.count("local") == 0);
    CHECK(conversation->snapshot().records.size() == 2);
    CHECK(conversation->snapshot().records.back().usage->input_tokens == std::nullopt);
    CHECK(conversation->snapshot().records.back().usage->output_tokens == 7);

    auto child = conversation->fork();
    child->append(Message::text(Role::User,"branch"));
    CHECK(child->snapshot().transcript.size() == conversation->snapshot().transcript.size() + 1);
    auto changed = conversation->snapshot().config;
    changed.settings.sampling.temperature = 0.5;
    conversation->configure(changed);
    expect(ErrorCode::Unsupported,[&] { conversation->switch_model({"local","small"}); });
    CHECK(conversation->snapshot().config.model.provider == "remote");
    conversation->switch_model({"local","small"}, Settings{});
    first->answer = reply("private state");
    first->answer.message.content.push_back(Opaque{{"local","small"},"continuation",{{"token","opaque"}}});
    conversation->step().result.value();
    const auto transcript = conversation->snapshot().transcript;
    expect(ErrorCode::IncompatibleState,[&] { conversation->switch_model({"local","large"}); });
    conversation->switch_model({"local","large"}, {}, StateTransfer::DropOpaque);
    CHECK(conversation->snapshot().transcript == transcript);
    CHECK(conversation->snapshot().context.back().content.size() == 1);

    auto tools = std::make_shared<ToolSet>(std::vector<Tool>{
        {ToolDefinition{"add","Add integers",{{"type","object"}}}, add,
         [](const Json& args) { if (!args.contains("a") || !args.contains("b")) throw std::invalid_argument("missing operands"); }}
    });
    changed = conversation->snapshot().config; changed.tools = tools;
    conversation->configure(changed);
    first->answer = {}; first->answer.finish = Finish::ToolCalls;
    first->answer.message.content = {ToolCall{"c1","add",{{"a",2},{"b",3}}}, ToolCall{"c2","add",{{"a",4},{"b",5}}}};
    conversation->step().result.value();
    CHECK(conversation->pending_tools().size() == 2);
    expect(ErrorCode::Busy,[&] { conversation->switch_model({"remote","large"}); });
    expect(ErrorCode::Busy,[&] { conversation->step(); });
    expect(ErrorCode::InvalidRequest,[&] { conversation->append(Message::text(Role::User,"too soon")); });
    const auto calls = conversation->pending_tools();
    auto result = tools->execute(calls.at("c2")).value();
    CHECK(result.value == 9 && !result.error);
    conversation->add_tool_result(result);
    expect(ErrorCode::InvalidRequest,[&] { conversation->add_tool_result(result); });
    conversation->add_tool_result(tools->execute(calls.at("c1")).value());
    CHECK(conversation->pending_tools().empty());
    CHECK(tools->execute({"bad","add",Json::object()}).value().error);
    first->answer = reply("done");
    conversation->step().result.value();

    auto compacted = module->create("compact", config);
    compacted->append(Message::text(Role::System,"pin",true));
    for (int i = 0; i < 5; ++i) compacted->append(Message::text(Role::User,"old"));
    auto compact_config = config;
    ContextPolicy policy; policy.max_input_tokens = 30; policy.reserve_output_tokens = 10;
    policy.count = [](const Request& request) { return TokenCount{request.messages.size() * 10, false}; };
    policy.compact = summary;
    compact_config.context = policy;
    compacted->configure(compact_config);
    first->answer = reply("after compaction");
    compacted->step().result.value();
    CHECK(compacted->snapshot().context.size() == 3 && compacted->snapshot().transcript.size() == 7);
    CHECK(compacted->snapshot().context[0].pinned);
    auto broken = module->create("broken", compact_config);
    broken->append(Message::text(Role::System,"pin",true));
    for (int i = 0; i < 5; ++i) broken->append(Message::text(Role::User,"old"));
    auto broken_config = compact_config;
    broken_config.context->compact = [](CompactionInput, std::stop_token) -> mt::future<std::vector<Message>>
    { co_return std::vector<Message>{Message::text(Role::User,"lost pins")}; };
    broken->configure(broken_config);
    const auto before = broken->snapshot();
    expect(ErrorCode::InvalidRequest,[&] { broken->step().result.value(); });
    CHECK(!broken->busy() && broken->snapshot().context == before.context);

    const auto retained = compacted->snapshot().transcript;
    expect(ErrorCode::InvalidRequest,[&] { compacted->replace_context({Message::text(Role::User,"lost pins")}); });
    compacted->replace_context({compacted->snapshot().context.front(), Message::text(Role::User,"retrieved context")});
    CHECK(compacted->snapshot().transcript == retained);
    CHECK(compacted->snapshot().context.back().id > retained.back().id);
    auto restored = module->restore("restored", compacted->snapshot());
    CHECK(restored->snapshot().context == compacted->snapshot().context);
    restored->replace_context({Message::text(Role::User,"fresh context")}, false);
    CHECK(restored->snapshot().context.size() == 1 && restored->snapshot().transcript == retained);
    auto forged = restored->snapshot().context;
    std::get<Text>(forged.front().content.front()).value = "changed without a new ID";
    expect(ErrorCode::InvalidRequest,[&] { restored->replace_context(forged); });
    expect(ErrorCode::InvalidRequest,[&] { module->bind("local", second); });

    auto asynchronous = module->create("async-compact", compact_config);
    for (int i = 0; i < 5; ++i) asynchronous->append(Message::text(Role::User,"old"));
    auto compaction = mt::future<std::vector<Message>>::make();
    auto async_config = compact_config;
    std::stop_token compact_stop;
    async_config.context->compact = [&](CompactionInput, std::stop_token stop)
    { compact_stop = stop; return compaction; };
    asynchronous->configure(async_config);
    const auto async_before = asynchronous->snapshot();
    const auto request_count = first->requests.size();
    auto compact_flight = asynchronous->step();
    CHECK(asynchronous->busy()); compact_flight.cancel();
    CHECK(compact_stop.stop_requested());
    compaction.block()->set_value(std::vector<Message>{Message::text(Role::User,"summary")});
    expect(ErrorCode::Cancelled,[&] { compact_flight.result.value(); });
    CHECK(!asynchronous->busy() && asynchronous->snapshot().context == async_before.context);
    CHECK(first->requests.size() == request_count);

    const auto valid_history = conversation->snapshot().context;
    first->answer = {}; first->answer.finish = Finish::ToolCalls;
    first->answer.message.content = {ToolCall{"unregistered","missing",Json::object()}};
    expect(ErrorCode::Protocol,[&] { conversation->step().result.value(); });
    first->answer.message.content = {ToolCall{"partial","add",Json::object()}};
    first->answer.finish = Finish::Length;
    expect(ErrorCode::Protocol,[&] { conversation->step().result.value(); });
    first->answer.finish = Finish::ToolCalls;
    first->answer.message.content.push_back(ToolCall{"parallel","add",Json::object()});
    StepOptions serial; serial.settings = conversation->snapshot().config.settings;
    serial.settings->parallel_tools = false;
    expect(ErrorCode::Protocol,[&] { conversation->step(serial).result.value(); });
    first->answer = reply("ignored required tool");
    StepOptions named; named.settings = conversation->snapshot().config.settings;
    named.settings->tool_choice = {ToolChoice::Kind::Named,"add"};
    expect(ErrorCode::Protocol,[&] { conversation->step(named).result.value(); });
    CHECK(!conversation->busy() && conversation->snapshot().context == valid_history);
    CHECK(conversation->snapshot().records.back().error == ErrorCode::Protocol);
    StepOptions expired; expired.deadline = std::chrono::steady_clock::now();
    const auto before_expiry = first->requests.size();
    expect(ErrorCode::Cancelled,[&] { conversation->step(expired).result.value(); });
    CHECK(first->requests.size() == before_expiry);
    first->answer = reply("done");

    first->immediate = false;
    auto flight = conversation->step();
    CHECK(conversation->busy());
    expect(ErrorCode::Busy,[&] { conversation->switch_model({"remote","large"}); });
    const auto old = conversation->snapshot().transcript;
    flight.cancel(); CHECK(first->token.stop_requested());
    std::thread complete([&] { first->finish(); }); complete.join();
    expect(ErrorCode::Cancelled,[&] { flight.result.value(); });
    CHECK(!conversation->busy() && conversation->snapshot().transcript == old);
    auto last = conversation->step();
    ctx->unload<sys::SystemConversations>();
    CHECK(first->token.stop_requested()); first->finish();
    expect(ErrorCode::Cancelled,[&] { last.result.value(); });
    expect(ErrorCode::Closed,[&] { conversation->append(Message::text(Role::User,"closed")); });

    Request invalid; invalid.model = {"local","large"}; invalid.settings.sampling.top_p = 2;
    expect(ErrorCode::InvalidRequest,[&] { first->generate(invalid); });
    invalid.settings = {}; invalid.settings.unknown_capabilities = UnknownCapabilities::Reject;
    invalid.settings.sampling.seed = 0;
    expect(ErrorCode::Unsupported,[&] { first->generate(invalid); });
}
