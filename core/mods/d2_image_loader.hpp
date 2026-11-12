#pragma once

#include <core/io/d2_module.hpp>
#include <core/types/d2_image.hpp>
#include <mt/future.hpp>
#include <filesystem>

namespace d2::image
{
    struct LoadOptions
    {
        std::size_t max_bytes{64 * 1024 * 1024};
        std::size_t max_pixels{Raster::max_pixels};
        void validate() const;
    };
}
namespace d2::sys
{
    class SystemImageLoader : public ModuleDecl<SystemImageLoader, "ImageLoader", Access::TSafe>
    {
    public:
        using ModuleDecl::ModuleDecl;
        virtual image::Raster::ptr decode(std::span<const unsigned char> bytes, image::LoadOptions options = {}) const = 0;
        virtual image::Raster::ptr load(const std::filesystem::path& path, image::LoadOptions options = {}) const;
        mt::future<image::Raster::ptr> load_async(std::filesystem::path path, image::LoadOptions options = {});
    };
    using image_loader = SystemImageLoader;
}
