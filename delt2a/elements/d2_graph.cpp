#include "d2_graph.hpp"

namespace d2::dx::graph
{
    void CyclicVerticalBar::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(Pixel::combine(data::foreground_color, data::background_color));
        ContainerHelper::_render_border(buffer);

        if (_data.empty())
            return;
        const auto [ width, height ] = box();
        const auto [ basex, basey ] = ContainerHelper::_border_base();
        const auto [ ibasex, ibasey ] = ContainerHelper::_border_base_inv();
        const auto [ xres, yres ] = _resolution();
        if (xres == 0 || yres == 0)
            return;

        const auto inner_w = (width > basex + ibasex) ? (width - basex - ibasex) : 0;
        const auto inner_h = (height > basey + ibasey) ? (height - basey - ibasey) : 0;
        if (inner_w == 0 || inner_h == 0)
            return;

        const auto n = _data.size();
        const auto slice = std::max<std::size_t>(1, n / std::max<std::size_t>(1, xres));

        if (data::high_resolution)
        {
            std::vector<int> cells(inner_w * inner_h, 0x00);
            std::vector<std::size_t> col_h(inner_w, 0);

            const auto quarters_cap = inner_h * 4;

            for (std::size_t i = 0; i < xres; i++)
            {
                const auto abs = (_start_offset + slice * i) % n;

                auto sum = 0.f;
                for (std::size_t k = 0; k < slice; k++)
                {
                    const std::size_t idx = (abs + k) % n;
                    sum += _data[idx];
                }

                auto data_point = sum / static_cast<float>(slice);
                if (!std::isfinite(data_point))
                    continue;
                data_point = std::clamp(data_point, 0.f, 1.f);

                const auto x = static_cast<std::size_t>(i / 2);
                if (x >= inner_w)
                    continue;

                const auto total_quarters = std::min<std::size_t>(
                    static_cast<std::size_t>(std::floor(static_cast<float>(quarters_cap) * data_point + 1e-6f)),
                    quarters_cap
                );

                const auto full = total_quarters / 4;
                const auto rem = total_quarters % 4;
                const auto yoff = std::min<std::size_t>(full + (rem ? 1 : 0), inner_h);

                if (yoff == 0)
                    continue;

                col_h[x] = std::max(col_h[x], yoff);

                for (std::size_t j = 0; j < yoff; j++)
                {
                    auto flags = 0x00;

                    if (j < full)
                    {
                        if (i % 2 == 0)
                        {
                            flags |=
                                charset::TopLeft |
                                charset::MidTopLeft |
                                charset::MidBotLeft |
                                charset::BotLeft;
                        }
                        else
                        {
                            flags |=
                                charset::TopRight |
                                charset::MidTopRight |
                                charset::MidBotRight |
                                charset::BotRight;
                        }
                    }
                    else if (j == full && rem != 0)
                    {
                        const auto dot = rem - 1;
                        if (i % 2 == 0)
                        {
                            switch (dot)
                            {
                            case 3: flags |= charset::TopLeft; [[fallthrough]];
                            case 2: flags |= charset::MidTopLeft; [[fallthrough]];
                            case 1: flags |= charset::MidBotLeft; [[fallthrough]];
                            case 0: flags |= charset::BotLeft; break;
                            }
                        }
                        else
                        {
                            switch (dot)
                            {
                            case 3: flags |= charset::TopRight; [[fallthrough]];
                            case 2: flags |= charset::MidTopRight; [[fallthrough]];
                            case 1: flags |= charset::MidBotRight; [[fallthrough]];
                            case 0: flags |= charset::BotRight; break;
                            }
                        }
                    }
                    else
                    {
                        continue;
                    }

                    const auto y = data::invert ? j : (inner_h - j - 1);
                    cells[
                        static_cast<std::size_t>(y) * inner_w +
                        static_cast<std::size_t>(x)
                    ] |= flags;
                }
            }

            for (std::size_t y = 0; y < inner_h; y++)
            {
                for (std::size_t x = 0; x < inner_w; x++)
                {
                    const auto flags = cells[y * inner_w + x];
                    if (flags == 0x00)
                        continue;

                    const auto j = data::invert ? y : (inner_h - y - 1);
                    const auto h = col_h[x];
                    const auto t = (h <= 1) ? 1.f : static_cast<float>(std::min<std::size_t>(j, h - 1)) / static_cast<float>(h - 1);

                    auto px = Pixel::interpolate(
                        data::min_color,
                        data::max_color,
                        t
                    );

                    px.v = charset::cell(flags);
                    buffer.at(
                        x + basex,
                        y + basey
                    ).blend(px);
                }
            }

            return;
        }

