#include "d2_checkbox.hpp"

namespace d2::dx
{
	void Checkbox::_submit()
	{
		if (data::on_change != nullptr)
			data::on_change(shared_from_this(), data::checkbox_value);
	}

	char Checkbox::_index_impl() const noexcept
	{
		return data::zindex;
	}
	Checkbox::unit_meta_flag Checkbox::_unit_report_impl() const noexcept
	{
		return UnitUpdateHelper::_report_units();
	}
	bool Checkbox::_provides_input_impl() const noexcept
	{
		return true;
	}

	Checkbox::BoundingBox Checkbox::_dimensions_impl() const noexcept
	{
		BoundingBox bbox;
		if (data::checkbox_value)
		{
			bbox = TextHelper::_text_bounding_box(data::value_on);
		}
		else
		{
			bbox = TextHelper::_text_bounding_box(data::value_off);
		}
		return bbox;
	}
	Checkbox::Position Checkbox::_position_impl() const noexcept
	{
		return {
			_resolve_units(data::x),
			_resolve_units(data::y)
		};
	}

	void Checkbox::_state_change_impl(State state, bool value)
	{
		if (state == State::Clicked && value)
		{
			data::checkbox_value = !data::checkbox_value;
			_submit();
			_signal_write(data::value_on.size() == data::value_off.size() ?
				Style : Masked
			);
		}
	}
	void Checkbox::_event_impl(IOContext::Event ev)
	{
		if (getstate(State::Keynavi) &&
			ev == IOContext::Event::KeyInput &&
			context()->input()->is_pressed(sys::SystemInput::Enter, sys::SystemInput::KeyMode::Press))
		{
			_submit();
		}
	}

	void Checkbox::_frame_impl(PixelBuffer::View buffer) noexcept
	{
		const auto color = Pixel::combine(
			data::foreground_color,
			getstate(Keynavi) ? data::focused_color : data::background_color
		);
		buffer.fill(color);
		TextHelper::_render_text(
			data::checkbox_value ? data::value_on : data::value_off,
			data::checkbox_value ? data::color_on : data::color_off,
			{ 0, 0 },
			box(),
			buffer
		);
	}
}
