#ifndef D2_SWITCH_HPP
#define D2_SWITCH_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace dx { class Switch; };
    namespace style
    {
        D2_UAI_INTERFACE(Switch,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                // Ptr | new | old
                std::function<void(TypedTreeIter<dx::Switch>, int, int)> on_change{ nullptr };
                std::function<void(TypedTreeIter<dx::Switch>, string, string)> on_change_values{ nullptr };
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
            ),
            D2_UAI_PROPS(
                Options,
                EnabledForegroundColor,
                EnabledBackgroundColor,
                DisabledForegroundColor,
                DisabledBackgroundColor,
                OnChange,
                OnChangeValues,
                SeparatorColor,
                DisableSeparator
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Options, options, Masked)
                D2_UAI_PROP(EnabledForegroundColor, enabled_foreground_color, Style)
                D2_UAI_PROP(EnabledBackgroundColor, enabled_background_color, Style)
                D2_UAI_PROP(DisabledForegroundColor, disabled_foreground_color, Style)
                D2_UAI_PROP(DisabledBackgroundColor, disabled_background_color, Style)
                D2_UAI_PROP(OnChange, on_change, 0x00)
                D2_UAI_PROP(OnChangeValues, on_change_values, 0x00)
                D2_UAI_PROP(SeparatorColor, separator_color, Style)
                D2_UAI_PROP(DisableSeparator, disable_separator, Style)
            )
        )

        D2_UAI_INTERFACE(VerticalSwitch,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                PixelForeground slider_background_color{ .v = charset::slider_vertical };
                Pixel slider_color{ .r = 255, .g = 255, .b = 255, .v = ' ' };
                value_type left_connector{ charset::box_border_connector_left };
                value_type right_connector{ charset::box_border_connector_right };
            ),
            D2_UAI_PROPS(
                SliderBackgroundColor,
                SliderColor,
                LeftConnector,
                RightConnector
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(SliderBackgroundColor, slider_background_color, Style)
                D2_UAI_PROP(SliderColor, slider_color, Style)
                D2_UAI_PROP(LeftConnector, left_connector, Style)
                D2_UAI_PROP(RightConnector, right_connector, Style)
            )
        )

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
            friend class ContainerHelper<Switch>;
            friend class TextHelper<Switch>;
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
            int _idx{ -1 };
            int _old{ -1 };

            virtual Unit _layout_impl(Element::Layout type) const override;
            virtual bool _provides_input_impl() const override;
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(IOContext::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) override;

            void _submit();
        public:
            void reset(int idx = -1);
            void reset(string choice);
            string choice();
            int index();
        };

        class VerticalSwitch :
            public style::UAIC<
                Switch,
                VerticalSwitch,
                style::IVerticalSwitch
            >
        {
        public:
            using data = style::UAIC<
                Switch,
                VerticalSwitch,
                style::IVerticalSwitch
            >;
            D2_UAI_CHAIN(Switch)
        private:
            int _scroll_offset{ 0 };
            int _selector{ -1 };
            bool _scrollbar{ false };
        protected:
            virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(IOContext::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) override;
        public:
            VerticalSwitch(
                const std::string& name,
                TreeState::ptr state,
                pptr parent
            ) : data(name, state, parent)
            {
                data::disable_separator = true;
            }
        };
    }
}

#endif // D2_SWITCH_HPP
