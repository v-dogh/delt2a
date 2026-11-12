#include "check.hpp"
#include "http_server.hpp"
#include "openai_fixtures.hpp"
#include <core/mods/inference/d2_std.hpp>
#include <os/inference/openai/d2_openai.hpp>
#include <os/net/curl/d2_http_curl.hpp>
#include <future>

using namespace d2;
using namespace d2::ai;
using namespace openai_test;
using namespace std::chrono_literals;

Json received(int fd)
{
    auto bytes = Server::read(fd);
    CHECK(bytes.starts_with("POST /v1/chat/completions "));
    return Json::parse(bytes.substr(bytes.find("\r\n\r\n") + 4));
}
void respond(int fd, std::string_view body, bool stream = true, int status = 200)
{
    Server::send(fd, "HTTP/1.1 " + std::to_string(status) + " Result\r\nContent-Type: " +
        (stream ? "text/event-stream" : "application/json") + "\r\nx-request-id: request-1\r\nRetry-After: 2\r\nContent-Length: " +
        std::to_string(body.size()) + "\r\n\r\n");
    Server::send(fd, body);
}
template<class T> void ready(mt::future<T>& future)
{
    const auto end = std::chrono::steady_clock::now() + 3s;
    while (!future.await_ready() && std::chrono::steady_clock::now() < end) std::this_thread::yield();
    CHECK(future.await_ready());
}

