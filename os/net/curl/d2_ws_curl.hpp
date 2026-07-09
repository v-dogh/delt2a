#pragma once

#include <absl/container/flat_hash_map.h>
#include <atomic>
#include <chrono>
#include <curl/curl.h>
#include <functional>
#include <memory>
#include <core/mods/d2_ws.hpp>
#include <mt/task_ring.hpp>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace d2::sys::net
{
    class SystemWSCurl : public SystemWS,
                         public d2::sys::ModuleImpl<SystemWSCurl, d2::sys::Load::Lazy>
    {
    private:
        enum class Phase
        {
            Idle,
            Connecting,
            Open,
            Closing,
            Closed,
            Reconnecting,
            Failed
        };

        struct Outgoing;
        struct Operation;

        struct CurlBuilder;
        struct CurlClient;
    private:
        std::jthread _worker{};
        mt::TaskRing<std::function<void()>, 8> _tasks{};

        std::atomic<CURLM*> _handle{nullptr};
        std::atomic<bool> _stop{true};

        absl::flat_hash_map<CURL*, std::shared_ptr<Operation>> _connecting{};
        absl::flat_hash_map<CURL*, std::shared_ptr<Operation>> _open{};
        absl::flat_hash_map<Operation*, std::shared_ptr<Operation>> _reconnecting{};

        void _wake();
        void _run(std::function<void()> task);

        void _start(std::shared_ptr<Operation> op);
        void _promote_open(std::shared_ptr<Operation> op);
        void _drive_open(std::shared_ptr<Operation> op);
        void _recv_available(std::shared_ptr<Operation> op);
        void _send_ping_if_needed(
            std::shared_ptr<Operation> op, std::chrono::steady_clock::time_point now
        );
        void
        _check_timeout(std::shared_ptr<Operation> op, std::chrono::steady_clock::time_point now);
        void _send_outgoing(std::shared_ptr<Operation> op, Outgoing packet);

        void _request_close(std::shared_ptr<Operation> op, std::uint16_t code, std::string reason);
        void _finish_close(std::shared_ptr<Operation> op, std::uint16_t code, std::string reason);
        void _transport_error(std::shared_ptr<Operation> op, std::string error);
        void _schedule_reconnect(std::shared_ptr<Operation> op);
        void _cleanup_open(std::shared_ptr<Operation> op);

        static std::string _curl_error(Operation& op, CURLcode code);
        static std::string _make_close_payload(std::uint16_t code, std::string_view reason);
        static std::pair<std::uint16_t, std::string> _parse_close_payload(std::string_view payload);
        static std::chrono::milliseconds _reconnect_delay(const Operation& op);

        virtual Status _load_impl() override;
        virtual Status _unload_impl() override;

        virtual Client::ptr _connect_impl(Connection connection) override;
    public:
        using SystemWS::SystemWS;
    };

    using ws_curl = SystemWSCurl;
} // namespace d2::sys::net
