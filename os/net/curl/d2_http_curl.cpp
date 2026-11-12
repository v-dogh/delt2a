#include "os/net/curl/d2_http_curl.hpp"

#include "os/net/curl/d2_curl_global.hpp"
#include <charconv>
#include <climits>
#include <limits>

namespace d2::sys::net
{
    struct SystemHTTPCurl::StreamState
    {
        std::atomic_bool cancelled = false;
        std::atomic_bool completed = false;

        std::atomic<std::uint64_t> downloaded = 0;
        std::atomic<std::uint64_t> uploaded = 0;

        std::atomic_bool has_download_total = false;
        std::atomic<std::uint64_t> download_total = 0;

        std::atomic_bool has_upload_total = false;
        std::atomic<std::uint64_t> upload_total = 0;
    };
    struct SystemHTTPCurl::Operation
    {
        SystemHTTPCurl* owner{nullptr};

        CURL* handle{nullptr};
        curl_slist* request_headers{nullptr};

        std::string method{};
        std::string url{};

        BodyMode body_mode{BodyMode::None};
        std::string request_body{};
        std::string builder_error{};

        mt::TaskQueue<std::string, true> upload_queue{};
        std::string upload_current{};
        std::size_t upload_offset{0};

        bool upload_paused{false};
        bool upload_finished{false};
        std::optional<std::uint64_t> upload_declared_total{};

        std::string response_body;
        absl::flat_hash_map<std::string, std::string> response_headers;

        int status = 0;
        CURLcode curl_code = CURLE_OK;
        std::array<char, CURL_ERROR_SIZE> error_buffer{};
        std::string error_string{};
        std::optional<Error> failure;
        std::optional<std::chrono::steady_clock::time_point> deadline;
        std::optional<std::chrono::milliseconds> connect_timeout;
        std::size_t max_response_bytes{0}, max_header_bytes{0}, header_bytes{0};
        bool follow_redirects{true}, headers_open{false};
        long max_redirects{10};

        std::function<void(Response&)> on_response;
        std::function<void(const Error&)> on_error;
        std::function<void(Chunk&)> on_chunk;
        std::function<void(Response&)> on_headers;

        std::shared_ptr<StreamState> stream_state = std::make_shared<StreamState>();

        std::atomic<bool> cancelled{false};

        bool setup();
        void callback_failed() noexcept
        {
            failure = Error{Error::Code::Callback, {}, std::current_exception()};
        }

        ~Operation();
    };

    struct SystemHTTPCurl::CurlBuilder : RequestBuilder
    {
        Operation& op;

        explicit CurlBuilder(Operation& op) : op(op) {}

        virtual void set(std::string_view header, std::string_view value) override;
        virtual void body(std::string_view value) override;
        virtual void stream() override;
    };
    struct SystemHTTPCurl::CurlChunk : Chunk
    {
        Operation& op;
        std::string_view data;

        std::size_t downloaded_snapshot{0};
        std::size_t downloaded_total_snapshot{0};

        explicit CurlChunk(
            Operation& op,
            std::string_view data,
            std::size_t downloaded,
            std::size_t downloaded_total
        ) :
            op(op), data(data), downloaded_snapshot(downloaded),
            downloaded_total_snapshot(downloaded_total)
        {
        }

        virtual std::string_view read() override;
        virtual void cancel() override;

        virtual std::size_t down_current() override;
        virtual std::size_t down_total() override;
    };
    struct SystemHTTPCurl::CurlResponse : Response
    {
        Operation& op;

        explicit CurlResponse(Operation& op) : op(op) {}

        virtual int status() override;
        virtual std::optional<std::string_view> get(std::string_view header) override;
        virtual std::string_view body() override;
    };
    struct SystemHTTPCurl::CurlStream : Stream
    {
        std::weak_ptr<SystemHTTPCurl> ptr;
        std::weak_ptr<Operation> op;
        std::shared_ptr<StreamState> state;

        explicit CurlStream(std::shared_ptr<Operation> op, std::weak_ptr<SystemHTTPCurl> ptr) :
            ptr(ptr), op(op), state(op->stream_state)
        {
        }
        virtual ~CurlStream() override;

