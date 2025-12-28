#include "d2_element_utils.hpp"
#include "../d2_platform_string.hpp"

namespace d2::dx::impl
{
    namespace tx
    {
        Element::BoundingBox _text_bounding_box(const string& value)
        {
            Element::BoundingBox box{ 0, 0 };
            if (value.empty())
                return box;
            int x = 0;
            for (decltype(auto) it : StringIterator(value))
            {
                if (it.size() == 1 && it[0] == '\n')
                {
                    box.width = std::max(x, box.width);
                    box.height++;
                }
                else
                    x++;
            }
            box.width = std::max(x, box.width);
            box.height++;
            return box;
        }
        Element::BoundingBox _paragraph_bounding_box(const string& value, int width, int height)
        {
            Element::BoundingBox box{ 0, 0 };
            for (auto it = StringIterator(value); !it.is_end();)
            {
                for (; !it.is_end() && it.is_space() || it.is_endline(); it.increment())
                {
                    if (it.is_endline() && (++box.height >= height))
                        return box;
                }
                if (it.is_end())
                    break;

                auto lit = it;
                auto last_space = it.end();
                int last_space_width = 0;
                int line_width = 0;

                for (; !lit.is_end() && line_width < width && !lit.is_endline(); lit.increment())
                {
                    line_width++;
                    if (lit.is_space())
                    {
                        last_space = lit;
                        last_space_width = line_width;
                    }
                }

                if (line_width > width && !last_space.is_end())
                {
                    line_width = last_space_width;
                    lit = last_space;
                }

                box.width = std::max(box.width, line_width);
                if (++box.height >= height)
                    break;

                it = lit;
                for (; !it.is_end() && it.is_space() && !it.is_endline(); it.increment());
            }
            return box;
        }

        void _render_text_simple(
            const string& value,
            Pixel color,
            style::IZText::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
            )
        {
            return _render_paragraph(
                value,
                color,
                alignment,
                pos, box,
                buffer
            );
        }
        void _render_text(
            const string& value,
            Pixel color,
            style::IZText::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
            )
        {
            return _render_paragraph(
                value,
                color,
                alignment,
                pos, box,
                buffer
            );
        }
        void _render_paragraph(
            const string& value,
            Pixel color,
            style::IZText::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
            )
        {
            int y = 0;
            for (auto it = StringIterator(value); !it.is_end();)
            {
                for (; !it.is_end() && it.is_space() || it.is_endline(); it.increment())
                {
                    if (it.is_endline() && (++y >= box.height))
                        return;
                }
                if (it.is_end())
                    break;

                auto lit = it;
                auto last_space = it.end();
                int last_space_width = 0;
                int line_width = 0;

                for (; !lit.is_end() && line_width < box.width && !lit.is_endline(); lit.increment())
                {
                    line_width++;
                    if (lit.is_space())
                    {
                        last_space = lit;
                        last_space_width = line_width;
                    }
                }

                if (line_width > box.width && !last_space.is_end())
                {
                    line_width = last_space_width;
                    lit = last_space;
                }

                int basis = 0;
                if (alignment == style::IZText::Alignment::Center)
                {
                    basis = (box.width - line_width) / 2;
                }
                else if (alignment == style::IZText::Alignment::Right)
                {
                    basis = box.width - line_width - 1;
                }

                const auto m = std::min(int(box.width - basis), line_width);
                for (std::size_t j = 0; !it.is_end() && j < m; it.increment(), j++)
                {
                    if (pos.x + basis + j >= buffer.width())
                        break;
                    auto& px = buffer.at(pos.x + basis + j, pos.y + y);
                    px.blend(color);
                    px.v = global_extended_code_page.write(it.current());
                }
                if (++y >= box.height)
                    break;

                it = lit;
                for (; !it.is_end() && it.is_space() && !it.is_endline(); it.increment());
            }
        }
    }
}
