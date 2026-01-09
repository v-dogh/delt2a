#include "d2_draggable_box.hpp"

namespace d2::dx
{
    bool VirtualBox::_is_being_resized() const
    {
        const auto [ width, height ] = box();
        return
            (data::vbox_options & VBoxOptions::Resizable) &&
            offset_.first == width - 1 && offset_.second == 0;
    }

    Unit VirtualBox::_layout_impl(enum Element::Layout type) const
    {
        switch (type)
        {
            case Element::Layout::X: return data::x;
            case Element::Layout::Y: return data::y;
            case Element::Layout::Width:
                if (data::width.getunits() == Unit::Auto)
                    return HDUnit(_perfect_width());
                return data::width;
            case Element::Layout::Height:
                if (minimized_)
                    return VDUnit(data::bar_height);
                return Box::_layout_impl(type);
        }
    }


    int VirtualBox::_get_border_impl(BorderType type, Element::cptr elem) const
    {
        if (type == BorderType::Top)
            return resolve_units(data::bar_height);
        return Box::_get_border_impl(type, elem);
    }
    bool VirtualBox::_provides_input_impl() const
    {
        return true;
    }

    void VirtualBox::_state_change_impl(State state, bool value)
    {
        Box::_state_change_impl(state, value);
        if (state == State::Clicked)
        {
            if (value)
            {
                const auto [ x, y ] = mouse_object_space();
                offset_.first = x;
                offset_.second = y;
            }
            else if (!_is_being_resized())
            {
                offset_ = { 0, 0 };
            }
        }
        else if (state == State::Keynavi)
        {
            _signal_write(Style);
        }
    }
    void VirtualBox::_event_impl(Screen::Event ev)
    {
        Box::_event_impl(ev);
        if (ev == Screen::Event::MouseInput)
        {
            const auto rlc = context()->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Left, sys::SystemInput::KeyMode::Release);
            const auto rc = context()->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Right, sys::SystemInput::KeyMode::Press);
            const auto lc = context()->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Left);
            if (getstate(Hovered) && !_is_being_resized())
            {
                if (getstate(Clicked) && lc && (data::vbox_options & VBoxOptions::Draggable))
                {
                    const auto [ x, y ] = context()->input()->mouse_position();
                    const auto nx = std::max(x - offset_.first, 0);
                    const auto ny = std::max(y - offset_.second, 0);
                    if (nx != resolve_units(Box::x) ||
                            ny != resolve_units(Box::y))
                    {
                        Box::x = nx;
                        Box::y = ny;
                        _signal_write(WriteType::Offset);
                    }
                }
                if (rc && (data::vbox_options & VBoxOptions::Minimizable))
                {
                    minimized_ = !minimized_;
                    _signal_write(WriteType::Dimensions);
                }
            }
            else if (rlc && _is_being_resized())
            {
                const auto [ x, y ] = mouse_object_space();
                const auto yoff = resolve_units(Box::height) - y;
                const auto bh = resolve_units(data::bar_height);
                if (x > 0 && (yoff - bh) > 0 &&
                        x != resolve_units(Box::width) &&
                        yoff != resolve_units(Box::height))
                {
                    Box::width = x;
                    Box::height = yoff;
                    Box::y = y + resolve_units(Box::y);
                    _signal_write(WriteType::Offset | WriteType::Dimensions);
                }
                offset_ = { 0, 0 };
            }
        }
        else if (ev == Screen::Event::KeyInput)
        {
            const auto left = context()->input()->is_pressed(sys::SystemInput::key('h'));
            const auto right = context()->input()->is_pressed(sys::SystemInput::key('l'));
            const auto down = context()->input()->is_pressed(sys::SystemInput::key('j'));
            const auto up = context()->input()->is_pressed(sys::SystemInput::key('k'));

            if ((data::vbox_options & data::Resizable) &&
                    context()->input()->is_pressed(sys::SystemInput::Shift))
            {
                if (left || right)
                {
                    const auto width = resolve_units(Box::width);
                    if (width > 1)
                    {
                        Box::width = width + (left ? -1 : 1);
                        _signal_write(WriteType::LayoutWidth);
                    }
                }
                if (down || up)
                {
                    const auto height = resolve_units(Box::height);
                    if (height > 1)
                    {
                        Box::height = height + (up ? -1 : 1);
                        _signal_write(WriteType::LayoutHeight);
                    }
                }
            }
            else if (data::vbox_options & data::Draggable)
            {
                if (left || right)
                {
                    const auto x = resolve_units(Box::x);
                    if (x > 1)
                    {
                        Box::x = x + (left ? -1 : 1);
                        _signal_write(WriteType::LayoutXPos);
                    }
                }
                if (down || up)
                {
                    const auto y = resolve_units(Box::y);
                    if (y > 1)
                    {
                        Box::y = y + (up ? -1 : 1);
                        _signal_write(WriteType::LayoutYPos);
                    }
                }
            }
        }
    }
    void VirtualBox::_frame_impl(PixelBuffer::View buffer)
    {
        if (!minimized_)
            Box::_frame_impl(buffer);

        const auto bh = resolve_units(data::bar_height);
        buffer.fill(
            getstate(Keynavi) ?
            data::focused_color.extend(data::bar_color.v) :
            data::bar_color,
            0, 0, buffer.width(), bh
        );

        const auto s = (buffer.width() < data::title.size()) ?
                       0 : (buffer.width() - data::title.size()) / 2;
        const auto w = std::min(int(data::title.size()), buffer.width());
        const auto p = bh / 2;
        for (std::size_t i = 0; i < w; i++)
        {
            auto& px = buffer.at(i + s, p);
            px.v = data::title[i];
        }

        if (data::vbox_options & VBoxOptions::Resizable)
        {
            buffer.at(buffer.width() - 1).v =
                      data::resize_point_value;
        }
    }

    void VirtualBox::close()
    {
        parent()
        ->traverse().asp()
        ->remove(shared_from_this());
    }
}
