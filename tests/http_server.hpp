#pragma once

#include "check.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <functional>
#include <thread>

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
