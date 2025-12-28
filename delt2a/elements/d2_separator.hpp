#ifndef D2_SEPARATOR_HPP
#define D2_SEPARATOR_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(Separator,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                value_type corner_left{ '<' };
                value_type corner_right{ '>' };
                bool override_corners{ false };
                bool hide_body{ false };
            ),
            D2_UAI_PROPS(
                CornerLeft,
                CornerRight,
                OverrideCorners,
                HideBody
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(CornerLeft, corner_left, Style)
                D2_UAI_PROP(CornerRight, corner_right, Style)
                D2_UAI_PROP(OverrideCorners, override_corners, Style)
                D2_UAI_PROP(HideBody, hide_body, Style)
            )
        )
    } // style
    namespace dx
    {
        class Separator :
            public style::UAI<
            Separator,
            style::ILayout,
            style::IColors,
            style::ISeparator
            >
        {
        public:
            friend class TextHelper;
            using data = style::UAI<Separator, style::ILayout, style::IColors, style::ISeparator>;
        protected:
            enum class Align
            {
                Vertical,
                Horizontal
            } _orientation;

            virtual void _frame_impl(PixelBuffer::View buffer) override;
        public:
            Separator(
                const std::string& name,
                TreeState::ptr state,
                pptr parent
            );
        };

        class VerticalSeparator : public Separator
        {
        public:
            VerticalSeparator(
                const std::string& name,
                TreeState::ptr state,
                pptr parent
            );
        };
    } // dx
} // d2

#endif // D2_SEPARATOR_HPP
