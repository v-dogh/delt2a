#ifndef D2_GRAPH_HPP
#define D2_GRAPH_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"
#include <numeric>

namespace d2
{
	namespace style
	{
		struct RangeGraph
		{
			Pixel max_color{ .a = 0, .v = '.' };
			Pixel min_color{ .a = 0, .v = '.' };
			int resolution{ 0 };
			bool automatic_resolution{ true };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP(Style)
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, max_color)
				D2_UAI_GET_VAR_A(1, min_color)
				D2_UAI_GET_VAR_A(2, resolution)
				D2_UAI_GET_VAR_A(3, automatic_resolution)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct IRangeGraph : RangeGraph, InterfaceHelper<IRangeGraph, PropBase, 4>
		{
			using data = RangeGraph;
			enum Property : uai_property
			{
				MaxColor = PropBase,
				MinColor,
				Resolution,
				AutomaticResolution,
			};
		};
		using IZRangeGraph = IRangeGraph<0>;
	}
	namespace dx::graph
	{
		class CyclicVerticalBar :
			public style::UAI<
				CyclicVerticalBar,
				style::ILayout,
				style::IContainer,
				style::IColors,
				style::IRangeGraph
			>,
			public impl::UnitUpdateHelper<CyclicVerticalBar>,
			public impl::ContainerHelper<CyclicVerticalBar>
		{
		public:
			friend class UnitUpdateHelper;
			friend class ContainerHelper;
			using data = style::UAI<
				CyclicVerticalBar,
				style::ILayout,
				style::IContainer,
				style::IColors,
				style::IRangeGraph
			>;
			using data::data;
		protected:
			// Data points are normalized from 0 to 1
			std::vector<float> _data{};
			int _start_offset = 0;

			virtual char _index_impl() const noexcept override
			{
				return data::zindex;
			}
			virtual unit_meta_flag _unit_report_impl() const noexcept override
			{
				return UnitUpdateHelper::_report_units();
			}

			virtual BoundingBox _dimensions_impl() const noexcept override
			{
				return {
					_resolve_units(data::width),
					_resolve_units(data::height)
				};
			}
			virtual Position _position_impl() const noexcept override
			{
				return {
					_resolve_units(data::x),
					_resolve_units(data::y)
				};
			}

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
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

			int _resolution() const noexcept
			{
				const auto [ width, height ] = box();
				const auto [ basex, basey ] = ContainerHelper::_border_base();
				const auto [ ibasex, ibasey ] = ContainerHelper::_border_base_inv();
				return data::automatic_resolution ?
					(width - (basex + ibasex)) : data::resolution;
			}
		public:
			void clear() noexcept
			{
				_data.clear();
				_data.shrink_to_fit();
				_signal_write(Style);
			}
			void push(float data_point) noexcept
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
		};
	}
}

#endif // D2_GRAPH_HPP
