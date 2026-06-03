#include "d2_http_curl.hpp"

#include "d2_curl_global.hpp"

namespace d2::sys::net
{
    struct SystemHTTPCurl::StreamState
    {
        std::atomic_bool cancelled = false;

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

        d2::IOContext::ptr ctx{nullptr};

        std::string method{};
        std::string url{};

        BodyMode body_mode{BodyMode::None};
        std::string request_body{};
        std::string builder_error{};

        mt::TaskRing<std::string, 4> upload_queue{};
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

        std::function<void(Response&)> on_response;
        std::function<void(std::string, d2::IOContext::ptr)> on_error;
        std::function<void(Chunk&)> on_chunk;

        std::shared_ptr<StreamState> stream_state{};

        std::atomic<bool> cancelled{false};

        bool setup();

        ~Operation();
    };

    struct SystemHTTPCurl::CurlBuilder : RequestBuilder
    {
        Operation& op;

        explicit CurlBuilder(Operation& op) : op(op)
        {
            ctx = op.ctx;
        }

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
            ctx = op.ctx;
        }

        virtual std::string_view read() override;
        virtual void cancel() override;

        virtual std::size_t down_current() override;
        virtual std::size_t down_total() override;
    };
    struct SystemHTTPCurl::CurlResponse : Response
    {
        Operation& op;

        explicit CurlResponse(Operation& op) : op(op)
        {
            ctx = op.ctx;
        }

        virtual int status() override;
        virtual std::optional<std::string_view> get(std::string_view header) override;
        virtual std::string_view body() override;
    };
    struct SystemHTTPCurl::CurlStream : Stream
    {
        d2::sys::module<SystemHTTPCurl> ptr;
        std::weak_ptr<Operation> op;

        explicit CurlStream(std::shared_ptr<Operation> op, d2::sys::module<SystemHTTPCurl> ptr) :
            ptr(ptr), op(op)
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
        if (locked->cancelled)
            return;

        std::string chunk(data);
        locked->upload_queue.enqueue(std::move(chunk));
        if (ptr)
        {
            ptr->_run(
                [op = locked]()
                {
                    if (!op->handle)
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

        if (ptr)
        {
            ptr->_run(
                [op = locked]()
                {
                    op->upload_finished = true;
                    if (!op->handle)
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
        if (ptr)
            ptr->_wake();
    }
    bool SystemHTTPCurl::CurlStream::is_open()
    {
        if (auto locked = op.lock())
            return locked->cancelled;
        return false;
    }

    std::size_t SystemHTTPCurl::CurlStream::up_current()
    {
        auto locked = op.lock();
        if (!locked || !locked->stream_state)
            return 0;
        return static_cast<std::size_t>(locked->stream_state->uploaded.load());
    }

    std::size_t SystemHTTPCurl::CurlStream::up_total()
    {
        auto locked = op.lock();
        if (!locked || !locked->stream_state)
            return 0;
        auto& st = *locked->stream_state;
        if (!st.has_upload_total)
            return 0;
        return static_cast<std::size_t>(st.upload_total.load());
    }

    // Callbacks

    std::size_t
    SystemHTTPCurl::_write_cb(char* ptr, std::size_t size, std::size_t nmemb, void* data)
    {
        auto* op = static_cast<Operation*>(data);
        const auto bytes = size * nmemb;
        if (op->cancelled)
            return CURL_WRITEFUNC_ERROR;
        if (op->on_chunk)
        {
            std::string_view buffer(ptr, bytes);
            std::size_t downloaded = bytes;
            std::size_t downloaded_total = 0;

            if (op->stream_state)
            {
                auto& st = *op->stream_state;

                downloaded = static_cast<std::size_t>(st.downloaded.fetch_add(bytes) + bytes);

                if (st.has_download_total)
                    downloaded_total = static_cast<std::size_t>(st.download_total.load());
            }

            SystemHTTPCurl::CurlChunk fragment(*op, buffer, downloaded, downloaded_total);
            op->on_chunk(fragment);
        }
        else
        {
            op->response_body.append(ptr, bytes);
        }

        return bytes;
    }

    std::size_t SystemHTTPCurl::_read_cb(char* ptr, std::size_t size, std::size_t nmemb, void* data)
    {
        auto* op = static_cast<Operation*>(data);
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

    std::size_t
    SystemHTTPCurl::_header_cb(char* buffer, std::size_t size, std::size_t n, void* data)
    {
        auto* op = static_cast<Operation*>(data);

        const auto bytes = size * n;
        std::string_view line(buffer, bytes);
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
            line.remove_suffix(1);

        const auto colon = line.find(':');
        if (colon == std::string_view::npos)
            return bytes;

        auto name = line.substr(0, colon);
        auto value = line.substr(colon + 1);
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())))
            value.remove_prefix(1);
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
            value.remove_suffix(1);

        const auto header = to_lower(name);
        op->response_headers[header] = std::string(value);
        if (header == "content-length" && op->stream_state)
        {
            try
            {
                const auto total = static_cast<std::uint64_t>(std::stoull(std::string(value)));

                op->stream_state->has_download_total = true;
                op->stream_state->download_total = total;
            }
            catch (...)
            {
            }
        }
        return bytes;
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

        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);

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
        if (auto* handle = _handle.load())
            curl_multi_wakeup(handle);
    }

    void SystemHTTPCurl::_complete(std::shared_ptr<Operation> op)
    {
        if (op->curl_code != CURLE_OK)
        {
            _fail(op, op->error_string);
            return;
        }
        if (!op->on_response)
            return;
        CurlResponse res(*op);
        op->on_response(res);
    }

    void SystemHTTPCurl::_fail(std::shared_ptr<Operation> op, std::string error)
    {
        if (error.empty())
            error = "Unknown HTTP transport error";
        if (op->on_error)
        {
            op->on_error(std::move(error), op->ctx);
            return;
        }
        D2_TLOG(Error, "HTTP request failed: ", error);
    }

    void SystemHTTPCurl::_run(std::function<void()> task)
    {
        _tasks.enqueue(std::move(task));
        _wake();
    }

    SystemHTTPCurl::Status SystemHTTPCurl::_load_impl()
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
                auto* handle = curl_multi_init();
                if (!handle)
                {
                    D2_TLOG(Critical, "Failed to initialize curl multi handle");
                    _handle = nullptr;
                    _stop = false;
                    _stop.notify_all();
                    return;
                }

                _handle = handle;
                _stop = false;
                _stop.notify_all();

                int running = 0;
                while (!_stop)
                {
                    {
                        std::function<void()> task;
                        while (_tasks.try_dequeue(task))
                            task();
                    }
                    {
                        const auto code = curl_multi_perform(handle, &running);
                        if (code != CURLM_OK)
                        {
                            D2_TLOG(
                                Critical,
                                "Unexpected curl multi perform error: ",
                                curl_multi_strerror(code)
                            );
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
                            curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &op->status);
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
                        int cnt = 0;
                        const auto code = curl_multi_poll(handle, nullptr, 0, 1000, &cnt);
                        if (code != CURLM_OK)
                        {
                            D2_TLOG(
                                Critical,
                                "Unexpected curl multi poll error: ",
                                curl_multi_strerror(code)
                            );
                        }
                    }
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

                curl_multi_cleanup(handle);
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
    SystemHTTPCurl::Status SystemHTTPCurl::_unload_impl()
    {
        _stop = true;
        _wake();
        if (_worker.joinable())
            _worker.join();
        CurlGlobal::release();
        return Status::Ok;
    }

    SystemHTTPCurl::Stream::ptr SystemHTTPCurl::request(Request request)
    {
        auto op = std::make_shared<Operation>();

        op->owner = this;
        op->ctx = context();
        op->method = std::string(request.method);
        op->url = std::string(request.url);
        op->on_response = std::move(request.on_response);
        op->on_error = std::move(request.on_error);
        op->on_chunk = std::move(request.on_chunk);

        CurlBuilder builder(*op);
        if (request.on_setup)
            request.on_setup(builder);
        if (op->on_chunk || op->body_mode == BodyMode::Streaming)
            op->stream_state = std::make_shared<StreamState>();

        Stream::ptr stream{nullptr};
        if (op->body_mode == BodyMode::Streaming)
            stream = std::make_unique<CurlStream>(op, ptr());

        _run(
            [this, op]() mutable
            {
                if (!op->setup())
                {
                    if (op->curl_code == CURLE_OK)
                        op->curl_code = CURLE_FAILED_INIT;
                    if (op->error_string.empty())
                        op->error_string = "Failed to initialize curl request handle";

                    _fail(op, op->error_string);
                    return;
                }

                const auto code = curl_multi_add_handle(_handle.load(), op->handle);
                if (code != CURLM_OK)
                {
                    op->curl_code = CURLE_FAILED_INIT;
                    op->error_string = curl_multi_strerror(code);
                    D2_TLOG(Error, "Failed to add curl request handle: ", op->error_string);
                    _fail(op, op->error_string);
                    return;
                }
                _ops.emplace(op->handle, op);
            }
        );

        return stream;
    }
} // namespace d2::sys::net
