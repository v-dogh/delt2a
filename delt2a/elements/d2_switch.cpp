#include "d2_switch.hpp"
#include "delt2a/d2_colors.hpp"

namespace d2::dx
{
    bool Switch::_provides_input_impl() const
    {
        return true;
    }

    void Switch::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        if (element.get() == this &&
                (data::width.getunits() == Unit::Auto ||
                 data::height.getunits() == Unit::Auto) &&
                prop == BorderWidth ||
                prop == Options ||
                prop == initial_property)
        {
            const auto [ xbasis, ybasis ] = ContainerHelper::_border_base();
            const auto [ ixbasis, iybasis ] = ContainerHelper::_border_base_inv();

            int perfect_width = 0;
            for (decltype(auto) it : data::options)
            {
                perfect_width = std::max(
                    perfect_width,
                    TextHelper::_text_bounding_box(it).width
                );
            }

            const auto [ rw, _1 ] = TextHelper::_text_bounding_box(data::select_pre);
            const auto [ ow, _2 ] = TextHelper::_text_bounding_box(data::select_post);
            _perfect_dimensions.width =
                (perfect_width * data::options.size()) +
                (data::options.size() - 1) + (xbasis + ixbasis) +
                rw + ow +
                data::width.raw();
            _perfect_dimensions.height = 1 + (ybasis + iybasis);

            _signal_write(WriteType::Dimensions);
        }
    }
    Unit Switch::_layout_impl(Element::Layout type) const
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
                    return VDUnit(_perfect_dimensions.height);
                return data::height;
        }
    }

    void Switch::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked && value)
        {
            const auto [ width, height ] = box();
            const auto [ x, y ] = mouse_object_space();
            const auto [ basisx, basisy ] = ContainerHelper::_border_base();
            const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();
            const auto w = width - basisx - ibasisx;
            const auto cw =
                ((w - (data::options.size() - 1)) /
                 (data::options.size())) + 1;

            if (cw > 0)
            {
                const auto idx = x / cw;
                const auto midx = x % cw;

                if (midx != (cw - 1) &&
                    idx != _idx &&
                    idx < data::options.size())
                {
                    _idx = idx;
                    _signal_write(Style);
                    _submit();
                }
            }
        }
        else if (state == State::Keynavi)
        {
            _signal_write(Style);
        }
    }
    void Switch::_event_impl(Screen::Event ev)
    {
        if (ev == Screen::Event::KeyInput)
        {
            if (context()->input()->is_pressed(sys::input::Tab))
            {
                _idx = ++_idx % data::options.size();
                _signal_write(Style);
                _submit();
            }
        }
    }
    void Switch::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(
            Pixel::combine(
                data::foreground_color,
                data::background_color
            )
        );

        if (!data::options.empty())
        {
            const auto [ width, height ] = box();
            const auto [ basisx, basisy ] = ContainerHelper::_border_base();
            const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();
            const int width_slice =
                (width - (data::options.size() - 1) - (basisx + ibasisx)) /
                data::options.size();

            const auto disabled_color = Pixel::combine(
                data::disabled_foreground_color,
                data::disabled_background_color
            );
            const auto enabled_color = Pixel::combine(
                data::enabled_foreground_color,
                (getstate(Keynavi) ? data::focused_color : data::enabled_background_color)
            );
            for (std::size_t i = 0; i < data::options.size(); i++)
            {
                const auto col = (i == _idx) ? enabled_color : disabled_color;
                const auto xoff = int(basisx + ((width_slice + 1) * i));
                if (data::expand_background)
                    buffer.fill(col, xoff, basisy, width_slice, 1);
                TextHelper::_render_text_simple(
                    data::options[i],
                    col,
                    style::IZText::Alignment::Center,
                    { xoff, basisy }, { width_slice, 1 },
                    buffer
                );
                if (i < data::options.size() - 1)
                    buffer.at(xoff + width_slice, basisy)
                        .blend(data::separator_color);
            }
        }

        ContainerHelper::_render_border(buffer);
    }
    void Switch::_submit()
    {
        if (_old == _idx)
            return;
        if (data::on_change != nullptr)
            data::on_change(
                std::static_pointer_cast<Switch>(shared_from_this()),
                _idx, _old
            );
        else if (data::on_change_values != nullptr)
            data::on_change_values(
                std::static_pointer_cast<Switch>(shared_from_this()),
                data::options[_idx], _old == -1 ?
                    d2::string("") : data::options[_old]
            );
        _old = _idx;
    }
    void Switch::reset(int idx)
    {
        if (_idx != idx)
        {
            _idx = idx;
            _signal_write(Style);
            _submit();
        }
    }
    void Switch::reset(string choice)
    {
        auto f = std::find(data::options.begin(), data::options.end(), choice);
        if (f != data::options.end())
            reset(f - data::options.begin());
    }
    string Switch::choice()
    {
        return data::options[_idx];
    }
    int Switch::index()
    {
        return _idx;
    }

    void VerticalSwitch::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        if (element.get() != this)
            return;
        if (type & WriteType::Dimensions)
        {
            const auto [ basisx, basisy ] = ContainerHelper::_border_base();
            const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();
            const auto height = layout(Element::Layout::Height) - basisy - ibasisy;
            const auto size = data::options.size();
            if (size > height)
            {
                _scrollbar = true;
                if (_scroll_offset > size - height)
                    _scroll_offset = size - height;
            }
            else
            {
                _scrollbar = false;
                _scroll_offset = 0;
            }
        }
        if ((data::width.getunits() == Unit::Auto ||
            data::height.getunits() == Unit::Auto) &&
            prop == BorderWidth ||
            prop == Options ||
            prop == SelectPre ||
            prop == SelectPost ||
            prop == initial_property)
        {
            const auto [ xbasis, ybasis ] = ContainerHelper::_border_base();
            const auto [ ixbasis, iybasis ] = ContainerHelper::_border_base_inv();

            int perfect_width = 0;
            for (decltype(auto) it : data::options)
            {
                perfect_width = std::max(
                    perfect_width,
                    TextHelper::_text_bounding_box(it).width
                );
            }

            const auto [ rw, _1 ] = TextHelper::_text_bounding_box(data::select_pre);
            const auto [ ow, _2 ] = TextHelper::_text_bounding_box(data::select_post);
            _perfect_dimensions.height =
                data::options.size() +
                (data::options.size() - 1) * !data::disable_separator + data::height.raw() +
                (ybasis + iybasis);
            _perfect_dimensions.width =
                perfect_width + (xbasis + ixbasis) +
                rw + ow;

            _signal_write(WriteType::Dimensions);
        }
    }
    void VerticalSwitch::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked && value)
        {
            const auto [ width, height ] = box();
            const auto [ x, y ] = mouse_object_space();
            const auto [ basisx, basisy ] = ContainerHelper::_border_base();
            const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();
            if (data::disable_separator)
            {
                const auto h = y - basisy;
                if (h >= 0 &&
                    _idx != h &&
                    h < data::options.size())
                {
                    _idx = h + _scroll_offset;
                    _signal_write(Style);
                    _submit();
                }
            }
            else
            {
                const auto h = height - basisy - ibasisy;
                const auto ch =
                    ((h - (data::options.size() - 1)) /
                     (data::options.size())) + 1;

                if (ch > 0)
                {
                    const auto idx = y / ch;
                    const auto midx = y % ch;

                    if (midx != (ch - 1) &&
                        idx != _idx &&
                        idx < data::options.size())
                    {
                        _idx = idx + _scroll_offset;
                        _signal_write(Style);
                        _submit();
                    }
                }
            }
        }
        else if (state == State::Keynavi)
        {
            if (value)
                _selector = _idx;
            _signal_write(Style);
        }
        else if (state == State::Focused && !value)
        {
            _selector = -1;
            _signal_write(Style);
        }
    }
    void VerticalSwitch::_event_impl(Screen::Event ev)
    {
        Switch::_event_impl(ev);
        if (ev == Screen::Event::KeyInput || ev == Screen::Event::MouseInput)
        {
            const auto in = context()->input();
            const auto [ basisx, basisy ] = ContainerHelper::_border_base();
            const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();
            const auto height = layout(Element::Layout::Height) - basisy - ibasisy;
            const auto size = data::options.size();
            auto update = true;
            if (in->is_pressed(sys::input::ArrowDown) ||
                in->is_pressed_mouse(sys::input::ScrollDown))
            {
                if (_selector == -1)
                    _selector = _idx;

                _selector = ++_selector % size;
                if (_selector == 0)
                    _scroll_offset = 0;
                else if (_selector > _scroll_offset + (height - 1) / 2)
                    _scroll_offset++;
            }
            else if (in->is_pressed(sys::input::ArrowUp) ||
                     in->is_pressed_mouse(sys::input::ScrollUp))
            {
                if (_selector == -1)
                    _selector = _idx;

                if (_selector == 0)
                {
                    _scroll_offset =
                        size > height ? size - height : 0;
                    _selector = size - 1;
                }
                else
                    --_selector;
                if (_scroll_offset &&
                    _selector < _scroll_offset + (height - 1) / 2)
                    _scroll_offset--;
            }
            else if (in->is_pressed(sys::input::Enter) && _selector != -1)
            {
                _idx = _selector;
            }
            else
                update = false;

            if (update)
            {
                _signal_write(Style);
                _submit();
            }
        }
    }
    void VerticalSwitch::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(
            Pixel::combine(
                data::foreground_color,
                data::background_color
            )
        );

        ContainerHelper::_render_border(buffer);

        if (!data::options.empty())
        {
            // Options

            const auto [ width, height ] = box();
            const auto [ basisx, basisy ] = ContainerHelper::_border_base();
            const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();

            const auto disabled_color = Pixel::combine(
                data::disabled_foreground_color,
                data::disabled_background_color
            );
            const auto enabled_color = Pixel::combine(
                data::enabled_foreground_color,
                (getstate(Keynavi) ?
                     data::enabled_background_color
                        .mask(data::focused_color, 0.35f)
                        .alpha(data::enabled_background_color.a) :
                     data::enabled_background_color)
            );
            const auto target_color = Pixel::combine(
                data::enabled_foreground_color,
                data::focused_color
            );
            const auto is_border = data::container_options & data::EnableBorder;
            const auto h = std::min<std::size_t>(data::options.size(), height - basisy - ibasisy);
            for (std::size_t i = 0; i < h; i++)
            {
                const auto idx = i + _scroll_offset;
                const auto yoff = int(basisy + !data::disable_separator + i);
                const auto color = (idx == _idx) ? enabled_color :
                    idx == _selector ? target_color : disabled_color;
                if (_idx == idx || _selector == idx)
                {
                    if (_idx == idx || !is_border)
                    {
                        buffer.fill(
                            color,
                            basisx, yoff,
                            width - ibasisx - basisx, 1
                        );
                    }
                    else
                    {
                        const auto bbg = px::background{
                            data::border_horizontal_color.rf,
                            data::border_horizontal_color.gf,
                            data::border_horizontal_color.bf,
                            255
                        };
                        const auto full = width - ibasisx - basisx;
                        const auto half = full / 2;
                        for (std::size_t i = 0; i < half; i++)
                        {
                            const auto c = Pixel::interpolate(
                                bbg,
                                color,
                                float(i) / half
                            );
                            buffer.at(i + basisx, yoff) = c;
                            buffer.at(full - i, yoff) = c;
                        }
                        buffer.at(half + basisx + 1, yoff) = color;
                    }
                    if (is_border)
                    {
                        buffer.at(0, yoff) = data::border_horizontal_color
                            .extend(data::right_connector).alpha(0.f, 1.f);
                        buffer.at(width - 1, yoff) = data::border_horizontal_color
                            .extend(data::left_connector).alpha(0.f, 1.f);
                    }
                }
                int aoff = 0;
                if (i == _idx || i == _selector)
                {
                    if (!data::select_pre.empty())
                    {
                        aoff = TextHelper::_text_bounding_box(data::select_pre).width;
                        TextHelper::_render_text_simple(
                            data::select_pre,
                            d2::Pixel::combine(data::pre_color, colors::w::transparent),
                            Alignment::Left,
                            { basisx, yoff },
                            { aoff, 1 },
                            buffer
                        );
                    }
                    if (!data::select_post.empty())
                    {
                        const auto w = TextHelper::_text_bounding_box(data::select_post).width;
                        TextHelper::_render_text_simple(
                            data::select_post,
                            d2::Pixel::combine(data::post_color, colors::w::transparent),
                            Alignment::Left,
                            { buffer.width() - w - 1, yoff },
                            { w, 1 },
                            buffer
                        );
                    }
                }
                TextHelper::_render_text_simple(
                    data::options[idx],
                    color.alpha(0.f, 1.f),
                    text_alignment,
                    { basisx + aoff, yoff },
                    { buffer.width() - basisx - ibasisx - _scrollbar - aoff, 1 },
                    buffer
                );
                if (!data::disable_separator)
                {
                    if (idx < h - 1)
                        buffer.at(basisx, yoff + 1)
                            .blend(data::separator_color);
                }
            }

            // Scrollbar

            if (_scrollbar)
            {
                for (std::size_t i = 0; i < height - basisy - ibasisy; i++)
                {
                    buffer.at(width - basisx - 1, basisy + i)
                        = data::slider_background_color;
                }
                const auto whole = data::options.size() - height;
                buffer.at(
                    width - basisx - 1,
                    basisy + (float(_scroll_offset) / whole) * (height - 1)
                ) = data::slider_color;
            }
        }
    }
}
