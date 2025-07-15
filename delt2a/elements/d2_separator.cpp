#include "d2_separator.hpp"

namespace d2::dx
{
	Separator::BoundingBox Separator::_dimensions_impl() const noexcept
	{
		return {
			_resolve_units(data::width),
			_resolve_units(data::height)
		};
	}
	Separator::Position Separator::_position_impl() const noexcept
	{
		return {
			_resolve_units(data::x),
			_resolve_units(data::y)
		};
	}

	Separator::unit_meta_flag Separator::_unit_report_impl() const noexcept
	{
		return UnitUpdateHelper::_report_units();
	}
	char Separator::_index_impl() const noexcept
	{
		return data::zindex;
	}

	void Separator::_frame_impl(PixelBuffer::View buffer) noexcept
	{
		auto color = Pixel::combine(data::foreground_color, data::background_color);
		buffer.fill(color);
		if (data::override_corners)
		{
			const auto [ width, height ] = box();
			if (data::orientation == data::Orientation::Vertical)
			{
				color.v = data::corner_left;
				for (std::size_t i = 0; i < width; i++)
					buffer.at(i) = color;
				color.v = data::corner_right;
				for (std::size_t i = 0; i < width; i++)
					buffer.at(i, height - 1) = color;
			}
			else if (data::orientation == data::Orientation::Horizontal)
			{
				color.v = data::corner_left;
				for (std::size_t i = 0; i < height; i++)
					buffer.at(0, i) = color;
				color.v = data::corner_right;
				for (std::size_t i = 0; i < height; i++)
					buffer.at(width - 1, i) = color;
			}
		}
	}
}
