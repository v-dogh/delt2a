#pragma once

#include <core/mods/d2_core.hpp>
#include <string>

namespace d2::sys
{
    class SystemImages : public ModuleDecl<SystemImages, "Images", Access::TUnsafe, SystemOutput>
    {
    public:
        struct Plan
        {
            std::string before, after;
            bool repaint{false};
        };
    private:
        BoundingBox _cell_pixels{};
        image::Scene _scene{};
        image::Composite _composite{};
        BoundingBox _viewport{};
        std::vector<pixel> _cells{};
        bool _invalid{true};
        bool _placed{false};
    protected:
        Status _load_impl() override;
        Status _unload_impl() override;
        virtual bool _repaint_on_change() const = 0;
        virtual BoundingBox _fallback_metrics() const { return {}; }
        virtual std::string _encode(const image::Composite& image, std::span<const pixel> cells, int columns) = 0;
        virtual std::string _erase() const = 0;
    public:
        using ModuleDecl::ModuleDecl;
        void configure(BoundingBox cell_pixels);
        Plan prepare(std::span<pixel> cells, BoundingBox viewport,
                     const image::Scene& scene, BoundingBox cell_pixels);
        Plan reset();
        void invalidate() noexcept { _invalid = true; }
    };
    using images = SystemImages;
}
