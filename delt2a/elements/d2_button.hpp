#ifndef D2_BUTTON_HPP
#define D2_BUTTON_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2::dx
{
    class Button :
        public style::UAI<
            Button,
            style::ILayout,
            style::IContainer,
            style::IColors,
            style::IResponsive<Button>::type,
            style::IText,
            style::IKeyboardNav
        >,
        public impl::TextHelper<Button>,
        public impl::ContainerHelper<Button>
    {
    public:
        friend class ContainerHelper<Button>;
        friend class TextHelper<Button>;
        using data = style::UAI<
            Button,
            style::ILayout,
            style::IContainer,
            style::IColors,
            style::IResponsive<Button>::type,
            style::IText,
            style::IKeyboardNav
        >;
        using data::data;
    protected:
        BoundingBox _text_dimensions{};

        virtual bool _provides_input_impl() const override;
        virtual Unit _layout_impl(enum Element::Layout type) const override;

        virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
        virtual void _state_change_impl(State state, bool value) override;
        virtual void _event_impl(sys::screen::Event ev) override;

        virtual void _frame_impl(PixelBuffer::View buffer) override;
    };
} // d2::elem

#endif // D2_BUTTON_HPP
