#include "d2_image.hpp"

namespace d2::dx
{
    void Image::_frame_impl(PixelBuffer::View buffer)
    {
        if (!data::path.empty())
        {
            buffer.fill(px::combined{ .a = 0, .v = ' ' });
            ContainerHelper::_render_border(buffer);
            const auto id = state()->context()->output()->image_id(data::path);
            const auto base = data::container_options & EnableBorder ? 1 : 0;
            buffer.at(base, base) = sys::output::image_constant;
            buffer.at(base + 1, base).v = static_cast<unsigned char>(id);
            buffer.at(base + 2, base).v = static_cast<unsigned char>((0xF0   & id) >> 1);
            buffer.at(base + 3, base).v = static_cast<unsigned char>((0xF00  & id) >> 2);
            buffer.at(base + 4, base).v = static_cast<unsigned char>((0xF000 & id) >> 3);
        }
        else
            buffer.fill(Pixel::combine(data::foreground_color, data::background_color));
    }
}
