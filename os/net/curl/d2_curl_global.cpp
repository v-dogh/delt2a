#include "os/net/curl/d2_curl_global.hpp"

namespace d2::sys::net
{
    CURLcode CurlGlobal::acquire()
    {
        std::lock_guard lock(_mtx);
        if (_refs++ == 0)
            return curl_global_init(CURL_GLOBAL_DEFAULT);
        return CURLE_OK;
    }
    CURLcode CurlGlobal::release()
    {
        std::lock_guard lock(_mtx);
        if (--_refs == 0)
            curl_global_cleanup();
        return CURLE_OK;
    }
} // namespace d2::sys::net
