#include <core/mods/inference/d2_std.hpp>
#include <os/inference/openai/d2_openai.hpp>
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace d2;
using namespace d2::ai;

mt::future<void> run(Conversation::ptr chat, std::shared_ptr<const ToolSet> tools)
{
    for (int round = 0; round < 16; ++round)
    {
        auto generation = chat->step({}, [](const Event& event)
        {
            if (const auto* text = std::get_if<TextDelta>(&event)) std::cout << text->text << std::flush;
        });
        const auto response = co_await generation.result;
        std::cout << '\n';
        if (response.finish != Finish::ToolCalls) co_return;
        for (const auto& [id, call] : chat->pending_tools())
            chat->add_tool_result(co_await tools->execute(call));
    }
    throw std::runtime_error("Example tool round limit reached");
}

int main(int argc, char** argv)
{
    if (argc < 4 || argc > 5)
    {
        std::cerr << "Usage: d2_inference ENDPOINT MODEL PROMPT [NEXT_MODEL]\n"
                     "Optional environment: D2_OPENAI_API_KEY, D2_OPENAI_TOKEN_FIELD, D2_OPENAI_REASONING_FIELD\n";
        return 2;
    }
    try
    {
        auto context = std::make_shared<IOContext>(mt::ConcurrentPool::make<>(), nullptr);
        context->load<sys::net::SystemHTTP, sys::SystemOpenAI, sys::SystemConversations>();
        auto provider = context->sys<sys::SystemProvider>()->ptr<sys::SystemOpenAI>();
        openai::Config endpoint; endpoint.endpoint = argv[1];
        if (auto key = std::getenv("D2_OPENAI_API_KEY")) endpoint.api_key = key;
        if (auto field = std::getenv("D2_OPENAI_TOKEN_FIELD")) endpoint.compatibility.token_limit_field = field;
        if (auto field = std::getenv("D2_OPENAI_REASONING_FIELD")) endpoint.compatibility.reasoning_field = field;
        provider->configure(std::move(endpoint));
        auto conversations = context->sys<sys::SystemConversations>();
        conversations->bind("endpoint", provider.ptr());
        Tool add;
        add.definition = {"add", "Add two integers", {
            {"type", "object"}, {"properties", {{"a", {{"type", "integer"}}}, {"b", {{"type", "integer"}}}}},
            {"required", {"a", "b"}}, {"additionalProperties", false}}};
        add.invoke = [](Json args, std::stop_token) -> mt::future<Json>
        {
            auto a = args.at("a").get<std::int32_t>(), b = args.at("b").get<std::int32_t>();
            co_return std::int64_t(a) + b;
        };
        add.validate = [](const Json& args)
        {
            if (args.size() != 2 || !args.contains("a") || !args.contains("b") ||
                !args["a"].is_number_integer() || !args["b"].is_number_integer())
                throw std::invalid_argument("Expected integer arguments a and b");
            for (const auto* key : {"a", "b"})
                if (args[key] < std::numeric_limits<std::int32_t>::min() ||
                    args[key] > std::numeric_limits<std::int32_t>::max())
                    throw std::invalid_argument("Example arguments must fit a signed 32-bit integer");
        };
        auto tools = std::make_shared<ToolSet>(std::vector<Tool>{std::move(add)});
        ConversationConfig config; config.model = {"endpoint", argv[2]}; config.tools = tools;
        auto chat = conversations->create("example", config);
        chat->append(Message::text(Role::User, argv[3]));
        run(chat, tools).value();
        if (argc == 5)
        {
            chat->switch_model({"endpoint", argv[4]}, {}, StateTransfer::DropOpaque);
            chat->append(Message::text(Role::User, "Briefly check the preceding answer."));
            run(chat, tools).value();
        }
        context->unload<sys::SystemConversations>();
        context->unload<sys::SystemOpenAI>();
        context->unload<sys::net::SystemHTTP>();
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
