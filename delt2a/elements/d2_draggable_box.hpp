#ifndef D2_DRAGGABLE_BOX_HPP
#define D2_DRAGGABLE_BOX_HPP

#include "d2_box.hpp"

namespace d2
{
    namespace style
    {
        struct DraggableBox
        {
            enum VBoxOptions : unsigned char
            {
                Resizable = 1 << 0,
                Draggable = 1 << 1,
                Minimizable = 1 << 2,

                Auto = Resizable | Draggable | Minimizable,
                Freezed = 0x00,
                Static = Draggable,
            };

            string title{};
            PixelForeground bar_color{ .v = charset::box_top_bar };
            Pixel::value_type resize_point_value{ charset::box_top_drag_spot };
            VDUnit bar_height{ 2.0_px };
            unsigned char vbox_options{ Auto };

            template<uai_property Property>
            auto at_style()
            {
                D2_UAI_SETUP(Style)
                D2_UAI_VAR_START
                D2_UAI_GET_VAR_A(0, title)
                D2_UAI_GET_VAR_A(1, bar_color)
                D2_UAI_GET_VAR_MUL(2, bar_height, Element::WriteType::LayoutHeight)
                D2_UAI_GET_VAR_A(3, bar_height)
                D2_UAI_GET_VAR_A(4, vbox_options)
                D2_UAI_VAR_END;
            }
        };

        template<std::size_t PropBase>
        struct IDraggableBox : DraggableBox, InterfaceHelper<IDraggableBox, PropBase, 5>
        {
            using data = DraggableBox;
            enum Property : uai_property
            {
                Title = PropBase,
                BarColor,
                BarHeight,
                ResizePointValue,
                VBoxOptions
            };
        };
    }
    namespace dx
    {
        class VirtualBox : public style::UAIC<Box, VirtualBox, style::IDraggableBox, style::IKeyboardNav>
        {
        public:
            using data = style::UAIC<Box, VirtualBox, style::IDraggableBox, style::IKeyboardNav>;
            using data::data;
            D2_UAI_CHAIN(Box)
        protected:
            std::pair<int, int> offset_{ 0, 0 };
            bool minimized_{ false };

            bool _is_being_resized() const noexcept;

            virtual Unit _layout_impl(enum Element::Layout type) const noexcept override;
            virtual bool _provides_input_impl() const noexcept override;
            virtual int _get_border_impl(BorderType type, Element::cptr elem) const noexcept override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(IOContext::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
        public:
            void close() noexcept;
        };
    }
}

#endif // D2_DRAGGABLE_BOX_HPP
