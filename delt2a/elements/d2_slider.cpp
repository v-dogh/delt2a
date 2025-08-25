#include "d2_slider.hpp"

namespace d2::dx
{
    Slider::Slider(
        const std::string& name,
        TreeState::ptr state,
        pptr parent
    ) :
        data(name, state, parent)
    {
        data::width = Unit(1);
        data::height = Unit(1);
    }

    void Slider::_submit()
    {
        const auto rel = relative_value();
        const auto abs = absolute_value();
        if (data::on_change != nullptr)
            data::on_change(shared_from_this(), rel, abs);
    }

    bool Slider::_provides_input_impl() const noexcept
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
                    _slider_spos = pos - _relative_to_absolute(_slider_pos);
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
    void Slider::_event_impl(IOContext::Event ev)
    {
        if (ev == IOContext::Event::MouseInput)
        {
            bool write = false;
            float npos = 0;
            if (getstate(Clicked))
            {
                const auto pos = _aligned_mouse();
                npos = _absolute_to_relative(std::clamp(int(pos - _slider_spos), 0, _aligned_width()));
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
        else if (ev == IOContext::Event::KeyInput)
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

    void Slider::_frame_impl(PixelBuffer::View buffer) noexcept
    {
        buffer.fill(data::slider_background_color);

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

    int Slider::_aligned_mouse() noexcept
    {
        return mouse_object_space().x;
    }
    int Slider::_aligned_width() noexcept
    {
        return layout(Element::Layout::Width) - resolve_units(data::slider_width);
    }
    int Slider::_relative_to_absolute(float rel) noexcept
    {
        const auto range = data::max - data::min;
        const auto aligned = rel - data::min;
        return static_cast<int>((aligned / range) * _aligned_width());
    }
    float Slider::_absolute_to_relative(int abs) noexcept
    {
        const auto range = data::max - data::min;
        return std::clamp(((float(abs) / _aligned_width()) * range) + data::min, data::min, data::max);
    }

    void Slider::reset_absolute(int value) noexcept
    {
        _slider_pos = _absolute_to_relative(value);
        _submit();
        _signal_write(WriteType::Style);
    }
    void Slider::reset_relative(float value) noexcept
    {
        _slider_pos = std::clamp(value, data::min, data::max);
        _submit();
        _signal_write(WriteType::Style);
    }

    float Slider::absolute_value() noexcept
    {
        return std::clamp(_slider_pos, data::min, data::max);
    }
    float Slider::relative_value() noexcept
    {
        return _relative_to_absolute(_slider_pos);
    }

    int VerticalSlider::_aligned_mouse() noexcept
    {
        return mouse_object_space().y;
    }
    int VerticalSlider::_aligned_width() noexcept
    {
        return layout(Element::Layout::Height) - resolve_units(data::slider_width);
    }
    void VerticalSlider::_frame_impl(PixelBuffer::View buffer) noexcept
    {
        buffer.fill(data::slider_background_color);

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
}
