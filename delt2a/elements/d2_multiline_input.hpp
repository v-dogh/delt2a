#ifndef D2_MULTILINE_INPUT_HPP
#define D2_MULTILINE_INPUT_HPP

#include "../d2_fragmented_buffer.hpp"
#include "d2_input.hpp"

namespace d2::dx
{
    class MultiInput :
        public style::UAI<
            MultiInput,
            style::ILayout,
            style::IColors,
            style::IInput
        >
    {
    public:
        using data = style::UAI<
            MultiInput,
            style::ILayout,
            style::IColors,
            style::IInput
        >;
        using data::data;
    protected:
        fb::FragmentedBuffer _buffer{};

        void _move_right(std::size_t cnt = 1);
        void _move_left(std::size_t cnt = 1);

        virtual Unit _layout_impl(Element::Layout type) const override;
        virtual bool _provides_input_impl() const override;
        virtual void _state_change_impl(State state, bool value) override;
        virtual void _event_impl(Screen::Event ev) override;
        virtual void _frame_impl(PixelBuffer::View buffer) override;
    public:
        virtual ~MultiInput();
    };
}

#endif // D2_MULTILINE_INPUT_HPP