        virtual void write(std::string_view data) override;
        virtual void finish() override;
        virtual void cancel() override;
        virtual bool is_open() override;

        virtual std::size_t up_current() override;
        virtual std::size_t up_total() override;
    };

    namespace
    {
        std::string to_lower(std::string_view str)
        {
            std::string lower;
            lower.reserve(str.size());
            for (unsigned char c : str)
            {
                if (c >= 'A' && c <= 'Z')
                    lower.push_back(static_cast<char>(c - 'A' + 'a'));
                else
                    lower.push_back(static_cast<char>(c));
            }
            return lower;
        }
        bool is_put(std::string_view method)
        {
            return method == "PUT" || method == "put";
        }
    } // namespace

    // States

    void SystemHTTPCurl::CurlBuilder::set(std::string_view header, std::string_view value)
    {
        std::string line;
        line.reserve(header.size() + value.size() + 2);
        line.append(header);
        line.append(": ");
        line.append(value);

        auto* next = curl_slist_append(op.request_headers, line.c_str());
        if (!next)
        {
            op.builder_error = "Failed to append curl request header";
            return;
        }
        op.request_headers = next;
    }

    void SystemHTTPCurl::CurlBuilder::body(std::string_view value)
    {
        if (op.body_mode == BodyMode::Streaming)
        {
            op.builder_error = "Cannot use body() after stream()";
            return;
        }
        op.body_mode = BodyMode::Fixed;
        op.request_body.assign(value);
    }

    void SystemHTTPCurl::CurlBuilder::stream()
    {
        if (op.body_mode == BodyMode::Fixed)
        {
            op.builder_error = "Cannot use stream() after body()";
            return;
        }
        op.body_mode = BodyMode::Streaming;
    }

    std::string_view SystemHTTPCurl::CurlChunk::read()
    {
        return data;
    }

    void SystemHTTPCurl::CurlChunk::cancel()
    {
        op.cancelled = true;

        if (op.stream_state)
            op.stream_state->cancelled = true;

        if (op.owner)
            op.owner->_wake();
    }

    std::size_t SystemHTTPCurl::CurlChunk::down_current()
    {
        return downloaded_snapshot;
    }

    std::size_t SystemHTTPCurl::CurlChunk::down_total()
    {
        return downloaded_total_snapshot;
    }

    int SystemHTTPCurl::CurlResponse::status()
    {
        return static_cast<int>(op.status);
    }

    std::optional<std::string_view> SystemHTTPCurl::CurlResponse::get(std::string_view header)
    {
        auto it = op.response_headers.find(to_lower(header));
        if (it == op.response_headers.end())
            return std::nullopt;
        return std::string_view(it->second);
    }

    std::string_view SystemHTTPCurl::CurlResponse::body()
    {
        return op.response_body;
    }

    SystemHTTPCurl::CurlStream::~CurlStream()
    {
        finish();
    }

    void SystemHTTPCurl::CurlStream::write(std::string_view data)
    {
        if (data.empty())
            return;

        auto locked = op.lock();
        if (!locked)
            return;
        if (locked->body_mode != BodyMode::Streaming)
            return;
        if (locked->cancelled || state->completed)
            return;

        std::string chunk(data);
        locked->upload_queue.enqueue(std::move(chunk));
        if (auto owner = ptr.lock())
        {
            owner->_run(
                [op = locked]()
                {
                    if (!op->handle || op->stream_state->completed)
                        return;

                    if (op->upload_paused)
                    {
                        op->upload_paused = false;
                        curl_easy_pause(op->handle, CURLPAUSE_CONT);
                    }
                }
            );
        }
    }
    void SystemHTTPCurl::CurlStream::finish()
    {
        auto locked = op.lock();

        if (!locked)
            return;
        if (locked->body_mode != BodyMode::Streaming)
            return;

        if (auto owner = ptr.lock())
        {
            owner->_run(
                [op = locked]()
                {
                    op->upload_finished = true;
                    if (!op->handle || op->stream_state->completed)
                        return;
                    if (op->upload_paused)
                    {
                        op->upload_paused = false;
                        curl_easy_pause(op->handle, CURLPAUSE_CONT);
                    }
                }
            );
        }
    }

