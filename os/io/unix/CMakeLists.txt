add_library(UnixSystemIO STATIC
    d2_unix_system_io.hpp
    d2_unix_system_io.cpp
)

delta_register_module(
    TYPE      "d2::sys::UnixTerminalInput"
    BASE      "d2::sys::SystemInput"
    HEADER    "d2_unix_system_io.hpp"
    TARGET    UnixSystemIO
    PLATFORMS Linux FreeBSD OpenBSD NetBSD DragonFly Darwin
)
delta_register_module(
    TYPE      "d2::sys::UnixTerminalOutput"
    BASE      "d2::sys::SystemOutput"
    HEADER    "d2_unix_system_io.hpp"
    TARGET    UnixSystemIO
    PLATFORMS Linux FreeBSD OpenBSD NetBSD DragonFly Darwin
)
