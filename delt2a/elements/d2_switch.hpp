#ifndef D2_SWITCH_HPP
#define D2_SWITCH_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace style
    {
        struct Switch
        {
            // Ptr | new | old
            std::function<void(TreeIter, int, int)> on_change{ nullptr };
            std::function<void(TreeIter, string, string)> on_change_values{ nullptr };
            std::vector<string> options{};

            PixelBackground disabled_background_color{};
            PixelForeground disabled_foreground_color{};
            PixelBackground enabled_background_color
            {
                .r = 255, .g = 255, .b = 255
            };
            PixelForeground enabled_foreground_color
            {
                .r = 0, .g = 0, .b = 0
            };
            Pixel separator_color{ .a = 0, .v = '|' };
            bool disable_separator{ false };

            template<uai_property Property>
            auto at_style()
            {
                D2_UAI_SETUP_EMPTY
                D2_UAI_VAR_START
                D2_UAI_GET_VAR(0, options, Masked)
                D2_UAI_GET_VAR(1, enabled_foreground_color, Style)
                D2_UAI_GET_VAR(2, enabled_background_color, Style)
                D2_UAI_GET_VAR(3, disabled_foreground_color, Style)
                D2_UAI_GET_VAR(4, disabled_background_color, Style)
                D2_UAI_GET_VAR_A(5, on_change)
                D2_UAI_GET_VAR_A(6, on_change_values)
                D2_UAI_GET_VAR(7, separator_color, Style)
                D2_UAI_GET_VAR(8, disable_separator, Style)
                D2_UAI_VAR_END;
            }
        };

        template<std::size_t PropBase>
        struct ISwitch : Switch, InterfaceHelper<ISwitch, PropBase, 9>
        {
            using data = Switch;
            enum Property : uai_property
            {
                Options = PropBase,
                EnabledForegroundColor,
                EnabledBackgroundColor,
                DisabledForegroundColor,
                DisabledBackgroundColor,
                OnChange,
                OnChangeValues,
                SeparatorColor,
                DisableSeparator
            };
        };
        using IZSwitch = ISwitch<0>;
    }
    namespace dx
    {
        class Switch :
            public style::UAI<
            Switch,
            style::ISwitch,
            style::ILayout,
            style::IContainer,
            style::IColors,
            style::IKeyboardNav
            >,
            public impl::ContainerHelper<Switch>,
            public impl::TextHelper<Switch>
        {
        public:
            friend class ContainerHelper;
            friend class TextHelper;
            using opts = std::vector<string>;
            using data = style::UAI<
                         Switch,
                         style::ISwitch,
                         style::ILayout,
                         style::IContainer,
                         style::IColors,
                         style::IKeyboardNav
                         >;
            using data::data;
        protected:
            BoundingBox _perfect_dimensions{};
            int _idx{ 0 };
            int _old{ 0 };

            virtual Unit _layout_impl(Element::Layout type) const noexcept override;
            virtual bool _provides_input_impl() const noexcept override;
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) noexcept override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(IOContext::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;

            void _submit();
        public:
            void reset(int idx) noexcept;
            void reset(string choice) noexcept;
            string choice() noexcept;
            int index() noexcept;
        };

        class VerticalSwitch : public Switch
        {
        protected:
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) noexcept override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
        public:
            VerticalSwitch(
                const std::string& name,
                TreeState::ptr state,
                pptr parent
            ) : Switch(name, state, parent)
            {
                data::disable_separator = true;
            }
        };
    }
}

#endif // D2_SWITCH_HPP
