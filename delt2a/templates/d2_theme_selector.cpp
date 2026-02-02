#include "d2_theme_selector.hpp"
#include "d2_widget_theme_base.hpp"
#include "d2_color_picker.hpp"
#include "../elements/d2_std.hpp"
#include "../d2_colors.hpp"
#include "d2_standard_widget_theme.hpp"
#include "d2_widget_theme_base.hpp"

namespace d2::ctm
{
    static ThemeSelector::eptr<ThemeSelectorWindow> _wcore(TreeState::ptr state)
    {
        return state->core()->traverse().as<ThemeSelectorWindow>();
    }
    static ThemeSelector::eptr<impl::ThemeSelectorBase> _bcore(TreeState::ptr state)
    {
        return state->core()->traverse().as<impl::ThemeSelectorBase>();
    }

    D2_STATELESS_TREE(theme_accent, int idx)
        D2_STYLE(Width, 2.0_pxi)
        D2_STYLE(Height, 1.0_px)
        D2_STYLE(X, 0.0_center)
        D2_STYLE(Y, idx)
        D2_ELEM(FlowBox)
            D2_STYLE(X, 0.0_pxi)
            D2_ELEM(Button)
                D2_STYLE(Value, charset::icon::arrow_up)
                D2_STYLE(X, 0.0_relative)
            D2_ELEM_END
            D2_ELEM(Button)
                D2_STYLE(Value, charset::icon::arrow_down)
                D2_STYLE(X, 0.0_relative)
            D2_ELEM_END
            D2_ELEM(Button)
                D2_STYLE(Value, charset::icon::cross)
                D2_STYLE(X, 0.0_relative)
            D2_ELEM_END
        D2_ELEM_END
    D2_TREE_END
    D2_STATELESS_TREE(theme_picker, bool window)
        D2_ANCHOR(FlowBox, themes)
        D2_ANCHOR(FlowBox, builder)
        if (window) D2_ELEM(Button, exit)
            D2_STYLE(Value, "<X>")
            D2_STYLE(X, 1.0_pxi)
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
            D2_STYLE(OnSubmit, [](TreeIter<Button> ptr) {
                _wcore(ptr->state())->close();
            })
        D2_ELEM_END
        D2_ELEM(FlowBox)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 1.0_px)
            D2_STYLE(Y, 0.0_relative)
            D2_ELEM(Switch)
                D2_STYLE(Width, 0.0_relative)
                D2_STYLE(X, 0.0_relative)
                D2_STYLE(Options, Switch::opts{ "Themes", "Builder" })
                D2_STYLE(OnChange, [=](TreeIter<Switch> ptr, int idx, int) {
                    themes->setstate(d2::Element::Display, idx == 0);
                    builder->setstate(d2::Element::Display, idx == 1);
                })
                D2_STYLES_APPLY(switch_color)
            D2_ELEM_END
            D2_ELEM(VerticalSeparator)
                D2_STYLE(X, 0.0_relative)
                D2_STYLE(Width, 1.0_px)
                D2_STYLE(Height, 1.0_pc)
                D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_border_vertical()))
            D2_ELEM_END
            D2_ELEM(Box)
                D2_STYLE(Width, 0.0_relative)
                D2_STYLE(X, 0.0_relative)
                D2_ELEM(Text)
                    D2_STYLE(ZIndex, Box::overlap)
                    D2_STYLE(Value, "Preview")
                    D2_STYLE(X, 0.0_center)
                    D2_STYLES_APPLY(bold_text_color)
                D2_ELEM_END
            D2_ELEM_END
        D2_ELEM_END
        D2_ELEM(FlowBox, logic)
            D2_STYLE(Y, 0.0_relative)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 0.0_relative)
            D2_ELEM(FlowBox, main)
                D2_STYLE(X, 0.0_relative)
                D2_STYLE(Width, 0.0_relative)
                D2_STYLE(Height, 1.0_pc)
                D2_ELEM_ANCHOR(themes)
                    D2_STYLE(Width, 1.0_pc)
                    D2_STYLE(Height, 1.0_pc)
                    D2_ELEM(VerticalSwitch, list)
                        D2_STYLES_APPLY(selector_color)
                    D2_ELEM_END
                D2_ELEM_END
                D2_ELEM_ANCHOR(builder)
                    D2_STYLE(Width, 1.0_pc)
                    D2_STYLE(Height, 1.0_pc)
                    D2_ELEM(FlowBox, options)
                        D2_STYLE(X, 0.0_relative)
                        D2_STYLE(Width, 0.0_relative)
                        D2_STYLE(Height, 1.0_pc)
                        D2_ELEM(Checkbox, immediate)
                            D2_STYLE(X, 1.0_relative)
                            D2_STYLE(Y, 1.0_px)
                        D2_ELEM_END
                        D2_ELEM(Text)
                            D2_STYLE(X, 1.0_relative)
                            D2_STYLE(Y, 1.0_px)
                            D2_STYLE(Value, "Immediate Mode")
                        D2_ELEM_END
                        D2_ELEM(Checkbox, preview)
                            D2_STYLE(X, 1.0_relative)
                            D2_STYLE(Y, 0.0_relative)
                        D2_ELEM_END
                        D2_ELEM(Text)
                            D2_STYLE(X, 1.0_relative)
                            D2_STYLE(Value, "Preview Mode")
                        D2_ELEM_END
                        D2_EMBED_ELEM_NAMED_BEGIN(ColorPicker, picker)
                            D2_STYLE(Y, 1.0_relative)
                            D2_STYLE(Width, 1.0_pc)
                            D2_STYLE(Height, 0.0_relative)
                            D2_STYLE(OnSubmit, [](TreeIter<ColorPicker> ptr, px::background color) {

                            })
                            D2_ELEM(Text)
                                D2_STYLE(ZIndex, Box::overlap)
                                D2_STYLE(Value, "Pick Accent")
                                D2_STYLE(X, 0.0_center)
                            D2_ELEM_END
                        D2_EMBED_END
                    D2_ELEM_END
                    D2_ELEM(VerticalSeparator)
                        D2_STYLE(X, 0.0_relative)
                        D2_STYLE(Width, 1.0_px)
                        D2_STYLE(Height, 1.0_pc)
                        D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_border_vertical()))
                    D2_ELEM_END
                    D2_ELEM(FlowBox, accents)
                        D2_STYLE(X, 0.0_relative)
                        D2_STYLE(Width, 0.0_relative)
                        D2_STYLE(Height, 1.0_pc)
                    D2_ELEM_END
                D2_ELEM_END
            D2_ELEM_END
            D2_ELEM(VerticalSeparator)
                D2_STYLE(X, 0.0_relative)
                D2_STYLE(Width, 1.0_px)
                D2_STYLE(Height, 1.0_pc)
                D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_border_vertical()))
            D2_ELEM_END
            D2_ELEM(ScrollFlowBox, preview)
                if (!state->screen()->has_theme("Preview"))
                    state->screen()->make_theme(style::Theme::make_named_copy(
                        "Preview",
                        state->screen()->theme<StandardTheme>()
                    ));
                D2_STYLE(X, 0.0_relative)
                D2_STYLE(Width, 0.0_relative)
                D2_STYLE(Height, 1.0_pc)
                D2_STYLE(BackgroundColor, D2_NVAR(WidgetTheme, Preview, wg_bg_primary()))
                D2_CONTEXT_AUTO(ptr->scrollbar())
                    D2_STYLES_APPLY(vertical_slider_color)
                D2_CONTEXT_END
                D2_ELEM(FlowBox)
                    D2_STYLE(Width, 1.0_pc)
                    D2_STYLE(Height, 1.0_pc)
                    D2_STYLE(Y, 0.0_relative)
                D2_ELEM_END
                D2_ELEM(FlowBox)
                    D2_STYLE(Width, 1.0_pc)
                    D2_STYLE(Height, 1.0_pc)
                    D2_STYLE(Y, 0.0_relative)
                D2_ELEM_END
            D2_ELEM_END
        D2_ELEM_END
    D2_TREE_END

    void impl::ThemeSelectorBase::push(px::background accent)
    {
        _accents.push_back(accent);
    }
    void impl::ThemeSelectorBase::clear()
    {
        _accents.clear();
    }

    void ThemeSelectorWindow::_state_change_impl(State state, bool value)
	{
        VirtualFlowBox::_state_change_impl(state, value);
		if (state == State::Created && value && empty())
		{
            data::width = 42.0_px;
            data::height = 16.0_px;
            data::zindex = 120;
            data::container_options |= ContainerOptions::EnableBorder;
            data::title = "<Pick Theme>";

			if (screen()->is_keynav())
			{
				const auto [ swidth, sheight ] = context()->input()->screen_size();
				const auto [ width, height ] = box();
                data::x = ((swidth - width) / 2);
                data::y = ((sheight - height) / 2);
			}
			else
			{
				const auto [ x, y ] = context()->input()->mouse_position();
                data::x = x - (box().width / 2);
                data::y = y;
			}

			auto& theme = this->state()->screen()->theme<WidgetTheme>();
            data::set<data::BorderHorizontalColor>(theme.wg_border_horizontal());
            data::set<data::BorderVerticalColor>(theme.wg_border_vertical());
            data::set<data::FocusedColor>(theme.wg_hbg_button());
            data::set<data::BarColor>(style::dynavar<[](const auto& value) {
                return value.extend(charset::box_top_bar);
            }>(theme.wg_border_horizontal()));


			theme_picker::create_at(
				traverse(),
				TreeState::make<TreeState>(
					parent()->traverse().asp(),
					traverse().asp()
                ),
                true
			);

            (at("logic")/"main"/"themes")->setstate(Element::Display, true);
            (at("logic")/"main"/"builder")->setstate(Element::Display, false);

			_update_theme_list();
		}
	}
    void ThemeSelectorWindow::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
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
        dx::VirtualFlowBox::_signal_write_impl(type, prop, element);
	}
    void ThemeSelectorWindow::_update_theme_list()
	{
		using namespace dx;
		auto& theme = this->state()->screen()->theme<WidgetTheme>();
        auto list = (at("logic")/"main"/"themes").asp();
		for (decltype(auto) it : data::themes)
		{
			auto elem = list->create<Button>().as<Button>();
			(*elem)
                .set<Button::Value>(it.first)
				.set<Button::ForegroundColor>(theme.wg_text())
				.set<Button::FocusedColor>(theme.wg_hbg_button())
				.set<Button::BackgroundColor>(d2::colors::w::transparent)
                .set<Button::OnSubmit>([](TreeIter<> elem) {
                    _bcore(elem->state())->submit(
						elem.as<Button>()->get<Button::Value>()
					);
				});
		}
	}

    void ThemeSelectorWindow::submit(const std::string& name)
	{
		if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ThemeSelectorWindow>(shared_from_this()),
                name
            );
		close();
	}
    void ThemeSelectorWindow::submit()
	{
		if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ThemeSelectorWindow>(shared_from_this()),
                _accents
            );
		close();
	}

    void ThemeSelector::_state_change_impl(State state, bool value)
    {
        FlowBox::_state_change_impl(state, value);
        if (state == State::Created && value && empty())
        {
            auto& theme = this->state()->screen()->theme<WidgetTheme>();
            data::set<data::ContainerBorder>(true);
            data::set<data::BorderHorizontalColor>(theme.wg_border_horizontal());
            data::set<data::BorderVerticalColor>(theme.wg_border_vertical());

            theme_picker::create_at(
                traverse(),
                TreeState::make<TreeState>(
                    parent()->traverse().asp(),
                    traverse().asp()
                ),
                false
            );

            (at("logic")/"main"/"themes")->setstate(Element::Display, true);
            (at("logic")/"main"/"builder")->setstate(Element::Display, false);

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
        dx::FlowBox::_signal_write_impl(type, prop, element);
    }
    void ThemeSelector::_update_theme_list()
    {
        using namespace dx;
        auto& theme = this->state()->screen()->theme<WidgetTheme>();
        auto list = (at("logic")/"main"/"themes").asp();
        for (decltype(auto) it : data::themes)
        {
            auto elem = list->create<Button>().as<Button>();
            (*elem)
                .set<Button::Value>(it.first)
                .set<Button::ForegroundColor>(theme.wg_text())
                .set<Button::FocusedColor>(theme.wg_hbg_button())
                .set<Button::BackgroundColor>(d2::colors::w::transparent)
                .set<Button::OnSubmit>([](TreeIter<> elem) {
                    _bcore(elem->state())->submit(
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
    }
    void ThemeSelector::submit()
    {
        if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ThemeSelector>(shared_from_this()),
                _accents
            );
    }
}

