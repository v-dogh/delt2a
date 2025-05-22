#ifndef D2_GRAPH_HPP
#define D2_GRAPH_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

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
			std::vector<float> _data{};
			int _start_offset = 0;

			virtual char _index_impl() const noexcept override;
			virtual unit_meta_flag _unit_report_impl() const noexcept override;
			virtual BoundingBox _dimensions_impl() const noexcept override;
			virtual Position _position_impl() const noexcept override;
			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;

			int _resolution() const noexcept;

		public:
			void clear() noexcept;
			void push(float data_point) noexcept;
		};
	}
}

#endif // D2_GRAPH_HPP
