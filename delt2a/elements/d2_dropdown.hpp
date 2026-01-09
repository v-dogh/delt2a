#ifndef D2_DROPDOWN_HPP
#define D2_DROPDOWN_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(Dropdown,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                string default_value{};

                PixelForeground selected_foreground_color{};
                PixelBackground selected_background_color{};

                PixelForeground option_foreground_color{};
                PixelBackground option_background_color{};

                bool show_choice{ true };

                std::vector<string> options{};
            ),
            D2_UAI_PROPS(
                DefaultValue,
                SelectedForeground,
                SelectedBackground,
                OptionForeground,
                OptionBackground,
                ShowChoice,
                Options
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(DefaultValue, default_value, Style)
                D2_UAI_PROP(SelectedForeground, selected_foreground_color, Style)
                D2_UAI_PROP(SelectedBackground, selected_background_color, Style)
                D2_UAI_PROP(OptionForeground, option_foreground_color, Style)
                D2_UAI_PROP(OptionBackground, option_background_color, Style)
                D2_UAI_PROP(ShowChoice, show_choice, Style)
                D2_UAI_PROP(Options, options, Style)
            )
        )
    }
    namespace dx
    {
        class Dropdown :
            public style::UAI<
                Dropdown,
                style::ILayout,
                style::IColors,
                style::IContainer,
                style::IDropdown,
                style::IKeyboardNav,
                style::IResponsive<Dropdown, string, std::size_t>::type
            >,
            public impl::TextHelper<Dropdown>,
            public impl::ContainerHelper<Dropdown>
        {
        public:
            friend class ContainerHelper<Dropdown>;
            friend class TextHelper<Dropdown>;
            using data = style::UAI<
                Dropdown,
                style::ILayout,
                style::IColors,
                style::IContainer,
                style::IDropdown,
                style::IKeyboardNav,
                style::IResponsive<Dropdown, string, std::size_t>::type
            >;
            using data::data;
            using opts = std::vector<string>;
        protected:
            BoundingBox _perfect_dimensions{};
            mutable int _cell_height{ 0 };
            int _soft_selected{ -1 };
            int _selected{ -1 };
            bool _opened{ false };

            int _border_width();

            virtual Unit _layout_impl(Element::Layout type) const override;
            virtual bool _provides_input_impl() const override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(Screen::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) override;
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
        public:
            void choose(int idx);
            void reset();
            void reset(const string& value);
            int index();
            string value();
        };
    }
}

#endif // D2_DROPDOWN_HPP
