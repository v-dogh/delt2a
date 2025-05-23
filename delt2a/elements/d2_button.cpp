#include "d2_button.hpp"

namespace d2::dx
{
	char Button::_index_impl() const noexcept
	{
		return data::zindex;
	}
	Button::unit_meta_flag Button::_unit_report_impl() const noexcept
	{
		return UnitUpdateHelper::_report_units();
	}
	bool Button::_provides_input_impl() const noexcept
	{
		return true;
	}

	Button::BoundingBox Button::_dimensions_impl() const noexcept
	{
		if (data::width.getunits() == Unit::Auto || data::height.getunits() == Unit::Auto)
		{
			const auto bw = (data::container_options & ContainerOptions::EnableBorder) ?
				_resolve_units(data::border_width) : 0;
			auto tbox = TextHelper::_text_bounding_box(data::text);
			tbox.width += (bw * 2);
			tbox.height += (bw * 2);

			if (data::width.getunits() != Unit::Auto) tbox.width = _resolve_units(data::width);
			if (data::height.getunits() != Unit::Auto) tbox.height = _resolve_units(data::height);

			return tbox;
		}
		else
		{
			return {
				_resolve_units(data::width),
				_resolve_units(data::height)
			};
		}
	}
	Button::Position Button::_position_impl() const noexcept
	{
		return {
			_resolve_units(data::x),
			_resolve_units(data::y)
		};
	}

	void Button::_state_change_impl(State state, bool value)
	{
		if (state == State::Clicked && value)
		{
			if (data::on_submit != nullptr)
				data::on_submit(shared_from_this());
		}
		else if (state == State::Keynavi)
		{
			_signal_write(Style);
		}
	}
	void Button::_event_impl(IOContext::Event ev)
	{
		if (getstate(State::Keynavi) &&
			ev == IOContext::Event::KeyInput &&
			context()->input()->is_pressed(sys::SystemInput::Enter, sys::SystemInput::KeyMode::Press))
		{
			if (data::on_submit != nullptr)
				data::on_submit(shared_from_this());
		}
	}

	void Button::_frame_impl(PixelBuffer::View buffer) noexcept
	{
		buffer.fill(
			Pixel::combine(
				data::foreground_color,
				getstate(State::Keynavi) ? data::focused_color : data::background_color
			)
		);

		const auto bbox = box();
		if (data::container_options & data::EnableBorder)
			ContainerHelper::_render_border(buffer);
		TextHelper::_render_text(
			data::text,
			data::foreground_color,
			style::Text::Alignment::Center,
			{}, bbox,
			buffer
		);
	}
}
