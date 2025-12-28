#ifndef D2_IMAGE_HPP
#define D2_IMAGE_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(Image,
            D2_UAI_OPTS(
                enum ImageOptions : unsigned char
                {

                };
            ),
            D2_UAI_FIELDS(
                std::string path;
                unsigned char image_options{ 0x00 };
            ),
            D2_UAI_PROPS(
                Path,
                ImageOptions
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Path, path, Style)
                D2_UAI_PROP(ImageOptions, image_options, Style)
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

#endif // D2_IMAGE_HPP
