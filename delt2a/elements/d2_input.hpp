#ifndef D2_INPUT_HPP
#define D2_INPUT_HPP

#include "../d2_tree_element.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace style
    {
        struct Input
        {
            enum InputOptions : unsigned char
            {
                DrawPtr = 1 << 0,
                Censor = 1 << 1,
                Blink = 1 << 2,
                AutoPtrColor = 1 << 3,

                Auto = DrawPtr | Blink | AutoPtrColor,
                Protected = Censor | AutoPtrColor | Auto,
            };

            string pre{};
            PixelBackground marked_mask{ .r = 170, .g = 170, .b = 170 };
            PixelForeground ptr_color{ .a = 255, .v = '_' };
            unsigned char input_options{ Auto };
            std::function<void(TreeIter, string)> on_submit{ nullptr };

            template<uai_property Property>
            auto at_style()
            {
                D2_UAI_SETUP(Masked)
                D2_UAI_VAR_START
                D2_UAI_GET_VAR_A(0, pre)
                D2_UAI_GET_VAR_A(1, marked_mask)
                D2_UAI_GET_VAR_A(2, ptr_color)
                D2_UAI_GET_VAR_A(3, input_options)
                D2_UAI_GET_VAR(4, on_submit, None)
                D2_UAI_VAR_END;
            }
        };

        template<std::size_t PropBase>
        struct IInput : Input, InterfaceHelper<IInput, PropBase, 5>
        {
            using data = Input;
            enum Property : uai_property
            {
                Pre = PropBase,
                MarkedMask,
                PtrColor,
                InputOptions,
                OnSubmit
            };
        };
        using IZInput = IInput<0>;
    }
    namespace dx
    {
        class Input :
            public style::UAI<
            Input,
            style::ILayout,
            style::IColors,
            style::IText,
            style::IInput
            >
        {
        public:
            using data = style::UAI<
                         Input,
                         style::ILayout,
                         style::IColors,
                         style::IText,
                         style::IInput
                         >;
            using data::data;
        protected:
            util::mt::future<void> btask_{};
            int ptr_{ 0 };
            int bound_lower_{ 0 };
            int highlight_start_{ 0 };
            int highlight_end_{ 0 };
            bool ptr_blink_{ false };

            void _write_ptr(Pixel& px) noexcept;
            bool _is_highlighted() noexcept;
            void _remove_highlighted() noexcept;
            void _begin_highlight_if() noexcept;
            void _reset_highlight() noexcept;
            void _reset_ptrs() noexcept;
            string _highlighted_value() noexcept;

            void _start_blink() noexcept;
            void _stop_blink() noexcept;
            void _freeze_blink() noexcept;
            void _unfreeze_blink() noexcept;

            void _move_right(std::size_t cnt = 1);
            void _move_left(std::size_t cnt = 1);

            virtual Unit _layout_impl(Element::Layout type) const noexcept override;
            virtual bool _provides_input_impl() const noexcept override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(IOContext::Event ev) override;
            virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
        public:
            virtual ~Input();
        };
    }
}

#endif // D2_INPUT_HPP
