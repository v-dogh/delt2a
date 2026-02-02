#ifndef D2_LINUX_OSCONF_HPP
#define D2_LINUX_OSCONF_HPP

#include "../d2_locale.hpp"
#include "d2_unix_system_io.hpp"
#ifndef D2_EXCLUDE_OS_EXTENSION
#   include "d2_linux_system_audio.hpp"
#endif

namespace d2
{
    template<>
    struct OSConfig<std::size_t(OS::Linux)> : OSConfig<std::size_t(OS::Default)>
    {
        using input = sys::UnixTerminalInput;
        using output = sys::UnixTerminalOutput;
        using screen =  sys::SystemScreen;
        using clipboard = sys::ext::LocalSystemClipboard;
#       ifndef D2_EXCLUDE_OS_EXT
            using audio = sys::ext::PipewireSystemAudio;
#       else
            using audio = void;
#       endif
    };
}

#endif // D2_LINUX_OSCONF_HPP
