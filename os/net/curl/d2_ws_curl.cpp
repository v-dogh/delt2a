#include "d2_ws_curl.hpp"

#include "d2_curl_global.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

namespace d2::sys::net
{
    struct SystemWSCurl::Outgoing
    {
        std::string payload{};
        Type type{Type::Text};
        std::size_t offset{0};
    };

    struct SystemWSCurl::Operation
    {
        SystemWSCurl* owner{nullptr};

        CURL* handle{nullptr};
        curl_slist* request_headers{nullptr};

        d2::IOContext::ptr ctx{nullptr};
        std::weak_ptr<Client> client{};

        std::string url{};
        std::string builder_error{};

        std::chrono::seconds ping_interval{30};
        std::chrono::seconds timeout{std::chrono::seconds::max()};

        bool reconnect{false};
        std::chrono::milliseconds reconnect_min_delay{500};
        std::chrono::milliseconds reconnect_max_delay{30000};
        std::size_t reconnect_max_attempts{0};
        std::size_t reconnect_attempts{0};
        std::chrono::steady_clock::time_point reconnect_at{};

        std::atomic<State> public_state{State::Created};
        Phase phase{Phase::Idle};

        std::atomic_bool close_requested{false};
        bool close_frame_sent{false};
        bool user_close_requested{false};
        std::uint16_t close_code{1000};
        std::string close_reason{};

        std::deque<Outgoing> outgoing{};
        std::optional<Outgoing> pending_send{};

        std::string message_buffer{};
        Type message_type{Type::Text};
        bool message_type_known{false};

        std::chrono::steady_clock::time_point last_rx{};
        std::chrono::steady_clock::time_point last_tx{};
        std::chrono::steady_clock::time_point last_ping{};

        std::array<char, CURL_ERROR_SIZE> error_buffer{};
        CURLcode curl_code{CURLE_OK};
        std::string error_string{};

        std::function<void(Client::ptr)> on_open{};
        std::function<void(RequestBuilder&)> on_setup{};
        std::function<void(std::string_view, Type, Client::ptr)> on_message{};
        std::function<void(std::string, d2::IOContext::ptr)> on_error{};
        std::function<void(std::uint16_t, std::string_view, d2::IOContext::ptr)> on_close{};

        bool setup();
        void reset_handle();
        ~Operation();
    };

    struct SystemWSCurl::CurlBuilder : RequestBuilder
    {
        Operation& op;

        explicit CurlBuilder(Operation& op) : op(op) {}

        virtual void set(std::string_view header, std::string_view value) override;
        virtual ~CurlBuilder() = default;
    };
    struct SystemWSCurl::CurlClient : Client
    {
        d2::sys::module<SystemWSCurl> owner{};
        std::weak_ptr<Operation> op{};

        CurlClient(
            d2::IOContext::ptr ctx,
            std::shared_ptr<Operation> op,
            d2::sys::module<SystemWSCurl> owner
        );

        virtual void send(std::string_view msg, Type type = Type::Text) override;
        virtual void close(std::uint16_t code = 1000, std::string_view reason = "") override;
        virtual State state() override;

        virtual ~CurlClient() override;
    };

    namespace
    {
        constexpr std::size_t RECV_BUFFER_SIZE = 64 * 1024;
        bool is_text_frame(const curl_ws_frame* meta)
        {
            return meta && ((meta->flags & CURLWS_TEXT) != 0);
        }
        bool is_binary_frame(const curl_ws_frame* meta)
        {
            return meta && ((meta->flags & CURLWS_BINARY) != 0);
        }
        bool is_cont_frame(const curl_ws_frame* meta)
        {
            return meta && ((meta->flags & CURLWS_CONT) != 0);
        }
        bool is_close_frame(const curl_ws_frame* meta)
        {
            return meta && ((meta->flags & CURLWS_CLOSE) != 0);
        }
        bool is_ping_frame(const curl_ws_frame* meta)
        {
            return meta && ((meta->flags & CURLWS_PING) != 0);
        }
        bool is_pong_frame(const curl_ws_frame* meta)
        {
            return meta && ((meta->flags & CURLWS_PONG) != 0);
        }
    } // namespace

