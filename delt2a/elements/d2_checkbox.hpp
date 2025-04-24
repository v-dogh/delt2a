#ifndef D2_CHECKBOX_HPP
#define D2_CHECKBOX_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
	namespace style
	{
		struct Checkbox
		{
			std::function<void(Element::TraversalWrapper, bool)> on_change{ nullptr };
			string label{};
			PixelForeground checkbox_color_true{ .v = '+' };
			PixelForeground checkbox_color_false{ .v = 'o' };
			bool checkbox_value{ false };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP_EMPTY
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, on_change)
				D2_UAI_GET_VAR(1, label, Masked)
				D2_UAI_GET_VAR(2, checkbox_color_true, Style)
				D2_UAI_GET_VAR(3, checkbox_color_false, Style)
				D2_UAI_GET_VAR(4, checkbox_value, Style)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct ICheckbox : Checkbox, InterfaceHelper<ICheckbox, PropBase, 5>
		{
			using data = Checkbox;
			enum Property : uai_property
			{
				OnChange = PropBase,
				Label,
				ColorTrue,
				ColorFalse,
				Value,
			};
		};
		using IZCheckbox = ICheckbox<0>;
	}
	namespace dx
	{
		class Checkbox :
			public style::UAI<Checkbox, style::ILayout, style::IColors, style::ICheckbox>,
			public impl::UnitUpdateHelper<Checkbox>,
			public impl::TextHelper<Checkbox>
		{
		public:
			friend class UnitUpdateHelper;
			friend class TextHelper;
			using data = style::UAI<Checkbox, style::ILayout, style::IColors, style::ICheckbox>;
			using data::data;
		protected:
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
				if (data::width.getunits() == Unit::Auto || data::height.getunits() == Unit::Auto)
				{
					BoundingBox bbox;
					if (data::width.getunits() == Unit::Auto) bbox.width = 1 + data::label.size();
					else bbox.width = _resolve_units(data::width);
					if (data::height.getunits() == Unit::Auto) bbox.height = 1;
					else bbox.height = _resolve_units(data::height);
					return bbox;
				}
				else
				{
					return {
						_resolve_units(data::width),
						_resolve_units(data::height)
					};
				}
			}
			virtual Position _position_impl() const noexcept override
			{
				return {
					_resolve_units(data::x),
					_resolve_units(data::y)
				};
			}

			virtual void _state_change_impl(State state, bool value) override
			{
				if (state == State::Clicked && value)
				{
					data::checkbox_value = !data::checkbox_value;
					_signal_write(Style);
				}
			}

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
			{
				const auto color = Pixel::combine(data::foreground_color, data::background_color);
				buffer.at(0) = Pixel::combine((data::checkbox_value ?
					data::checkbox_color_true : data::checkbox_color_false), data::background_color);
				TextHelper::_render_text_simple(
					data::label, color,
					{ 1, 0 },
					box(),
					buffer
				);
			}
		};
	}
}

#endif // D2_CHECKBOX_HPP
