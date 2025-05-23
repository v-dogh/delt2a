#ifndef D2_SLIDER_HPP
#define D2_SLIDER_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
	namespace style
	{
		struct Slider
		{
			float min{ 0 };
			float max{ 0 };
			PixelForeground slider_background_color{ .v = '-' };
			Pixel slider_color{ .r = 255, .g = 255, .b = 255, .v = ' ' };
			HDUnit slider_width{ 1 };
			std::function<void(Element::TraversalWrapper, float, float)> on_change{};

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP(Style)
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, max)
				D2_UAI_GET_VAR_A(1, min)
				D2_UAI_GET_VAR_A(2, slider_background_color)
				D2_UAI_GET_VAR_A(3, slider_color)
				D2_UAI_GET_VAR_A(4, slider_width)
				D2_UAI_GET_VAR(5, on_change, None)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct ISlider : Slider, InterfaceHelper<ISlider, PropBase, 6>
		{
			using data = Slider;
			enum Property : uai_property
			{
				Max = PropBase,
				Min,
				BackgroundColor,
				SliderColor,
				SliderWidth,
				OnChange,
			};
		};
		using IZSlider = ISlider<0>;
	}
	namespace dx
	{
		class Slider :
			public style::UAI<Slider, style::ILayout, style::ISlider, style::IKeyboardNav>,
			public impl::UnitUpdateHelper<Slider>
		{
		public:
			friend class UnitUpdateHelper;
			using data = style::UAI<Slider, style::ILayout, style::ISlider, style::IKeyboardNav>;
			using data::data;
		protected:
			int slider_spos_{ 0 };
			int slider_pos_{ 0 };

			void _submit();

			virtual char _index_impl() const noexcept override;
			virtual unit_meta_flag _unit_report_impl() const noexcept override;
			virtual bool _provides_input_impl() const noexcept override;

			virtual BoundingBox _dimensions_impl() const noexcept override;
			virtual Position _position_impl() const noexcept override;

			virtual void _state_change_impl(State state, bool value) override;
			virtual void _event_impl(IOContext::Event ev) override;

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;

			virtual float _absolute_value();
			virtual float _relative_value();
		public:
			virtual void reset_absolute(int value = 0) noexcept;
			virtual void reset_relative(float value = 0) noexcept;

			float absolute_value() noexcept;
			float relative_value() noexcept;
		};

		class VerticalSlider : public Slider
		{
		public:
			using Slider::Slider;
		protected:
			virtual void _state_change_impl(State state, bool value) override;
			virtual void _event_impl(IOContext::Event ev) override;
			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
			virtual float _relative_value() override;
		public:
			virtual void reset_absolute(int value = 0) noexcept override;
			virtual void reset_relative(float value = 0) noexcept override;
		};
	}
}

#endif // D2_SLIDER_HPP
