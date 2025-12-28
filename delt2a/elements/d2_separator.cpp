#include "d2_separator.hpp"

namespace d2::dx
{
    void Separator::_frame_impl(PixelBuffer::View buffer)
    {
        auto color = Pixel::combine(data::foreground_color, data::background_color);
        if (!data::hide_body) buffer.fill(color);
        else buffer.fill(data::background_color);
        if (data::override_corners)
        {
            const auto [ width, height ] = box();
            if (_orientation == Align::Vertical)
            {
                color.v = data::corner_left;
                for (std::size_t i = 0; i < width; i++)
                    buffer.at(i) = color;
                color.v = data::corner_right;
                for (std::size_t i = 0; i < width; i++)
                    buffer.at(i, height - 1) = color;
            }
            else if (_orientation == Align::Horizontal)
            {
                color.v = data::corner_left;
                for (std::size_t i = 0; i < height; i++)
                    buffer.at(0, i) = color;
                color.v = data::corner_right;
                for (std::size_t i = 0; i < height; i++)
                    buffer.at(width - 1, i) = color;
            }
        }
    }

    Separator::Separator(
        const std::string& name,
        TreeState::ptr state,
        pptr parent
        ) : data(name, state, parent)
    {
        _orientation = Align::Horizontal;
        data::foreground_color = d2::px::foreground{ .v = d2::charset::switch_separator_horizontal };
        data::background_color = d2::px::background{ .a = 0 };
    }
    VerticalSeparator::VerticalSeparator(
        const std::string& name,
        TreeState::ptr state,
        pptr parent
        ) : Separator(name, state, parent)
    {
        _orientation = Align::Vertical;
        data::foreground_color = d2::px::foreground{ .v = d2::charset::switch_separator_vertical };
        data::background_color = d2::px::background{ .a = 0 };
    }
}
