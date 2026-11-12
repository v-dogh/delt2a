add_library(ImagesKitty STATIC d2_kitty_images.hpp d2_kitty_images.cpp)
add_library(DeltaAscii::ImagesKitty ALIAS ImagesKitty)
set_target_properties(ImagesKitty PROPERTIES EXPORT_NAME ImagesKitty)
delta_register_module(
    TYPE "d2::sys::SystemImagesKitty"
    BASE "d2::sys::SystemImages"
    HEADER "d2_kitty_images.hpp"
    TARGET ImagesKitty
    PLATFORMS Linux FreeBSD OpenBSD NetBSD DragonFly Darwin
)
