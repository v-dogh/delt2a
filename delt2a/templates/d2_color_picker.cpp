#include "d2_color_picker.hpp"
#include "d2_widget_theme_base.hpp"

namespace d2::ctm
{
	namespace
	{
		D2_STYLESHEET_BEGIN(rgba_picker)
			D2_STYLE(X, 0.0_relative)
            D2_STYLE(Width, 5.0_pxi)
			D2_STYLE(Min, 0)
			D2_STYLE(Max, 255)
            D2_STYLE(Step, 1)
			D2_STYLE(SliderColor, D2_VAR(WidgetTheme, wg_bg_button()))
			D2_STYLE(BackgroundColor, D2_VAR(WidgetTheme, wg_bg_button()))
			D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
        D2_STYLESHEET_END
	}

    ColorPicker::ColorPicker(
        const std::string& name,
        TreeState::ptr state,
        pptr parent,
        std::function<void(TypedTreeIter<ColorPicker>, px::background)> callback
        ) : data(name, state, parent)
    {
        data::on_submit = std::move(callback);
    }


	ColorPicker::eptr<ColorPicker> ColorPicker::_core(TreeState::ptr state)
	{
		return state->core()->traverse().as<ColorPicker>();
	}

	void ColorPicker::_state_change_impl(State state, bool value)
	{
		VirtualBox::_state_change_impl(state, value);
		if (state == State::Created && value && empty())
		{
            VirtualBox::width = 46.0_px;
            VirtualBox::height = 10.0_px;
			VirtualBox::zindex = 120;
            VirtualBox::container_options |= ContainerOptions::EnableBorder;
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

			auto& src = this->state()->screen()->theme<WidgetTheme>();
            data::set<VirtualBox::BorderHorizontalColor>(src.wg_border_horizontal());
			data::set<VirtualBox::BorderVerticalColor>(src.wg_border_vertical());
			data::set<VirtualBox::FocusedColor>(src.wg_hbg_button());
			data::set<VirtualBox::BarColor>(style::dynavar<[](const auto& value) {
				return value.extend('.');
            }>(src.wg_border_horizontal()));

			using namespace dx;
			D2_STATELESS_TREE(color_picker)
				// Controls
				D2_ELEM(Button, exit)
					D2_STYLE(Value, "<X>")
					D2_STYLE(X, 1.0_pxi)
					D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
					D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
                    D2_STYLE(OnSubmit, [state](TreeIter ptr) {
						_core(state)->close();
					})
				D2_ELEM_END
				D2_ELEM(Button, submit)
					D2_STYLE(Value, "<Ok>")
					D2_STYLE(X, 5.0_pxi)
					D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
					D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
                    D2_STYLE(OnSubmit, [state](TreeIter ptr) {
						_core(state)->submit();
					})
				D2_ELEM_END
				// Colors
                D2_ELEM(FlowBox, colors)
					D2_STYLE(Width, 0.5_pc)
					D2_STYLE(Height, 12.0_px)
					D2_STYLE(X, 1.0_relative)
					D2_STYLE(Y, 1.0_px)
					D2_ELEM(Text)
						D2_STYLE(Value, "R: ")
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
					D2_ELEM_END
					D2_ELEM(Slider, r)
						D2_STYLES_APPLY(rgba_picker)
					D2_ELEM_END
					D2_ELEM(Text)
						D2_STYLE(Value, "G: ")
						D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(Y, 0.0_relative)
					D2_ELEM_END
					D2_ELEM(Slider, g)
						D2_STYLES_APPLY(rgba_picker)
					D2_ELEM_END
					D2_ELEM(Text)
						D2_STYLE(Value, "B: ")
						D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(Y, 0.0_relative)
					D2_ELEM_END
					D2_ELEM(Slider, b)
						D2_STYLES_APPLY(rgba_picker)
					D2_ELEM_END
					D2_ELEM(Text)
						D2_STYLE(Value, "A: ")
						D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(Y, 0.0_relative)
					D2_ELEM_END
					D2_ELEM(Slider, a)
						D2_STYLES_APPLY(rgba_picker)
					D2_ELEM_END
                D2_ELEM_END
				// Statistics
                D2_ELEM(FlowBox, statistics)
					D2_STYLE(X, 0.0_pxi)
					D2_STYLE(Y, 1.0_px)
					D2_STYLE(Width, 0.5_pc)
					D2_STYLE(Height, 1.0_pxi)
					D2_ELEM(Text)
						D2_STYLE(Value, "RGBA")
						D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
						D2_STYLE(X, 0.0_center)
						D2_STYLE(Y, 0.0_relative)
					D2_ELEM_END
					D2_ELEM(Text, color-rgba)
						D2_STYLE(Value, "[0][0][0][0]")
						D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
						D2_STYLE(X, 0.0_center)
						D2_STYLE(Y, 0.0_relative)
					D2_ELEM_END
                D2_ELEM_END
			D2_TREE_END

			color_picker::create_at(
				traverse(),
				TreeState::make<TreeState>(
					this->state()->screen(),
					this->state()->context(),
					parent()->traverse().asp(),
					traverse().asp()
				)
			);

            at("colors").foreach([this](TreeIter elem) {
				if (elem.is_type<Slider>())
				{
                    elem.as<Slider>()->set<Slider::OnSubmit>([this](TypedIter<Slider> elem, auto, auto) {
						const auto color = _get_color();
						(*(traverse()/"statistics"/"color-rgba").as<Text>())
							.set<Text::BackgroundColor>(color)
							.set<Text::Value>(std::format("[{}][{}][{}][{}]",
								color.r, color.g, color.b, color.a
							));
					});
				}
                return true;
			});
		}
	}

	px::background ColorPicker::_get_color() const
	{
		auto colors = at("colors").asp();
		return {
			static_cast<std::uint8_t>(colors->at("r").as<dx::Slider>()->absolute_value()),
			static_cast<std::uint8_t>(colors->at("g").as<dx::Slider>()->absolute_value()),
			static_cast<std::uint8_t>(colors->at("b").as<dx::Slider>()->absolute_value()),
			static_cast<std::uint8_t>(colors->at("a").as<dx::Slider>()->absolute_value())
		};
	}

	void ColorPicker::submit()
	{
		if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ColorPicker>(shared_from_this()),
                _get_color()
            );
		close();
	}
}
