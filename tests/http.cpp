#include "check.hpp"
#include <core/mods/d2_http.hpp>
#include <os/net/curl/d2_http_curl.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <future>
#include <thread>

using namespace std::chrono_literals;
using HTTP = d2::sys::net::SystemHTTP;
using Code = HTTP::Error::Code;

struct Server
{
    int listener{-1};
    unsigned short port{0};
    std::jthread worker;
    std::exception_ptr error;
    Server(std::function<void(int)> handle, int connections = 1)
    {
        listener = socket(AF_INET, SOCK_STREAM, 0); CHECK(listener >= 0);
        sockaddr_in address{}; address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        CHECK(bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
        socklen_t size = sizeof(address);
        CHECK(getsockname(listener, reinterpret_cast<sockaddr*>(&address), &size) == 0);
        port = ntohs(address.sin_port); CHECK(listen(listener, 16) == 0);
        worker = std::jthread([this, handle, connections](std::stop_token stop)
        {
            try
            {
                for (int i = 0; i < connections && !stop.stop_requested();)
                {
                    pollfd ready{listener, POLLIN, 0};
                    if (poll(&ready, 1, 50) <= 0 || stop.stop_requested()) continue;
                    int client = accept(listener, nullptr, nullptr);
                    if (client < 0) continue;
                    timeval timeout{3,0};
                    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                    try { handle(client); }
                    catch (...) { close(client); throw; }
                    close(client); ++i;
                }
            }
            catch (...) { error = std::current_exception(); }
        });
    }
    ~Server() { worker.request_stop(); if (worker.joinable()) worker.join(); close(listener); }
    void check() { worker.join(); if (error) std::rethrow_exception(error); }
    std::string url() const { return "http://127.0.0.1:" + std::to_string(port) + "/"; }
    static void send(int fd, std::string_view bytes)
    {
        while (!bytes.empty())
        {
            auto count = ::send(fd, bytes.data(), bytes.size(), MSG_NOSIGNAL);
            CHECK(count > 0); bytes.remove_prefix(count);
        }
    }
    static std::string read(int fd)
    {
        std::string request; char data[2048];
        while (request.find("\r\n\r\n") == request.npos)
        {
            auto count = recv(fd, data, sizeof(data), 0); CHECK(count > 0);
            request.append(data, count);
        }
        auto length = request.find("Content-Length:");
        if (length != request.npos)
        {
            auto end = request.find("\r\n\r\n") + 4 + std::stoul(request.substr(length + 15));
            while (request.size() < end)
            {
                auto count = recv(fd, data, sizeof(data), 0); CHECK(count > 0);
                request.append(data, count);
            }
        }
        return request;
    }
};
struct Outcome { int status{0}; std::string body; std::optional<HTTP::Error> error; };
struct Probe
{
    std::promise<Outcome> done;
    std::atomic<int> responses{0}, errors{0};
    std::string chunks;
    std::vector<int> statuses;
    HTTP::Request request(std::string url)
    {
        HTTP::Request request; request.url = std::move(url);
        request.on_headers = [this](auto& response) { statuses.push_back(response.status()); };
        request.on_response = [this](auto& response)
        { ++responses; done.set_value({response.status(), std::string(response.body()), {}}); };
        request.on_error = [this](const auto& error) { ++errors; done.set_value({0, {}, error}); };
        return request;
    }
};
template<class T> T ready(std::future<T>& future)
{ CHECK(future.wait_for(3s) == std::future_status::ready); return future.get(); }
void callback_error(const Outcome& result, std::string expected)
{
    CHECK(result.error && result.error->code == Code::Callback && result.error->cause);
    try { std::rethrow_exception(result.error->cause); }
    catch (const std::runtime_error& error) { CHECK(error.what() == expected); return; }
    throw std::runtime_error("Missing original callback exception");
}
int main()
{
    auto context = std::make_shared<d2::IOContext>(mt::ConcurrentPool::make<>(), nullptr);
    context->load<d2::sys::net::SystemHTTPCurl>();
    auto http = context->sys<HTTP>();
    {
        std::promise<void> headers, release;
        auto headers_seen = headers.get_future(); auto body_release = release.get_future();
        Server server([&](int fd)
        {
            const auto request = Server::read(fd); CHECK(request.starts_with("POST /"));
            CHECK(request.ends_with("{\"model\":\"test\"}"));
            Server::send(fd, "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nContent-Length: 6\r\n\r\n");
            ready(body_release); Server::send(fd, "abcdef");
        });
        Probe probe; auto future = probe.done.get_future(); auto request = probe.request(server.url());
        request.method = HTTP::Method::Post;
        request.on_setup = [](auto& builder) { builder.body("{\"model\":\"test\"}"); };
        request.on_headers = [&](auto& response)
        {
            CHECK(response.status() == 200 && response.get("CONTENT-TYPE") == "text/event-stream");
            CHECK(response.body().empty()); headers.set_value();
        };
        request.on_chunk = [&](auto& chunk) { probe.chunks += chunk.read(); };
        auto handle = http->request(std::move(request)); CHECK(handle);
        ready(headers_seen); CHECK(handle->is_open());
        CHECK(future.wait_for(0ms) == std::future_status::timeout);
        release.set_value(); auto result = ready(future);
        CHECK(result.status == 200 && !result.error && result.body.empty());
        CHECK(probe.chunks == "abcdef" && !handle->is_open()); server.check();
    }
    // Cancellation and timeout must work while the server sends no bytes.
    for (bool upload : {false, true}) for (bool timeout : {false, true})
    {
        std::promise<void> arrived, release;
        auto incoming = arrived.get_future(); auto finish = release.get_future();
        Server server([&](int fd) { Server::read(fd); arrived.set_value(); ready(finish); });
        Probe probe; auto future = probe.done.get_future(); auto request = probe.request(server.url());
        request.method = HTTP::Method::Post;
        request.on_setup = [upload](auto& builder)
        {
            if (upload) { builder.set("Expect", ""); builder.stream(); }
            else builder.body("{}");
        };
        if (timeout) request.timeout = 200ms;
        auto handle = http->request(request); CHECK(handle); ready(incoming);
        if (!timeout) handle->cancel();
        auto result = ready(future); release.set_value(); server.check();
        CHECK(result.error && result.error->code == (timeout ? Code::Timeout : Code::Cancelled));
        CHECK(!handle->is_open() && probe.responses == 0 && probe.errors == 1);
    }
    {
        Server server([](int fd)
        {
            auto request = Server::read(fd);
            const auto start = request.find("\r\n\r\n") + 4;
            while (request.find("0\r\n\r\n", start) == request.npos)
            {
                char data[1024]; auto n = recv(fd, data, sizeof(data), 0); CHECK(n > 0);
                request.append(data, n);
            }
            std::string body;
            for (auto pos = start;;)
            {
                auto end = request.find("\r\n", pos); CHECK(end != request.npos);
                const auto length = std::stoul(request.substr(pos, end - pos), nullptr, 16);
                if (!length) break;
                body += request.substr(end + 2, length); pos = end + 2 + length + 2;
            }
            CHECK(body == "abcdef");
            Server::send(fd, "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok");
        });
        Probe probe; auto future = probe.done.get_future(); auto request = probe.request(server.url());
        request.method = HTTP::Method::Post;
        request.on_setup = [](auto& builder) { builder.set("Expect", ""); builder.stream(); };
        auto handle = http->request(request); handle->write("abc"); handle->write("def"); handle->finish();
        const auto result = ready(future);
        CHECK(!result.error && result.body == "ok" && !handle->is_open());
        CHECK(handle->up_current() >= 6); server.check();
    }
    {
        Probe probe; auto future = probe.done.get_future(); auto request = probe.request("http://127.0.0.1:1/");
        request.deadline = std::chrono::steady_clock::now() - 1ms;
        auto handle = http->request(request); CHECK(ready(future).error->code == Code::Timeout);
        CHECK(!handle->is_open());
    }
    for (int phase = 0; phase < 4; ++phase)
    {
        Probe probe; auto future = probe.done.get_future();
        Server server([](int fd)
        { Server::read(fd); Server::send(fd, "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nabc"); });
        auto request = probe.request(server.url());
        auto fail = [](auto&) { throw std::runtime_error("callback sentinel"); };
        if (phase == 0) request.on_setup = fail;
        if (phase == 1) request.on_headers = fail;
        if (phase == 2) request.on_chunk = fail;
        if (phase == 3) request.on_response = fail;
        auto handle = http->request(request);
        callback_error(ready(future), "callback sentinel");
        CHECK(!handle->is_open() && probe.errors == 1);
        if (phase != 0) server.check();
    }
    for (int kind = 0; kind < 3; ++kind)
    {
        Server server([](int fd)
        { Server::read(fd); Server::send(fd, "HTTP/1.1 200 OK\r\nContent-Length: 6\r\n\r\nabcdef"); });
        Probe probe; auto future = probe.done.get_future(); auto request = probe.request(server.url());
        if (kind == 2) request.max_header_bytes = 8;
        else request.max_response_bytes = 3;
        if (kind == 1) request.on_chunk = [](auto&) {};
        auto handle = http->request(request); auto result = ready(future);
        CHECK(result.error && result.error->code == Code::Limit && !handle->is_open()); server.check();
    }
    {
        Server server([](int fd)
        {
            Server::read(fd);
            Server::send(fd, "HTTP/1.1 100 Continue\r\nX-Old: interim\r\n\r\n"
                             "HTTP/1.1 429 Too Many Requests\r\nRetry-After: 2\r\nContent-Length: 3\r\n\r\nbad");
        });
        Probe probe; auto future = probe.done.get_future(); auto request = probe.request(server.url());
        request.on_headers = [&](auto& response)
        {
            probe.statuses.push_back(response.status());
            if (response.status() == 429) CHECK(!response.get("X-Old") && response.get("Retry-After") == "2");
        };
        auto handle = http->request(request); auto result = ready(future);
        CHECK(!result.error && result.status == 429 && result.body == "bad");
        CHECK((probe.statuses == std::vector<int>{100,429})); server.check();
    }
    {
        std::promise<void> paused, release;
        auto pause = paused.get_future(); auto resume = release.get_future();
        Server server([](int fd)
        { Server::read(fd); Server::send(fd, "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\nx"); });
        Probe first; auto first_result = first.done.get_future(); auto request = first.request(server.url());
        request.on_chunk = [&](auto&) { paused.set_value(); ready(resume); };
        auto first_handle = http->request(request); ready(pause);
        std::vector<std::unique_ptr<Probe>> probes;
        std::vector<std::future<Outcome>> results;
        std::vector<HTTP::Stream::ptr> handles;
        for (int i = 0; i < 32; ++i)
        {
            auto probe = std::make_unique<Probe>(); results.push_back(probe->done.get_future());
            handles.push_back(http->request(probe->request(server.url()))); probes.push_back(std::move(probe));
        }
        // Each queued request must settle even when unloading races with submission.
        std::jthread shutdown([&] { http->unload(); });
        release.set_value(); shutdown.join(); ready(first_result); server.check();
        for (std::size_t i = 0; i < results.size(); ++i)
        {
            auto result = ready(results[i]); CHECK(result.error);
            CHECK(result.error->code == Code::Cancelled || result.error->code == Code::Transport);
            CHECK(probes[i]->errors == 1 && probes[i]->responses == 0 && !handles[i]->is_open());
        }
        Probe offline; auto done = offline.done.get_future();
        // Direct module reference bypasses lazy reload through ModulePtr.
        auto implementation = http.ptr(); implementation->unload();
        auto handle = implementation->request(offline.request(server.url()));
        CHECK(ready(done).error->code == Code::Unavailable && !handle->is_open());
        implementation->load();
    }
    for (bool follow : {false, true})
    {
        Server server([](int fd)
        {
            const auto request = Server::read(fd);
            if (request.starts_with("GET /next "))
                Server::send(fd, "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nok");
            else Server::send(fd, "HTTP/1.1 302 Found\r\nLocation: /next\r\nX-Old: redirect\r\nContent-Length: 0\r\n\r\n");
        }, follow ? 2 : 1);
        Probe probe; auto future = probe.done.get_future(); auto request = probe.request(server.url());
        request.follow_redirects = follow;
        request.on_headers = [&](auto& response)
        {
            probe.statuses.push_back(response.status());
            if (response.status() == 200) CHECK(!response.get("X-Old"));
        };
        auto handle = http->request(request); auto result = ready(future);
        CHECK(!result.error && result.status == (follow ? 200 : 302));
        CHECK(probe.statuses.size() == (follow ? 2 : 1)); server.check();
    }
    {
        Server server([](int fd) { Server::read(fd); });
        Probe probe; auto future = probe.done.get_future();
        auto handle = http->request(probe.request(server.url()));
        const auto result = ready(future);
        CHECK(result.error && result.error->code == Code::Transport && result.error->native_code);
        server.check();
    }
    {
        Probe probe; auto future = probe.done.get_future(); auto request = probe.request("http://127.0.0.1:1/");
        request.timeout = 0ms;
        auto handle = http->request(request);
        CHECK(ready(future).error->code == Code::InvalidRequest && !handle->is_open());
    }
}
