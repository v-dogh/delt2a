#include "d2_slider.hpp"

namespace d2::dx
{
    Slider::Slider(
        const std::string& name,
        TreeState::ptr state,
        pptr parent
    ) : data(name, state, parent)
    {
        data::width = Unit(1);
        data::height = Unit(0.0_auto);
        data::foreground_color = px::foreground{ .v = charset::slider_horizontal };
    }

    void Slider::_submit()
    {
        const auto rel = relative_value();
        const auto abs = absolute_value();
        if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<Slider>(shared_from_this()),
                rel, abs
            );
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
    void Slider::_event_impl(sys::screen::Event ev)
    {
        if (data::max == data::min)
            return;
        if (ev == sys::screen::Event::MouseInput)
        {
            bool write = false;
            float npos = 0;
            if (getstate(Clicked))
            {
                const auto pos = _aligned_mouse();
                npos = _absolute_to_relative(std::clamp(
                    int(pos - _slider_spos),
                    0, _aligned_width()
                ));
                write = true;
            }
            else if (getstate(Hovered))
            {
                if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollUp))
                {
                    if (_slider_pos < data::max)
                    {
                        npos = std::clamp(_slider_pos + data::step, data::min, data::max);;
                        write = true;
                    }
                }
                else if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollDown))
                {
                    if (_slider_pos > data::min)
                    {
                        npos = std::clamp(_slider_pos - data::step, data::min, data::max);
                        write = true;
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
        else if (ev == sys::screen::Event::KeyInput)
        {
            if (context()->input()->is_pressed(sys::SystemInput::ArrowLeft) ||
                    context()->input()->is_pressed(sys::SystemInput::ArrowDown))
            {
                if (_slider_pos - data::step > data::min)
                {
                    _slider_pos -= data::step;
                    _signal_write(Style);
                    _submit();
                }
            }
            else if (context()->input()->is_pressed(sys::SystemInput::ArrowRight) ||
                     context()->input()->is_pressed(sys::SystemInput::ArrowUp))
            {
                if (_slider_pos + data::step < data::max)
                {
                    _slider_pos += data::step;
                    _signal_write(Style);
                    _submit();
                }
            }
        }
    }

    void Slider::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(Pixel::combine(
            data::foreground_color,
            data::background_color
        ));

        const auto sw = resolve_units(data::slider_width);
        auto sc = getstate(State::Keynavi) ? px::combined(data::focused_color) : data::slider_color;
        sc.v = data::slider_color.v;
        const auto abs = _relative_to_absolute(_slider_pos);
        for (std::size_t i = 0; i < sw; i++)
        {
            if (abs + i >= buffer.width()) break;
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
                ((float(abs) / _aligned_width()) * range) + data::min,
                data::min, data::max
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
        return _relative_to_absolute(_slider_pos);
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
        buffer.fill(Pixel::combine(
            data::foreground_color,
            data::background_color
        ));

        const auto sw = resolve_units(data::slider_width);
        auto sc = getstate(State::Keynavi) ? px::combined(data::focused_color) : data::slider_color;
        sc.v = data::slider_color.v;
        const auto abs = _relative_to_absolute(_slider_pos);
        for (std::size_t i = 0; i < sw; i++)
        {
            if (abs + i >= buffer.height()) break;
            buffer.at(0, abs + i) = data::slider_color;
        }
    }

    VerticalSlider::VerticalSlider(
        const std::string& name,
        TreeState::ptr state,
        pptr parent
        ) : Slider(name, state, parent)
    {
        data::height = Unit(1);
        data::width = Unit(0.0_auto);
        data::foreground_color = px::foreground{ .v = charset::slider_vertical };
    }
}
