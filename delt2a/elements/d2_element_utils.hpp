#ifndef D2_ELEMENT_UTILS_HPP
#define D2_ELEMENT_UTILS_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"

namespace d2::dx::impl
{
    template<typename Base>
    class TextHelper
    {
    protected:
        Element::BoundingBox _text_bounding_box(const string& value) const noexcept
        {
            Element::BoundingBox box{ 0, 0 };
            int x = 0;
            for (std::size_t i = 0;; i++)
            {
                if (i >= value.size() || value[i] == '\n')
                {
                    box.width = std::max(box.width, x);
                    x = 0;
                    box.height++;
                    if (i >= value.size())
                        break;
                    continue;
                }
                x++;
            }
            return box;
        }
        Element::BoundingBox _paragraph_bounding_box(const string& value, int width = INT_MAX,
                int height = INT_MAX) const noexcept
        {
            Element::BoundingBox box{ 0, 0 };
            for (std::size_t i = 0; i < value.size();)
            {
                while (i < value.size() && std::isspace(value[i]))
                {
                    if (value[i] == '\n')
                    {
                        if (++box.height >= height)
                            return box;
                        i++;
                    }
                    else
                        i++;
                }

                if (i >= value.size())
                    break;

                int line_end = i;
                int last_space = -1;
                int line_width = 0;

                while (line_end < value.size() && line_width < width)
                {
                    char ch = value[line_end];
                    if (ch == '\n')
                        break;
                    if (std::isspace(ch))
                        last_space = line_end;

                    line_end++;
                    line_width++;
                }

                if (line_width == width && last_space > int(i))
                {
                    line_end = last_space;
                }

                int actual_width = line_end - i;
                box.width = std::max(box.width, int(line_end - i));

                if (++box.height >= height)
                    break;

                i = line_end;
                while (i < value.size() && std::isspace(value[i]) && value[i] != '\n')
                    i++;
                if (i < value.size() && value[i] == '\n')
                    i++;
            }
            return box;
        }

        void _render_text_simple(
            const string& value,
            Pixel color,
            style::Text::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        ) const noexcept
        {
            if (alignment == style::Text::Alignment::Center)
            {
                pos.x = pos.x + ((box.width - value.size()) / 2);
                pos.y = pos.y + ((box.height - 1) / 2);
            }
            else if (alignment == style::Text::Alignment::Right)
            {
                pos.x = pos.x + (box.width - value.size() - 1);
            }

            const auto m = std::min(
                               buffer.width() - pos.x,
                               std::min(int(value.size()), box.width)
                           );
            for (std::size_t i = 0; i < m; i++)
            {
                auto& px = buffer.at(pos.x + i, pos.y);
                color.v = value[i] < 32 ?
                          char(248) : value[i];
                px.blend(color);
            }
        }
        void _render_text_simple(
            const string& value,
            Pixel color,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        ) const noexcept
        {
            _render_text_simple(
                value,
                color,
                style::Text::Alignment::Left,
                pos, box,
                buffer
            );
        }
        void _render_text(
            const string& value,
            Pixel color,
            style::Text::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        ) const noexcept
        {
            const auto absx = pos.x + box.width;
            const auto absy = pos.y + box.height;

            int x = 0;
            int y = 0;
            for (std::size_t i = 0; i < value.size(); i++)
            {
                if (value[i] == '\n')
                {
                    y++;
                    x = 0;
                    continue;
                }
                else
                {
                    const auto xp = x + pos.x;
                    const auto yp = y + pos.y;
                    if (yp >= absy || xp >= absx)
                        break;
                    auto& px = buffer.at(xp, yp);
                    color.v = value[i] < 32 ?
                              char(248) : value[i];
                    px.blend(color);
                    x++;
                }
            }
        }
        void _render_text(
            const string& value,
            Pixel color,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        ) const noexcept
        {
            _render_text(
                value,
                color,
                style::Text::Alignment::Left,
                pos, box,
                buffer
            );
        }
        void _render_paragraph(
            const string& value,
            Pixel color,
            style::Text::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        ) const noexcept
        {
            int y = pos.y;
            for (std::size_t i = 0; i < value.size();)
            {
                while (i < value.size() && std::isspace(value[i]))
                {
                    if (value[i] == '\n')
                    {
                        if (++y >= box.height)
                            return;
                        i++;
                    }
                    else
                        i++;
                }

                if (i >= value.size())
                    break;

                int line_end = i;
                int last_space = -1;
                int line_width = 0;

                while (line_end < value.size() && line_width < box.width)
                {
                    char ch = value[line_end];
                    if (ch == '\n')
                        break;
                    if (std::isspace(ch))
                        last_space = line_end;

                    line_end++;
                    line_width++;
                }

                if (line_width == box.width && last_space > int(i))
                {
                    line_end = last_space;
                }

                int basis = 0;
                int actual_width = line_end - i;
                if (alignment == style::Text::Alignment::Center)
                {
                    basis = pos.x + ((box.width - actual_width) / 2);
                }
                else if (alignment == style::Text::Alignment::Right)
                {
                    basis = pos.x + (box.width - actual_width - 1);
                }

                const auto m = std::min(
                                   int(value.size() - i),
                                   std::min(int(box.width - basis), actual_width)
                               );
                for (std::size_t j = 0; j < m; j++)
                {
                    const auto off = i + j;
                    if (pos.x + basis + j >= buffer.width())
                        break;
                    auto& px = buffer.at(pos.x + basis + j, pos.y + y);
                    color.v = value[off] < 32 ?
                              char(248) : value[off];
                    px.blend(color);
                }

                if (++y >= box.height)
                    break;

                i = line_end;
                while (i < value.size() && std::isspace(value[i]) && value[i] != '\n')
                    i++;
                if (i < value.size() && value[i] == '\n')
                    i++;
            }
        }
        void _render_paragraph(
            const string& value,
            Pixel color,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        ) noexcept
        {
            _render_paragraph(
                value,
                color,
                style::Text::Alignment::Left,
                pos, box,
                buffer
            );
        }
    };