    void SystemHTTPCurl::CurlStream::cancel()
    {
        if (auto locked = op.lock())
        {
            locked->cancelled = true;
            if (locked->stream_state)
                locked->stream_state->cancelled = true;
        }
        if (auto owner = ptr.lock())
            owner->_wake();
    }
    bool SystemHTTPCurl::CurlStream::is_open()
    { return !state->completed && !state->cancelled; }

    std::size_t SystemHTTPCurl::CurlStream::up_current()
    { return static_cast<std::size_t>(state->uploaded.load()); }

    std::size_t SystemHTTPCurl::CurlStream::up_total()
    { return state->has_upload_total ? static_cast<std::size_t>(state->upload_total.load()) : 0; }

    // Callbacks

    std::size_t
    SystemHTTPCurl::_write_cb(char* ptr, std::size_t size, std::size_t nmemb, void* data)
    {
        auto* op = static_cast<Operation*>(data);
        try
        {
            if (size && nmemb > std::numeric_limits<std::size_t>::max() / size) return 0;
            const auto bytes = size * nmemb;
            if (op->cancelled) return 0;
            auto& state = *op->stream_state;
            const auto previous = state.downloaded.load();
            if (op->max_response_bytes && (previous > op->max_response_bytes ||
                bytes > op->max_response_bytes - previous))
            {
                op->failure = Error{Error::Code::Limit, "HTTP response exceeded byte limit"};
                return 0;
            }
            state.downloaded += bytes;
            if (op->on_chunk)
            {
                CurlChunk fragment(*op, {ptr, bytes}, state.downloaded,
                    state.has_download_total ? state.download_total.load() : 0);
                op->on_chunk(fragment);
            }
            else op->response_body.append(ptr, bytes);
            return op->cancelled ? 0 : bytes;
        }
        catch (...) { op->callback_failed(); return 0; }
    }

    std::size_t SystemHTTPCurl::_read_cb(char* ptr, std::size_t size, std::size_t nmemb, void* data)
    {
        auto* op = static_cast<Operation*>(data);
        try
        {
            if (size && nmemb > std::numeric_limits<std::size_t>::max() / size) return CURL_READFUNC_ABORT;
            const auto capacity = size * nmemb;
            if (op->cancelled)
                return CURL_READFUNC_ABORT;
            if (capacity == 0)
                return 0;

            std::size_t written = 0;
            while (written < capacity)
            {
                if (op->upload_offset >= op->upload_current.size())
                {
                    op->upload_current.clear();
                    op->upload_offset = 0;
                    std::string next;
                    while (op->upload_queue.try_dequeue(next))
                    {
                        if (!next.empty())
                        {
                            op->upload_current = std::move(next);
                            break;
                        }
                    }
                    if (op->upload_current.empty())
                        break;
                }

                const auto remaining_chunk = op->upload_current.size() - op->upload_offset;
                const auto remaining_buffer = capacity - written;
                const auto amount = std::min(remaining_chunk, remaining_buffer);

                std::memcpy(ptr + written, op->upload_current.data() + op->upload_offset, amount);

                op->upload_offset += amount;
                written += amount;
            }

            if (written > 0)
                return written;

            if (op->upload_finished)
                return 0;

            op->upload_paused = true;
            return CURL_READFUNC_PAUSE;
        }
        catch (...) { op->callback_failed(); return CURL_READFUNC_ABORT; }
    }