    void SystemWSCurl::CurlBuilder::set(std::string_view header, std::string_view value)
    {
        std::string line;
        line.reserve(header.size() + value.size() + 2);
        line.append(header);
        line.append(": ");
        line.append(value);

        auto* next = curl_slist_append(op.request_headers, line.c_str());
        if (!next)
        {
            op.builder_error = "Failed to append curl websocket request header";
            return;
        }

        op.request_headers = next;
    }

    SystemWSCurl::CurlClient::CurlClient(
        d2::IOContext::ptr ctx, std::shared_ptr<Operation> op, d2::sys::module<SystemWSCurl> owner
    ) : Client(std::move(ctx)), owner(std::move(owner)), op(std::move(op))
    {
    }

    SystemWSCurl::CurlClient::~CurlClient()
    {
        close(1000, "client released");
    }

    void SystemWSCurl::CurlClient::send(std::string_view msg, Type type)
    {
        if (msg.empty())
            return;

        auto locked = op.lock();
        if (!locked || !owner)
            return;

        std::string payload(msg);
        owner->_run(
            [op = std::move(locked),
             payload = std::move(payload),
             type,
             owner = this->owner]() mutable
            {
                const auto state = op->public_state.load();
                if (state == State::Closing || state == State::Closed || state == State::Failed)
                    return;
                owner->_send_outgoing(op, Outgoing{std::move(payload), type, 0});
            }
        );
    }
    void SystemWSCurl::CurlClient::close(std::uint16_t code, std::string_view reason)
    {
        auto locked = op.lock();
        if (!locked || !owner)
            return;

        std::string reason_copy(reason);
        owner->_run(
            [owner = owner, op = std::move(locked), code, reason = std::move(reason_copy)]() mutable
            { owner->_request_close(op, code, std::move(reason)); }
        );
    }
    SystemWSCurl::State SystemWSCurl::CurlClient::state()
    {
        auto locked = op.lock();
        if (!locked)
            return State::Closed;
        return locked->public_state.load();
    }

    bool SystemWSCurl::Operation::setup()
    {
        reset_handle();

        if (!builder_error.empty())
        {
            curl_code = CURLE_FAILED_INIT;
            error_string = builder_error;
            return false;
        }

        auto* easy = curl_easy_init();
        if (!easy)
        {
            curl_code = CURLE_FAILED_INIT;
            error_string = "Failed to initialize curl easy handle";
            return false;
        }

        handle = easy;
        error_buffer[0] = '\0';

        curl_easy_setopt(easy, CURLOPT_ERRORBUFFER, error_buffer.data());
        curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
        curl_easy_setopt(easy, CURLOPT_CONNECT_ONLY, 2L);

        if (request_headers)
            curl_easy_setopt(easy, CURLOPT_HTTPHEADER, request_headers);

        if (timeout != std::chrono::seconds::max())
        {
            const auto timeout_ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
            curl_easy_setopt(easy, CURLOPT_TIMEOUT_MS, static_cast<long>(timeout_ms));
        }

        curl_easy_setopt(easy, CURLOPT_TCP_KEEPALIVE, 1L);

        curl_code = CURLE_OK;
        error_string.clear();
        return true;
    }
    void SystemWSCurl::Operation::reset_handle()
    {
        if (handle)
        {
            curl_easy_cleanup(handle);
            handle = nullptr;
        }

        pending_send.reset();
        message_buffer.clear();
        message_type = Type::Text;
        message_type_known = false;
        close_frame_sent = false;
        close_requested = false;
        close_code = 1000;
        close_reason.clear();
        curl_code = CURLE_OK;
        error_string.clear();
        error_buffer[0] = '\0';
    }
    SystemWSCurl::Operation::~Operation()
    {
        if (request_headers)
            curl_slist_free_all(request_headers);
        if (handle)
            curl_easy_cleanup(handle);
    }

    std::string SystemWSCurl::_curl_error(Operation& op, CURLcode code)
    {
        if (op.error_buffer[0] != '\0')
            return std::string(op.error_buffer.data());
        return curl_easy_strerror(code);
    }

