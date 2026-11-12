#pragma once

#include <absl/container/flat_hash_map.h>
#include <atomic>
#include <curl/curl.h>
#include <functional>
#include <memory>
#include <mutex>
#include <core/mods/d2_http.hpp>
#include <mt/task_queue.hpp>
#include <string>
#include <thread>

namespace d2::sys::net
{
    class SystemHTTPCurl : public SystemHTTP,
                           public d2::sys::ModuleImpl<SystemHTTPCurl, d2::sys::Load::Lazy>
    {
    private:
        enum class BodyMode
        {
            None,
            Fixed,
            Streaming
        };

        struct StreamState;
        struct Operation;

        struct CurlBuilder;
        struct CurlChunk;
        struct CurlResponse;
        struct CurlStream;
    private:
        std::jthread _worker{};
        std::mutex _handle_mtx{};
        std::mutex _submit_mtx{};
        bool _accepting{false};
        bool _curl_acquired{false};
        mt::TaskQueue<std::function<void()>, true> _tasks{};

        std::atomic<CURLM*> _handle{nullptr};
        std::atomic<bool> _stop{true};

        absl::flat_hash_map<CURL*, std::shared_ptr<Operation>> _ops{};

        static std::size_t _write_cb(char* ptr, std::size_t size, std::size_t nmemb, void* data);
        static std::size_t _read_cb(char* ptr, std::size_t size, std::size_t nmemb, void* data);
        static std::size_t _header_cb(char* buffer, std::size_t size, std::size_t n, void* data);

        static int _progress_cb(
            void* data, curl_off_t dtotal, curl_off_t dnow, curl_off_t utotal, curl_off_t unow
        );

        void _wake();
        void _complete(std::shared_ptr<Operation> op);
        void _fail(std::shared_ptr<Operation> op, Error error);
        void _report_error(const std::shared_ptr<Operation>& op, const Error& error) noexcept;
        bool _run(std::function<void()> task);
        void _sweep();

        virtual Status _load_impl() override;
        virtual Status _unload_impl() override;

        virtual Stream::ptr _request_impl(Request request) override;
    public:
        using SystemHTTP::SystemHTTP;
        ~SystemHTTPCurl() override { unload(); }
    };
    using http_curl = SystemHTTPCurl;
} // namespace d2::sys::net
