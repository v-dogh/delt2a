#include "check.hpp"
#include "core/io/d2_context.hpp"
#include "os/net/curl/d2_http_curl.hpp"
#include "os/net/curl/d2_ws_curl.hpp"

int main()
{
    auto context = std::make_shared<d2::IOContext>(mt::ConcurrentPool::make<>(), nullptr);
    for (int i = 0; i < 3; ++i)
    {
        auto http = d2::sys::SystemModule::make<d2::sys::net::SystemHTTPCurl>(context);
        auto ws = d2::sys::SystemModule::make<d2::sys::net::SystemWSCurl>(context);
        http->load();
        ws->load();
        CHECK(http->status() == d2::sys::SystemModule::Status::Ok);
        CHECK(ws->status() == d2::sys::SystemModule::Status::Ok);
        http->unload();
        CHECK(http->status() == d2::sys::SystemModule::Status::Offline);
        http->load();
        CHECK(http->status() == d2::sys::SystemModule::Status::Ok);
    }
}
