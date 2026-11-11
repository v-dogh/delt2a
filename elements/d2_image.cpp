#include "elements/d2_image.hpp"

namespace d2::dx
{
    void Image::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(pixel::combine(data::foreground_color, data::background_color));
        if (data::source)
        {
            const auto begin = _border_base(), end = _border_base_inv();
            const auto left = std::clamp(begin.x, 0, buffer.width());
            const auto top = std::clamp(begin.y, 0, buffer.height());
            const auto width = std::max(0, buffer.width() - left - std::clamp(end.x, 0, buffer.width()));
            const auto height = std::max(0, buffer.height() - top - std::clamp(end.y, 0, buffer.height()));
            if (width && height)
                buffer.image(data::source, {left, top, width, height}, data::source_rect);
        }
        ContainerHelper::_render_border(buffer);
    }
}
