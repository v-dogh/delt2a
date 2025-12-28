#include "d2_text.hpp"

namespace d2::dx
{
    void Text::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        if (element.get() == this &&
                (data::width.getunits() == Unit::Auto ||
                 data::height.getunits() == Unit::Auto) &&
                (prop == Value ||
                 prop == TextOptions ||
                 prop == TextAlignment ||
                 prop == TextPreserveWordBoundaries ||
                 prop == TextHandleNewlines ||
                 prop == initial_property))
        {
            if (data::text_options & HandleNewlines)
            {
                if (data::text_options & PreserveWordBoundaries)
                {
                    _text_dimensions = TextHelper::_paragraph_bounding_box(data::text,
                                       (data::width.getunits() == Unit::Auto ? INT_MAX : resolve_units(data::width)),
                                       (data::height.getunits() == Unit::Auto ? INT_MAX : resolve_units(data::height)));
                }
                else
                {
                    _text_dimensions = TextHelper::_text_bounding_box(data::text);
                }
            }
            else
            {
                _text_dimensions =
                {
                    static_cast<int>((data::width.getunits() == Unit::Auto) ? data::text.size() : resolve_units(data::width)),
                    static_cast<int>((data::height.getunits() == Unit::Auto) ? 1 : resolve_units(data::height))
                };
            }
            _signal_write(WriteType::Dimensions);
        }
    }
    Unit Text::_layout_impl(Element::Layout type) const
    {
        switch (type)
        {
            case Element::Layout::X: return data::x;
            case Element::Layout::Y: return data::y;
            case Element::Layout::Width:
                if (data::width.getunits() == Unit::Auto)
                    return HDUnit(_text_dimensions.width);
                return data::width;
            case Element::Layout::Height:
                if (data::height.getunits() == Unit::Auto)
                    return VDUnit(_text_dimensions.height);
                return data::height;
        }
    }
    void Text::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(data::background_color);
        if (data::text_options & HandleNewlines)
        {
            if (data::text_options & PreserveWordBoundaries)
            {
                TextHelper::_render_paragraph(
                    data::text,
                    Pixel::combine(data::foreground_color, data::background_color),
                    data::alignment,
                    {}, box(),
                    buffer
                );
            }
            else
            {
                TextHelper::_render_text(
                    data::text,
                    Pixel::combine(data::foreground_color, data::background_color),
                    data::alignment,
                    {}, box(),
                    buffer
                );
            }
        }
        else
        {
            TextHelper::_render_text_simple(
                data::text,
                Pixel::combine(data::foreground_color, data::background_color),
                {}, box(),
                buffer
            );
        }
    }

    Text::Text(
        const std::string& name,
        TreeState::ptr state,
        pptr parent
    ) : data(name, state, parent)
    {
        data::background_color.a = 0;
    }
}
