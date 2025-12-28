#ifndef D2_TEXT_HPP
#define D2_TEXT_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2::dx
{
    class Text :
        public style::UAI<Text, style::ILayout, style::IText, style::IColors>,
        public impl::TextHelper<Text>
    {
    public:
        friend class UnitUpdateHelper;
        friend class TextHelper;
        using data = style::UAI<Text, style::ILayout, style::IText, style::IColors>;
    protected:
        BoundingBox _text_dimensions{};

        virtual Unit _layout_impl(Element::Layout type) const override;
        virtual void _frame_impl(PixelBuffer::View buffer) override;
        virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
    public:
        Text(
            const std::string& name,
            TreeState::ptr state,
            pptr parent
        );
    };
} // d2::elem

#endif // D2_TEXT_HPP
