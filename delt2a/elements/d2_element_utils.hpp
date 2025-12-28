#ifndef D2_ELEMENT_UTILS_HPP
#define D2_ELEMENT_UTILS_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"

namespace d2::dx::impl
{
    namespace tx
    {
        Element::BoundingBox _text_bounding_box(const string& value);
        Element::BoundingBox _paragraph_bounding_box(const string& value, int width = INT_MAX, int height = INT_MAX);

        void _render_text_simple(
            const string& value,
            Pixel color,
            style::IZText::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
            );
        void _render_text(
            const string& value,
            Pixel color,
            style::IZText::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
            );
        void _render_paragraph(
            const string& value,
            Pixel color,
            style::IZText::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
            );
    }
    namespace cnt
    {

    }

    template<typename Base>
    class TextHelper
    {
    protected:
        Element::BoundingBox _text_bounding_box(const string& value) const
        {
            return impl::tx::_text_bounding_box(value);
        }
        Element::BoundingBox _paragraph_bounding_box(const string& value, int width = INT_MAX, int height = INT_MAX) const
        {
            return impl::tx::_paragraph_bounding_box(value, width, height);
        }

        void _render_text_simple(
            const string& value,
            Pixel color,
            style::IZText::Alignment alignment,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        ) const
        {
            impl::tx::_render_text_simple(
                value,
                color,
                alignment,
                pos, box,
                buffer
            );
        }
        void _render_text_simple(
            const string& value,
            Pixel color,
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        ) const
        {
            impl::tx::_render_text_simple(
                value,
                color,
                style::IZText::Alignment::Left,
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
        ) const
        {
            impl::tx::_render_text(
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
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        ) const
        {
            impl::tx::_render_text(
                value,
                color,
                style::IZText::Alignment::Left,
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
        ) const
        {
            impl::tx::_render_paragraph(
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
            Element::Position pos,
            Element::BoundingBox box,
            PixelBuffer::View buffer
        )
        {
            impl::tx::_render_paragraph(
                value,
                color,
                style::IZText::Alignment::Left,
                pos, box,
                buffer
            );
        }
    };

    template<typename Base>
    class ContainerHelper
    {
    protected:
        Element::Position _border_base() const
        {
            auto& p = static_cast<const Base&>(*this);
            const auto is_border = p.container_options & Base::ContainerOptions::EnableBorder;
            if (is_border)
            {
                const auto bw = p.resolve_units(p.border_width);
                const auto et = !(p.container_options & Base::ContainerOptions::DisableBorderTop);
                const auto el = !(p.container_options & Base::ContainerOptions::DisableBorderLeft);

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
        Element::Position _border_base_inv() const
        {
            auto& p = static_cast<const Base&>(*this);
            const auto is_border = p.container_options & Base::ContainerOptions::EnableBorder;
            if (is_border)
            {
                const auto bw = p.resolve_units(p.border_width);
                const auto eb = !(p.container_options & Base::ContainerOptions::DisableBorderBottom);
                const auto er = !(p.container_options & Base::ContainerOptions::DisableBorderRight);

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
        void _render_border(PixelBuffer::View buffer) const
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
                const auto bw = p.resolve_units(p.border_width);

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
                    if (et && el) tl.blend(bch.extend(p.corners.top_left));
                    if (et && er) tr.blend(bch.extend(p.corners.top_right));
                    if (eb && el) bl.blend(bch.extend(p.corners.bottom_left));
                    if (eb && er) br.blend(bch.extend(p.corners.bottom_right));
                }
            }
        }
    };
}

#endif // D2_ELEMENT_UTILS_HPP
