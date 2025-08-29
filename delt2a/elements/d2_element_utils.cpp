#include "d2_element_utils.hpp"

namespace d2::dx::impl
{
    namespace tx
    {
        Element::BoundingBox _text_bounding_box(const string& value) noexcept
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
        Element::BoundingBox _paragraph_bounding_box(const string& value, int width, int height) noexcept
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
            ) noexcept
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
        void _render_text(
            const string& value,
            Pixel color,
            style::Text::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
            ) noexcept
        {
            const auto absx = pos.x + box.width;
            const auto absy = pos.y + box.height;

            int x = 0;
            int y = 0;
            if (alignment == style::Text::Alignment::Center ||
                alignment == style::Text::Alignment::Right)
            {
                for (std::size_t i = 0; i < value.size();)
                {
                    int j = i;
                    for (; j < value.size() && value[j] != '\n'; j++);

                    const auto xoff = (alignment == style::Text::Alignment::Center) ?
                                          static_cast<std::size_t>(std::max(0, (box.width - j) / 2)) :
                                          static_cast<std::size_t>(std::max(0, box.width - j - 1));

                    while (i < value.size() && value[i] != '\n')
                    {
                        const auto xp = x + xoff + pos.x;
                        const auto yp = y + pos.y;
                        if (yp >= absy || xp >= absx)
                            break;
                        auto& px = buffer.at(xp, yp);
                        color.v = value[i] < 32 ?
                                      char(248) : value[i];
                        px.blend(color);
                        x++;
                        i++;
                    }

                    y++;
                    x = 0;
                }
            }
            else
            {
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
        }
        void _render_paragraph(
            const string& value,
            Pixel color,
            style::Text::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
            ) noexcept
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
    }
}
