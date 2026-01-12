#include "d2_theme_selector.hpp"
#include "d2_widget_theme_base.hpp"
#include "d2_color_picker.hpp"
#include "../elements/d2_std.hpp"
#include "../d2_colors.hpp"

namespace d2::ctm
{
    ThemeSelector::ThemeSelector(
        const std::string& name,
        TreeState::ptr state,
        pptr parent,
        std::function<void(TypedTreeIter<ThemeSelector>, theme)> callback
    ) : data(name, state, parent)
    {
        data::on_submit = std::move(callback);
    }

	ThemeSelector::eptr<ThemeSelector> ThemeSelector::_core(TreeState::ptr state)
	{
		return state->core()->traverse().as<ThemeSelector>();
	}

	void ThemeSelector::_state_change_impl(State state, bool value)
	{
		VirtualBox::_state_change_impl(state, value);
		if (state == State::Created && value && empty())
		{
			VirtualBox::width = 42.0_px;
			VirtualBox::height = 16.0_px;
			VirtualBox::zindex = 120;
            VirtualBox::container_options |= ContainerOptions::EnableBorder;
			VirtualBox::title = "<Pick Theme>";

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

			auto& theme = this->state()->screen()->theme<WidgetTheme>();
            data::set<VirtualBox::BorderHorizontalColor>(theme.wg_border_horizontal());
			data::set<VirtualBox::BorderVerticalColor>(theme.wg_border_vertical());
			data::set<VirtualBox::FocusedColor>(theme.wg_hbg_button());
			data::set<VirtualBox::BackgroundColor>(style::dynavar<[](const auto& value) {
                return value.alpha(0.3f);
			}>(theme.wg_bg_secondary()));
            data::set<VirtualBox::BarColor>(style::dynavar<[](const auto& value) {
                return value.extend(charset::box_top_bar);
            }>(theme.wg_border_horizontal()));

			using namespace dx;
			D2_STATELESS_TREE(theme_picker)
				D2_ELEM(Button, exit)
					D2_STYLE(Value, "<X>")
					D2_STYLE(X, 1.0_pxi)
					D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
					D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
                    D2_STYLE(OnSubmit, [state](TreeIter ptr) {
						_core(state)->close();
					})
				D2_ELEM_END
				D2_ELEM(Switch)
					D2_STYLE(Width, 1.0_pc)
					D2_STYLE(EnabledForegroundColor, D2_VAR(WidgetTheme, wg_hbg_button()))
					D2_STYLE(EnabledBackgroundColor, d2::colors::w::transparent)
					D2_STYLE(DisabledForegroundColor, D2_VAR(WidgetTheme, wg_text()))
					D2_STYLE(DisabledBackgroundColor, d2::colors::w::transparent)
					D2_STYLE(SeparatorColor, D2_VAR(WidgetTheme, wg_border_vertical()))
					D2_STYLE(FocusedColor, D2_DYNAVAR(WidgetTheme, wg_hbg_button(), value.alpha(0.7f)))
					D2_STYLE(Options, Switch::opts{ "Themes", "Accents" })
                    D2_STYLE(OnChange, [](TreeIter ptr, int idx, int) {
						(ptr->parent()->traverse()/"logic"/"themes")
							->setstate(d2::Element::Display, idx == 0);
						(ptr->parent()->traverse()/"logic"/"accents")
							->setstate(d2::Element::Display, idx == 1);
					})
				D2_ELEM_END
                D2_ELEM(Box, logic)
					D2_STYLE(Y, 1.0_px)
					D2_STYLE(Width, 1.0_pc)
					D2_STYLE(Height, 3.0_pxi)
					D2_ELEM(ScrollBox, themes)
						D2_STYLE(Width, 1.0_pc)
						D2_STYLE(Height, 1.0_pc)
						(*ptr.as<ScrollBox>()->scrollbar().as<VerticalSlider>())
                            .set<VerticalSlider::SliderColor>(D2_VAR(WidgetTheme, wg_bg_button()))
                            .set<VerticalSlider::BackgroundColor>(D2_VAR(WidgetTheme, wg_bg_button()));
					D2_ELEM_END
                    D2_ELEM(FlowBox, accents)
						D2_STYLE(Width, 1.0_pc)
						D2_STYLE(Height, 1.0_pc)
                        D2_ELEM(FlowBox, controls)
                            D2_STYLE(X, 0.0_relative)
                            D2_STYLE(Y, 1.0_px)
                            D2_STYLE(Width, 0.5_pc)
                            D2_STYLE(Height, 1.0_pxi)
                            D2_ELEM(Button)
                                D2_STYLE(Value, "Push Accent")
                                D2_STYLE(X, 0.0_center)
                                D2_STYLE(Y, 0.0_relative)
                                D2_STYLE(OnSubmit, [](TreeIter ptr) {
                                    auto p = ptr->screen()->traverse().asp()->override<ColorPicker>("__color_picker__");
                                    p->set<ColorPicker::OnSubmit>([local = ptr->parent()->traverse()](TreeIter ptr, px::background color) {
                                        _core(local->state())->_accents.push_back(color);
                                        (*(local->parent()->traverse()/"list"/"accents").asp()
                                            ->create<Text>())
                                            .set<Text::X>(0.0_center)
                                            .set<Text::Y>(0.0_relative)
                                            .set<Text::ForegroundColor>(color.alpha(std::max(color.a / 255.f, 0.6f)))
                                            .set<Text::Value>(std::format("[{} {} {} {}]",
                                                color.r, color.g, color.b, color.a
                                            ));
                                    });
                                })
                                D2_STYLES_APPLY(button_react)
                            D2_ELEM_END
                            D2_ELEM(Button)
                                D2_STYLE(Value, "Clear Accents")
                                D2_STYLE(X, 0.0_center)
                                D2_STYLE(Y, 0.0_relative)
                                D2_STYLE(OnSubmit, [](TreeIter ptr) {
                                    (_core(ptr->state())->traverse()/"logic"/"accents"/"list"/"accents").asp()->clear();
                                    _core(ptr->state())->_accents.clear();
                                })
                                D2_STYLES_APPLY(button_react)
                            D2_ELEM_END
							D2_ELEM(Button)
								D2_STYLE(Value, "Submit Accents")
								D2_STYLE(X, 0.0_center)
                                D2_STYLE(Y, 0.0_relative)
                                D2_STYLE(OnSubmit, [](TreeIter ptr) {
									_core(ptr->state())->submit();
								})
                                D2_STYLES_APPLY(button_react)
							D2_ELEM_END
                        D2_ELEM_END
                        D2_ELEM(VerticalSeparator)
                            D2_STYLE(ZIndex, 1)
                            D2_STYLE(X, 0.0_center)
                            D2_STYLE(Width, 1.0_px)
                            D2_STYLE(Height, 1.0_pc)
                            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_border_vertical()))
                        D2_ELEM_END
                        D2_ELEM(FlowBox, list)
                            D2_STYLE(X, 0.0_relative)
                            D2_STYLE(Y, 1.0_px)
                            D2_STYLE(Width, 0.5_pc)
                            D2_STYLE(Height, 1.0_pxi)
                            D2_ELEM(Text)
                                D2_STYLE(Value, "Accent List")
                                D2_STYLE(X, 0.0_center)
                                D2_STYLE(Y, 0.0_relative)
                                D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
                            D2_ELEM_END
                            D2_ELEM(FlowBox, accents)
                                D2_STYLE(Width, 1.0_pc)
                                D2_STYLE(Height, 2.0_pxi)
                                D2_STYLE(Y, 0.0_relative)
                            D2_ELEM_END
                        D2_ELEM_END
                    D2_ELEM_END
                D2_ELEM_END
			D2_TREE_END

