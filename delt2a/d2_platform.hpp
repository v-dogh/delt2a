#ifndef D2_PLATFORM_HPP
#define D2_PLATFORM_HPP

#ifndef D2_LOCALE_OS
#ifdef __unix__
#define D2_LOCALE_OS OS::Unix
#include "os/d2_unix_system_io.hpp"
#elif __linux__
#define D2_LOCALE_OS OS::Linux
#elif _WIN32
#define D2_LOCALE_OS OS::Windows
#elif __APPLE__ || __MACH__
#define D2_LOCALE_OS OS::MacOS
#else
#error(Unknown/Unsupported operating system, use D2_LOCALE_OS to set your own configuration)
#endif
#endif

namespace d2
{
	using os = OSConfig<std::size_t(D2_LOCALE_OS)>;
}

#endif // D2_PLATFORM_H