int main()
{
    auto context = std::make_shared<IOContext>(mt::ConcurrentPool::make<>(), nullptr);
    context->load<sys::net::SystemHTTPCurl>();
    context->load<sys::SystemOpenAI>();
    auto provider = context->sys<sys::SystemProvider>()->ptr<sys::SystemOpenAI>();
    auto configure = [&](const Server& server, bool stream = true)
    {
        openai::Config config;
        config.endpoint = server.url() + "v1/chat/completions";
        config.compatibility.stream = stream;
        config.timeout = 2s;
        provider->configure(config);
    };
    {
        int step = 0;
        Server server([&](int fd)
        {
            auto body = received(fd);
            if (step == 0)
            {
                CHECK(body["model"] == "test" && body["tools"][0]["function"]["name"] == "add");
                auto delta = chunk({{"tool_calls", Json::array({{{"index", 0}, {"id", "call-1"}, {"type", "function"},
                    {"function", {{"name", "add"}, {"arguments", "{\"a\":2,\"b\":3}"}}}}})}}, "tool_calls");
                respond(fd, event(delta) + "data: [DONE]\n\n");
            }
            else if (step == 1)
            {
                CHECK(body["messages"].back()["role"] == "tool");
                CHECK(body["messages"].back()["tool_call_id"] == "call-1");
                CHECK(body["messages"].back()["content"] == "5");
                respond(fd, text_stream());
            }
            else
            {
                CHECK(body["model"] == "second" && body["stream"] == false);
                CHECK(body["messages"].size() == 5 && body["max_tokens"] == 200);
                CHECK(body["messages"][1]["tool_calls"][0]["id"] == "call-1");
                respond(fd, response("switched").dump(), false);
            }
            ++step;
        }, 3);
        configure(server);
        auto config = provider->config();
        auto alternate = config.compatibility;
        alternate.stream = false; alternate.token_limit_field = "max_tokens";
        config.models["second"].compatibility = alternate;
        provider->configure(config);
        context->load<sys::SystemConversations>();
        auto conversations = context->sys<sys::SystemConversations>();
        conversations->bind("local", provider.ptr());
        Tool add;
        add.definition = {"add", "Add two integers", {{"type", "object"}}};
        add.invoke = [](Json args, std::stop_token) -> mt::future<Json>
        { co_return args.at("a").get<int>() + args.at("b").get<int>(); };
        auto tools = std::make_shared<ToolSet>(std::vector<Tool>{std::move(add)});
        ConversationConfig conversation; conversation.model = {"local", "test"}; conversation.tools = tools;
        auto chat = conversations->create("test", conversation);
        chat->append(Message::text(Role::User, "add 2 and 3"));
        auto first = chat->step(); ready(first.result);
        CHECK(first.result.value().finish == Finish::ToolCalls);
        for (const auto& [id, call] : chat->pending_tools()) chat->add_tool_result(tools->execute(call).value());
        std::string text;
        auto second = chat->step({}, [&](const Event& event)
        { if (auto value = std::get_if<TextDelta>(&event)) text += value->text; });
        ready(second.result); auto answer = second.result.value();
        CHECK(text == "hello" && answer.usage.input_tokens == 12 && answer.request_id == "request-1");
        CHECK(answer.timing.first_output && answer.timing.total);
        Settings settings; settings.sampling.max_output_tokens = 200;
        chat->switch_model({"local", "second"}, settings);
        chat->append(Message::text(Role::User, "continue"));
        auto third = chat->step(); ready(third.result);
        CHECK(std::get<Text>(third.result.value().message.content[0]).value == "switched");
        CHECK(chat->snapshot().records.size() == 3);
        CHECK(chat->snapshot().transcript[1].origin->name == "test");
        server.check();
    }
    {
        Server server([&](int fd)
        {
            auto bytes = Server::read(fd);
            CHECK(bytes.find("Authorization: Bearer test-key") != bytes.npos);
            CHECK(bytes.find("X-Endpoint: test") != bytes.npos);
            respond(fd, R"({"error":{"message":"slow down","code":"rate_limit_exceeded"}})", false, 429);
        });
        configure(server);
        auto config = provider->config(); config.api_key = "test-key"; config.headers["X-Endpoint"] = "test";
        provider->configure(config);
        auto future = provider->generate(request()); ready(future);
        try { future.value(); CHECK(false); }
        catch (const Error& error)
        {
            CHECK(error.code == ErrorCode::RateLimit && error.http_status == 429);
            CHECK(error.retry_after == 2s && error.request_id == "request-1");
            CHECK(error.native["error"]["code"] == "rate_limit_exceeded");
        }
        server.check();
    }
    for (bool timeout : {false, true})
    {
        std::promise<void> seen, release;
        auto seen_future = seen.get_future(), released = release.get_future();
        Server server([&](int fd) { received(fd); seen.set_value(); released.wait(); });
        configure(server);
        auto input = request();
        if (timeout) input.deadline = std::chrono::steady_clock::now() + 100ms;
        std::stop_source stop;
        auto future = provider->generate(input, {}, stop.get_token());
        CHECK(seen_future.wait_for(3s) == std::future_status::ready);
        if (!timeout) stop.request_stop();
        ready(future);
        expect(timeout ? ErrorCode::Timeout : ErrorCode::Cancelled, [&] { future.value(); });
        release.set_value(); server.check();
    }
    {
        Server server([&](int fd) { received(fd); respond(fd, text_stream()); });
        configure(server);
        auto future = provider->generate(request(), [](const Event&) { throw std::runtime_error("handler failure"); });
        ready(future);
        try { future.value(); CHECK(false); }
        catch (const std::runtime_error& error) { CHECK(std::string(error.what()) == "handler failure"); }
        server.check();
    }
    for (const auto& bytes : {std::string("data: broken\n\n"), event(chunk({{"content", "partial"}})),
                              event(chunk({{"content", "done"}}, "stop"))})
    {
        Server server([&](int fd) { received(fd); respond(fd, bytes); });
        configure(server);
        auto future = provider->generate(request()); ready(future);
        expect(ErrorCode::Protocol, [&] { future.value(); }); server.check();
    }
    {
        Server server([&](int fd) { received(fd); respond(fd, text_stream()); });
        configure(server);
        const auto original = std::make_exception_ptr(Error(ErrorCode::Provider, "shared handler error"));
        auto future = provider->generate(request(), [&](const Event&) { std::rethrow_exception(original); });
        ready(future);
        try { future.value(); CHECK(false); }
        catch (const Error& error) { CHECK(error.request_id == "request-1"); }
        try { std::rethrow_exception(original); }
        catch (const Error& error) { CHECK(error.request_id.empty()); }
        server.check();
    }
    {
        Server server([&](int fd) { received(fd); respond(fd, text_stream()); });
        configure(server);
        auto config = provider->config(); config.max_event_bytes = 16; provider->configure(config);
        auto future = provider->generate(request()); ready(future);
        expect(ErrorCode::Protocol, [&] { future.value(); }); server.check();
    }
    {
        std::promise<void> seen, release;
        auto seen_future = seen.get_future(), released = release.get_future();
        Server server([&](int fd) { received(fd); seen.set_value(); released.wait(); });
        configure(server);
        auto future = provider->generate(request());
        CHECK(seen_future.wait_for(3s) == std::future_status::ready);
        provider->unload(); ready(future);
        expect(ErrorCode::Cancelled, [&] { future.value(); });
        expect(ErrorCode::Closed, [&] { provider->generate(request()); });
        release.set_value(); server.check(); provider->load();
    }
    {
        Server server([&](int fd) { received(fd); respond(fd, text_stream()); });
        configure(server);
        auto future = provider->generate(request(), [&](const Event&) { provider->unload(); });
        ready(future); expect(ErrorCode::Cancelled, [&] { future.value(); });
        server.check(); provider->load();
    }
    {
        Server server([&](int fd) { received(fd); respond(fd, text_stream()); }, 8);
        configure(server);
        std::vector<mt::future<Response>> requests;
        for (int i = 0; i < 8; ++i) requests.push_back(provider->generate(request()));
        for (auto& future : requests)
        { ready(future); CHECK(std::get<Text>(future.value().message.content[0]).value == "hello"); }
        server.check();
    }
    {
        Server server([&](int fd)
        {
            const auto bytes = Server::read(fd);
            CHECK(bytes.find("Authorization:") == bytes.npos);
            auto body = Json::parse(bytes.substr(bytes.find("\r\n\r\n") + 4));
            CHECK(body["model"] == "test" && body["custom"] == 42);
            respond(fd, text_stream());
        });
        context->load<sys::SystemOpenAI>("second-endpoint");
        auto other = context->sys<sys::SystemProvider>("second-endpoint")->ptr<sys::SystemOpenAI>();
        openai::Config config; config.endpoint = server.url() + "v1/chat/completions";
        other->configure(config);
        auto input = request(); input.model.provider = "remote";
        input.settings.extensions["local"] = {{"model", "must-not-leak"}};
        input.settings.extensions["remote"] = {{"custom", 42}};
        auto future = other->generate(input); ready(future);
        CHECK(future.value().message.origin->provider == "remote");
        server.check(); context->unload<sys::SystemOpenAI>("second-endpoint");
    }
    {
        std::promise<void> seen, release;
        auto seen_future = seen.get_future(), released = release.get_future();
        Server server([&](int fd) { received(fd); seen.set_value(); released.wait(); respond(fd, text_stream()); });
        configure(server);
        auto future = provider->generate(request());
        CHECK(seen_future.wait_for(3s) == std::future_status::ready);
        auto config = provider->config();
        config.compatibility.stream = false;
        provider->configure(config);
        release.set_value(); ready(future);
        CHECK(std::get<Text>(future.value().message.content[0]).value == "hello");
        server.check();
    }
    context->unload<sys::SystemConversations>();
    context->unload<sys::SystemOpenAI>();
    context->unload<sys::net::SystemHTTP>();
}
