#include "d2_switch.hpp"
#include <d2_colors.hpp>
#include <d2_styles.hpp>

namespace d2::dx
{
    bool Switch::_provides_input_impl() const
    {
        return true;
    }

    void Switch::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        if (element.get() == this &&
                (data::width.getunits() == Unit::Auto || data::height.getunits() == Unit::Auto) &&
                prop == BorderWidth ||
            prop == Options || prop == initial_property)
        {
            const auto [xbasis, ybasis] = ContainerHelper::_border_base();
            const auto [ixbasis, iybasis] = ContainerHelper::_border_base_inv();

            int perfect_width = 0;
            for (decltype(auto) it : data::options)
            {
                perfect_width = std::max(perfect_width, TextHelper::_text_bounding_box(it).width);
            }

            const auto [rw, _1] = TextHelper::_text_bounding_box(data::select_pre);
            const auto [ow, _2] = TextHelper::_text_bounding_box(data::select_post);
            _perfect_dimensions.width = (perfect_width * data::options.size()) +
                                        (data::options.size() - 1) + (xbasis + ixbasis) + rw + ow +
                                        data::width.raw();
            _perfect_dimensions.height = 1 + (ybasis + iybasis);

            _signal_write(WriteType::Dimensions);
        }
    }
    Unit Switch::_layout_impl(Element::Layout type) const
    {
        switch (type)
        {
        case Element::Layout::X:
            return data::x;
        case Element::Layout::Y:
            return data::y;
        case Element::Layout::Width:
            if (data::width.getunits() == Unit::Auto)
                return Unit(_perfect_dimensions.width + data::width.raw());
            return data::width;
        case Element::Layout::Height:
            if (data::height.getunits() == Unit::Auto)
                return Unit(_perfect_dimensions.height + data::height.raw());
            return data::height;
        }
    }

    void Switch::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked && value)
        {
            const auto [width, height] = box();
            const auto [x, y] = mouse_object_space();
            const auto [basisx, basisy] = ContainerHelper::_border_base();
            const auto [ibasisx, ibasisy] = ContainerHelper::_border_base_inv();
            const auto w = width - basisx - ibasisx;
            const auto cw = ((w - (data::options.size() - 1)) / (data::options.size())) + 1;

            if (cw > 0)
            {
                const auto idx = x / cw;
                const auto midx = x % cw;

                if (midx != (cw - 1) && idx != _idx && idx < data::options.size())
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
    void Switch::_event_impl(in::InputFrame& frame)
    {
        if (frame.had_event(in::Event::KeyInput))
        {
            if (frame.active(in::special::Tab, in::mode::Press))
            {
                _idx = ++_idx % data::options.size();
                _signal_write(Style);
                _submit();
            }
        }
    }
    void Switch::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(Pixel::combine(data::foreground_color, data::background_color));

        if (!data::options.empty())
        {
            const auto [width, height] = box();
            const auto [basisx, basisy] = ContainerHelper::_border_base();
            const auto [ibasisx, ibasisy] = ContainerHelper::_border_base_inv();
            const int width_slice =
                (width - (data::options.size() - 1) - (basisx + ibasisx)) / data::options.size();

            const auto disabled_color =
                Pixel::combine(data::disabled_foreground_color, data::disabled_background_color);
            const auto enabled_color = Pixel::combine(
                data::enabled_foreground_color,
                (getstate(Keynavi) ? data::focused_color : data::enabled_background_color)
            );
            for (std::size_t i = 0; i < data::options.size(); i++)
            {
                const auto col = (i == _idx) ? enabled_color : disabled_color;
                auto xoff = int(basisx + ((width_slice + 1) * i));
                if (data::expand_background)
                    buffer.fill_blend(col, xoff, basisy, width_slice, 1);
                if (!data::select_pre.empty())
                {
                    const auto width = TextHelper::_text_bounding_box(data::select_pre).width;
                    TextHelper::_render_text_simple(
                        data::select_pre,
                        data::pre_color,
                        style::IZText::Alignment::Left,
                        {xoff, basisy},
                        {width, 1},
                        buffer
                    );
                    xoff += width;
                }
                {
                    const auto width = TextHelper::_text_bounding_box(data::options[i]);
                    TextHelper::_render_text_simple(
                        data::options[i],
                        col,
                        style::IZText::Alignment::Center,
                        {xoff, basisy},
                        {width_slice, 1},
                        buffer
                    );
                    xoff += width_slice;
                }
                {
                    const auto width = TextHelper::_text_bounding_box(data::select_pre).width;
                    TextHelper::_render_text_simple(
                        data::select_pre,
                        data::pre_color,
                        style::IZText::Alignment::Left,
                        {xoff, basisy},
                        {width, 1},
                        buffer
                    );
                    xoff += width;
                }
                if (!data::disable_separator)
                {
                    if (i < data::options.size() - 1)
                        buffer.at(xoff, basisy).blend(data::separator_color);
                }
            }
        }

        ContainerHelper::_render_border(buffer);
    }
    void Switch::_submit()
    {
        if (_old == _idx)
            return;
        if (data::on_change != nullptr)
            data::on_change(std::static_pointer_cast<Switch>(shared_from_this()), _idx, _old);
        else if (data::on_change_values != nullptr)
            data::on_change_values(
                std::static_pointer_cast<Switch>(shared_from_this()),
                data::options[_idx],
                _old == invalid ? d2::string("") : data::options[_old]
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
        return _idx == -1 ? "" : data::options[_idx];
    }
    int Switch::index()
    {
        return _idx;
    }

    void VerticalSwitch::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        if (element.get() != this)
            return;
        if (prop == Options)
        {
            if (_idx >= data::options.size())
                _idx = -1;
        }
        if (type & WriteType::Dimensions)
        {
            const auto [basisx, basisy] = ContainerHelper::_border_base();
            const auto [ibasisx, ibasisy] = ContainerHelper::_border_base_inv();
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
        if ((data::width.getunits() == Unit::Auto || data::height.getunits() == Unit::Auto) &&
            (prop == ContainerBorder || prop == ContainerOptions || prop == BorderWidth ||
             prop == Options || prop == SelectPre || prop == SelectPost ||
             prop == initial_property))
        {
            const auto [xbasis, ybasis] = ContainerHelper::_border_base();
            const auto [ixbasis, iybasis] = ContainerHelper::_border_base_inv();

            int perfect_width = 0;
            for (decltype(auto) it : data::options)
            {
                perfect_width = std::max(perfect_width, TextHelper::_text_bounding_box(it).width);
            }

            const auto [rw, _1] = TextHelper::_text_bounding_box(data::select_pre);
            const auto [ow, _2] = TextHelper::_text_bounding_box(data::select_post);
            _perfect_dimensions.height = data::options.size() +
                                         (data::options.size() - 1) * !data::disable_separator +
                                         data::height.raw() + (ybasis + iybasis);
            _perfect_dimensions.width = perfect_width + (xbasis + ixbasis) + rw + ow;

            _signal_write(WriteType::Dimensions);
        }
    }
    void VerticalSwitch::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked && value)
        {
            const auto [width, height] = box();
            const auto [x, y] = mouse_object_space();
            const auto [basisx, basisy] = ContainerHelper::_border_base();
            const auto [ibasisx, ibasisy] = ContainerHelper::_border_base_inv();
            if (data::disable_separator)
            {
                const auto h = y - basisy;
                if (h >= 0 && _idx != h && h < data::options.size())
                {
                    _idx = h + _scroll_offset;
                    _signal_write(Style);
                    _submit();
                }
            }
            else
            {
                const auto h = height - basisy - ibasisy;
                const auto ch = ((h - (data::options.size() - 1)) / (data::options.size())) + 1;

                if (ch > 0)
                {
                    const auto idx = y / ch;
                    const auto midx = y % ch;

                    if (midx != (ch - 1) && idx != _idx && idx < data::options.size())
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
            _selector = invalid;
            _signal_write(Style);
        }
    }
    void VerticalSwitch::_event_impl(in::InputFrame& frame)
    {
        Switch::_event_impl(frame);

        auto update = false;

        auto scroll_up = [&]
        {
            const auto [basisx, basisy] = ContainerHelper::_border_base();
            const auto [ibasisx, ibasisy] = ContainerHelper::_border_base_inv();
            const auto height = layout(Element::Layout::Height) - basisy - ibasisy;
            const auto size = data::options.size();

            if (_selector == invalid)
                _selector = _idx;

            if (_selector == 0)
            {
                _scroll_offset = size > height ? size - height : 0;
                _selector = size - 1;
            }
            else
                --_selector;
            if (_scroll_offset && _selector < _scroll_offset + (height - 1) / 2)
                _scroll_offset--;

            update = true;
        };
        auto scroll_down = [&]
        {
            const auto [basisx, basisy] = ContainerHelper::_border_base();
            const auto [ibasisx, ibasisy] = ContainerHelper::_border_base_inv();
            const auto height = layout(Element::Layout::Height) - basisy - ibasisy;
            const auto size = data::options.size();

            if (_selector == invalid)
                _selector = _idx;

            _selector = ++_selector % size;
            if (_selector == 0)
                _scroll_offset = 0;
            else if (_selector > _scroll_offset + (height - 1) / 2)
                _scroll_offset++;

            update = true;
        };

        if (frame.had_event(in::Event::KeyInput))
        {
            if (frame.active(in::special::ArrowDown, in::mode::Hold))
                scroll_down();
            else if (frame.active(in::special::ArrowUp, in::mode::Hold))
                scroll_up();
            else if (frame.active(in::special::Enter, in::mode::Hold) && _selector != invalid)
            {
                _idx = _selector;
                update = true;
            }
        }
        if (frame.had_event(in::Event::ScrollWheelMovement))
        {
            auto scroll = frame.scroll_delta().second;
            if (scroll > 0)
                while (scroll--)
                    scroll_up();
            else if (scroll < 0)
                while (scroll++)
                    scroll_down();
        }

        if (update)
        {
            _signal_write(Style);
            _submit();
        }
    }
    void VerticalSwitch::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(Pixel::combine(data::foreground_color, data::background_color));
        ContainerHelper::_render_border(buffer);

        if (data::options.empty())
            return;

        const auto [width, height] = box();
        const auto [basisx, basisy] = ContainerHelper::_border_base();
        const auto [ibasisx, ibasisy] = ContainerHelper::_border_base_inv();

        const int inner_x0 = basisx;
        const int inner_x1 = width - ibasisx - 1;
        const int inner_y0 = basisy;
        const int inner_y1 = height - ibasisy - 1;

        const int inner_w = std::max(0, inner_x1 - inner_x0 + 1);
        const int inner_h = std::max(0, inner_y1 - inner_y0 + 1);

        if (inner_w <= 0 || inner_h <= 0)
            return;

        const int row_pitch = data::disable_separator ? 1 : 2;
        const std::size_t option_count = data::options.size();
        const std::size_t visible_count = std::min<std::size_t>(
            option_count > _scroll_offset ? option_count - _scroll_offset : 0,
            std::max(0, inner_h / row_pitch)
        );

        const bool is_border = data::container_options & data::EnableBorder;
        const int scrollbar_w = _scrollbar ? 1 : 0;
        const int content_w = std::max(0, inner_w - scrollbar_w);

        const auto disabled_color =
            Pixel::combine(data::disabled_foreground_color, data::disabled_background_color);

        const auto enabled_bg =
            getstate(Keynavi) ? data::enabled_background_color.mask(data::focused_color, 0.35f)
                                    .alpha(data::enabled_background_color.a)
                              : data::enabled_background_color;

        const auto enabled_color = Pixel::combine(data::enabled_foreground_color, enabled_bg);

        const auto target_color =
            Pixel::combine(data::enabled_foreground_color, data::focused_color);

        for (std::size_t i = 0; i < visible_count; ++i)
        {
            const std::size_t idx = i + _scroll_offset;
            const int yoff = inner_y0 + int(i * row_pitch);

            if (yoff < inner_y0 || yoff > inner_y1)
                continue;

            const bool selected = (idx == static_cast<std::size_t>(_idx));
            const bool targeted = (idx == static_cast<std::size_t>(_selector));
            const bool active = selected || targeted;

            const auto color = selected ? enabled_color : targeted ? target_color : disabled_color;

            int left_pad = 0;
            int right_pad = 0;

            if (active)
            {
                if (!data::select_pre.empty())
                    left_pad = TextHelper::_text_bounding_box(data::select_pre).width;

                if (!data::select_post.empty())
                    right_pad = TextHelper::_text_bounding_box(data::select_post).width;
            }

            if (active && content_w > 0)
            {
                if (selected || !is_border)
                {
                    buffer.fill_blend(color, inner_x0, yoff, content_w, 1);
                }
                else
                {
                    const auto bbg = px::background{
                        data::border_horizontal_color.rf,
                        data::border_horizontal_color.gf,
                        data::border_horizontal_color.bf,
                        255
                    };

                    const int content_x0 = inner_x0;
                    const int content_x1 = inner_x0 + content_w - 1;
                    const int full = std::max(1, content_x1 - content_x0 + 1);
                    const int half = std::max(1, full / 2);

                    for (int j = 0; j < half; ++j)
                    {
                        const float t = static_cast<float>(j) / static_cast<float>(half);
                        const auto c = Pixel::interpolate(bbg, color, t);
                        buffer.at(content_x0 + j, yoff) = c;
                        buffer.at(content_x1 - j, yoff) = c;
                    }

                    buffer.at(content_x0 + (full / 2), yoff) = color;
                }

                if (is_border)
                {
                    buffer.at(0, yoff) =
                        data::border_horizontal_color.extend(data::right_connector).alpha(0.f, 1.f);
                    buffer.at(width - 1, yoff) =
                        data::border_horizontal_color.extend(data::left_connector).alpha(0.f, 1.f);
                }
            }

            if (active)
            {
                if (!data::select_pre.empty() && left_pad > 0)
                {
                    TextHelper::_render_text_simple(
                        data::select_pre,
                        d2::Pixel::combine(data::pre_color, colors::w::transparent),
                        Alignment::Left,
                        {inner_x0, yoff},
                        {left_pad, 1},
                        buffer
                    );
                }

                if (!data::select_post.empty() && right_pad > 0)
                {
                    TextHelper::_render_text_simple(
                        data::select_post,
                        d2::Pixel::combine(data::post_color, colors::w::transparent),
                        Alignment::Left,
                        {inner_x0 + content_w - right_pad, yoff},
                        {right_pad, 1},
                        buffer
                    );
                }
            }

            {
                const int text_x = inner_x0 + left_pad;
                const int text_w = std::max(0, content_w - left_pad - right_pad);

                if (text_w > 0)
                {
                    TextHelper::_render_text_simple(
                        data::options[idx],
                        color.alpha(0.f, 1.f),
                        text_alignment,
                        {text_x, yoff},
                        {text_w, 1},
                        buffer
                    );
                }
            }

            if (!data::disable_separator && (i + 1 < visible_count))
            {
                const int sep_y = yoff + 1;
                if (sep_y >= inner_y0 && sep_y <= inner_y1)
                {
                    for (int x = inner_x0; x < inner_x0 + content_w; ++x)
                        buffer.at(x, sep_y).blend(data::separator_color);
                }
            }
        }

        // Scrollbar
        if (_scrollbar && option_count > visible_count && inner_h > 0)
        {
            const int sx = width - basisx - 1;

            for (int y = inner_y0; y <= inner_y1; ++y)
                buffer.at(sx, y) = data::slider_background_color;

            const std::size_t total_items = option_count;
            const std::size_t visible_items = visible_count;
            const std::size_t scroll_range =
                total_items > visible_items ? total_items - visible_items : 0;

            const int track_h = inner_h;
            const int thumb_h = std::max(
                1,
                int((double(visible_items) / double(std::max<std::size_t>(1, total_items))) *
                    track_h)
            );

            const int thumb_range = std::max(0, track_h - thumb_h);

            const int thumb_y =
                inner_y0 +
                (scroll_range == 0
                     ? 0
                     : int((double(_scroll_offset) / double(scroll_range)) * thumb_range));

            for (int y = 0; y < thumb_h; ++y)
            {
                const int py = thumb_y + y;
                if (py >= inner_y0 && py <= inner_y1)
                    buffer.at(sx, py) = data::slider_color;
            }
        }
    }
} // namespace d2::dx
