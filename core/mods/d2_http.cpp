#include "core/mods/d2_http.hpp"

#include <core/mods/d2_session.hpp>

namespace d2::sys::net
{
    SystemHTTP::Stream::ptr SystemHTTP::request(Request request)
    {
        auto ctx = context();
        if (!ctx) throw std::logic_error("HTTP context is unavailable");
        if (auto s = ctx->sys_if<session>())
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
