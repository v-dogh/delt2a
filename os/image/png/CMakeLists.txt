option(D2_IMAGE_PNG "Build the libpng image loader when available" ON)
if (NOT D2_IMAGE_PNG)
    return()
endif()
find_package(PNG QUIET)
if (NOT PNG_FOUND)
    message(STATUS "libpng not found: PNG loader omitted")
    return()
endif()
add_library(PngLoader STATIC d2_png_loader.hpp d2_png_loader.cpp)
add_library(DeltaAscii::PngLoader ALIAS PngLoader)
set_target_properties(PngLoader PROPERTIES EXPORT_NAME PngLoader)
target_link_libraries(PngLoader PUBLIC PNG::PNG)
set_property(GLOBAL PROPERTY DELTA_IMAGE_PNG TRUE)
delta_register_module(
    TYPE "d2::sys::SystemPngLoader"
    BASE "d2::sys::SystemImageLoader"
    HEADER "d2_png_loader.hpp"
    TARGET PngLoader
)
