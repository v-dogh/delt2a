#include "d2_dropdown.hpp"

namespace d2::dx
{
    int Dropdown::_border_width()
    {
        if (!(data::container_options & data::EnableBorder))
            return 0;

        const auto bbox = box();
        auto bw = resolve_units(data::border_width);
        [[ unlikely ]] if (bw > bbox.height || bw > bbox.width)
        {
            data::border_width = Unit(std::min(bbox.width, bbox.height), Unit::Px);
            bw = resolve_units(data::border_width);
        }
        return bw;
    }

    void Dropdown::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        if (element.get() == this &&
                (data::width.getunits() == Unit::Auto ||
                 data::height.getunits() == Unit::Auto) &&
                (prop == BorderWidth ||
                 prop == initial_property))
        {
            const auto bw = (data::container_options & data::EnableBorder) ? resolve_units(data::border_width) : 0;

            int perfect_width = 0;
            int perfect_height = 0;
            {
                const auto [ width, height ] = TextHelper::_text_bounding_box(data::default_value);
                perfect_width = std::max(width, perfect_width);
                perfect_height = std::max(height, perfect_height);
            }
            for (decltype(auto) it : data::options)
            {
                const auto [ width, height ] = TextHelper::_text_bounding_box(it);
                perfect_width = std::max(width, perfect_width);
                perfect_height = std::max(height, perfect_height);
            }

            const auto cnt = (data::options.size() * _opened) + 1;
            _perfect_dimensions.width = perfect_width + (bw * 2) + data::width.raw();
            _perfect_dimensions.height = (perfect_height + bw) * cnt + bw + data::height.raw();
            _cell_height = perfect_height;

            _signal_write(WriteType::Dimensions);
        }
    }
    Unit Dropdown::_layout_impl(Element::Layout type) const
    {
        switch (type)
        {
            case Element::Layout::X: return data::x;
            case Element::Layout::Y: return data::y;
            case Element::Layout::Width:
                if (data::width.getunits() == Unit::Auto)
                    return HDUnit(_perfect_dimensions.width);
                return data::width;
            case Element::Layout::Height:
                if (data::height.getunits() == Unit::Auto)
                {
                    return VDUnit(_perfect_dimensions.height);
                }
                else
                {
                    const auto bw = (data::container_options & data::EnableBorder) ? resolve_units(data::border_width) : 0;
                    const auto cnt = (data::options.size() * _opened) + 1;
                    const auto height = int(resolve_units(data::height));
                    _cell_height = height - (bw * 2);
                    return VDUnit(int((height - bw) * cnt + bw));
                }
        }
    }

    bool Dropdown::_provides_input_impl() const
    {
        return true;
    }

    void Dropdown::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked && value)
        {
            const auto [ x, y ] = mouse_object_space();
            const auto bcw = _border_width();
            const auto yoff = (_cell_height / 2) + bcw;
            const auto c = (y - yoff) / (_cell_height + bcw);
            if (c >= 1)
            {
                choose(c - 1);
            }
            else
            {
                _opened = !_opened;
            }
            _signal_write(WriteType::Masked);
        }
        else if (state == State::Focused && !value)
        {
            _opened = false;
            _signal_write(WriteType::Masked);
        }
        else if (state == State::Keynavi)
        {
            _opened = value;
            _signal_write(Masked);
        }
    }
    void Dropdown::_event_impl(IOContext::Event ev)
    {
        if (ev == IOContext::Event::KeyInput)
        {
            if (context()->input()->is_pressed(sys::SystemInput::ArrowLeft) ||
                    context()->input()->is_pressed(sys::SystemInput::ArrowDown))
            {
                _soft_selected = (_soft_selected + 1) % data::options.size();
                _signal_write(Style);
            }
            else if (context()->input()->is_pressed(sys::SystemInput::ArrowRight) ||
                     context()->input()->is_pressed(sys::SystemInput::ArrowUp))
            {
                const auto s = int(data::options.size());
                _soft_selected = ((_soft_selected - 1) % s + s) % s;
                _signal_write(Style);
            }
            else if (context()->input()->is_pressed(sys::SystemInput::Enter))
            {
                if (_opened && _soft_selected != -1)
                {
                    choose(_soft_selected);
                }
                else
                {
                    _opened = !_opened;
                    _signal_write(Masked);
                }
                _soft_selected = -1;
            }
        }
    }

    void Dropdown::_frame_impl(PixelBuffer::View buffer)
    {
        const auto bbox = box();
        const auto color = Pixel::combine(data::foreground_color, data::background_color);
        const auto color_focused = Pixel::combine(data::foreground_color, data::focused_color);

        buffer.fill(color);

        const auto bw = _border_width();
        const auto yoff = (_cell_height / 2) + bw;
        {
            if (data::show_choice && _selected != -1)
            {
                const auto dims = TextHelper::_text_bounding_box(data::options[_selected]);
                const auto xoff = ((bbox.width - dims.width - bw) / 2) + bw;
                TextHelper::_render_text(
                    data::options[_selected], color,
                { xoff, yoff },
                { bbox.width, _cell_height },
                buffer
                );
            }
            else
            {
                const auto dims = TextHelper::_text_bounding_box(data::default_value);
                const auto xoff = ((bbox.width - dims.width - bw) / 2) + bw;

                TextHelper::_render_text(
                    data::default_value, color,
                { xoff, yoff },
                { bbox.width, _cell_height },
                buffer
                );
            }
        }

        if (_opened)
        {
            for (std::size_t i = 0; i < data::options.size(); i++)
            {
                const auto dims = TextHelper::_text_bounding_box(data::options[i]);
                const auto xoff = ((bbox.width - dims.width - bw) / 2) + bw;
                const auto yoff2 = int(yoff + ((i + 1) * (_cell_height + bw)));
                TextHelper::_render_text(
                    data::options[i], (i == _soft_selected ? color_focused : color),
                { xoff, yoff2 },
                { bbox.width, _cell_height },
                buffer
                );
            }
        }

        const bool is_border = data::container_options & ContainerOptions::EnableBorder;
        const bool is_topfix = data::container_options & ContainerOptions::TopFix;
        if (is_border)
        {
            ContainerHelper::_render_border(buffer);

            const auto& bch = data::border_horizontal_color;
            const auto ch = _cell_height + bw;
            const auto opt = _opened * data::options.size();

            for (std::size_t c = 1; c < opt + 1; c++)
                for (std::size_t i = 0; i < bw; i++)
                    for (std::size_t x = is_topfix; x < bbox.width - is_topfix; x++)
                    {
                        buffer.at(x, (ch * c) - i) = bch;
                    }
        }
    }

    void Dropdown::choose(int idx)
    {
        if (idx >= 0)
        {
            if (idx >= data::options.size())
                throw std::runtime_error{ "Invalid option selected" };
            if (data::on_submit != nullptr)
                data::on_submit(
                    std::static_pointer_cast<Dropdown>(shared_from_this()),
                    options[idx], idx
                );
        }
        _opened = false;
        _selected = idx;
        _signal_write(WriteType::Masked);
    }
    void Dropdown::reset()
    {
        choose(-1);
    }
    void Dropdown::reset(const string& value)
    {
        data::default_value = value;
        reset();
    }
    int Dropdown::index()
    {
        if (_selected == -1)
            return 0;
        return _selected;
    }
    string Dropdown::value()
    {
        if (_selected == -1)
            return data::options[0];
        return data::options[_selected];
    }
}
