#include "d2_switch.hpp"

namespace d2::dx
{
    bool Switch::_provides_input_impl() const noexcept
    {
        return true;
    }

    void Switch::_signal_write_impl(write_flag type, unsigned int prop, ptr element) noexcept
    {
        if (element.get() == this &&
                (data::width.getunits() == Unit::Auto ||
                 data::height.getunits() == Unit::Auto) &&
                prop == BorderWidth ||
                prop == Options)
        {
            const auto [ xbasis, ybasis ] = ContainerHelper::_border_base();
            const auto [ ixbasis, iybasis ] = ContainerHelper::_border_base_inv();

            int perfect_width = 0;
            for (decltype(auto) it : data::options)
            {
                perfect_width = std::max(
                                    perfect_width,
                                    int(it.size())
                                );
            }

            _perfect_dimensions.width =
                (perfect_width * data::options.size()) +
                (data::options.size() - 1) + (xbasis + ixbasis) +
                data::width.raw();
            _perfect_dimensions.height = 1 + (ybasis + iybasis);

            _signal_write(WriteType::Dimensions);
        }
    }
    Unit Switch::_layout_impl(Element::Layout type) const noexcept
    {
        switch (type)
        {
            case Element::Layout::X: return data::x;
            case Element::Layout::Y: return data::y;
            case Element::Layout::Width:
                if (data::width.getunits() == Unit::Auto)
                    return HDUnit(_perfect_dimensions.width);
                return data::width;
            case Element::Layout::Height:
                if (data::height.getunits() == Unit::Auto)
                    return VDUnit(_perfect_dimensions.height);
                return data::height;
        }
    }

    void Switch::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked && value)
        {
            const auto [ width, height ] = box();
            const auto [ x, y ] = mouse_object_space();
            const auto [ basisx, basisy ] = ContainerHelper::_border_base();
            const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();
            const auto w = width - basisx - ibasisx;
            const auto cw =
                ((w - (data::options.size() - 1)) /
                 (data::options.size())) + 1;

            if (cw > 0)
            {
                const auto idx = x / cw;
                const auto midx = x % cw;

                if (midx != (cw - 1) &&
                        idx != _idx)
                {
                    _idx = idx;
                    _signal_write(Style);
                    _submit();
                }
            }
        }
        else if (state == State::Keynavi)
        {
            _signal_write(Style);
        }
    }
    void Switch::_event_impl(IOContext::Event ev)
    {
        if (ev == IOContext::Event::KeyInput)
        {
            if (context()->input()->is_pressed(sys::SystemInput::Tab))
            {
                _idx = ++_idx % data::options.size();
                _signal_write(Style);
                _submit();
            }
        }
    }
    void Switch::_frame_impl(PixelBuffer::View buffer) noexcept
    {
        buffer.fill(
            Pixel::combine(
                data::foreground_color,
                data::background_color
            )
        );

        const auto [ width, height ] = box();
        const auto [ basisx, basisy ] = ContainerHelper::_border_base();
        const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();
        const int width_slice =
            (width - (data::options.size() - 1) - (basisx + ibasisx)) /
            data::options.size();

        const auto disabled_color = Pixel::combine(
                                        data::disabled_foreground_color,
                                        data::disabled_background_color
                                    );
        const auto enabled_color = Pixel::combine(
                                       data::enabled_foreground_color,
                                       (getstate(Keynavi) ? data::focused_color : data::enabled_background_color)
                                   );
        for (std::size_t i = 0; i < data::options.size(); i++)
        {
            const auto xoff = int(basisx + ((width_slice + 1) * i));
            TextHelper::_render_text_simple(
                data::options[i],
                (i == _idx) ? enabled_color : disabled_color,
                style::Text::Alignment::Center,
            { xoff, basisy }, { width_slice, 1 },
            buffer
            );
            if (i < data::options.size() - 1)
                buffer.at(xoff + width_slice, basisy)
                      .blend(data::separator_color);
        }

        ContainerHelper::_render_border(buffer);
    }
    void Switch::_submit()
    {
        if (data::on_change != nullptr)
            data::on_change(shared_from_this(), _idx);
    }
    void Switch::reset(int idx) noexcept
    {
        if (_idx != idx)
        {
            _idx = idx;
            _signal_write(Style);
            _submit();
        }
    }
}
