#ifndef D2_CHECKBOX_HPP
#define D2_CHECKBOX_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "../d2_colors.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(Checkbox,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                string value_on{ charset::checkbox_true };
                string value_off{ charset::checkbox_false };
                PixelForeground color_on_foreground{};
                PixelBackground color_on_background{ d2::colors::w::transparent };
                PixelForeground color_off_foreground{};
                PixelBackground color_off_background{ d2::colors::w::transparent };
                bool checkbox_value{ false };
            ),
            D2_UAI_PROPS(
                ValueOn,
                ValueOff,
                ColorOnForeground,
                ColorOnBackground,
                ColorOffForeground,
                ColorOffBackground,
                Value
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(ValueOn, value_on, Masked)
                D2_UAI_PROP(ValueOff, value_off, Masked)
                D2_UAI_PROP(ColorOnForeground, color_on_foreground, Style)
                D2_UAI_PROP(ColorOnBackground, color_on_background, Style)
                D2_UAI_PROP(ColorOffForeground, color_off_foreground, Style)
                D2_UAI_PROP(ColorOffBackground, color_off_background, Style)
                D2_UAI_PROP(Value, checkbox_value, Masked)
            )
        )
    }
    namespace dx
    {
        class Checkbox :
            public style::UAI<
                Checkbox,
                style::ILayout,
                style::IColors,
                style::ICheckbox,
                style::IKeyboardNav,
                style::IResponsive<Checkbox, bool>::type
            >,
            public impl::TextHelper<Checkbox>
        {
        public:
            friend class TextHelper;
            using data = style::UAI<
                Checkbox,
                style::ILayout,
                style::IColors,
                style::ICheckbox,
                style::IKeyboardNav,
                style::IResponsive<Checkbox, bool>::type
            >;
            using data::data;
        protected:
            BoundingBox _text_dimensions{};

            void _submit();

            virtual Unit _layout_impl(Element::Layout type) const override;
            virtual bool _provides_input_impl() const override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(sys::screen::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) override;
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
        };
    }
}

#endif // D2_CHECKBOX_HPP
