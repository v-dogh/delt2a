#pragma once

#include <d2_io_handler.hpp>
#include <d2_module.hpp>

namespace d2::sys::net
{
    class SystemWS : public d2::sys::AbstractModule<SystemWS, "WebSockets", d2::sys::Access::TSafe>
    {
    public:
        enum class State
        {
            Created,
            Connecting,
            Open,
            Closing,
            Closed,
            Failed,
            Reconnecting
        };
        enum class Type
        {
            Text,
            Binary
        };

        struct Client
        {
            using ptr = std::shared_ptr<Client>;

            virtual void send(std::string_view msg, Type type = Type::Text) = 0;
            virtual void close(std::uint16_t code = 1000, std::string_view reason = "") = 0;
            virtual State state() = 0;

            virtual ~Client() = default;
        };
        struct RequestBuilder
        {
            virtual void set(std::string_view header, std::string_view value) = 0;
        };
        struct Connection
        {
            std::string url{};

            std::chrono::seconds ping_interval{30};
            std::chrono::seconds timeout{std::chrono::seconds::max()};

            bool reconnect{false};
            std::chrono::milliseconds reconnect_min_delay{500};
            std::chrono::milliseconds reconnect_max_delay{30000};
            std::size_t reconnect_max_attempts{0};

            std::function<void(RequestBuilder&)> on_setup{};
            std::function<void(Client::ptr)> on_open{};
            std::function<void(std::string_view, Type, Client::ptr)> on_message{};
            std::function<void(std::string)> on_error{};
            std::function<void(std::uint16_t, std::string_view)> on_close{};
        };
    protected:
        virtual Client::ptr _connect_impl(Connection connection) = 0;
    public:
        Client::ptr connect(Connection connection);

        using AbstractModule::AbstractModule;
    };
    using ws = SystemWS;
} // namespace d2::sys::net
