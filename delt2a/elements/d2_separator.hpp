#ifndef D2_SEPARATOR_HPP
#define D2_SEPARATOR_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
	namespace style
	{
		struct Separator
		{
			enum class Orientation
			{
				Vertical,
				Horizontal
			};

			Orientation orientation{ Orientation::Horizontal };
			value_type corner_left{ '<' };
			value_type corner_right{ '>' };
			bool override_corners{ false };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP(Style)
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, orientation)
				D2_UAI_GET_VAR_A(1, corner_left)
				D2_UAI_GET_VAR_A(2, corner_right)
				D2_UAI_GET_VAR_A(3, override_corners)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct ISeparator : Separator, InterfaceHelper<ISeparator, PropBase, 4>
		{
			using data = Separator;
			enum Property : uai_property
			{
				Direction = PropBase,
				CornerLeft,
				CornerRight,
				OverrideCorners,
			};
		};
		using IZSeparator = ISeparator<0>;
	} // style
	namespace dx
	{
		class Separator :
			public style::UAI<Separator, style::ILayout, style::IColors, style::ISeparator>,
			public impl::UnitUpdateHelper<Separator>
		{
		public:
			friend class UnitUpdateHelper;
			friend class TextHelper;
			using data = style::UAI<Separator, style::ILayout, style::IColors, style::ISeparator>;
		protected:
			virtual BoundingBox _dimensions_impl() const noexcept override;
			virtual Position _position_impl() const noexcept override;

			virtual unit_meta_flag _unit_report_impl() const noexcept override;
			virtual char _index_impl() const noexcept override;

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
		public:
			using data::data;
		};
	} // dx
} // d2

#endif // D2_SEPARATOR_HPP
