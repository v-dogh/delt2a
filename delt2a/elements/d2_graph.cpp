#include "d2_graph.hpp"
#include <numeric>

namespace d2::dx::graph
{
	char CyclicVerticalBar::_index_impl() const noexcept
	{
		return data::zindex;
	}

	CyclicVerticalBar::unit_meta_flag CyclicVerticalBar::_unit_report_impl() const noexcept
	{
		return UnitUpdateHelper::_report_units();
	}

	CyclicVerticalBar::BoundingBox CyclicVerticalBar::_dimensions_impl() const noexcept
	{
		return {
			_resolve_units(data::width),
			_resolve_units(data::height)
		};
	}

	CyclicVerticalBar::Position CyclicVerticalBar::_position_impl() const noexcept
	{
		return {
			_resolve_units(data::x),
			_resolve_units(data::y)
		};
	}

	void CyclicVerticalBar::_frame_impl(PixelBuffer::View buffer) noexcept
	{
		buffer.fill(Pixel::combine(data::foreground_color, data::background_color));
		ContainerHelper::_render_border(buffer);

		if (!_data.empty())
		{
			const auto [ width, height ] = box();
			const auto [ basex, basey ] = ContainerHelper::_border_base();
			const auto [ ibasex, ibasey ] = ContainerHelper::_border_base_inv();
			const auto fh = height - (basey + ibasey);
			const auto dw = _resolution();
			const auto slice = (dw == 0) ? 0 : _data.size() / dw;

			for (std::size_t i = basex; i < dw; i++)
			{
				const auto abs = ((slice * i) + _start_offset) % _data.size();
				if (abs >= _data.size())
					break;

				const auto data_point =
					std::accumulate(
						_data.begin() + abs,
						_data.begin() + abs + slice, 0.f
					) / slice;

				const auto ch = fh * data_point;
				for (std::size_t j = 0; j < ch; j++)
				{
					auto px = Pixel::interpolate(
						data::min_color,
						data::max_color,
						j / ch
					);
					px.v = data::min_color.v;
					buffer.at(
						i, height - ibasey - j - 1
					).blend(px);
				}
			}
		}
	}

	int CyclicVerticalBar::_resolution() const noexcept
	{
		const auto [ width, height ] = box();
		const auto [ basex, basey ] = ContainerHelper::_border_base();
		const auto [ ibasex, ibasey ] = ContainerHelper::_border_base_inv();
		return data::automatic_resolution ?
			(width - (basex + ibasex)) : data::resolution;
	}

	void CyclicVerticalBar::clear() noexcept
	{
		_data.clear();
		_data.shrink_to_fit();
		_signal_write(Style);
	}

	void CyclicVerticalBar::push(float data_point) noexcept
	{
		const auto res = _resolution();
		if (_data.size() != res)
		{
			if (_start_offset >= res)
				_start_offset = res - 1;
			_data.resize(res);
		}
		_data[_start_offset++] = std::clamp(data_point, 0.f, 1.f);
		if (_start_offset >= _data.size())
			_start_offset = 0;
		_signal_write(Style);
	}
}
