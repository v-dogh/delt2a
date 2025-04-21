#ifndef D2_COLOR_PICKER_HPP
#define D2_COLOR_PICKER_HPP

#include "../elements/d2_std.hpp"
#include "d2_popup_theme_base.hpp"

namespace d2
{
	namespace style
	{
		struct ColorPicker
		{
			std::function<void(Element::TraversalWrapper, px::background)> on_submit{ nullptr };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP_EMPTY
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, on_submit)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct IColorPicker : ColorPicker, InterfaceHelper<IColorPicker, PropBase, 1>
		{
			using data = ColorPicker;
			enum Property : uai_property
			{
				OnSubmit = PropBase,
			};
		};
		using IZColorPicker = IColorPicker<0>;
	}
	namespace templ
	{
		namespace
		{			
			D2_STYLESHEET_BEGIN(rgba_picker)
				D2_STYLE(X, 0.0_relative)
				D2_STYLE(Width, 3.0_pxi)
				D2_STYLE(Min, 0)
				D2_STYLE(Max, 255)
				D2_STYLE(SliderColor, D2_VAR(PopupTheme, pt_bg_button()))
				D2_STYLE(BackgroundColor, D2_VAR(PopupTheme, pt_bg_button()))
				D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
			D2_STYLESHEET_END(rgba_picker)
		}

		class ColorPicker :
			public dx::VirtualBox,
			public style::UAIC<dx::VirtualBox::data, ColorPicker, style::IColorPicker>
		{
		public:
			using data = style::UAIC<VirtualBox::data, ColorPicker, style::IColorPicker>;
			D2_UAI_CHAIN(VirtualBox)
		protected:
			static eptr<ColorPicker> _core(TreeState::ptr state)
			{
				return
					state->core()->traverse().as<ColorPicker>();
			}

			virtual void _state_change_impl(State state, bool value) override
			{
				VirtualBox::_state_change_impl(state, value);
				if (state == State::Swapped && !value && empty())
				{
					VirtualBox::width = 42.0_px;
					VirtualBox::height = 8.0_px;
					VirtualBox::zindex = 120;
					VirtualBox::container_options =
						ContainerOptions::EnableBorder | ContainerOptions::TopFix;
					VirtualBox::title = "<Pick Color>";

					if (screen()->is_keynav())
					{
						const auto [ swidth, sheight ] = context()->input()->screen_size();
						const auto [ width, height ] = box();
						VirtualBox::x = ((swidth - width) / 2);
						VirtualBox::y = ((sheight - height) / 2);
					}
					else
					{
						const auto [ x, y ] = context()->input()->mouse_position();
						VirtualBox::x = x - (box().width / 2);
						VirtualBox::y = y;
					}

					auto& src = this->state()->screen()->theme<PopupTheme>();
					data::set<VirtualBox::BorderHorizontalColor>(src.pt_border_horizontal());
					data::set<VirtualBox::BorderVerticalColor>(src.pt_border_vertical());
					data::set<VirtualBox::FocusedColor>(src.pt_hbg_button());
					data::set<VirtualBox::BarColor>(style::dynavar<[](const auto& value) {
						return value.extend('.');
					}>(src.pt_border_horizontal()));

					using namespace dx;
					D2_STATELESS_TREE(color_picker)
						// Controls
						D2_ELEM(Button, exit)
							D2_STYLE(Value, "<X>")
							D2_STYLE(X, 1.0_pxi)
							D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
							D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
							D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr) {
								_core(state)->close();
							})
						D2_ELEM_END(exit)
						D2_ELEM(Button, submit)
							D2_STYLE(Value, "<Ok>")
							D2_STYLE(X, 5.0_pxi)
							D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
							D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
							D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr) {
								_core(state)->submit();
							})
						D2_ELEM_END(submit)
						// Colors
						D2_ELEM_NESTED(Box, colors)
							D2_STYLE(Width, 0.5_pc)
							D2_STYLE(Height, 12.0_px)
							D2_STYLE(X, 1.0_relative)
							D2_STYLE(Y, 1.0_px)
						D2_ELEM_NESTED_BODY(colors)
							D2_ELEM_UNNAMED(Text)
								D2_STYLE(Value, "R: ")
								D2_STYLE(X, 1.0_relative)
								D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
							D2_UELEM_END
							D2_ELEM(Slider, r)
								D2_STYLES_APPLY(rgba_picker)
							D2_ELEM_END(r)
							D2_ELEM_UNNAMED(Text)
								D2_STYLE(Value, "G: ")
								D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
								D2_STYLE(X, 1.0_relative)
								D2_STYLE(Y, 0.0_relative)
							D2_UELEM_END
							D2_ELEM(Slider, g)
								D2_STYLES_APPLY(rgba_picker)
							D2_ELEM_END(g)
							D2_ELEM_UNNAMED(Text)
								D2_STYLE(Value, "B: ")
								D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
								D2_STYLE(X, 1.0_relative)
								D2_STYLE(Y, 0.0_relative)
							D2_UELEM_END
							D2_ELEM(Slider, b)
								D2_STYLES_APPLY(rgba_picker)
							D2_ELEM_END(b)
							D2_ELEM_UNNAMED(Text)
								D2_STYLE(Value, "A: ")
								D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
								D2_STYLE(X, 1.0_relative)
								D2_STYLE(Y, 0.0_relative)
							D2_UELEM_END
							D2_ELEM(Slider, a)
								D2_STYLES_APPLY(rgba_picker)
							D2_ELEM_END(a)
						D2_ELEM_NESTED_END(colors)
						// Statistics
						D2_ELEM_NESTED(Box, statistics)
							D2_STYLE(X, 0.0_pxi)
							D2_STYLE(Y, 1.0_px)
							D2_STYLE(Width, 0.5_pc)
							D2_STYLE(Height, 1.0_pxi)
						D2_ELEM_NESTED_BODY(statistics)
							D2_ELEM_UNNAMED(Text)
								D2_STYLE(Value, "RGBA")
								D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
								D2_STYLE(X, 0.0_center)
								D2_STYLE(Y, 0.0_relative)
							D2_UELEM_END
							D2_ELEM(Text, color-rgba)
								D2_STYLE(Value, "[0][0][0][0]")
								D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
								D2_STYLE(X, 0.0_center)
								D2_STYLE(Y, 0.0_relative)
							D2_ELEM_END(color-rgba)
						D2_ELEM_NESTED_END(statistics)
					D2_TREE_END(color_picker)