    std::string SystemWSCurl::_make_close_payload(std::uint16_t code, std::string_view reason)
    {
        std::string payload;
        payload.reserve(2 + reason.size());
        payload.push_back(static_cast<char>((code >> 8) & 0xff));
        payload.push_back(static_cast<char>(code & 0xff));
        payload.append(reason);
        return payload;
    }
    std::pair<std::uint16_t, std::string>
    SystemWSCurl::_parse_close_payload(std::string_view payload)
    {
        if (payload.size() < 2)
            return {1005, {}};
        const auto hi = static_cast<unsigned char>(payload[0]);
        const auto lo = static_cast<unsigned char>(payload[1]);
        const auto code = static_cast<std::uint16_t>((hi << 8) | lo);
        return {code, std::string(payload.substr(2))};
    }
    std::chrono::milliseconds SystemWSCurl::_reconnect_delay(const Operation& op)
    {
        auto delay = op.reconnect_min_delay;
        const auto shifts = std::min<std::size_t>(op.reconnect_attempts, 16);

        for (std::size_t i = 0; i < shifts; ++i)
        {
            if (delay >= op.reconnect_max_delay / 2)
                return op.reconnect_max_delay;
            delay *= 2;
        }

        return std::min(delay, op.reconnect_max_delay);
    }

    void SystemWSCurl::_wake()
    {
        if (auto* handle = _handle.load())
            curl_multi_wakeup(handle);
    }
    void SystemWSCurl::_run(std::function<void()> task)
    {
        _tasks.enqueue(std::move(task));
        _wake();
    }
    void SystemWSCurl::_start(std::shared_ptr<Operation> op)
    {
        op->public_state = State::Connecting;
        op->phase = Phase::Connecting;
        op->last_rx = std::chrono::steady_clock::now();
        op->last_tx = op->last_rx;
        op->last_ping = op->last_rx;

        if (!op->setup())
        {
            _transport_error(
                op, op->error_string.empty() ? "Failed to setup websocket" : op->error_string
            );
            return;
        }

        auto* multi = _handle.load();
        if (!multi)
        {
            op->curl_code = CURLE_FAILED_INIT;
            _transport_error(op, "WebSocket module is not loaded");
            return;
        }

        const auto code = curl_multi_add_handle(multi, op->handle);
        if (code != CURLM_OK)
        {
            op->curl_code = CURLE_FAILED_INIT;
            _transport_error(op, curl_multi_strerror(code));
            return;
        }

        _connecting.emplace(op->handle, op);
    }

