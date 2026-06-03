add_library(LocalSystemClipboard STATIC
    d2_clipboard.hpp
    d2_clipboard.cpp
)

delta_register_module(
    TYPE   "d2::sys::LocalSystemClipboard"
    BASE   "d2::sys::SystemClipboard"
    HEADER "d2_clipboard.hpp"
    TARGET LocalSystemClipboard
)
