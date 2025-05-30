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
			string value_on{ D2_STRING("x") };
			string value_off{ D2_STRING("o") };
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
			void _submit();

			virtual char _index_impl() const noexcept override;
			virtual unit_meta_flag _unit_report_impl() const noexcept override;
			virtual bool _provides_input_impl() const noexcept override;

			virtual BoundingBox _dimensions_impl() const noexcept override;
			virtual Position _position_impl() const noexcept override;

			virtual void _state_change_impl(State state, bool value) override;
			virtual void _event_impl(IOContext::Event ev) override;

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
		};
	}
}

#endif // D2_CHECKBOX_HPP
