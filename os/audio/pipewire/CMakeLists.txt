find_package(PkgConfig QUIET)
if(NOT PkgConfig_FOUND)
    return()
endif()
pkg_check_modules(PIPEWIRE QUIET IMPORTED_TARGET libpipewire-0.3)
if(NOT PIPEWIRE_FOUND)
    return()
endif()

add_library(PipewireSystemAudio STATIC
    d2_pipewire_audio.hpp
    d2_pipewire_audio.cpp
)

target_include_directories(PipewireSystemAudio PUBLIC ${PIPEWIRE_INCLUDE_DIRS})
target_compile_options    (PipewireSystemAudio PUBLIC ${PIPEWIRE_CFLAGS_OTHER})
target_link_libraries     (PipewireSystemAudio PUBLIC ${PIPEWIRE_LIBRARIES})

delta_register_module(
    TYPE      "d2::sys::PipewireSystemAudio"
    BASE      "d2::sys::SystemAudio"
    HEADER    "d2_pipewire_audio.hpp"
    TARGET    PipewireSystemAudio
    PLATFORMS Linux FreeBSD OpenBSD
)
