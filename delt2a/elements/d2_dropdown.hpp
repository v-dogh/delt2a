#ifndef D2_DROPDOWN_HPP
#define D2_DROPDOWN_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace style
    {
        struct Dropdown
        {
            string default_value{};

            PixelForeground selected_foreground_color{};
            PixelBackground selected_background_color{};

            PixelForeground option_foreground_color{};
            PixelBackground option_background_color{};

            bool show_choice{ true };

            std::vector<string> options{};
            std::function<void(TreeIter, const string&, std::size_t)> on_submit{ nullptr };

            template<uai_property Property>
            auto at_style()
            {
                D2_UAI_SETUP(Style)
                D2_UAI_VAR_START
                D2_UAI_GET_VAR_A(0, default_value)
                D2_UAI_GET_VAR_A(1, selected_foreground_color)
                D2_UAI_GET_VAR_A(2, selected_background_color)
                D2_UAI_GET_VAR_A(3, option_foreground_color)
                D2_UAI_GET_VAR_A(4, option_background_color)
                D2_UAI_GET_VAR_A(5, show_choice)
                D2_UAI_GET_VAR_A(6, options)
                D2_UAI_GET_VAR(7, on_submit, None)
                D2_UAI_VAR_END;
            }
        };

        template<std::size_t PropBase>
        struct IDropdown : Dropdown, InterfaceHelper<IDropdown, PropBase, 8>
        {
            using data = Dropdown;
            enum Property : uai_property
            {
                DefaultValue = PropBase,
                SelectedForeground,
                SelectedBackground,
                OptionForeground,
                OptionBackground,
                ShowChoice,
                Options,
                OnSubmit
            };
        };
        using IZDropdown = IDropdown<0>;
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
            style::IKeyboardNav
            >,
            public impl::TextHelper<Dropdown>,
            public impl::ContainerHelper<Dropdown>
        {
        public:
            friend class ContainerHelper;
            friend class TextHelper;
            using data = style::UAI<
                         Dropdown,
                         style::ILayout,
                         style::IColors,
                         style::IContainer,
                         style::IDropdown,
                         style::IKeyboardNav
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

            virtual Unit _layout_impl(Element::Layout type) const noexcept override;
            virtual bool _provides_input_impl() const noexcept override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(IOContext::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) noexcept override;
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
