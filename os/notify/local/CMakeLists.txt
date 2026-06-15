add_library(LocalSystemNotify STATIC
    d2_notify.hpp
    d2_notify.cpp
)

delta_register_module(
    TYPE   "d2::sys::LocalSystemNotify"
    BASE   "d2::sys::SystemNotify"
    HEADER "d2_notify.hpp"
    TARGET LocalSystemNotify
)