			theme_picker::create_at(
				traverse(),
				TreeState::make<TreeState>(
					this->state()->screen(),
					parent()->traverse().asp(),
					traverse().asp()
				)
			);

			(at("logic")/"themes")->setstate(Element::Display, true);
			(at("logic")/"accents")->setstate(Element::Display, false);

			_update_theme_list();
		}
	}

	void ThemeSelector::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
	{
		if (element.get() == this)
		{
			if (prop == data::MaxAccents)
			{

			}
			else if (prop == data::Themes)
			{
				_update_theme_list();
			}
		}
        dx::VirtualBox::_signal_write_impl(type, prop, element);
	}

	void ThemeSelector::_update_theme_list()
	{
		using namespace dx;
		auto& theme = this->state()->screen()->theme<WidgetTheme>();
		auto list = (at("logic")/"themes").asp();
		for (decltype(auto) it : data::themes)
		{
			auto elem = list->create<Button>().as<Button>();
			(*elem)
				.set<Button::Value>(it)
				.set<Button::ForegroundColor>(theme.wg_text())
				.set<Button::FocusedColor>(theme.wg_hbg_button())
				.set<Button::BackgroundColor>(d2::colors::w::transparent)
                .set<Button::OnSubmit>([](TreeIter elem) {
					_core(elem->state())->submit(
						elem.as<Button>()->get<Button::Value>()
					);
				});
		}
	}

	void ThemeSelector::submit(const std::string& name)
	{
		if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ThemeSelector>(shared_from_this()),
                name
            );
		close();
	}
	void ThemeSelector::submit()
	{
		if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ThemeSelector>(shared_from_this()),
                _accents
            );
		close();
	}
}

