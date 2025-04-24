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
			string value_on{ "x" };
			string value_off{ "o" };
			Pixel color_on{};
			Pixel color_off{};
			bool checkbox_value{ false };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP_EMPTY
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, on_change)
				D2_UAI_GET_VAR(1, value_on, Masked)
				D2_UAI_GET_VAR(2, value_off, Masked)
				D2_UAI_GET_VAR(3, color_on, Style)
				D2_UAI_GET_VAR(4, color_off, Style)
				D2_UAI_GET_VAR(5, checkbox_value, Masked)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct ICheckbox : Checkbox, InterfaceHelper<ICheckbox, PropBase, 6>
		{
			using data = Checkbox;
			enum Property : uai_property
			{
				OnChange = PropBase,
				ValueOn,
				ValueOff,
				ColorOn,
				ColorOff,
				Value,
			};
		};
		using IZCheckbox = ICheckbox<0>;
	}
	namespace dx
	{
		class Checkbox :
			public style::UAI<Checkbox, style::ILayout, style::IColors, style::ICheckbox, style::IKeyboardNav>,
			public impl::UnitUpdateHelper<Checkbox>,
			public impl::TextHelper<Checkbox>
		{
		public:
			friend class UnitUpdateHelper;
			friend class TextHelper;
			using data = style::UAI<Checkbox, style::ILayout, style::IColors, style::ICheckbox, style::IKeyboardNav>;
			using data::data;
		protected:
			void _submit()
			{
				if (data::on_change != nullptr)
					data::on_change(shared_from_this(), data::checkbox_value);
			}

			virtual char _index_impl() const noexcept override
			{
				return data::zindex;
			}
			virtual unit_meta_flag _unit_report_impl() const noexcept override
			{
				return UnitUpdateHelper::_report_units();
			}
			virtual bool _provides_input_impl() const noexcept override
			{
				return true;
			}

			virtual BoundingBox _dimensions_impl() const noexcept override
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
					_submit();
					_signal_write(data::value_on.size() == data::value_off.size() ?
						Style : Masked
					);
				}
			}
			virtual void _event_impl(IOContext::Event ev) override
			{
				if (getstate(State::Keynavi) &&
					ev == IOContext::Event::KeyInput &&
					context()->input()->is_pressed(sys::SystemInput::Enter, sys::SystemInput::KeyMode::Press))
				{
					_submit();
				}
			}


			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
			{
				const auto color = Pixel::combine(
					data::foreground_color,
					getstate(Keynavi) ? data::focused_color : data::background_color
				);
				TextHelper::_render_text(
					data::checkbox_value ? data::value_on : data::value_off,
					data::checkbox_value ? data::color_on : data::color_off,
					{ 0, 0 },
					box(),
					buffer
				);
			}
		};
	}
}

#endif // D2_CHECKBOX_HPP
