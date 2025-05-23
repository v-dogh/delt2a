#include "d2_slider.hpp"

namespace d2::dx
{
	void Slider::_submit()
	{
		const auto rel = _relative_value();
		const auto abs = _absolute_value();
		if (data::on_change != nullptr)
			data::on_change(shared_from_this(), rel, abs);
	}

	char Slider::_index_impl() const noexcept
	{
		return data::zindex;
	}
	Slider::unit_meta_flag Slider::_unit_report_impl() const noexcept
	{
		return UnitUpdateHelper::_report_units();
	}
	bool Slider::_provides_input_impl() const noexcept
	{
		return true;
	}

	Slider::BoundingBox Slider::_dimensions_impl() const noexcept
	{
		BoundingBox bbox;
		if (data::width.getunits() == Unit::Auto) bbox.width = 1;
		else bbox.width = _resolve_units(data::width);
		if (data::height.getunits() == Unit::Auto) bbox.height = 1;
		else bbox.height = _resolve_units(data::height);
		return bbox;
	}
	Slider::Position Slider::_position_impl() const noexcept
	{
		return {
			_resolve_units(data::x),
			_resolve_units(data::y)
		};
	}

	void Slider::_state_change_impl(State state, bool value)
	{
		if (state == State::Clicked)
		{
			if (value)
			{
				const auto [ x, y ] = mouse_object_space();
				const auto sw = _resolve_units(data::slider_width);
				if (x >= slider_pos_ && x < slider_pos_ + sw)
				{
					slider_spos_ = x - slider_pos_;
				}
			}
			else
			{
				slider_spos_ = 0;
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
			int npos = 0;
			if (getstate(Clicked))
			{
				const auto bbox = box();
				const auto [ x, y ] = mouse_object_space();
				const auto sw = _resolve_units(data::slider_width);
				npos = std::max(0, std::min(
					int(bbox.width - sw),
					int(x + slider_spos_)
				));
				write = true;
			}
			else if (getstate(Hovered))
			{
				const auto bbox = box();
				if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollUp))
				{
					const auto sw = _resolve_units(data::slider_width);
					if (sw < bbox.width && slider_pos_ < bbox.width - sw)
					{
						npos = slider_pos_ + 1;
						write = true;
					}
				}
				else if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollDown))
				{
					if (slider_pos_)
					{
						npos = slider_pos_ - 1;
						write = true;
					}
				}
			}

			if (write && npos != slider_pos_)
			{
				slider_pos_ = npos;
				_signal_write(WriteType::Style);
				_submit();
			}
		}
		else if (ev == IOContext::Event::KeyInput)
		{
			if (context()->input()->is_pressed(sys::SystemInput::ArrowLeft) ||
				context()->input()->is_pressed(sys::SystemInput::ArrowDown))
			{
				if (slider_pos_ > 0)
				{
					slider_pos_--;
					_signal_write(Style);
					_submit();
				}
			}
			else if (context()->input()->is_pressed(sys::SystemInput::ArrowRight) ||
					 context()->input()->is_pressed(sys::SystemInput::ArrowUp))
			{
				if (slider_pos_ < box().width - 1)
				{
					slider_pos_++;
					_signal_write(Style);
					_submit();
				}
			}
		}
	}

	void Slider::_frame_impl(PixelBuffer::View buffer) noexcept
	{
		buffer.fill(data::slider_background_color);

		const auto sw = _resolve_units(data::slider_width);
		auto sc = getstate(State::Keynavi) ?
			px::combined(data::focused_color) : data::slider_color;
		sc.v = data::slider_color.v;
		for (std::size_t i = 0; i < sw; i++)
		{
			if (slider_pos_ + i >= buffer.width()) break;
			buffer.at(
				slider_pos_ + i
			) = sc;
		}
	}

	float Slider::_absolute_value()
	{
		return std::clamp(
			data::min + (_relative_value() * (data::max - data::min)),
			data::min,
			data::max
		);
	}
	float Slider::_relative_value()
	{
		const auto bbox = box();
		const auto sw = _resolve_units(data::slider_width);
		return std::clamp(float(slider_pos_) / (bbox.width - sw - 1), 0.f, 1.f);
	}

	void Slider::reset_absolute(int value) noexcept
	{
		slider_pos_ = (value / (data::max - data::min)) * box().width;
		_submit();
		_signal_write(WriteType::Style);
	}
	void Slider::reset_relative(float value) noexcept
	{
		slider_pos_ = box().width * value;
		_submit();
		_signal_write(WriteType::Style);
	}

	float Slider::absolute_value() noexcept
	{
		return _absolute_value();
	}
	float Slider::relative_value() noexcept
	{
		return std::clamp(
			data::min + (_absolute_value() * (data::max - data::min)),
			data::min,
			data::max
		);
	}

	void VerticalSlider::_state_change_impl(State state, bool value)
	{
		if (state == State::Clicked)
		{
			if (value)
			{
				const auto [ x, y ] = mouse_object_space();
				const auto sw = _resolve_units(data::slider_width);
				if (y >= slider_pos_ && y < slider_pos_ + sw)
				{
					slider_spos_ = y - slider_pos_;
				}
			}
			else
			{
				slider_spos_ = 0;
			}
		}
	}
	void VerticalSlider::_event_impl(IOContext::Event ev)
	{
		bool write = false;
		int npos = 0;
		if (getstate(Clicked))
		{
			const auto bbox = box();
			const auto [ x, y ] = mouse_object_space();
			const auto sw = _resolve_units(data::slider_width);
			npos = std::max(0, std::min(
				int(bbox.height - sw),
				int(y + slider_spos_)
			));
			write = true;
		}
		else if (getstate(Hovered))
		{
			const auto bbox = box();
			if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollUp))
			{
				const auto sw = _resolve_units(data::slider_width);
				if (sw < bbox.height && slider_pos_ < bbox.height - sw)
				{
					npos = slider_pos_ + 1;
					write = true;
				}
			}
			else if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollDown))
			{
				if (slider_pos_)
				{
					npos = slider_pos_ - 1;
					write = true;
				}
			}
		}

		if (write && npos != slider_pos_)
		{
			slider_pos_ = npos;
			_signal_write(WriteType::Style);
			_submit();
		}
	}
	void VerticalSlider::_frame_impl(PixelBuffer::View buffer) noexcept
	{
		buffer.fill(data::slider_background_color);

		const auto sw = _resolve_units(data::slider_width);
		for (std::size_t i = 0; i < sw; i++)
		{
			if (slider_pos_ + i >= buffer.height()) break;
			buffer.at(
				0,
				slider_pos_ + i
			) = data::slider_color;
		}
	}
	float VerticalSlider::_relative_value()
	{
		const auto bbox = box();
		const auto sw = _resolve_units(data::slider_width);
		return std::clamp(float(slider_pos_) / (bbox.height - sw), 0.f, 1.f);
	}
	void VerticalSlider::reset_absolute(int value) noexcept
	{
		slider_pos_ = (value / (data::max - data::min)) * box().height;
		_submit();
		_signal_write(WriteType::Style);
	}
	void VerticalSlider::reset_relative(float value) noexcept
	{
		slider_pos_ = box().width * value;
		_submit();
		_signal_write(WriteType::Style);
	}
}
