#pragma once

#include <core/tree/d2_tree_element.hpp>
#include <core/tree/d2_styles.hpp>
#include <elements/d2_element_utils.hpp>
#include <core/mods/d2_image_loader.hpp>

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(Image,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                image::Raster::ptr source{};
                image::Rect source_rect{};
            ),
            D2_UAI_PROPS(
                Source,
                SourceRect
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Source, source, Style)
                D2_UAI_PROP(SourceRect, source_rect, Style)
            )
        )
    }
    namespace dx
    {
        class Image :
            public style::UAI<
                Image,
                style::ILayout,
                style::IContainer,
                style::IColors,
                style::IImage
            >,
            public impl::ContainerHelper<Image>
        {
        private:
            std::shared_ptr<const char> _load_ticket;
            static mt::co_future<bool> _finish_load(std::weak_ptr<Image> target, IOContext::wptr context,
                std::shared_ptr<const char> ticket, mt::future<image::Raster::ptr> pending);
            virtual void _frame_impl(PixelBuffer::View buffer) override;
            void _signal_base_impl(write_flag type, style::uai_property prop) override;
        public:
            friend class ContainerHelper<Image>;
            using data = style::UAI<
                Image,
                style::ILayout,
                style::IContainer,
                style::IColors,
                style::IImage
            >;
            using data::data;
            mt::co_future<bool> load(std::filesystem::path path, image::LoadOptions options = {},
                                     const std::string& loader = {});
            void cancel_load();
        };
    }
}

