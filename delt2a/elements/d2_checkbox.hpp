#ifndef D2_CHECKBOX_HPP
#define D2_CHECKBOX_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
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
                Pixel color_on{};
                Pixel color_off{};
                bool checkbox_value{ false };
            ),
            D2_UAI_PROPS(
                ValueOn,
                ValueOff,
                ColorOn,
                ColorOff,
                Value
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(ValueOn, value_on, Masked)
                D2_UAI_PROP(ValueOff, value_off, Masked)
                D2_UAI_PROP(ColorOn, color_on, Style)
                D2_UAI_PROP(ColorOff, color_off, Style)
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
            virtual void _event_impl(Screen::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) override;
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
        };
    }
}

#endif // D2_CHECKBOX_HPP
