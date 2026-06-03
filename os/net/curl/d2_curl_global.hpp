#pragma once

#include <curl/curl.h>
#include <mutex>

namespace d2::sys::net
{
    class CurlGlobal
    {
    private:
        static inline std::size_t _refs{0};
        static inline std::mutex _mtx{};
    public:
        static CURLcode acquire();
        static CURLcode release();
    };
} // namespace d2::sys::net

