#include "d2_http.hpp"

#include <mods/d2_session.hpp>

namespace d2::sys::net
{
    SystemHTTP::Stream::ptr SystemHTTP::request(Request request)
    {
        if (auto s = context()->sys_if<session>())
        {
            request.on_setup = [=, callback = std::move(request.on_setup)](RequestBuilder& builder)
            {
                s->process(
                    request.url,
                    request.method,
                    [&](std::string_view header, std::string_view value)
                    { builder.set(header, value); }
                );
                if (callback)
                    callback(builder);
            };
        }
        return _request_impl(std::move(request));
    }
} // namespace d2::sys::net