    void SystemWSCurl::_promote_open(std::shared_ptr<Operation> op)
    {
        op->phase = Phase::Open;
        op->public_state = State::Open;
        op->reconnect_attempts = 0;
        op->last_rx = std::chrono::steady_clock::now();
        op->last_tx = op->last_rx;
        op->last_ping = op->last_rx;

        _open.emplace(op->handle, op);
        if (op->on_open)
        {
            if (auto client = op->client.lock())
                op->on_open(client);
        }

        while (!op->outgoing.empty() && op->phase == Phase::Open && !op->pending_send)
        {
            auto packet = std::move(op->outgoing.front());
            op->outgoing.pop_front();
            _send_outgoing(op, std::move(packet));
        }
    }
    void SystemWSCurl::_drive_open(std::shared_ptr<Operation> op)
    {
        if (!op || !op->handle)
            return;

        if (op->close_requested && !op->close_frame_sent)
        {
            const auto payload = _make_close_payload(op->close_code, op->close_reason);
            std::size_t sent = 0;
            const auto code =
                curl_ws_send(op->handle, payload.data(), payload.size(), &sent, 0, CURLWS_CLOSE);

            if (code == CURLE_AGAIN)
                return;

            if (code != CURLE_OK)
            {
                _transport_error(op, _curl_error(*op, code));
                return;
            }

            op->phase = Phase::Closing;
            op->public_state = State::Closing;
            op->close_frame_sent = true;
            op->last_tx = std::chrono::steady_clock::now();
        }

        if (op->public_state.load() == State::Failed || op->public_state.load() == State::Closed)
            return;
        _recv_available(op);
        if (op->public_state.load() == State::Failed || op->public_state.load() == State::Closed)
            return;

        const auto now = std::chrono::steady_clock::now();
        _send_ping_if_needed(op, now);
        _check_timeout(op, now);
    }
    void SystemWSCurl::_send_outgoing(std::shared_ptr<Operation> op, Outgoing packet)
    {
        if (op->phase != Phase::Open)
        {
            op->outgoing.push_back(std::move(packet));
            return;
        }
        if (!op->pending_send)
            op->pending_send = std::move(packet);

        auto& msg = *op->pending_send;
        const auto flags = msg.type == Type::Text ? CURLWS_TEXT : CURLWS_BINARY;
        while (msg.offset < msg.payload.size())
        {
            std::size_t sent = 0;
            const auto code = curl_ws_send(
                op->handle,
                msg.payload.data() + msg.offset,
                msg.payload.size() - msg.offset,
                &sent,
                0,
                flags
            );

            if (code == CURLE_AGAIN)
                return;

            if (code != CURLE_OK)
            {
                _transport_error(op, _curl_error(*op, code));
                return;
            }

            msg.offset += sent;
            op->last_tx = std::chrono::steady_clock::now();

            if (sent == 0)
                return;
        }

        op->pending_send.reset();
        if (!op->outgoing.empty())
        {
            auto next = std::move(op->outgoing.front());
            op->outgoing.pop_front();
            _send_outgoing(op, std::move(next));
        }
    }
    void SystemWSCurl::_recv_available(std::shared_ptr<Operation> op)
    {
        std::array<char, RECV_BUFFER_SIZE> buffer{};
        while (true)
        {
            std::size_t received = 0;
            const curl_ws_frame* meta = nullptr;
            const auto code =
                curl_ws_recv(op->handle, buffer.data(), buffer.size(), &received, &meta);

            if (code == CURLE_AGAIN)
                return;

            if (code != CURLE_OK)
            {
                if (op->phase == Phase::Closing || op->close_requested)
                    _finish_close(op, op->close_code, op->close_reason);
                else
                    _transport_error(op, _curl_error(*op, code));
                return;
            }

            op->last_rx = std::chrono::steady_clock::now();
            if (is_ping_frame(meta) || is_pong_frame(meta))
                continue;
            if (is_close_frame(meta))
            {
                std::string close_payload;
                close_payload.append(buffer.data(), received);

                while (meta && meta->bytesleft > 0)
                {
                    std::size_t more = 0;
                    const curl_ws_frame* next_meta = nullptr;
                    const auto more_code =
                        curl_ws_recv(op->handle, buffer.data(), buffer.size(), &more, &next_meta);

                    if (more_code == CURLE_AGAIN)
                        break;

                    if (more_code != CURLE_OK)
                        break;

                    close_payload.append(buffer.data(), more);
                    meta = next_meta;
                }

                auto [code_value, reason] = _parse_close_payload(close_payload);
                if (!op->close_frame_sent)
                    _request_close(op, code_value == 1005 ? 1000 : code_value, reason);
                _finish_close(op, code_value, std::move(reason));
                return;
            }

            if (received == 0)
                continue;
            if (is_text_frame(meta))
            {
                op->message_type = Type::Text;
                op->message_type_known = true;
            }
            else if (is_binary_frame(meta))
            {
                op->message_type = Type::Binary;
                op->message_type_known = true;
            }
            else if (!is_cont_frame(meta) && !op->message_type_known)
            {
                op->message_type = Type::Binary;
                op->message_type_known = true;
            }

            op->message_buffer.append(buffer.data(), received);
            if (meta && meta->bytesleft == 0)
            {
                if (op->on_message)
                {
                    if (auto client = op->client.lock())
                    {
                        std::string message = std::move(op->message_buffer);
                        op->message_buffer.clear();
                        op->on_message(std::string_view(message), op->message_type, client);
                    }
                    else
                    {
                        op->message_buffer.clear();
                    }
                }
                else
                {
                    op->message_buffer.clear();
                }
                op->message_type = Type::Text;
                op->message_type_known = false;
            }
        }
    }
    void SystemWSCurl::_send_ping_if_needed(
        std::shared_ptr<Operation> op, std::chrono::steady_clock::time_point now
    )
    {
        if (op->phase != Phase::Open)
            return;
        if (op->ping_interval <= std::chrono::seconds::zero())
            return;
        if (now - op->last_ping < op->ping_interval)
            return;

        std::size_t sent = 0;
        const auto code = curl_ws_send(op->handle, "", 0, &sent, 0, CURLWS_PING);
        if (code == CURLE_AGAIN)
            return;

        if (code != CURLE_OK)
        {
            _transport_error(op, _curl_error(*op, code));
            return;
        }

        op->last_ping = now;
        op->last_tx = now;
    }

