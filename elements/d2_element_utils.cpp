#include "elements/d2_element_utils.hpp"
#include <core/platform/d2_text_layout.hpp>

namespace d2::dx::impl::tx
{
    BoundingBox _text_bounding_box(const string& value)
    {
        return text::Layout(value).bounds();
    }
    BoundingBox _paragraph_bounding_box(const string& value, int width, int height)
    {
        const auto result = text::Layout(value, {.width = std::max(0, width), .wrap = text::Wrap::Word}).bounds();
        return {std::min(result.width, std::max(0, width)), std::min(result.height, std::max(0, height))};
    }
    namespace
    {
        void render(const string& value, pixel color, style::IZText::Alignment alignment,
                    Position pos, BoundingBox box, PixelBuffer::View buffer,
                    text::Wrap wrap, bool newlines)
        {
            if (box.empty() || buffer.empty())
                return;
            const auto left = std::clamp(pos.x, 0, buffer.width());
            const auto top = std::clamp(pos.y, 0, buffer.height());
            const auto right = std::clamp<long long>(static_cast<long long>(pos.x) + box.width, left, buffer.width());
            const auto bottom = std::clamp<long long>(static_cast<long long>(pos.y) + box.height, top, buffer.height());
            text::Layout(value, {.width = box.width, .wrap = wrap,
                                .alignment = static_cast<text::Alignment>(alignment), .newlines = newlines})
                .draw(PixelBuffer::View(buffer, left, top, right - left, bottom - top), color,
                      {pos.x - left, pos.y - top});
        }
    }
    void _render_text_simple(const string& value, pixel color, style::IZText::Alignment alignment,
                             Position pos, BoundingBox box, PixelBuffer::View buffer)
    {
        render(value, color, alignment, pos, box, buffer, text::Wrap::None, false);
    }
    void _render_text(const string& value, pixel color, style::IZText::Alignment alignment,
                     Position pos, BoundingBox box, PixelBuffer::View buffer)
    {
        render(value, color, alignment, pos, box, buffer, text::Wrap::None, true);
    }
    void _render_paragraph(const string& value, pixel color, style::IZText::Alignment alignment,
                          Position pos, BoundingBox box, PixelBuffer::View buffer)
    {
        render(value, color, alignment, pos, box, buffer, text::Wrap::Word, true);
    }
} // namespace d2::dx::impl::tx
