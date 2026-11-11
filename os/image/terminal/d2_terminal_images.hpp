#pragma once

#include <core/mods/d2_images.hpp>

namespace d2::image
{
    std::string encode_kitty(const Composite& image, std::uint32_t id);
    std::string encode_sixel(const Composite& image, std::span<const pixel> cells, int columns);
}
namespace d2::sys
{
    class TerminalImages : public SystemImages, public ModuleImpl<TerminalImages, Load::Immediate>
    {
    private:
        Config _config{};
        Protocol _previous{Protocol::Off};
        image::Scene _scene{};
        image::Composite _composite{};
        BoundingBox _viewport{};
        std::vector<pixel> _cells{};
        std::uint32_t _id{0};
        bool _invalid{true}, _placed{false};
    protected:
        Status _load_impl() override { return Status::Ok; }
        Status _unload_impl() override;
    public:
        using SystemImages::SystemImages;
        void configure(Config config) override;
        Config config() const override { return _config; }
        Plan prepare(std::span<pixel> cells, BoundingBox viewport,
                     const image::Scene& scene, BoundingBox cell_pixels) override;
        Plan reset() override;
        void invalidate() noexcept override { _invalid = true; }
        image::Raster::ptr decode_png(std::span<const unsigned char> bytes) const override;
        image::Raster::ptr load_png(const std::string& path) const override;
    };
}
