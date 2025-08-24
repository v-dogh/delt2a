#include "d2_button.hpp"

namespace d2::dx
{
    bool Button::_provides_input_impl() const noexcept
    {
        return true;
    }
    Unit Button::_layout_impl(enum Element::Layout type) const noexcept
    {
        switch (type)
        {
            case Element::Layout::X: return data::x;
            case Element::Layout::Y: return data::y;
            case Element::Layout::Width:
                if (data::width.getunits() == Unit::Auto)
                    return HDUnit(_text_dimensions.width);
                return data::width;
            case Element::Layout::Height:
                if (data::height.getunits() == Unit::Auto)
                    return VDUnit(_text_dimensions.height);
                return data::height;
        }
    }

    void Button::_signal_write_impl(write_flag type, unsigned int prop, ptr element) noexcept
    {
        if (element.get() == this && prop == Value &&
                (data::width.getunits() == Unit::Auto ||
                 data::height.getunits() == Unit::Auto))
        {
            const auto bw = (data::container_options & ContainerOptions::EnableBorder) ?
                            resolve_units(data::border_width) : 0;
            _text_dimensions = TextHelper::_text_bounding_box(data::text);
            _text_dimensions.width += (bw * 2);
            _text_dimensions.height += (bw * 2);
        }
    }
    void Button::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked && value)
        {
            if (data::on_submit != nullptr)
                data::on_submit(shared_from_this());
        }
        else if (state == State::Keynavi)
        {
            _signal_write(Style);
        }
    }
    void Button::_event_impl(IOContext::Event ev)
    {
        if (getstate(State::Keynavi) &&
                ev == IOContext::Event::KeyInput &&
                context()->input()->is_pressed(sys::SystemInput::Enter, sys::SystemInput::KeyMode::Press))
        {
            if (data::on_submit != nullptr)
                data::on_submit(shared_from_this());
        }
    }

    void Button::_frame_impl(PixelBuffer::View buffer) noexcept
    {
        buffer.fill(
            Pixel::combine(
                data::foreground_color,
                getstate(State::Keynavi) ? data::focused_color : data::background_color
            )
        );

        const auto bbox = box();
        if (data::container_options & data::EnableBorder)
            ContainerHelper::_render_border(buffer);
        TextHelper::_render_text(
            data::text,
            data::foreground_color,
            style::Text::Alignment::Center,
            {}, bbox,
            buffer
        );
    }
}
