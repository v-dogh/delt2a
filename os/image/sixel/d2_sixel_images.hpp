#pragma once

#include <core/mods/d2_images.hpp>

namespace d2
{
    namespace sys
    {
        class SystemImagesSixel : public SystemImages, public ModuleImpl<SystemImagesSixel, Load::Lazy>
        {
        protected:
            bool _repaint_on_change() const override { return true; }
            std::string _encode(const image::Composite& image, std::span<const pixel> cells, int columns) override;
            std::string _erase() const override;
        public:
            using SystemImages::SystemImages;
        };

        namespace img 
        {
            using sixel = SystemImagesSixel;
        }
    }

    namespace image
    {
        std::string encode_sixel(const Composite& image, std::span<const pixel> cells, int columns);
    }
}
