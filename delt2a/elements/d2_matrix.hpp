#ifndef D2_MATRIX_HPP
#define D2_MATRIX_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "../d2_model.hpp"

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(Model,
            D2_UAI_OPTS(
                enum MaskMode
                {
                    ApplyFg = 1 << 0,
                    ApplyBg = 1 << 1,
                    InterpFg = 1 << 2,
                    InterpBg = 1 << 3,
                };
            ),
            D2_UAI_FIELDS(
                MatrixModel::ptr model{ nullptr };
                PixelBackground background_mask{};
                PixelForeground foreground_mask{};
                unsigned char mask_options{ 0x00 };
            ),
            D2_UAI_PROPS(
                Model,
                BackgroundMask,
                ForegroundMask,
                MaskOptions
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Model, model, Masked)
                D2_UAI_PROP(BackgroundMask, background_mask, Masked)
                D2_UAI_PROP(ForegroundMask, foreground_mask, Masked)
                D2_UAI_PROP(MaskOptions, mask_options, Masked)
            )
        )
    }
    namespace dx
    {
        class Matrix : public style::UAI<Matrix, style::ILayout, style::IModel>
        {
        public:
            friend class UnitUpdateHelper;
            using data = style::UAI<Matrix, style::ILayout, style::IModel>;
            using data::data;
        protected:
            virtual Unit _layout_impl(enum Element::Layout type) const override;
            virtual BoundingBox _reserve_buffer_impl() const override;
            virtual PixelBuffer::View _fetch_pixel_buffer_impl() const override;
            virtual bool _provides_buffer_impl() const override;

            virtual void _frame_impl(PixelBuffer::View buffer) override;
        };
    }
}

#endif // D2_MATRIX_HPP