    void SystemWSCurl::_check_timeout(
        std::shared_ptr<Operation> op, std::chrono::steady_clock::time_point now
    )
    {
        if (op->timeout == std::chrono::seconds::max())
            return;
        if (now - op->last_rx <= op->timeout)
            return;
        _transport_error(op, "WebSocket timed out");
    }

    void SystemWSCurl::_request_close(
        std::shared_ptr<Operation> op, std::uint16_t code, std::string reason
    )
    {
        if (!op)
            return;

        const auto state = op->public_state.load();
        if (state == State::Closed || state == State::Failed || state == State::Closing)
            return;

        op->user_close_requested = true;
        op->close_requested = true;
        op->close_code = code;
        op->close_reason = std::move(reason);

        if (state == State::Created || state == State::Connecting || state == State::Reconnecting)
        {
            _finish_close(op, code, op->close_reason);
            return;
        }
        op->phase = Phase::Closing;
        op->public_state = State::Closing;
    }
    void SystemWSCurl::_finish_close(
        std::shared_ptr<Operation> op, std::uint16_t code, std::string reason
    )
    {
        if (!op)
            return;

        _cleanup_open(op);
        op->phase = Phase::Closed;
        op->public_state = State::Closed;
        if (op->on_close)
            op->on_close(code, std::string_view(reason), op->ctx);
    }
    void SystemWSCurl::_transport_error(std::shared_ptr<Operation> op, std::string error)
    {
        if (!op)
            return;
        if (error.empty())
            error = "Unknown WebSocket transport error";

        _cleanup_open(op);
        if (op->reconnect && !op->user_close_requested)
        {
            _schedule_reconnect(op);
            return;
        }

        op->phase = Phase::Failed;
        op->public_state = State::Failed;
        if (op->on_error)
            op->on_error(error, op->ctx);
        else
            D2_TLOG(Error, "WebSocket failed: ", error);
        if (op->on_close)
            op->on_close(1006, std::string_view(error), op->ctx);
    }
    void SystemWSCurl::_schedule_reconnect(std::shared_ptr<Operation> op)
    {
        if (!op)
            return;

        if (op->reconnect_max_attempts != 0 && op->reconnect_attempts >= op->reconnect_max_attempts)
        {
            op->phase = Phase::Failed;
            op->public_state = State::Failed;

            const std::string error = "WebSocket reconnect attempts exhausted";
            if (op->on_error)
                op->on_error(error, op->ctx);
            else
                D2_TLOG(Error, error);

            if (op->on_close)
                op->on_close(1006, error, op->ctx);
            return;
        }

        op->phase = Phase::Reconnecting;
        op->public_state = State::Reconnecting;
        op->reconnect_at = std::chrono::steady_clock::now() + _reconnect_delay(*op);
        ++op->reconnect_attempts;
        _reconnecting.emplace(op.get(), op);
    }
    void SystemWSCurl::_cleanup_open(std::shared_ptr<Operation> op)
    {
        if (!op)
            return;
        if (op->handle)
        {
            if (auto* multi = _handle.load())
                curl_multi_remove_handle(multi, op->handle);
            _connecting.erase(op->handle);
            _open.erase(op->handle);
        }
        _reconnecting.erase(op.get());
        op->reset_handle();
    }

