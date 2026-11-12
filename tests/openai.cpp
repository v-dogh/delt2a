#include "check.hpp"
#include "openai_fixtures.hpp"
#include <os/inference/openai/d2_openai_protocol.hpp>
#include <thread>

using namespace d2;
using namespace d2::ai;
using namespace d2::ai::openai;
using namespace openai_test;
namespace wire = d2::ai::openai::detail;

struct InlineHTTP : sys::net::SystemHTTP, sys::ModuleImpl<InlineHTTP, sys::Load::Immediate>
{
    using SystemHTTP::SystemHTTP;
    struct Handle : Stream
    {
        void write(std::string_view) override {}
        void finish() override {}
        void cancel() override {}
        bool is_open() override { return false; }
        std::size_t up_current() override { return 0; }
        std::size_t up_total() override { return 0; }
    };
    Stream::ptr _request_impl(SystemHTTP::Request request) override
    {
        request.on_error({SystemHTTP::Error::Code::Transport, "inline error", {}, {}});
        return std::make_unique<Handle>();
    }
};

int main()
{
    auto input = request();
    Compatibility compatibility;
    Config config;
    wire::validate_config(config);
    auto bad_config = config; bad_config.endpoint = "file:///tmp/test";
    expect(ErrorCode::InvalidRequest, [&] { wire::validate_config(bad_config); });
    bad_config = config; bad_config.headers["X-Test"] = "bad\r\nheader";
    expect(ErrorCode::InvalidRequest, [&] { wire::validate_config(bad_config); });
    bad_config = config; bad_config.api_key = "test"; bad_config.headers["authorization"] = "other";
    expect(ErrorCode::InvalidRequest, [&] { wire::validate_config(bad_config); });

    input.settings.sampling.temperature = 0;
    input.settings.sampling.max_output_tokens = 123;
    input.settings.parallel_tools = false;
    input.settings.extensions["local"] = {{"chat_template_kwargs", {{"enable_thinking", false}}}};
    input.settings.extensions["remote"] = {{"model", "must-not-leak"}};
    input.settings.output = {OutputFormat::Kind::JsonSchema, "result", {{"type", "object"}}, true};
    auto encoded = wire::encode(input, compatibility);
    CHECK(encoded["temperature"] == 0 && encoded["parallel_tool_calls"] == false);
    CHECK(encoded["max_completion_tokens"] == 123 && encoded["model"] == "test");
    CHECK(encoded["response_format"]["json_schema"]["strict"] == true);
    CHECK(encoded["chat_template_kwargs"]["enable_thinking"] == false);
    CHECK(encoded["stream_options"]["include_usage"] == true);
    input.settings.extensions["local"]["model"] = "override";
    expect(ErrorCode::InvalidRequest, [&] { wire::encode(input, compatibility); });
    input.settings.extensions["local"].erase("model");
    input.settings.sampling.top_k = 40;
    expect(ErrorCode::Unsupported, [&] { wire::encode(input, compatibility); });
    compatibility.extended_sampling = true;
    compatibility.token_limit_field = "max_tokens";
    compatibility.include_usage = false;
    compatibility.developer_as_system = true;
    input.messages.insert(input.messages.begin(), Message::text(Role::Developer, "instructions"));
    encoded = wire::encode(input, compatibility);
    CHECK(encoded["top_k"] == 40 && encoded["max_tokens"] == 123);
    CHECK(!encoded.contains("stream_options") && encoded["messages"][0]["role"] == "system");
    input.messages.push_back({Role::User, {Text{"image"}, Image{"data:image/png;base64,AA==", "image/png"}}});
    encoded = wire::encode(input, compatibility);
    CHECK(encoded["messages"].back()["content"][1]["image_url"]["url"] == "data:image/png;base64,AA==");
    input.messages.push_back({Role::Assistant, {ToolCall{"call", "add", {{"a", 1}}}}});
    input.messages.push_back({Role::Tool, {ToolResult{"call", {{"value", 2}}, false}}});
    encoded = wire::encode(input, compatibility);
    CHECK(encoded["messages"].back()["tool_call_id"] == "call");
    CHECK(Json::parse(encoded["messages"].back()["content"].get<std::string>())["value"] == 2);

    const auto bytes = std::string("\xef\xbb\xbf: heartbeat\r\n\r\n") + text_stream();
    for (std::size_t split = 0; split <= bytes.size(); ++split)
    {
        std::string text;
        wire::Decoder decoder(request(), {}, [&](const Event& event)
        { if (auto value = std::get_if<TextDelta>(&event)) text += value->text; });
        wire::EventStream parser(1024 * 1024);
        auto emit = [&](auto type, auto data) { decoder.event(type, data); };
        parser.feed(std::string_view(bytes).substr(0, split), emit);
        parser.feed(std::string_view(bytes).substr(split), emit);
        parser.finish();
        auto answer = decoder.finish();
        CHECK(text == "hello" && std::get<Text>(answer.message.content.front()).value == text);
        CHECK(answer.usage.input_tokens == 12 && answer.usage.cached_input_tokens == 4);
        CHECK(answer.usage.reasoning_tokens == 1 && answer.reported_model == "served-test");
    }
    {
        std::vector<std::string> events;
        wire::EventStream parser(100);
        const std::string bytes = "event: message\rdata: first\rdata: second\r\r: comment\nid: ignored\n\ndata: last\n\n";
        for (char c : bytes) parser.feed(std::string_view(&c, 1), [&](auto, auto data) { events.emplace_back(data); });
        parser.finish(); CHECK((events == std::vector<std::string>{"first\nsecond", "last"}));
        parser.feed("data: partial", [](auto, auto) {});
        expect(ErrorCode::Protocol, [&] { parser.finish(); });
        wire::EventStream limited(5);
        expect(ErrorCode::Protocol, [&] { limited.feed("data: too large", [](auto, auto) {}); });
    }
    {
        auto tool_delta = [](int index, Json function, std::string id = "")
        {
            Json call{{"index", index}, {"function", std::move(function)}};
            if (!id.empty()) { call["id"] = id; call["type"] = "function"; }
            return chunk({{"tool_calls", Json::array({call})}});
        };
        std::vector<Event> events;
        wire::Decoder decoder(request(), {}, [&](const auto& event) { events.push_back(event); });
        decoder.event("", tool_delta(1, {{"name", "add"}, {"arguments", "{\"a\":"}}, "two").dump());
        decoder.event("", chunk({{"content", "Calculating"}}).dump());
        decoder.event("", tool_delta(0, {{"name", "add"}, {"arguments", "{}"}}, "one").dump());
        decoder.event("", tool_delta(1, {{"arguments", "2}"}}).dump());
        decoder.event("", chunk(Json::object(), "tool_calls").dump());
        decoder.event("", "[DONE]");
        auto answer = decoder.finish();
        CHECK(answer.finish == Finish::ToolCalls && answer.message.content.size() == 3);
        CHECK(std::get<ToolCall>(answer.message.content[0]).arguments["a"] == 2);
        CHECK(std::get<ToolCall>(answer.message.content[2]).id == "one");
        CHECK(std::get<ToolDelta>(events.front()).block == 0);
        CHECK(std::get<ToolDelta>(events[3]).block == 0);
    }
    {
        Compatibility options; options.reasoning_field = "reasoning_content";
        wire::Decoder decoder(request(), options, {});
        decoder.event("", chunk({{"reasoning_content", "thinking"}}).dump());
        decoder.event("", chunk({{"content", "answer"}}, "stop").dump());
        decoder.event("", "[DONE]");
        auto answer = decoder.finish();
        auto replay = request(); replay.messages.push_back(answer.message);
        CHECK(wire::encode(replay, options)["messages"].back()["reasoning_content"] == "thinking");
        expect(ErrorCode::IncompatibleState, [&] { wire::encode(replay, {}); });
        wire::Decoder unsupported(request(), {}, {});
        expect(ErrorCode::Protocol, [&] { unsupported.event("", chunk({{"reasoning_content", "state"}}).dump()); });
    }
    {
        const auto bytes = event(chunk({{"content", "e\xcc\x81界"}}, "stop")) + "data: [DONE]\n\n";
        wire::Decoder decoder(request(), {}, {});
        wire::EventStream parser(1024);
        for (char c : bytes) parser.feed(std::string_view(&c, 1), [&](auto type, auto data) { decoder.event(type, data); });
        parser.finish();
        CHECK(std::get<Text>(decoder.finish().message.content[0]).value == "e\xcc\x81界");
    }
    for (auto arguments : {"{", "[]", "null"})
    {
        auto value = response();
        value["choices"][0]["message"] = {{"tool_calls", Json::array({{{"type", "function"}, {"id", "call"},
            {"function", {{"name", "add"}, {"arguments", arguments}}}}})}};
        value["choices"][0]["finish_reason"] = "tool_calls";
        Compatibility options; options.stream = false;
        wire::Decoder decoder(request(), options, {}); decoder.json(value.dump());
        expect(ErrorCode::Protocol, [&] { decoder.finish(); });
    }
    for (auto malformed : {"{", "null", "{\"choices\":{}}", "{\"choices\":[]}"})
    {
        wire::Decoder decoder(request(), {}, {});
        expect(ErrorCode::Protocol, [&] { decoder.event("", malformed); });
    }
    {
        wire::Decoder decoder(request(), {}, {});
        decoder.event("", chunk({{"content", "partial"}}).dump());
        expect(ErrorCode::Protocol, [&] { decoder.finish(); });
        expect(ErrorCode::Protocol, [&] { decoder.event("", "[DONE]"); });
    }
    {
        wire::Decoder decoder(request(), {}, {});
        decoder.event("", chunk({{"content", "finished"}}, "stop").dump());
        expect(ErrorCode::Protocol, [&] { decoder.finish(); });
        decoder.event("", "[DONE]");
        expect(ErrorCode::Protocol, [&] { decoder.event("", chunk(Json::object()).dump()); });
    }
    {
        Compatibility options; options.stream = false;
        wire::Decoder decoder(request(), options, {});
        auto value = response("界");
        decoder.json(value.dump());
        CHECK(std::get<Text>(decoder.finish().message.content[0]).value == "界");
        value["choices"][0]["message"] = {{"role", "assistant"}, {"content", nullptr}, {"refusal", "No"}};
        wire::Decoder refusal(request(), options, {}); refusal.json(value.dump());
        CHECK(refusal.finish().finish == Finish::Refusal);
        value["usage"]["prompt_tokens"] = -1;
        wire::Decoder invalid(request(), options, {});
        expect(ErrorCode::Protocol, [&] { invalid.json(value.dump()); });
    }
    CHECK(wire::provider_error(401, "not json").code == ErrorCode::Authentication);
    CHECK(wire::provider_error(400, R"({"error":{"code":"context_length_exceeded","message":"too long"}})").code == ErrorCode::ContextLimit);
    CHECK(wire::retry_after("2") == std::chrono::seconds(2));
    CHECK(wire::retry_after("Wed, 21 Oct 2015 07:28:00 GMT") == std::chrono::milliseconds(0));
    CHECK(!wire::retry_after("nonsense") && !wire::retry_after("18446744073709551615"));
    {
        auto context = std::make_shared<IOContext>(mt::ConcurrentPool::make<>(), nullptr);
        context->load<InlineHTTP>(); context->load<sys::SystemOpenAI>();
        auto provider = context->sys<sys::SystemProvider>()->ptr<sys::SystemOpenAI>();
        expect(ErrorCode::Transport, [&] { provider->generate(request()).value(); });
        context->unload<sys::SystemOpenAI>();
        expect(ErrorCode::Closed, [&] { provider->generate(request()); });
    }
}
