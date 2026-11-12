add_library(ImagesSixel STATIC d2_sixel_images.hpp d2_sixel_images.cpp)
add_library(DeltaAscii::ImagesSixel ALIAS ImagesSixel)
set_target_properties(ImagesSixel PROPERTIES EXPORT_NAME ImagesSixel)
delta_register_module(
    TYPE "d2::sys::SystemImagesSixel"
    BASE "d2::sys::SystemImages"
    HEADER "d2_sixel_images.hpp"
    TARGET ImagesSixel
    PLATFORMS Linux FreeBSD OpenBSD NetBSD DragonFly Darwin
)