    SystemWSCurl::Status SystemWSCurl::_load_impl()
    {
        const auto code = CurlGlobal::acquire();
        if (code != CURLE_OK)
        {
            D2_TLOG(Critical, "Unexpected curl error: ", curl_easy_strerror(code));
            return Status::Failure;
        }

        _worker = context()->launch_thread(
            [this]()
            {
                auto* multi = curl_multi_init();
                if (!multi)
                {
                    D2_TLOG(Critical, "Failed to initialize curl multi handle");
                    _handle = nullptr;
                    _stop = false;
                    _stop.notify_all();
                    return;
                }

                _handle = multi;
                _stop = false;
                _stop.notify_all();

                int running = 0;
                while (!_stop)
                {
                    std::function<void()> task;
                    while (_tasks.try_dequeue(task))
                        task();

                    const auto perform_code = curl_multi_perform(multi, &running);
                    if (perform_code != CURLM_OK)
                    {
                        D2_TLOG(
                            Critical,
                            "Unexpected curl multi perform error: ",
                            curl_multi_strerror(perform_code)
                        );
                    }

                    int queued = 0;
                    while (auto* msg = curl_multi_info_read(multi, &queued))
                    {
                        if (msg->msg != CURLMSG_DONE)
                            continue;
                        auto* easy = msg->easy_handle;
                        auto it = _connecting.find(easy);
                        if (it == _connecting.end())
                            continue;

                        auto op = it->second;
                        _connecting.erase(it);

                        op->curl_code = msg->data.result;
                        if (op->curl_code != CURLE_OK)
                        {
                            curl_multi_remove_handle(multi, easy);
                            _transport_error(op, _curl_error(*op, op->curl_code));
                            continue;
                        }
                        _promote_open(std::move(op));
                    }
                    {
                        auto copy = std::vector<std::shared_ptr<Operation>>{};
                        copy.reserve(_open.size());
                        for (auto& [_, op] : _open)
                            copy.push_back(op);
                        for (auto& op : copy)
                            _drive_open(op);
                    }
                    {
                        const auto now = std::chrono::steady_clock::now();
                        auto ready = std::vector<std::shared_ptr<Operation>>{};
                        for (auto& [_, op] : _reconnecting)
                        {
                            if (now >= op->reconnect_at)
                                ready.push_back(op);
                        }
                        for (decltype(auto) op : ready)
                        {
                            _reconnecting.erase(op.get());
                            if (!op->user_close_requested)
                                _start(op);
                        }
                    }

                    int cnt = 0;
                    const auto poll_code = curl_multi_poll(multi, nullptr, 0, 50, &cnt);
                    if (poll_code != CURLM_OK)
                    {
                        D2_TLOG(
                            Critical,
                            "Unexpected curl multi poll error: ",
                            curl_multi_strerror(poll_code)
                        );
                    }
                }

                for (auto& [_, op] : _connecting)
                {
                    op->user_close_requested = true;
                    op->phase = Phase::Closed;
                    op->public_state = State::Closed;
                    if (op->handle)
                        curl_multi_remove_handle(multi, op->handle);
                    if (op->on_close)
                        op->on_close(1001, "WebSocket module unloaded", op->ctx);
                    op->reset_handle();
                }
                for (auto& [_, op] : _open)
                {
                    op->user_close_requested = true;
                    op->phase = Phase::Closed;
                    op->public_state = State::Closed;

                    if (op->handle)
                        curl_multi_remove_handle(multi, op->handle);
                    if (op->on_close)
                        op->on_close(1001, "WebSocket module unloaded", op->ctx);
                    op->reset_handle();
                }

                _connecting.clear();
                _open.clear();
                _reconnecting.clear();

                curl_multi_cleanup(multi);
                _handle = nullptr;
            }
        );

        while (_stop)
            _stop.wait(true);
        if (!_handle.load())
        {
            if (_worker.joinable())
                _worker.join();
            CurlGlobal::release();
            return Status::Failure;
        }

        return Status::Ok;
    }
    SystemWSCurl::Status SystemWSCurl::_unload_impl()
    {
        _stop = true;
        _wake();
        if (_worker.joinable())
            _worker.join();
        CurlGlobal::release();
        return Status::Ok;
    }

    SystemWSCurl::Client::ptr SystemWSCurl::connect(Connection connection)
    {
        auto op = std::make_shared<Operation>();
        op->owner = this;
        op->ctx = context();
        op->url = std::move(connection.url);
        op->ping_interval = connection.ping_interval;
        op->timeout = connection.timeout;
        op->reconnect = connection.reconnect;
        op->reconnect_min_delay = connection.reconnect_min_delay;
        op->reconnect_max_delay = connection.reconnect_max_delay;
        op->reconnect_max_attempts = connection.reconnect_max_attempts;
        op->on_setup = std::move(connection.on_setup);
        op->on_message = std::move(connection.on_message);
        op->on_error = std::move(connection.on_error);
        op->on_close = std::move(connection.on_close);
        op->on_open = std::move(connection.on_open);

        CurlBuilder builder(*op);
        if (op->on_setup)
            op->on_setup(builder);

        auto client = std::make_shared<CurlClient>(op->ctx, op, ptr());
        op->client = client;

        _run([this, op]() { _start(op); });

        return client;
    }
} // namespace d2::sys::net
