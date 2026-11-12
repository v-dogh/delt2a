#include "d2_png_loader.hpp"
#include <png.h>

namespace d2::sys
{
    image::Raster::ptr SystemPngLoader::decode(std::span<const unsigned char> bytes, image::LoadOptions options) const
    {
        options.validate();
        if (bytes.empty() || bytes.size() > options.max_bytes)
            throw std::invalid_argument("PNG input byte limit");
        png_image png{};
        png.version = PNG_IMAGE_VERSION;
        struct Cleanup { png_image& png; ~Cleanup() { png_image_free(&png); } } cleanup{png};
        if (!png_image_begin_read_from_memory(&png, bytes.data(), bytes.size()))
            throw std::invalid_argument(std::string("PNG header: ") + png.message);
        if (!png.width || !png.height || png.width > options.max_pixels / png.height)
            throw std::length_error("PNG exceeds image pixel limit");
        png.format = PNG_FORMAT_RGBA;
        std::vector<image::RGBA> pixels(std::size_t(png.width) * png.height);
        if (!png_image_finish_read(&png, nullptr, pixels.data(), 0, nullptr))
            throw std::invalid_argument(std::string("PNG decode: ") + png.message);
        return image::Raster::rgba(int(png.width), int(png.height), std::move(pixels));
    }
}