    std::size_t
    SystemHTTPCurl::_header_cb(char* buffer, std::size_t size, std::size_t n, void* data)
    {
        auto* op = static_cast<Operation*>(data);
        try
        {
            if (size && n > std::numeric_limits<std::size_t>::max() / size) return 0;
            const auto bytes = size * n;
            if (op->cancelled) return 0;
            if (op->max_header_bytes && (op->header_bytes > op->max_header_bytes ||
                bytes > op->max_header_bytes - op->header_bytes))
            {
                op->failure = Error{Error::Code::Limit, "HTTP headers exceeded byte limit"};
                return 0;
            }
            op->header_bytes += bytes;
            std::string_view line(buffer, bytes);
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.remove_suffix(1);
            if (line.starts_with("HTTP/"))
            {
                const auto space = line.find(' ');
                int status = 0;
                if (space == line.npos || std::from_chars(line.data() + space + 1,
                    line.data() + line.size(), status).ec != std::errc{} || status < 100 || status > 999)
                    return 0;
                op->status = status;
                op->response_headers.clear(); op->response_body.clear();
                op->stream_state->has_download_total = false;
                op->headers_open = true;
            }
            else if (line.empty())
            {
                if (op->headers_open)
                {
                    op->headers_open = false;
                    if (op->on_headers)
                    {
                        CurlResponse response(*op);
                        op->on_headers(response);
                    }
                }
            }
            else if (const auto colon = line.find(':'); colon != line.npos)
            {
                auto value = line.substr(colon + 1);
                while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) value.remove_prefix(1);
                while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) value.remove_suffix(1);
                const auto header = to_lower(line.substr(0, colon));
                op->response_headers[header] = std::string(value);
                if (header == "content-length")
                {
                    std::uint64_t total = 0;
                    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), total);
                    if (parsed.ec == std::errc{} && parsed.ptr == value.data() + value.size())
                    {
                        op->stream_state->download_total = total;
                        op->stream_state->has_download_total = true;
                    }
                }
            }
            return op->cancelled ? 0 : bytes;
        }
        catch (...) { op->callback_failed(); return 0; }
    }

    int SystemHTTPCurl::_progress_cb(
        void* data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow
    )
    {
        auto* op = static_cast<Operation*>(data);
        if (op->cancelled)
            return 1;
        if (op->stream_state)
        {
            auto& st = *op->stream_state;

            st.uploaded = static_cast<std::uint64_t>(std::max<curl_off_t>(0, ulnow));

            if (dltotal > 0)
            {
                st.has_download_total = true;
                st.download_total = static_cast<std::uint64_t>(dltotal);
            }

            if (ultotal > 0)
            {
                st.has_upload_total = true;
                st.upload_total = static_cast<std::uint64_t>(ultotal);
            }
            else if (op->upload_declared_total)
            {
                st.has_upload_total = true;
                st.upload_total = *op->upload_declared_total;
            }
            else
            {
                st.has_upload_total = false;
            }
        }
        return 0;
    }

    // Rest

    bool SystemHTTPCurl::Operation::setup()
    {
        if (!builder_error.empty())
        {
            curl_code = CURLE_FAILED_INIT;
            error_string = builder_error;
            return false;
        }

        auto* handle = curl_easy_init();
        if (!handle)
        {
            curl_code = CURLE_FAILED_INIT;
            error_string = "Failed to initialize curl easy handle";
            return false;
        }
        this->handle = handle;

        error_buffer[0] = '\0';
        curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buffer.data());

        curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

        if (!method.empty())
            curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, method.c_str());

        if (request_headers)
            curl_easy_setopt(handle, CURLOPT_HTTPHEADER, request_headers);

        curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, _progress_cb);
        curl_easy_setopt(handle, CURLOPT_XFERINFODATA, this);

        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, _write_cb);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);

        curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, _header_cb);
        curl_easy_setopt(handle, CURLOPT_HEADERDATA, this);

        curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, follow_redirects ? 1L : 0L);
        curl_easy_setopt(handle, CURLOPT_MAXREDIRS, max_redirects);
        if (connect_timeout) curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, long(connect_timeout->count()));
        if (deadline)
        {
            const auto remaining = std::chrono::ceil<std::chrono::milliseconds>(*deadline - std::chrono::steady_clock::now());
            curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, long(std::clamp<std::int64_t>(remaining.count(), 1, LONG_MAX)));
        }

        if (body_mode == BodyMode::Fixed)
        {
            const char* data = request_body.empty() ? "" : request_body.data();

            curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data);
            curl_easy_setopt(
                handle, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(request_body.size())
            );
        }
        else if (body_mode == BodyMode::Streaming)
        {
            curl_easy_setopt(handle, CURLOPT_READFUNCTION, _read_cb);
            curl_easy_setopt(handle, CURLOPT_READDATA, this);

            if (!stream_state)
                stream_state = std::make_shared<StreamState>();
            if (upload_declared_total)
            {
                stream_state->has_upload_total = true;
                stream_state->upload_total = *upload_declared_total;

                if (is_put(method))
                {
                    curl_easy_setopt(
                        handle,
                        CURLOPT_INFILESIZE_LARGE,
                        static_cast<curl_off_t>(*upload_declared_total)
                    );
                }
                else
                {
                    curl_easy_setopt(
                        handle,
                        CURLOPT_POSTFIELDSIZE_LARGE,
                        static_cast<curl_off_t>(*upload_declared_total)
                    );
                }
            }
            else
            {
                auto* next = curl_slist_append(request_headers, "Transfer-Encoding: chunked");
                if (!next)
                {
                    curl_code = CURLE_FAILED_INIT;
                    error_string = "Failed to append chunked transfer header";
                    return false;
                }
                request_headers = next;
                curl_easy_setopt(handle, CURLOPT_HTTPHEADER, request_headers);
            }

            if (is_put(method))
            {
                curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
            }
            else
            {
                curl_easy_setopt(handle, CURLOPT_POST, 1L);
            }
        }

        return true;
    }

    SystemHTTPCurl::Operation::~Operation()
    {
        if (request_headers)
            curl_slist_free_all(request_headers);
        if (handle)
            curl_easy_cleanup(handle);
    }

    void SystemHTTPCurl::_wake()
    {
        std::lock_guard lock(_handle_mtx);
        if (auto* handle = _handle.load())
            curl_multi_wakeup(handle);
    }

    void SystemHTTPCurl::_complete(std::shared_ptr<Operation> op)
    {
        if (op->failure) { _fail(op, std::move(*op->failure)); return; }
        if (op->cancelled) { _fail(op, {Error::Code::Cancelled, "HTTP request cancelled"}); return; }
        if ((op->deadline && std::chrono::steady_clock::now() >= *op->deadline) || op->curl_code == CURLE_OPERATION_TIMEDOUT)
        { _fail(op, {Error::Code::Timeout, "HTTP request timed out", {}, int(op->curl_code)}); return; }
        if (op->curl_code != CURLE_OK)
        { _fail(op, {Error::Code::Transport, op->error_string, {}, int(op->curl_code)}); return; }
        if (op->stream_state->completed.exchange(true)) return;
        try
        {
            if (op->on_response)
            {
                CurlResponse response(*op);
                op->on_response(response);
            }
        }
        catch (...) { _report_error(op, {Error::Code::Callback, "HTTP response callback failed", std::current_exception()}); }
    }
    void SystemHTTPCurl::_report_error(const std::shared_ptr<Operation>& op, const Error& error) noexcept
    {
        try
        {
            if (op->on_error) op->on_error(error);
            else D2_TLOG(Error, "HTTP request failed: ", error.message);
        }
        catch (...) { /* A failing error handler must not unwind through the transport worker. */ }
    }
    void SystemHTTPCurl::_fail(std::shared_ptr<Operation> op, Error error)
    {
        if (op->stream_state->completed.exchange(true)) return;
        if (error.message.empty()) error.message = error.code == Error::Code::Callback ?
            "HTTP callback failed" : "HTTP transport failed";
        _report_error(op, error);
    }
    bool SystemHTTPCurl::_run(std::function<void()> task)
    {
        {
            std::lock_guard lock(_submit_mtx);
            if (!_accepting) return false;
            _tasks.enqueue(std::move(task));
        }
        _wake();
        return true;
    }
    void SystemHTTPCurl::_sweep()
    {
        const auto now = std::chrono::steady_clock::now();
        for (auto it = _ops.begin(); it != _ops.end();)
        {
            auto op = it->second;
            if (!op->cancelled && (!op->deadline || now < *op->deadline)) { ++it; continue; }
            curl_multi_remove_handle(_handle.load(), op->handle);
            _ops.erase(it++);
            _complete(std::move(op));
        }
    }

    SystemHTTPCurl::Status SystemHTTPCurl::_load_impl()
    {
        auto initialized = mt::future<bool>::make();
        const auto code = CurlGlobal::acquire();
        if (code != CURLE_OK)
        {
            D2_TLOG(Critical, "Unexpected curl error: ", curl_easy_strerror(code));
            return Status::Failure;
        }

        _curl_acquired = true;
        try
        {
            _worker = context()->launch_thread(
                [this, initialized]()
                {
                    auto* handle = curl_multi_init();
                    if (!handle)
                    {
                        D2_TLOG(Critical, "Failed to initialize curl multi handle");
                        _handle = nullptr;
                        initialized.block()->set_value(false);
                        return;
                    }

                    _handle = handle;
                    { std::lock_guard lock(_submit_mtx); _stop = false; _accepting = true; }
                    initialized.block()->set_value(true);

                    int running = 0;
                    while (!_stop)
                    {
                        {
                            std::function<void()> task;
                            while (_tasks.try_dequeue(task))
                                task();
                        }
                        _sweep();
                        {
                            const auto code = curl_multi_perform(handle, &running);
                            if (code != CURLM_OK)
                            {
                                D2_TLOG(
                                    Critical,
                                    "Unexpected curl multi perform error: ",
                                    curl_multi_strerror(code)
                                );
                                for (auto& [_, op] : _ops)
                                    op->failure = Error{Error::Code::Transport, curl_multi_strerror(code)};
                                break;
                            }
                        }
                        {
                            int queued = 0;
                            while (auto* msg = curl_multi_info_read(handle, &queued))
                            {
                                if (msg->msg != CURLMSG_DONE)
                                    continue;

                                auto* easy = msg->easy_handle;
                                auto it = _ops.find(easy);
                                if (it == _ops.end())
                                    continue;

                                auto op = it->second;
                                long status = 0;
                                curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &status);
                                op->status = int(status);
                                op->curl_code = msg->data.result;
                                if (op->cancelled)
                                {
                                    op->curl_code = CURLE_ABORTED_BY_CALLBACK;
                                    op->error_string = "Request cancelled";
                                }
                                else if (op->curl_code == CURLE_OK)
                                {
                                    op->error_string.clear();
                                }
                                else if (op->error_buffer[0] != '\0')
                                {
                                    op->error_string = std::string(op->error_buffer.data());
                                }
                                else
                                {
                                    op->error_string = curl_easy_strerror(op->curl_code);
                                }

                                curl_multi_remove_handle(handle, easy);
                                _ops.erase(it);

                                _complete(std::move(op));
                            }
                        }
                        {
                            _sweep();
                            int timeout = 1000;
                            const auto now = std::chrono::steady_clock::now();
                            for (const auto& [_, op] : _ops)
                                if (op->deadline)
                                    timeout = int(std::min<std::int64_t>(timeout, std::max<std::int64_t>(0,
                                        std::chrono::ceil<std::chrono::milliseconds>(*op->deadline - now).count())));
                            int cnt = 0;
                            const auto code = curl_multi_poll(handle, nullptr, 0, timeout, &cnt);
                            if (code != CURLM_OK)
                            {
                                D2_TLOG(
                                    Critical,
                                    "Unexpected curl multi poll error: ",
                                    curl_multi_strerror(code)
                                );
                                for (auto& [_, op] : _ops)
                                    op->failure = Error{Error::Code::Transport, curl_multi_strerror(code)};
                                break;
                            }
                        }
                    }

                    {
                        std::lock_guard lock(_submit_mtx);
                        _accepting = false; _stop = true;
                    }
                    {
                        std::function<void()> task;
                        while (_tasks.try_dequeue(task)) task();
                    }
                    for (auto& [_, op] : _ops)
                    {
                        curl_multi_remove_handle(handle, op->handle);
                        op->cancelled = true;
                        op->curl_code = CURLE_ABORTED_BY_CALLBACK;
                        op->error_string = "HTTP module unloaded before request completed";
                        _complete(op);
                    }

                    _ops.clear();

                    {
                        std::lock_guard lock(_handle_mtx);
                        _handle = nullptr;
                        curl_multi_cleanup(handle);
                    }
                }
            );
        }
        catch (...)
        {
            if (std::exchange(_curl_acquired, false)) CurlGlobal::release();
            throw;
        }
        if (!initialized.value())
        {
            if (_worker.joinable())
                _worker.join();
            if (std::exchange(_curl_acquired, false))
                CurlGlobal::release();
            return Status::Failure;
        }
        return Status::Ok;
    }
    SystemHTTPCurl::Status SystemHTTPCurl::_unload_impl()
    {
        if (_worker.joinable() && _worker.get_id() == std::this_thread::get_id())
            throw std::logic_error("Cannot unload curl from its callback thread");
        { std::lock_guard lock(_submit_mtx); _accepting = false; _stop = true; }
        _wake();
        if (_worker.joinable())
            _worker.join();
        if (std::exchange(_curl_acquired, false))
            CurlGlobal::release();
        return Status::Ok;
    }

    SystemHTTPCurl::Stream::ptr SystemHTTPCurl::_request_impl(Request request)
    {
        const auto started = std::chrono::steady_clock::now();
        auto op = std::make_shared<Operation>();
        op->owner = this;
        op->method = std::string(request.method); op->url = std::move(request.url);
        op->on_response = std::move(request.on_response); op->on_error = std::move(request.on_error);
        op->on_chunk = std::move(request.on_chunk); op->on_headers = std::move(request.on_headers);
        op->deadline = request.deadline; op->connect_timeout = request.connect_timeout;
        op->max_response_bytes = request.max_response_bytes; op->max_header_bytes = request.max_header_bytes;
        op->follow_redirects = request.follow_redirects; op->max_redirects = request.max_redirects;
        auto self = std::static_pointer_cast<SystemHTTPCurl>(shared_from_this());
        auto stream = std::make_unique<CurlStream>(op, self);
        if ((request.timeout && (request.timeout->count() <= 0 || *request.timeout >
                std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::time_point::max() - started))) ||
            (request.connect_timeout && (request.connect_timeout->count() <= 0 || request.connect_timeout->count() > LONG_MAX)) ||
            request.max_redirects < 0)
        {
            _fail(op, {Error::Code::InvalidRequest, "Invalid HTTP timeout or redirect limit"});
            return stream;
        }
        if (request.timeout)
        {
            const auto deadline = started + *request.timeout;
            op->deadline = op->deadline ? std::min(*op->deadline, deadline) : deadline;
        }
        CurlBuilder builder(*op);
        try { if (request.on_setup) request.on_setup(builder); }
        catch (...)
        {
            _fail(op, {Error::Code::Callback, "HTTP setup callback failed", std::current_exception()});
            return stream;
        }
        if (!_run([this, op]()
        {
            try
            {
                if (_stop) { _fail(op, {Error::Code::Cancelled, "HTTP module unloaded"}); return; }
                if (op->cancelled || (op->deadline && std::chrono::steady_clock::now() >= *op->deadline))
                { _complete(op); return; }
                if (!op->setup())
                { _fail(op, {Error::Code::InvalidRequest, op->error_string, {}, int(op->curl_code)}); return; }
                // Reserve the registry entry before handing the easy handle to curl.
                _ops.emplace(op->handle, op);
                const auto code = curl_multi_add_handle(_handle.load(), op->handle);
                if (code != CURLM_OK)
                {
                    _ops.erase(op->handle);
                    _fail(op, {Error::Code::Transport, curl_multi_strerror(code)});
                }
            }
            catch (...) { _fail(op, {Error::Code::Transport, "HTTP submission failed", std::current_exception()}); }
        })) _fail(op, {Error::Code::Unavailable, "HTTP module is not accepting requests"});
        return stream;
    }
} // namespace d2::sys::net
