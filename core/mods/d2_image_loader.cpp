#include "d2_image_loader.hpp"
#include <core/io/d2_context.hpp>
#include <fstream>
#include <limits>

namespace d2::image
{
    void LoadOptions::validate() const
    {
        if (!max_bytes || max_bytes > std::size_t(std::numeric_limits<std::streamsize>::max()) ||
            !max_pixels || max_pixels > Raster::max_pixels)
            throw std::invalid_argument("Invalid image load limits");
    }
}
namespace d2::sys
{
    image::Raster::ptr SystemImageLoader::load(const std::filesystem::path& path, image::LoadOptions options) const
    {
        options.validate();
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) throw std::runtime_error("Cannot open image file: " + path.string());
        const auto size = file.tellg();
        if (size < 0 || static_cast<std::uintmax_t>(size) > options.max_bytes)
            throw std::length_error("Image input exceeds byte limit");
        std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
        file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size)))
            throw std::runtime_error("Cannot read image file: " + path.string());
        return decode(bytes, options);
    }
    mt::future<image::Raster::ptr> SystemImageLoader::load_async(std::filesystem::path path, image::LoadOptions options)
    {
        options.validate();
        auto ctx = context();
        if (!ctx) throw std::logic_error("Image loader has no IO context");
        return ctx->scheduler()->launch([self = ptr().ptr(), path = std::move(path), options]
        {
            return self->load(path, options);
        });
    }
}
