#pragma once

#include <core/tree/d2_tree_element.hpp>
#include <core/tree/d2_styles.hpp>
#include <elements/d2_element_utils.hpp>

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
            virtual void _frame_impl(PixelBuffer::View buffer) override;
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
        };
    }
}

