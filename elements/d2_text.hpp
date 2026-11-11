#pragma once

#include <core/tree/d2_styles.hpp>
#include <core/tree/d2_tree_element.hpp>
#include <elements/d2_element_utils.hpp>

namespace d2::dx
{
    class Text : public style::UAI<Text, style::ILayout, style::IText, style::IColors, style::ISelectable>,
                 public impl::TextHelper<Text>
    {
    public:
        friend class UnitUpdateHelper;
        friend class TextHelper;
        using data = style::UAI<Text, style::ILayout, style::IText, style::IColors, style::ISelectable>;
    protected:
        mutable text::Layout _text_layout{};
        mutable text::Layout _measurement{};
        text::Options _options(int width) const;

        virtual Unit _layout_impl(Element::Layout type) const override;
        virtual void _frame_impl(PixelBuffer::View buffer) override;
        virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
    public:
        Text(const std::string& name, TreeState::ptr state);
        const text::Layout* text_layout() const override;
        void select_text(std::optional<text::Range> range) override;
    };
} // namespace d2::dx
