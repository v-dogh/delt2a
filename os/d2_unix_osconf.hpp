#ifndef D2_UNIX_OSCONF_HPP
#define D2_UNIX_OSCONF_HPP

#include <d2_locale.hpp>
#include <mods/d2_ext.hpp>
#include <os/d2_unix_system_io.hpp>

namespace d2
{
    template<> struct OSConfig<std::size_t(OS::Unix)> : OSConfig<std::size_t(OS::Default)>
    {
        using input = sys::UnixTerminalInput;
        using output = sys::UnixTerminalOutput;
        using clipboard = sys::ext::LocalSystemClipboard;
    };
} // namespace d2

#endif // D2_UNIX_OSCONF_HPP
