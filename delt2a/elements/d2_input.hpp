#ifndef D2_INPUT_HPP
#define D2_INPUT_HPP

#include "../d2_tree_element.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(Input,
            D2_UAI_OPTS(
                enum InputOptions : unsigned char
                {
                    DrawPtr = 1 << 0,
                    Censor = 1 << 1,
                    Blink = 1 << 2,
                    AutoPtrColor = 1 << 3,

                    Auto = DrawPtr | Blink | AutoPtrColor,
                    Protected = Censor | AutoPtrColor | Auto,
                };
            ),
            D2_UAI_FIELDS(
                string pre{};
                string hint{};
                PixelBackground marked_mask{ .r = 170, .g = 170, .b = 170 };
                PixelForeground ptr_color{ .a = 255, .v = '_' };
                unsigned char input_options{ Auto };
            ),
            D2_UAI_PROPS(
                Pre,
                Hint,
                MarkedMask,
                PtrColor,
                InputOptions
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Pre, pre, Masked)
                D2_UAI_PROP(Hint, hint, Masked)
                D2_UAI_PROP(MarkedMask, marked_mask, Masked)
                D2_UAI_PROP(PtrColor, ptr_color, Masked)
                D2_UAI_PROP(InputOptions, input_options, Masked)
            )
        )
    }
    namespace dx
    {
        class Input :
            public style::UAI<
                Input,
                style::ILayout,
                style::IColors,
                style::IText,
                style::IInput,
                style::IResponsive<Input, string>::type
            >
        {
        public:
            using data = style::UAI<
                Input,
                style::ILayout,
                style::IColors,
                style::IText,
                style::IInput,
                style::IResponsive<Input, string>::type
            >;
            using data::data;
        protected:
            IOContext::future<void> btask_{};
            int ptr_{ 0 };
            int bound_lower_{ 0 };
            int highlight_start_{ 0 };
            int highlight_end_{ 0 };
            bool ptr_blink_{ false };

            void _write_ptr(Pixel& px);
            bool _is_highlighted();
            void _remove_highlighted();
            void _begin_highlight_if();
            void _reset_highlight();
            void _reset_ptrs();
            string _highlighted_value();

            void _start_blink();
            void _stop_blink();
            void _freeze_blink();
            void _unfreeze_blink();

            void _move_right(std::size_t cnt = 1);
            void _move_left(std::size_t cnt = 1);

            virtual Unit _layout_impl(Element::Layout type) const override;
            virtual bool _provides_input_impl() const override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(IOContext::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) override;
        public:
            virtual ~Input();
        };
    }
}

#endif // D2_INPUT_HPP
