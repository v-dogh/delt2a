#include "core/mods/d2_ws.hpp"

#include <core/mods/d2_session.hpp>

namespace d2::sys::net
{
    SystemWS::Client::ptr SystemWS::connect(Connection connection)
    {
        if (auto s = context()->sys_if<session>())
        {
            // Trampoline
            connection.on_setup =
                [=, callback = std::move(connection.on_setup)](RequestBuilder& builder)
            {
                s->process(
                    connection.url,
                    "UPGRADE",
                    [&](std::string_view header, std::string_view value)
                    { builder.set(header, value); }
                );
                if (callback)
                    callback(builder);
            };
        }
        return _connect_impl(std::move(connection));
    }
} // namespace d2::sys::net
