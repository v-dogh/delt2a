#pragma once

#include <core/mods/d2_images.hpp>

namespace d2
{
    namespace sys 
    {
        class SystemImagesKitty : public SystemImages, public ModuleImpl<SystemImagesKitty, Load::Lazy>
        {
        private:
            std::uint32_t _id{0};
        protected:
            bool _repaint_on_change() const override { return false; }
            BoundingBox _fallback_metrics() const override { return {8, 16}; }
            std::string _encode(const image::Composite& image, std::span<const pixel> cells, int columns) override;
            std::string _erase() const override;
        public:
            using SystemImages::SystemImages;
        };

        namespace img 
        {
            using kitty = SystemImagesKitty;
        }
    }
    namespace image
    {
        std::string encode_kitty(const Composite& image, std::uint32_t id);
    }
}