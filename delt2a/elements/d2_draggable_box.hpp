#ifndef D2_DRAGGABLE_BOX_HPP
#define D2_DRAGGABLE_BOX_HPP

#include "d2_box.hpp"

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(DraggableBox,
            D2_UAI_OPTS(
                enum VBoxOptions : unsigned char
                {
                    Resizable = 1 << 0,
                    Draggable = 1 << 1,
                    Minimizable = 1 << 2,

                    Auto = Resizable | Draggable | Minimizable,
                    Freezed = 0x00,
                    Static = Draggable,
                };
            ),
            D2_UAI_FIELDS(
                string title{};
                PixelForeground bar_color{ .v = charset::box_top_bar };
                Pixel::value_type resize_point_value{ charset::box_top_drag_spot };
                VDUnit bar_height{ 2.0_px };
                unsigned char vbox_options{ Auto };
            ),
            D2_UAI_PROPS(
                Title,
                BarColor,
                BarHeight,
                ResizePointValue,
                VBoxOptions
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Title, title, Style)
                D2_UAI_PROP(BarColor, bar_color, Style)
                D2_UAI_PROP(BarHeight, bar_height, Style | LayoutHeight)
                D2_UAI_PROP(ResizePointValue, resize_point_value, Style)
                D2_UAI_PROP(VBoxOptions, vbox_options, Style)
            )
        )
    }
    namespace dx
    {
        class VirtualBox : public style::UAIC<Box, VirtualBox, style::IDraggableBox, style::IKeyboardNav>
        {
        public:
            using data = style::UAIC<Box, VirtualBox, style::IDraggableBox, style::IKeyboardNav>;
            D2_UAI_CHAIN(Box)
        protected:
            std::pair<int, int> offset_{ 0, 0 };
            bool minimized_{ false };

            bool _is_being_resized() const;

            virtual Unit _layout_impl(enum Element::Layout type) const override;
            virtual bool _provides_input_impl() const override;
            virtual int _get_border_impl(BorderType type, Element::cptr elem) const override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(IOContext::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) override;
        public:
            void close();
        };
    }
}

#endif // D2_DRAGGABLE_BOX_HPP
