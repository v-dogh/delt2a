#pragma once

#include <core/mods/d2_core.hpp>
#include <string>

namespace d2::sys
{
    class SystemImages : public ModuleDecl<SystemImages, "Images", Access::TUnsafe, SystemOutput>
    {
    public:
        enum class Protocol { Off, Kitty, Sixel };
        struct Config
        {
            Protocol protocol{Protocol::Off};
            BoundingBox cell_pixels{0, 0};
        };
        struct Plan
        {
            std::string before, after;
            bool repaint{false};
        };
        using ModuleDecl::ModuleDecl;
        virtual void configure(Config config) = 0;
        virtual Config config() const = 0;
        virtual Plan prepare(std::span<pixel> cells, BoundingBox viewport,
                             const image::Scene& scene, BoundingBox cell_pixels) = 0;
        virtual Plan reset() = 0;
        virtual void invalidate() noexcept = 0;
        virtual image::Raster::ptr decode_png(std::span<const unsigned char> bytes) const = 0;
        virtual image::Raster::ptr load_png(const std::string& path) const = 0;
    };
    using images = SystemImages;
}