        for (std::size_t i = 0; i < xres; i++)
        {
            const auto abs = (_start_offset + slice * i) % n;

            auto sum = 0.f;
            for (std::size_t k = 0; k < slice; k++)
            {
                const std::size_t idx = (abs + k) % n;
                sum += _data[idx];
            }

            auto data_point = sum / static_cast<float>(slice);
            if (!std::isfinite(data_point))
                continue;
            data_point = std::clamp(data_point, 0.f, 1.f);

            const auto x = i;
            if (x >= inner_w)
                continue;

            const auto yabs = static_cast<float>(yres) * data_point;
            auto yoff = static_cast<std::size_t>(std::floor(yabs));
            if (yoff == 0)
                continue;

            yoff = std::min<std::size_t>(yoff, inner_h);
            for (std::size_t j = 0; j < yoff; j++)
            {
                const auto t = (yoff <= 1) ? 1.f : static_cast<float>(j) / static_cast<float>(yoff - 1);
                auto px = Pixel::interpolate(
                    data::min_color,
                    data::max_color,
                    t
                );

                px.v = data::min_color.v;

                const auto y = data::invert ? (basey + j) : (height - ibasey - j - 1);
                if (y < basey || y >= height - ibasey)
                    continue;

                buffer.at(
                    x + basex,
                    y
                ).blend(px);
            }
        }
    }
    std::pair<int, int> CyclicVerticalBar::_resolution() const
    {
        const auto [ width, height ] = box();
        const auto [ basex, basey ] = ContainerHelper::_border_base();
        const auto [ ibasex, ibasey ] = ContainerHelper::_border_base_inv();
        const auto yres = height - (basey + ibasey);
        const auto xres = width - (basex + ibasex);
        if (data::automatic_resolution)
            return {
                xres * (data::high_resolution ? 2 : 1),
                yres * (data::high_resolution ? 4 : 1)
            };
        return { data::resolution, yres };
    }
    void CyclicVerticalBar::clear()
    {
        _data.clear();
        _data.shrink_to_fit();
        _signal_write(Style);
    }
    void CyclicVerticalBar::push(float data_point)
    {
        const auto [ xres, yres ] = _resolution();
        if (_data.size() != xres)
        {
            if (_start_offset >= xres)
                _start_offset = xres - 1;
            _data.resize(xres);
        }
        _data[_start_offset++] = std::clamp(data_point, 0.f, 1.f);
        if (_start_offset >= _data.size())
            _start_offset = 0;
        _signal_write(Style);
    }
    void CyclicVerticalBar::rescale(float scale)
    {
        if (scale == 1.f) return;
        std::transform(_data.begin(), _data.end(), _data.begin(), [&](float point) {
            return scale * point;
        });
    }
    void CyclicVerticalBar::rescale(float scale, float trans)
    {
        if (scale == 1.f && trans == 0.f) return;
        std::transform(_data.begin(), _data.end(), _data.begin(), [&](float point) {
            return scale * point + trans;
        });
    }

    float NodeWaveGraph::_lerp(float t, float a, float b)
    {
        return a + t * (b - a);
    }

    void NodeWaveGraph::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked)
        {
            if (value)
            {
                _focused_node = nullptr;
                _start_point = mouse_object_space().x;
            }
            else
            {
                const auto x = mouse_object_space().x;
                _start_point = std::min<std::size_t>(x, _start_point);
                _end_point = std::max<std::size_t>(x, _start_point);
                if (x == _start_point)
                {
                    const auto start = float(_start_point) / layout(Element::Layout::Width);
                    for (decltype(auto) it : _nodes)
                        if (it.start > it.start && start < it.end)
                        {
                            _focused_node = &it;
                            break;
                        }
                    _start_point = ~0ull;
                }
                _signal_write(Style);
            }
        }
        else if (state == State::Focused && !value)
        {
            _focused_node = nullptr;
        }
    }
    void NodeWaveGraph::_frame_impl(d2::PixelBuffer::View buffer)
    {
        buffer.fill(data::background_color);

        const auto color_i = d2::px::combined::combine(data::node_color_i, data::background_color);
        const auto color_a = d2::px::combined::combine(data::node_color_a, data::background_color);
        const auto color_b = d2::px::combined::combine(data::node_color_b, data::background_color);
        const auto color_focused = d2::px::combined::combine(data::node_color_focused, data::background_color);
        const auto height = 4 * buffer.height();
        const auto width = 2 * buffer.width() * (frequency / resolution);
        const auto start = _offset;
        const auto abs_start = _offset * width;
        auto color = color_i;
        bool color_ctr = false;
        std::size_t x = 0;
        float prev_end = 0.f;

        auto inc = [&]() {
            auto rel = (x + abs_start) / width;
            x++;
            rel = (x + abs_start) / width;
        };
        auto write = [&, pmask = std::size_t(0), prev_cell = int(-1)] (float y) mutable {
            auto t = (y + 1.f) * 0.5f;
            if (t <= 0.f)
                t = 0.f;
            if (t >= 1.f)
                t = std::nextafter(1.f, 0.f);

            const auto off = static_cast<std::size_t>(t * static_cast<float>(height));
            const auto cell_u = off / 4;
            const auto dot    = off % 4;

            const auto cell = static_cast<int>(cell_u);
            const auto h = static_cast<int>(buffer.height());
            const auto row  = h - cell - 1;
            if (row < 0 || row >= h)
            {
                inc();
                return;
            }

            std::size_t right = 0;
            std::size_t left = 0;

            if ((x & 1) == 0)
            {
                switch (dot)
                {
                case 0: left = d2::charset::BotLeft; break;
                case 1: left = d2::charset::MidBotLeft; break;
                case 2: left = d2::charset::MidTopLeft; break;
                case 3: left = d2::charset::TopLeft; break;
                }

                pmask = left;
                prev_cell = cell;

                buffer.at(x / 2, row) = color;
            }
            else
            {
                switch (dot)
                {
                case 0: right = d2::charset::BotRight; break;
                case 1: right = d2::charset::MidBotRight; break;
                case 2: right = d2::charset::MidTopRight; break;
                case 3: right = d2::charset::TopRight; break;
                }

                if (prev_cell == cell)
                {
                    const std::size_t mask = pmask | right;
                    buffer.at(x / 2, row) = color;
                    buffer.at(x / 2, row).v =
                        d2::global_extended_code_page.write(d2::charset::cell(mask));
                }
                else
                {
                    if (prev_cell >= 0 && prev_cell < h && pmask)
                    {
                        buffer.at(x / 2, h - prev_cell - 1) = color;
                        buffer.at(x / 2, h - prev_cell - 1).v =
                            d2::global_extended_code_page.write(d2::charset::cell(pmask));
                    }
                    buffer.at(x / 2, row) = color;
                    buffer.at(x / 2, row).v =
                        d2::global_extended_code_page.write(d2::charset::cell(right));
                }
                pmask = 0;
                prev_cell = -1;
            }
            inc();
        };

        for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
        {
            const auto zero = it->interpolator(it->phase, it->frequency, 0, it->parameters);
            auto rel = (x + abs_start) / width;

            if (getstate(State::Focused))
            {
                color = color_focused;
                while (x / 2 >= _start_point / buffer.width() &&
                       x / 2 < _end_point / buffer.width())
                    write(0);
            }

            if (rel < it->start)
            {
                color = color_i;
                while (x / 2 < buffer.width() && rel < it->start)
                    write(_lerp(it->start - rel, prev_end, zero));
            }
            if (x / 2 >= buffer.width())
                return;

            color = _focused_node && _focused_node->id == it->id ?
                        color_focused : (color_ctr ? color_a : color_b);
            while (x / 2 < buffer.width() && rel < it->end)
                write(it->interpolator(it->phase, it->frequency, (float(x) / width) - start, it->parameters));
            if (x / 2 >= buffer.width())
                return;
            color_ctr = !color_ctr;
        }

        if (getstate(State::Focused) && _nodes.empty() && _start_point != ~0ull)
        {
            color = color_focused;
            x = _start_point * 2;
            while (x / 2 < buffer.width() && x / 2 < _end_point)
                write(0.f);
        }
    }

    std::size_t NodeWaveGraph::push()
    {
        const auto width = box().width;
        if (_start_point != ~0ull)
        {
            return push(
                _start_point / width,
                _end_point / width
                );
        }
        return ~0ull;
    }
    std::size_t NodeWaveGraph::push(float start, float end)
    {
        if (_nodes.empty())
        {
            _nodes.push_front({ _id++, start, end });
        }
        auto closest = _nodes.begin();
        for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
        {
            if (start > it->start &&
                it->start < closest->start)
            {
                closest = it;
            }
        }

        if (end < closest->end)
        {
            auto cpy = closest;
            cpy->end = start;
            _nodes.insert(++closest, { _id++, start, end });
            _nodes.insert(++closest, *cpy)
                ->start = end;
        }
        else
        {
            _nodes.insert(closest, { _id++, start, end });
        }
        _signal_write(Style);
        return _id - 1;
    }
    void NodeWaveGraph::pop(std::size_t id)
    {
        _signal_write(Style);
    }
    void NodeWaveGraph::set_parameters(std::size_t id, const NodePair& node)
    {
        auto it = std::find_if(_nodes.begin(), _nodes.end(), [&](const auto& node) {
            return node.id == id;
        });
        it->phase = node.phase;
        it->frequency = node.frequency;
        it->parameters = node.parameters;
        it->interpolator = node.interpolator;
        _signal_write(Style);
    }
}
