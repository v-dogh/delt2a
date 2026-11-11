add_library(TerminalImages STATIC d2_terminal_images.hpp d2_terminal_images.cpp)
option(D2_IMAGE_PNG "Enable PNG decoding through libpng when available" ON)
if (D2_IMAGE_PNG)
    find_package(PNG QUIET)
    if (PNG_FOUND)
        target_link_libraries(TerminalImages PUBLIC PNG::PNG)
        target_compile_definitions(TerminalImages PUBLIC D2_IMAGE_PNG)
        set_property(GLOBAL PROPERTY DELTA_IMAGE_PNG TRUE)
    else()
        message(STATUS "libpng not found: terminal images accept RGB/RGBA sources; PNG decoding is disabled")
    endif()
endif()
delta_register_module(
    TYPE "d2::sys::TerminalImages"
    BASE "d2::sys::SystemImages"
    HEADER "d2_terminal_images.hpp"
    TARGET TerminalImages
    PLATFORMS Linux FreeBSD OpenBSD NetBSD DragonFly Darwin
)
