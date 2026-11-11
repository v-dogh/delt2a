#include "elements/d2_text.hpp"

namespace d2::dx
{
    text::Options Text::_options(int width) const
    {
        return {.width = std::max(0, width),
                .wrap = (data::text_options & PreserveWordBoundaries) ? text::Wrap::Word : text::Wrap::None,
                .alignment = static_cast<text::Alignment>(data::alignment),
                .newlines = bool(data::text_options & HandleNewlines)};
    }
    void Text::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        if (element.get() != this)
            return;
        if (prop == Value || prop == TextOptions || prop == TextAlignment ||
            prop == TextPreserveWordBoundaries || prop == TextHandleNewlines ||
            prop == initial_property)
            _signal_write(WriteType::Dimensions);
        else if ((type & DimensionsWidthUpdated) && data::height.getunits() == Unit::Auto)
            _signal_write(WriteType::LayoutHeight);
    }
    Unit Text::_layout_impl(Element::Layout type) const
    {
        switch (type)
        {
        case Element::Layout::X:
            return data::x;
        case Element::Layout::Y:
            return data::y;
        case Element::Layout::Width:
            if (data::width.getunits() != Unit::Auto)
                return data::width;
            _measurement.reset(data::text, _options(INT_MAX));
            return Unit(_measurement.bounds().width + data::width.raw());
        case Element::Layout::Height:
            if (data::height.getunits() != Unit::Auto)
                return data::height;
            _measurement.reset(data::text, _options(layout(Element::Layout::Width)));
            return Unit(_measurement.bounds().height + data::height.raw());
        }
        return {};
    }
    const text::Layout* Text::text_layout() const
    {
        const auto previous = _text_layout.revision();
        _text_layout.reset(data::text, _options(box().width));
        if (previous != _text_layout.revision())
            const_cast<Text*>(this)->_text_selection.reset();
        return &_text_layout;
    }
    void Text::select_text(std::optional<text::Range> range)
    {
        if (range)
        {
            const auto count = text_layout()->size();
            range->begin = std::min(range->begin, count);
            range->end = std::min(range->end, count);
        }
        if (_text_selection == range)
            return;
        _text_selection = range;
        _signal_write(WriteType::Style);
    }
    void Text::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(data::background_color);
        text_layout()->draw(buffer, pixel::combine(data::foreground_color, data::background_color),
                            {}, selectable() ? _text_selection : std::nullopt, data::highlight_mask);
    }
    Text::Text(const std::string& name, TreeState::ptr state) : data(name, state)
    {
        data::background_color.a = 0;
    }
} // namespace d2::dx
