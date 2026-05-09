#include "d2_slider.hpp"

namespace d2::dx
{
    Slider::Slider(const std::string& name, TreeState::ptr state) : data(name, state)
    {
        data::width = 0.0_auto;
        data::height = 1.0_px;
        data::foreground_color = px::foreground{.v = charset::slider_horizontal};
    }

    void Slider::_submit()
    {
        const auto rel = relative_value();
        const auto abs = absolute_value();
        if (data::on_submit != nullptr)
            data::on_submit(std::static_pointer_cast<Slider>(shared_from_this()), rel, abs);
    }
    bool Slider::_provides_input_impl() const
    {
        return true;
    }

    void Slider::_state_change_impl(State state, bool value)
    {
        if (state == State::Clicked)
        {
            if (value)
            {
                const auto pos = _aligned_mouse();
                const auto sw = resolve_units(data::slider_width);
                const auto abs = _relative_to_absolute(_slider_pos);
                if (pos >= abs && pos < abs + sw)
                    _slider_spos = pos - abs;
                else
                    _slider_spos = sw / 2;
            }
            else
            {
                _slider_spos = 0;
            }
        }
        else if (state == State::Keynavi)
        {
            _signal_write(Style);
        }
    }
    void Slider::_event_impl(in::InputFrame& frame)
    {
        if (data::max == data::min)
            return;

        bool write = false;
        float npos = 0;
        if ((frame.had_event(in::Event::MouseMovement) &&
             frame.active(in::mouse::Left, in::mode::Hold) && getstate(Focused)) ||
            getstate(Clicked))
        {
            const auto pos = _aligned_mouse();
            npos = _absolute_to_relative(std::clamp(int(pos - _slider_spos), 0, _aligned_width()));
            write = true;
        }
        if (frame.had_event(in::Event::ScrollWheelMovement) && getstate(Hovered))
        {
            const auto scroll = frame.scroll_delta().second;
            if (scroll)
            {
                npos = std::clamp(_slider_pos + data::step * scroll, data::min, data::max);
                write = true;
            }
        }

        if (frame.had_event(in::Event::KeyInput))
        {
            if (frame.active(in::special::ArrowLeft, in::mode::Hold) ||
                frame.active(in::special::ArrowDown, in::mode::Hold))
            {
                if (_slider_pos - data::step > data::min)
                {
                    _slider_pos -= data::step;
                    _signal_write(Style);
                    _submit();
                }
            }
            else if (
                frame.active(in::special::ArrowRight, in::mode::Hold) ||
                frame.active(in::special::ArrowUp, in::mode::Hold)
            )
            {
                if (_slider_pos + data::step < data::max)
                {
                    _slider_pos += data::step;
                    _signal_write(Style);
                    _submit();
                }
            }
        }

        if (write && npos != _slider_pos)
        {
            _slider_pos = npos;
            _signal_write(WriteType::Style);
            _submit();
        }
    }

    void Slider::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(Pixel::combine(data::foreground_color, data::background_color));

        const auto sw = resolve_units(data::slider_width);
        auto sc = getstate(State::Keynavi) ? px::combined(data::focused_color) : data::slider_color;
        sc.v = data::slider_color.v;
        const auto abs = _relative_to_absolute(_slider_pos);
        for (std::size_t i = 0; i < sw; i++)
        {
            if (abs + i >= buffer.width())
                break;
            buffer.at(abs + i) = sc;
        }
    }

    int Slider::_aligned_mouse()
    {
        return mouse_object_space().x;
    }
    int Slider::_aligned_width()
    {
        return layout(Element::Layout::Width) - resolve_units(data::slider_width);
    }
    int Slider::_relative_to_absolute(float rel)
    {
        const auto range = data::max - data::min;
        const auto aligned = rel - data::min;
        if (range)
            return static_cast<int>((aligned / range) * _aligned_width());
        return 0;
    }
    float Slider::_absolute_to_relative(int abs)
    {
        const auto range = data::max - data::min;
        if (range)
        {
            return std::clamp(
                ((float(abs) / _aligned_width()) * range) + data::min, data::min, data::max
            );
        }
        return 0;
    }

    void Slider::reset_absolute(float value)
    {
        _slider_pos = value;
        _submit();
        _signal_write(WriteType::Style);
    }
    void Slider::reset_relative(float value)
    {
        reset_absolute(data::min + (data::max - data::min) * value);
    }

    float Slider::absolute_value()
    {
        return std::clamp(_slider_pos, data::min, data::max);
    }
    float Slider::relative_value()
    {
        const auto range = data::max - data::min;
        if (range)
            return std::clamp((absolute_value() - data::min) / range, 0.0f, 1.0f);
        return 0.0f;
    }

    int VerticalSlider::_aligned_mouse()
    {
        return mouse_object_space().y;
    }
    int VerticalSlider::_aligned_width()
    {
        return layout(Element::Layout::Height) - resolve_units(data::slider_width);
    }
    void VerticalSlider::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(Pixel::combine(data::foreground_color, data::background_color));

        const auto sw = resolve_units(data::slider_width);
        auto sc = getstate(State::Keynavi) ? px::combined(data::focused_color) : data::slider_color;
        sc.v = data::slider_color.v;
        const auto abs = _relative_to_absolute(_slider_pos);
        for (std::size_t i = 0; i < sw; i++)
        {
            if (abs + i >= buffer.height())
                break;
            buffer.at(0, abs + i) = data::slider_color;
        }
    }

    VerticalSlider::VerticalSlider(const std::string& name, TreeState::ptr state) :
        Slider(name, state)
    {
        data::height = Unit(1);
        data::width = Unit(0.0_auto);
        data::foreground_color = px::foreground{.v = charset::slider_vertical};
    }
} // namespace d2::dx
