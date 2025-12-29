#include "d2_checkbox.hpp"

namespace d2::dx
{
    void Checkbox::_submit()
    {
        if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<Checkbox>(shared_from_this()),
                data::checkbox_value
            );
    }

    bool Checkbox::_provides_input_impl() const
    {
        return true;
    }

    void Checkbox::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        if (element.get() == this &&
                (prop == Value ||
                 prop == initial_property ||
                    ((data::checkbox_value && prop == ValueOn) ||
                    (!data::checkbox_value && prop == ValueOff))))
        {
            if (data::checkbox_value)
            {
                _text_dimensions = TextHelper::_text_bounding_box(data::value_on);
            }
            else
            {
                _text_dimensions = TextHelper::_text_bounding_box(data::value_off);
            }
            _signal_write(WriteType::Dimensions);
        }
    }
    Unit Checkbox::_layout_impl(enum Element::Layout type) const
    {
        switch (type)
        {
            case Element::Layout::X: return data::x;
            case Element::Layout::Y: return data::y;
            case Element::Layout::Width: return HDUnit(_text_dimensions.width);
            case Element::Layout::Height: return VDUnit(_text_dimensions.height);
        }
    }

    void Checkbox::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked && value)
        {
            data::checkbox_value = !data::checkbox_value;
            _submit();
            _signal_write(data::value_on.size() == data::value_off.size() ?
                          Style : Masked
                         );
        }
    }
    void Checkbox::_event_impl(IOContext::Event ev)
    {
        if (getstate(State::Keynavi) &&
                ev == IOContext::Event::KeyInput &&
                context()->input()->is_pressed(sys::SystemInput::Enter, sys::SystemInput::KeyMode::Press))
        {
            _submit();
        }
    }

    void Checkbox::_frame_impl(PixelBuffer::View buffer)
    {
        const auto color = Pixel::combine(
                               data::foreground_color,
                               getstate(Keynavi) ? data::focused_color : data::background_color
                           );
        buffer.fill(color);
        TextHelper::_render_text(
            data::checkbox_value ? data::value_on : data::value_off,
            data::checkbox_value ? data::color_on : data::color_off,
            { 0, 0 },
            box(),
            buffer
        );
    }
}