					color_picker::create_at(
						traverse(),
						TreeState::make<TreeState>(
							this->state()->screen(),
							this->state()->context(),
							parent()->traverse().asp(),
							traverse().asp()
						)
					);

					at("colors").foreach([this](TraversalWrapper elem) {
						if (elem.is_type<Slider>())
						{
							elem.as<Slider>()->set<Slider::OnChange>([this](TraversalWrapper elem, auto, auto) {
								const auto color = _get_color();
								(*(traverse()/"statistics"/"color-rgba").as<Text>())
									.set<Text::BackgroundColor>(color)
									.set<Text::Value>(std::format("[{}][{}][{}][{}]",
										color.r, color.g, color.b, color.a
									));
							});
						}
					});
				}
			}

			px::background _get_color() const noexcept
			{
				auto colors = at("colors").asp();
				return {
					static_cast<std::uint8_t>(colors->at("r").as<dx::Slider>()->absolute_value()),
					static_cast<std::uint8_t>(colors->at("g").as<dx::Slider>()->absolute_value()),
					static_cast<std::uint8_t>(colors->at("b").as<dx::Slider>()->absolute_value()),
					static_cast<std::uint8_t>(colors->at("a").as<dx::Slider>()->absolute_value())
				};
			}
		public:
			using VirtualBox::VirtualBox;

			void submit()
			{
				if (data::on_submit != nullptr)
					data::on_submit(shared_from_this(), _get_color());
				close();
			}
		};
	}
}

#endif // D2_COLOR_PICKER_HPP
