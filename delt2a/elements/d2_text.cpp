#include "d2_text.hpp"

namespace d2::dx
{
	Text::BoundingBox Text::_dimensions_impl() const noexcept
	{
		int width = 0;
		int height = 0;
		if (data::width.getunits() == Unit::Auto ||
			data::height.getunits() == Unit::Auto)
		{
			if (data::text_options & HandleNewlines)
			{
				if (data::text_options & PreserveWordBoundaries)
				{
					const auto [ bwidth, bheight ] =
						TextHelper::_paragraph_bounding_box(data::text,
						(data::width.getunits() == Unit::Auto ? INT_MAX : _resolve_units(data::width)),
						(data::height.getunits() == Unit::Auto ? INT_MAX : _resolve_units(data::height)));
					width = bwidth;
					height = (data::height.getunits() == Unit::Auto) ?
						bheight : _resolve_units(data::height);
				}
				else
				{
					const auto [ bwidth, bheight ] =
						TextHelper::_text_bounding_box(data::text);
					width = (data::width.getunits() == Unit::Auto) ?
						bwidth : _resolve_units(data::width);
					height = (data::height.getunits() == Unit::Auto) ?
						bheight : _resolve_units(data::height);
				}
			}
			else
			{
				width = (data::width.getunits() == Unit::Auto) ?
					data::text.size() : _resolve_units(data::width);
				height = (data::height.getunits() == Unit::Auto) ?
					1 : _resolve_units(data::height);
			}
		}
		else
		{
			width = _resolve_units(data::width);
			height = _resolve_units(data::height);
		}
		return { width, height };
	}
	Text::Position Text::_position_impl() const noexcept
	{
		return {
			_resolve_units(data::x),
			_resolve_units(data::y)
		};
	}

	Text::unit_meta_flag Text::_unit_report_impl() const noexcept
	{
		return UnitUpdateHelper::_report_units();
	}
	char Text::_index_impl() const noexcept
	{
		return data::zindex;
	}

	void Text::_frame_impl(PixelBuffer::View buffer) noexcept
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
		ptr parent
	) : data(name, state, parent)
	{
		data::background_color.a = 0;
	}
}
