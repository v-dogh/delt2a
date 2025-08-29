#ifndef D2_SEPARATOR_HPP
#define D2_SEPARATOR_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"

namespace d2
{
    namespace style
    {
        struct Separator
        {
            value_type corner_left{ '<' };
            value_type corner_right{ '>' };
            bool override_corners{ false };
            bool hide_body{ false };

            template<uai_property Property>
            auto at_style()
            {
                D2_UAI_SETUP(Style)
                D2_UAI_VAR_START
                D2_UAI_GET_VAR_A(0, corner_left)
                D2_UAI_GET_VAR_A(1, corner_right)
                D2_UAI_GET_VAR_A(2, override_corners)
                D2_UAI_GET_VAR_A(3, hide_body)
                D2_UAI_VAR_END;
            }
        };

        template<std::size_t PropBase>
        struct ISeparator : Separator, InterfaceHelper<ISeparator, PropBase, 4>
        {
            using data = Separator;
            enum Property : uai_property
            {
                CornerLeft = PropBase,
                CornerRight,
                OverrideCorners,
                HideBody,
            };
        };
        using IZSeparator = ISeparator<0>;
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

            virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
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