    template<typename Base>
    class ContainerHelper
    {
    protected:
        Element::Position _border_base() const noexcept
        {
            auto& p = static_cast<const Base&>(*this);
            const bool is_border = p.container_options & Base::ContainerOptions::EnableBorder;
            if (is_border)
            {
                const auto bbox = p.box();
                const auto obw = p.resolve_units(p.border_width);
                const auto bw = ((obw > bbox.height || obw > bbox.width) ?
                                 std::min(bbox.width, bbox.height) : obw);

                const bool et = !(p.container_options & Base::ContainerOptions::DisableBorderTop);
                const bool el = !(p.container_options & Base::ContainerOptions::DisableBorderLeft);

                return
                {
                    el * bw,
                    et * bw
                };
            }
            return
            {
                0, 0
            };
        }
        Element::Position _border_base_inv() const noexcept
        {
            auto& p = static_cast<const Base&>(*this);
            const bool is_border = p.container_options & Base::ContainerOptions::EnableBorder;
            if (is_border)
            {
                const auto bbox = p.box();
                const auto obw = p.resolve_units(p.border_width);
                const auto bw = ((obw > bbox.height || obw > bbox.width) ?
                                 std::min(bbox.width, bbox.height) : obw);

                const bool eb = !(p.container_options & Base::ContainerOptions::DisableBorderBottom);
                const bool er = !(p.container_options & Base::ContainerOptions::DisableBorderRight);

                return
                {
                    er * bw,
                    eb * bw
                };
            }
            return
            {
                0, 0
            };
        }
        void _render_border(PixelBuffer::View buffer) const noexcept
        {
            auto& p = static_cast<const Base&>(*this);
            const auto bbox = p.box();

            const bool is_border = p.container_options & Base::ContainerOptions::EnableBorder;
            const bool is_topfix = p.container_options & Base::ContainerOptions::TopFix;
            const bool is_corners = p.container_options & Base::ContainerOptions::OverrideCorners;

            if (is_border)
            {
                const auto& bcv = p.border_vertical_color;
                const auto& bch = p.border_horizontal_color;
                const auto obw = p.resolve_units(p.border_width);
                const auto bw = ((obw > bbox.height || obw > bbox.width) ?
                                 std::min(bbox.width, bbox.height) : obw);

                const bool et = !(p.container_options & Base::ContainerOptions::DisableBorderTop);
                const bool eb = !(p.container_options & Base::ContainerOptions::DisableBorderBottom);
                const bool el = !(p.container_options & Base::ContainerOptions::DisableBorderLeft);
                const bool er = !(p.container_options & Base::ContainerOptions::DisableBorderRight);

                // Top
                if (et)
                    for (std::size_t i = 0; i < bw; i++)
                        for (std::size_t x = 0; x < bbox.width; x++)
                        {
                            buffer.at(x, i).blend(bch);
                        }

                // Bottom
                if (eb)
                    for (std::size_t i = 0; i < bw; i++)
                        for (std::size_t x = 0; x < bbox.width; x++)
                        {
                            buffer.at(x, bbox.height - 1 - i).blend(bch);
                        }

                // Left
                if (el)
                    for (std::size_t i = 0; i < bw; i++)
                        for (std::size_t y = is_topfix; y < bbox.height; y++)
                        {
                            buffer.at(i, y).blend(bcv);
                        }

                // Right
                if (er)
                    for (std::size_t i = 0; i < bw; i++)
                        for (std::size_t y = is_topfix; y < bbox.height; y++)
                        {
                            buffer.at(bbox.width - 1 - i, y).blend(bcv);
                        }

                // Corndogs

                if (is_corners)
                {
                    auto& tl = buffer.at(0);
                    auto& tr = buffer.at(bbox.width - 1);
                    auto& bl = buffer.at(0, bbox.height - 1);
                    auto& br = buffer.at(bbox.width - 1, bbox.height - 1);
                    if (et && el) tl.blend(Pixel{ .a = bch.a, .v = p.corners.top_left });
                    if (et && er) tr.blend(Pixel{ .a = bch.a, .v = p.corners.top_right });
                    if (eb && el) bl.blend(Pixel{ .a = bch.a, .v = p.corners.bottom_left });
                    if (eb && er) br.blend(Pixel{ .a = bch.a, .v = p.corners.bottom_right });
                }
            }
        }
    };
}

#endif // D2_ELEMENT_UTILS_HPP
