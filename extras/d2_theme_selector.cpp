#include "d2_theme_selector.hpp"
#include "../elements/d2_std.hpp"
#include "d2_standard_widget_theme.hpp"
#include "d2_widget_theme_base.hpp"
#include <d2_colors.hpp>
#include <templates/d2_color_picker.hpp>

namespace d2::ex
{
    // Sadly these have to be copied over for the internal preview theme
    namespace impl
    {
        style::IZThemeSelector::theme_list default_themes()
        {
            return {
                // Core
                {{"Slate", "Core"}, {d2::colors::b::slate_blue}},
                {{"Midnight", "Core"}, {d2::colors::b::midnight_blue}},
                {{"Indigo Ink", "Core"}, {d2::colors::b::indigo}},
                {{"Steel", "Core"}, {d2::colors::b::steel_blue}},
                {{"Mono Gray", "Core"}, {d2::colors::w::gray}},
                // Cool
                {{"Ocean", "Cool"}, {d2::colors::b::midnight_blue, d2::colors::b::turquoise}},
                {{"Arctic Cyan", "Cool"}, {d2::colors::b::dark_slate_blue, d2::colors::b::cyan}},
                {{"Nordish Blue", "Cool"},
                 {d2::px::background{120, 140, 170, 255}, d2::colors::b::steel_blue}},
                {{"Blueprint", "Cool"},
                 {d2::colors::b::dark_slate_blue, d2::colors::b::royal_blue, d2::colors::b::cyan}},
                {{"Graphite/Ice", "Cool"},
                 {d2::colors::w::dark_gray, d2::colors::b::steel_blue, d2::colors::b::cyan}},
                // Warm
                {{"Coral", "Warm"}, {d2::colors::r::coral}},
                {{"Crimson", "Warm"}, {d2::colors::r::crimson}},
                {{"Firebrick", "Warm"}, {d2::colors::r::firebrick}},
                {{"Chrome Yellow", "Warm"}, {d2::colors::r::chrome_yellow}},
                {{"Ember", "Warm"}, {d2::px::background{160, 110, 70, 255}, d2::colors::r::salmon}},
                {{"Sunset Coral", "Warm"},
                 {d2::px::background{120, 90, 110, 255}, d2::colors::r::coral}},
                {{"Coffee", "Warm"},
                 {d2::px::background{170, 140, 110, 255}, d2::colors::r::chrome_yellow}},
                {{"Autumn", "Warm"},
                 {d2::px::background{150, 120, 80, 255},
                  d2::colors::r::firebrick,
                  d2::colors::r::chrome_yellow}},
                // Natural
                {{"Forest", "Nature"}, {d2::colors::g::forest_green}},
                {{"Sea Green", "Nature"}, {d2::colors::g::sea_green}},
                {{"Lime Punch", "Nature"}, {d2::colors::g::lime}},
                {{"Terminal Mint", "Nature"}, {d2::colors::w::gray, d2::colors::g::aquamarine}},
                {{"Evergreen", "Nature"},
                 {d2::colors::w::gray, d2::colors::g::forest_green, d2::colors::g::aquamarine}},
                {{"Moss/Clay", "Nature"},
                 {d2::px::background{140, 130, 100, 255},
                  d2::colors::g::olive,
                  d2::colors::r::coral}},
                // Retro
                {{"Paper/Ink", "Terminal"}, {d2::colors::w::light_gray, d2::colors::b::indigo}},
                {{"Old CRT", "Terminal"},
                 {d2::px::background{180, 220, 180, 255}, d2::colors::g::chartreuse}},
                {{"Contrast Blue", "Terminal"}, {d2::colors::w::silver, d2::colors::b::royal_blue}},
                {{"Contrast Red", "Terminal"}, {d2::colors::w::silver, d2::colors::r::crimson}},
                {{"Graphite/Lime", "Terminal"}, {d2::colors::w::dark_gray, d2::colors::g::lime}},
                // Neon
                {{"Cyberpunk", "Neon"},
                 {d2::px::background{90, 70, 120, 255},
                  d2::colors::r::magenta,
                  d2::colors::b::cyan}},
                {{"Vaporwave", "Neon"},
                 {d2::px::background{120, 80, 160, 255},
                  d2::colors::r::salmon,
                  d2::colors::b::turquoise}},
                {{"Neon Arcade", "Neon"},
                 {d2::px::background{80, 80, 100, 255},
                  d2::colors::g::chartreuse,
                  d2::colors::r::magenta}},
                // Minimal
                {{"Steel/Ember", "Minimal"},
                 {d2::colors::b::steel_blue, d2::colors::r::coral, d2::colors::r::chrome_yellow}},
                {{"Monochrome", "Minimal"},
                 {d2::colors::w::dark_gray, d2::colors::w::silver, d2::colors::b::slate_blue}}
            };
        }

        D2_STYLESHEET_BEGIN(button_react)
        D2_STYLE(BackgroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_bg_button()))
        D2_STYLE(ForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_fg_button()))
        D2_INTERP_TWOWAY_AUTO(
            Hovered,
            500,
            Linear,
            BackgroundColor,
            D2_NVAR(WidgetTheme, __Preview__, wg_hbg_button())
        );
        D2_INTERP_TWOWAY_AUTO(
            Clicked,
            100,
            Linear,
            BackgroundColor,
            D2_NVAR(WidgetTheme, __Preview__, wg_pbg_button())
        );
        D2_INTERP_TWOWAY_AUTO(
            Clicked,
            100,
            Linear,
            ForegroundColor,
            D2_NVAR(WidgetTheme, __Preview__, wg_pfg_button())
        );
        D2_STYLE(FocusedColor, D2_NVAR(WidgetTheme, __Preview__, wg_hbg_button()))
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(icon_react)
        D2_STYLE(
            ForegroundColor,
            D2_NDYNAVAR(
                WidgetTheme, __Preview__, wg_fg_button(), value.stylize(d2::px::foreground::Bold)
            )
        )
        auto dvar1 = D2_NDYNAVAR(
            WidgetTheme, __Preview__, wg_hbg_button(), value.stylize(d2::px::foreground::Bold)
        );
        auto dvar2 = D2_NDYNAVAR(
            WidgetTheme, __Preview__, wg_pbg_button(), value.stylize(d2::px::foreground::Bold)
        );
        D2_INTERP_TWOWAY_AUTO(Hovered, 500, Linear, ForegroundColor, dvar1);
        D2_INTERP_TWOWAY_AUTO(Clicked, 100, Linear, ForegroundColor, dvar2);
        D2_STYLE(FocusedColor, D2_NVAR(WidgetTheme, __Preview__, wg_hbg_button()))
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(focus_color)
        D2_STYLE(FocusedColor, D2_NVAR(WidgetTheme, __Preview__, wg_hbg_button()))
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(border_color)
        D2_STYLE(ContainerBorder, true)
        D2_STYLE(BorderHorizontalColor, D2_NVAR(WidgetTheme, __Preview__, wg_border_horizontal()))
        D2_STYLE(BorderVerticalColor, D2_NVAR(WidgetTheme, __Preview__, wg_border_vertical()))
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(border_color_rough)
        D2_STYLE(ContainerBorder, true)
        D2_STYLE(BorderHorizontalColor, D2_NVAR(WidgetTheme, __Preview__, wg_border_horizontal()))
        D2_STYLE(BorderVerticalColor, D2_NVAR(WidgetTheme, __Preview__, wg_border_vertical()))
        D2_STYLE(CornerTopLeft, charset::box_tl_corner_rough)
        D2_STYLE(CornerTopRight, charset::box_tr_corner_rough)
        D2_STYLE(CornerBottomLeft, charset::box_bl_corner_rough)
        D2_STYLE(CornerBottomRight, charset::box_br_corner_rough)
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(switch_color)
        D2_STYLE(EnabledForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_hbg_button()))
        D2_STYLE(EnabledBackgroundColor, colors::w::transparent)
        D2_STYLE(DisabledForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_text()))
        D2_STYLE(DisabledBackgroundColor, colors::w::transparent)
        D2_STYLE(SeparatorColor, D2_NVAR(WidgetTheme, __Preview__, wg_border_vertical()))
        D2_STYLE(
            FocusedColor, D2_NDYNAVAR(WidgetTheme, __Preview__, wg_hbg_button(), value.alpha(0.7f))
        )
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(filled_switch_color)
        D2_STYLE(EnabledForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_pbg_button()))
        D2_STYLE(EnabledBackgroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_pfg_button()))
        D2_STYLE(DisabledForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_fg_button()))
        D2_STYLE(DisabledBackgroundColor, colors::w::transparent)
        D2_STYLE(SeparatorColor, D2_NVAR(WidgetTheme, __Preview__, wg_border_vertical()))
        D2_STYLE(
            FocusedColor, D2_NDYNAVAR(WidgetTheme, __Preview__, wg_hbg_button(), value.alpha(0.7f))
        )
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(selector_color)
        D2_STYLE(EnabledForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_bg_secondary()))
        D2_STYLE(
            EnabledBackgroundColor,
            D2_NDYNAVAR(
                WidgetTheme,
                __Preview__,
                wg_border_horizontal(),
                px::background{value.rf, value.gf, value.bf, value.af}
            )
        )
        D2_STYLE(DisabledForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_text()))
        D2_STYLE(DisabledBackgroundColor, colors::w::transparent)
        D2_STYLE(
            SliderBackgroundColor,
            D2_NDYNAVAR(
                WidgetTheme, __Preview__, wg_hbg_button(), value.extend(charset::slider_vertical)
            )
        )
        D2_STYLE(
            SliderColor,
            D2_NDYNAVAR(
                WidgetTheme,
                __Preview__,
                wg_hbg_button(),
                value.extend(charset::slider_thumb_vertical)
            )
        )
        D2_STYLE(
            FocusedColor, D2_NDYNAVAR(WidgetTheme, __Preview__, wg_hbg_button(), value.alpha(0.7f))
        )
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(input_color)
        D2_STYLE(PtrColor, D2_NVAR(WidgetTheme, __Preview__, wg_text_ptr()))
        D2_STYLE(ForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_text()))
        D2_STYLE(BackgroundColor, colors::w::transparent)
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(text_color)
        D2_STYLE(ForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_text()))
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(bold_text_color)
        D2_STYLE(
            ForegroundColor,
            D2_NDYNAVAR(
                WidgetTheme, __Preview__, wg_text(), value.stylize(d2::px::foreground::Bold)
            )
        )
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(slider_color)
        D2_STYLE(
            SliderColor,
            D2_NDYNAVAR(
                WidgetTheme,
                __Preview__,
                wg_fg_button(),
                value.extend(d2::charset::slider_thumb_horizontal)
            )
        )
        D2_STYLE(
            ForegroundColor,
            D2_NDYNAVAR(
                WidgetTheme,
                __Preview__,
                wg_fg_button(),
                value.extend(d2::charset::slider_horizontal)
            )
        )
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(vertical_slider_color)
        D2_STYLE(
            SliderColor,
            D2_NDYNAVAR(
                WidgetTheme,
                __Preview__,
                wg_fg_button(),
                value.extend(d2::charset::slider_thumb_vertical)
            )
        )
        D2_STYLE(
            ForegroundColor,
            D2_NDYNAVAR(
                WidgetTheme, __Preview__, wg_fg_button(), value.extend(d2::charset::slider_vertical)
            )
        )
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(checkbox_color)
        D2_STYLE(ColorOnForeground, D2_NVAR(WidgetTheme, __Preview__, wg_text()))
        D2_STYLE(ColorOffForeground, D2_NVAR(WidgetTheme, __Preview__, wg_text()))
        D2_STYLESHEET_END
        D2_STYLESHEET_BEGIN(scrollbox_color)
        D2_CONTEXT_AUTO(ptr->scrollbar())
        D2_STYLES_APPLY(vertical_slider_color)
        D2_CONTEXT_END
        D2_STYLESHEET_END
    } // namespace impl

    static ThemeSelector::eptr<ThemeSelectorWindow> _wcore(TreeState::ptr state)
    {
        return state->core()->traverse().as<ThemeSelectorWindow>();
    }
    static ThemeSelector::eptr<impl::ThemeSelectorBase> _bcore(TreeState::ptr state)
    {
        return state->core()->traverse().as<impl::ThemeSelectorBase>();
    }
    static ThemeSelector::eptr<style::UniversalAccessInterfaceBase> _ucore(TreeState::ptr state)
    {
        return state->core()->traverse().as<style::UniversalAccessInterfaceBase>();
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
    D2_ANCHOR(ScrollFlowBox, themes)
    D2_ANCHOR(FlowBox, builder)
    D2_ANCHOR(Checkbox, preview_toggle)
    D2_ANCHOR(Checkbox, immediate_toggle)
    auto apply_accents = [=](StandardTheme* ptr, const style::IZThemeSelector::accent_vec& accents)
    {
        if (!accents.empty())
        {
            if (accents.size() == 1)
            {
                StandardTheme::accent_dynamic(ptr, accents[0]);
            }
            else if (accents.size() == 2)
            {
                StandardTheme::accent_dynamic_duotone(ptr, accents[0], accents[1]);
            }
            else
            {
                StandardTheme::accent_dynamic_tritone(ptr, accents[0], accents[1], accents[2]);
            }
        }
    };
    auto update_accents = [=](const style::IZThemeSelector::accent_vec& accents)
    {
        if (preview_toggle->get<Checkbox::Value>() || immediate_toggle->get<Checkbox::Value>())
        {
            auto* theme = &state->screen()->theme<StandardTheme>("__Preview__");
            apply_accents(theme, accents);
        }
        if (immediate_toggle->get<Checkbox::Value>())
        {
            auto* theme = &state->screen()->theme<StandardTheme>();
            apply_accents(theme, accents);
        }
    };
    if (window)
        D2_ELEM(Button, exit)
    D2_STYLE(Value, "<X>")
    D2_STYLE(X, 1.0_pxi)
    D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
    D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
    D2_STYLE(OnSubmit, [](TreeIter<Button> ptr) { _wcore(ptr->state())->close(); })
    D2_ELEM_END
    D2_ELEM(FlowBox)
    D2_STYLE(Width, 1.0_pc)
    D2_STYLE(Height, 1.0_px)
    D2_STYLE(Y, 0.0_relative)
    D2_ELEM(Switch)
    D2_STYLE(Width, 0.0_relative)
    D2_STYLE(X, 0.0_relative)
    D2_STYLE(Options, Switch::opts{"Themes", "Builder"})
    D2_STYLE(
        OnChange,
        [=](TreeIter<Switch> ptr, int idx, int)
        {
            themes->setstate(d2::Element::Display, idx == 0);
            builder->setstate(d2::Element::Display, idx == 1);
        }
    )
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
    {
        _ucore(state)->apply_get_for<style::IThemeSelector, style::IZThemeSelector::Themes>(
            [&](const style::IZThemeSelector::theme_list& list)
            {
                for (std::size_t i = 0; i < list.size(); i++)
                {
                    auto& theme = list[i];
                    D2_ELEM(FlowBox)
                    D2_STYLE(Width, 1.0_pc)
                    D2_STYLE(Y, 0.0_relative)
                    auto add_accent = [&](px::background accent)
                    {
                        D2_ELEM(Text)
                        D2_STYLE(
                            Value,
                            std::format("[ {} {} {} {} ]", accent.r, accent.g, accent.b, accent.a)
                        )
                        D2_STYLE(X, 1.0_px)
                        D2_STYLE(Y, 0.0_relative)
                        D2_STYLE(ForegroundColor, accent.stylize(px::foreground::Style::Bold))
                        D2_ELEM_END
                    };
                    D2_STYLE(Width, 1.0_pc)
                    D2_ELEM(Text)
                    D2_STYLE(Value, theme.first.first)
                    D2_STYLE(X, 1.0_px)
                    D2_STYLE(Y, 0.0_relative)
                    D2_STYLES_APPLY(bold_text_color)
                    D2_ELEM_END
                    D2_ELEM(Text)
                    D2_STYLE(Value, theme.first.second)
                    D2_STYLE(X, 1.0_px)
                    D2_STYLE(Y, 0.0_relative)
                    D2_STYLES_APPLY(text_color)
                    D2_ELEM_END
                    for (decltype(auto) it : theme.second)
                        add_accent(it);
                    D2_STATIC(is_focused, bool)
                    D2_ON(Focused)
                    _ucore(state)
                        ->apply_get_for<style::IThemeSelector, style::IZThemeSelector::Themes>(
                            [&](const style::IZThemeSelector::theme_list& list)
                            { update_accents(list[i].second); }
                        );
                    D2_ON_END
                    D2_OFF(Focused)
                    *is_focused = false;
                    D2_OFF_END
                    D2_ON(Clicked)
                    if (*is_focused)
                        _ucore(state)
                            ->apply_get_for<style::IThemeSelector, style::IZThemeSelector::Themes>(
                                [&](const style::IZThemeSelector::theme_list& list)
                                {
                                    {
                                        auto* theme = &state->screen()->theme<StandardTheme>();
                                        apply_accents(theme, list[i].second);
                                    }
                                    {
                                        auto* theme =
                                            &state->screen()->theme<StandardTheme>("__Preview__");
                                        apply_accents(theme, list[i].second);
                                    }
                                }
                            );
                    *is_focused = true;
                    D2_ON_END
                    D2_STYLES_APPLY(border_color_rough)
                    D2_ELEM_END
                }
            }
        );
    }
    D2_STYLES_APPLY(scrollbox_color)
    D2_ELEM_END
    D2_ELEM_ANCHOR(builder)
    D2_ANCHOR(ScrollFlowBox, accents)
    D2_STATIC_STRUCT(data, d2::TreeIter<Box> focused;)
    auto set_accent = [=](px::background color)
    {
        if (data->focused != nullptr)
        {
            _bcore(state)->push(accents->index_of(data->focused) - 1, color);
            auto txt = (data->focused / "indicator").as<Text>();
            txt->set<Text::ForegroundColor>(color);
            txt->set<Text::Value>(
                std::format("[ {} {} {} {} ]", color.r, color.g, color.b, color.a)
            );
            update_accents(_bcore(state)->accents());
        }
    };
    auto add_accent = [=]()
    {
        D2_CONTEXT_AUTO(accents)
        D2_ELEM(FlowBox)
        auto root = ptr;
        D2_STYLE(X, 0.0_center)
        D2_STYLE(Y, 0.0_relative)
        D2_STYLE(Width, 2.0_pxi)
        D2_ELEM(Text, indicator)
        D2_STYLE(Value, "[ 0 0 0 0 ]")
        D2_STYLE(X, 1.0_px)
        D2_STYLES_APPLY(bold_text_color)
        D2_ELEM_END
        D2_ANCHOR(FlowBox, controls)
        D2_ELEM_ANCHOR(controls)
        D2_STYLE(X, 0.0_pxi)
        D2_STYLE(Height, 1.0_px)
        D2_ELEM(Button, up)
        D2_STYLE(Value, d2::charset::icon::arrow_up)
        D2_STYLE(X, 0.0_relative)
        D2_STYLE(
            OnSubmit,
            [=](d2::TreeIter<Button>)
            {
                const auto pidx = accents->index_of(root);
                const auto idx = pidx - 1;
                if (idx != 0)
                {
                    const auto prev = accents->at(pidx - 2);
                    const auto ptr = accents->extract(root);
                    accents->create_after(prev, ptr);
                    _bcore(state)->swap(idx, idx - 1);
                    update_accents(_bcore(state)->accents());
                }
            }
        )
        D2_STYLES_APPLY(icon_react)
        D2_ELEM_END
        D2_ELEM(Button, down)
        D2_STYLE(Value, d2::charset::icon::arrow_down)
        D2_STYLE(X, 0.0_relative)
        D2_STYLE(
            OnSubmit,
            [=](d2::TreeIter<Button>)
            {
                const auto pidx = accents->index_of(root);
                const auto idx = pidx - 1;
                const auto nxt = accents->at(pidx + 1);
                if (nxt != nullptr)
                {
                    const auto ptr = accents->extract(root);
                    accents->create_after(nxt, ptr);
                    _bcore(state)->swap(idx, idx + 1);
                    update_accents(_bcore(state)->accents());
                }
            }
        )
        D2_STYLES_APPLY(icon_react)
        D2_ELEM_END
        D2_ELEM(Button, remove)
        D2_STYLE(Value, "x")
        D2_STYLE(X, 1.0_relative)
        D2_STYLE(
            OnSubmit,
            [=](d2::TreeIter<Button> ptr)
            {
                _bcore(state)->remove(accents->index_of(root) - 1);
                accents->remove(root);
                update_accents(_bcore(state)->accents());
            }
        )
        D2_STYLES_APPLY(icon_react)
        D2_ELEM_END
        controls->setstate(Element::Display, false);
        D2_ELEM_END
        D2_ON_EXPR(RcHover, controls->setstate(Element::Display, true))
        D2_OFF_EXPR(RcHover, controls->setstate(Element::Display, false))
        D2_ON(Focused)
        if (data->focused != nullptr)
        {
            D2_CONTEXT_AUTO(data->focused)
            D2_STYLES_APPLY(border_color_rough)
            D2_CONTEXT_END
        }
        data->focused = ptr;
        D2_STYLES_APPLY(border_color)
        D2_ON_END
        D2_STYLES_APPLY(border_color_rough)
        D2_ELEM_END
        D2_CONTEXT_END
        _bcore(state)->push({});
    };
    D2_STYLE(Width, 1.0_pc)
    D2_STYLE(Height, 1.0_pc)
    D2_ELEM(FlowBox, options)
    D2_STYLE(X, 0.0_relative)
    D2_STYLE(Width, 0.0_relative)
    D2_STYLE(Height, 1.0_pc)
    D2_EMBED_ELEM_NAMED_BEGIN(ColorPicker, picker)
    D2_STYLE(Width, 1.0_pc)
    D2_STYLE(Height, 1.0_pc)
    D2_STYLE(OnSubmit, [=](TreeIter<ColorPicker> ptr, px::background color) { set_accent(color); })
    D2_ELEM(Text)
    D2_STYLE(ZIndex, Box::overlap)
    D2_STYLE(Value, "Pick Accent")
    D2_STYLE(X, 0.0_center)
    D2_STYLES_APPLY(bold_text_color)
    D2_ELEM_END
    D2_EMBED_END
    D2_ELEM_END
    D2_ELEM(VerticalSeparator)
    D2_STYLE(X, 0.0_relative)
    D2_STYLE(Width, 1.0_px)
    D2_STYLE(Height, 1.0_pc)
    D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_border_vertical()))
    D2_ELEM_END
    D2_ELEM_ANCHOR(accents)
    D2_STYLE(X, 0.0_relative)
    D2_STYLE(Width, 0.0_relative)
    D2_STYLE(Height, 1.0_pc)
    D2_ELEM(FlowBox)
    D2_STYLE(Y, 0.0_relative)
    D2_STYLE(Width, 1.0_pc)
    D2_ELEM(Button)
    D2_STYLE(Value, "+")
    D2_STYLE(Width, -1.0_relative)
    D2_STYLE(X, 0.0_relative)
    D2_STYLE(OnSubmit, [=](d2::TreeIter<Button>) { add_accent(); })
    D2_STYLES_APPLY(button_react)
    D2_ELEM_END
    D2_ELEM(Button)
    D2_STYLE(Value, "Apply")
    D2_STYLE(Width, 1.0_relative)
    D2_STYLE(X, 1.0_relative)
    D2_STYLE(
        OnSubmit,
        [=](d2::TreeIter<Button>)
        {
            apply_accents(&state->screen()->theme<StandardTheme>(), _bcore(state)->accents());
            apply_accents(
                &state->screen()->theme<StandardTheme>("__Preview__"), _bcore(state)->accents()
            );
        }
    )
    D2_STYLES_APPLY(button_react)
    D2_ELEM_END
    D2_ELEM_END
    D2_ELEM_END
    D2_ELEM_END
    D2_ELEM_END
    D2_ELEM(VerticalSeparator)
    D2_STYLE(X, 0.0_relative)
    D2_STYLE(Width, 1.0_px)
    D2_STYLE(Height, 1.0_pc)
    D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_border_vertical()))
    D2_ELEM_END
    D2_ELEM(FlowBox)
    D2_STYLE(X, 0.0_relative)
    D2_STYLE(Width, 0.0_relative)
    D2_STYLE(Height, 1.0_pc)
    D2_ELEM_ANCHOR(immediate_toggle)
    D2_STYLE(X, 1.0_relative)
    D2_STYLE(
        OnSubmit, [=](d2::TreeIter<Checkbox>, bool) { update_accents(_bcore(state)->accents()); }
    )
    D2_STYLES_APPLY(checkbox_color)
    D2_ELEM_END
    D2_ELEM(Text)
    D2_STYLE(X, 1.0_relative)
    D2_STYLE(Value, "Immediate Mode")
    D2_STYLES_APPLY(bold_text_color)
    D2_ELEM_END
    D2_ELEM_ANCHOR(preview_toggle)
    D2_STYLE(Value, true)
    D2_STYLE(X, 1.0_relative)
    D2_STYLE(Y, 0.0_relative)
    D2_STYLE(
        OnSubmit,
        [=](d2::TreeIter<Checkbox>, bool value) { update_accents(_bcore(state)->accents()); }
    )
    D2_STYLES_APPLY(checkbox_color)
    D2_ELEM_END
    D2_ELEM(Text)
    D2_STYLE(X, 1.0_relative)
    D2_STYLE(Value, "Preview Mode")
    D2_STYLES_APPLY(bold_text_color)
    D2_ELEM_END
    D2_ELEM(ScrollFlowBox, preview)
    if (!state->screen()->has_theme("__Preview__"))
        state->screen()->make_theme(
            style::Theme::make_named_copy("__Preview__", state->screen()->theme<StandardTheme>())
        );
    D2_STYLE(X, 0.0_relative)
    D2_STYLE(Y, 0.0_relative)
    D2_STYLE(Width, 1.0_pc)
    D2_STYLE(Height, 2.0_pxi)
    D2_STYLES_APPLY(impl::scrollbox_color)
    D2_ELEM(FlowBox)
    D2_STYLE(Width, 1.0_pc)
    D2_STYLE(Height, 2.0_pc)
    D2_STYLE(Y, 0.0_relative)
    D2_STYLE(BackgroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_bg_secondary()))
    D2_ELEM(Text)
    D2_STYLE(
        Value, "Lorem Ipsum Dolor, <something something>\nBlah Blah Blah, some sample words\nlol"
    )
    D2_STYLE(TextAlignment, Text::Alignment::Center)
    D2_STYLE(X, 0.0_center)
    D2_STYLE(Y, 1.0_relative)
    D2_STYLES_APPLY(impl::text_color)
    D2_ELEM_END
    D2_ELEM(Separator)
    D2_STYLE(Width, 1.0_pc)
    D2_STYLE(Y, 1.0_relative)
    D2_STYLE(ForegroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_border_horizontal()))
    D2_ELEM_END
    D2_ELEM(FlowBox)
    D2_STYLE(X, 0.0_center)
    D2_STYLE(Y, 1.0_relative)
    D2_ELEM(Checkbox)
    D2_STYLE(X, 0.0_relative)
    D2_STYLES_APPLY(impl::checkbox_color)
    D2_ELEM_END
    D2_ELEM(Text)
    D2_STYLE(Value, "Enable Something (?)")
    D2_STYLE(X, 1.0_relative)
    D2_STYLES_APPLY(impl::text_color)
    D2_ELEM_END
    D2_ELEM_END
    D2_ELEM(Slider)
    D2_STYLE(Width, 2.0_pxi)
    D2_STYLE(X, 0.0_center)
    D2_STYLE(Y, 1.0_relative)
    D2_STYLES_APPLY(impl::slider_color)
    D2_ELEM_END
    D2_ELEM(Switch)
    D2_STYLE(Options, Switch::opts{"Option A", "Option B", "Option C"})
    D2_STYLE(Width, 2.0_pxi)
    D2_STYLE(X, 0.0_center)
    D2_STYLE(Y, 1.0_relative)
    D2_STYLES_APPLY(impl::switch_color)
    D2_ELEM_END
    D2_ELEM(VerticalSwitch)
    D2_STYLE(Options, Switch::opts{"Option A", "Option B", "Option C"})
    D2_STYLE(Width, 2.0_pxi)
    D2_STYLE(X, 0.0_center)
    D2_STYLE(Y, 1.0_relative)
    D2_STYLES_APPLY(impl::selector_color)
    D2_STYLES_APPLY(impl::border_color)
    D2_ELEM_END
    D2_STYLES_APPLY(impl::border_color)
    D2_ELEM_END
    D2_ELEM(FlowBox)
    D2_STYLE(Width, 1.0_pc)
    D2_STYLE(Height, 1.0_pc)
    D2_STYLE(Y, 0.0_relative)
    D2_STYLE(BackgroundColor, D2_NVAR(WidgetTheme, __Preview__, wg_bg_secondary()))
    D2_STYLES_APPLY(impl::border_color_rough)
    D2_ELEM_END
    D2_ELEM_END
    D2_ELEM_END
    D2_ELEM_END
    D2_TREE_END

    ThemeSelector::ThemeSelector(const std::string& name, TreeState::ptr state) : data(name, state)
    {
        data::themes = impl::default_themes();
    }
    ThemeSelectorWindow::ThemeSelectorWindow(const std::string& name, TreeState::ptr state) :
        data(name, state)
    {
        data::themes = impl::default_themes();
    }

    void impl::ThemeSelectorBase::swap(std::size_t a, std::size_t b)
    {
        std::swap(_accents[a], _accents[b]);
    }
    void impl::ThemeSelectorBase::remove(std::size_t idx)
    {
        _accents.erase(_accents.begin() + idx);
    }
    void impl::ThemeSelectorBase::push(px::background accent)
    {
        _accents.push_back(accent);
    }
    void impl::ThemeSelectorBase::push(std::size_t idx, px::background accent)
    {
        _accents[idx] = accent;
    }
    void impl::ThemeSelectorBase::clear()
    {
        _accents.clear();
    }
    const style::IZThemeSelector::accent_vec& impl::ThemeSelectorBase::accents()
    {
        return _accents;
    }

    void ThemeSelectorWindow::_state_change_impl(State state, bool value)
    {
        if (state == State::Created && value)
        {
            data::width = 42.0_px;
            data::height = 16.0_px;
            data::zindex = 120;
            data::container_options |= ContainerOptions::EnableBorder;
            data::title = "<Pick Theme>";

            if (screen()->is_keynav())
            {
                const auto [swidth, sheight] = context()->input()->screen_size();
                const auto [width, height] = box();
                data::x = ((swidth - width) / 2);
                data::y = ((sheight - height) / 2);
            }
            else
            {
                const auto [x, y] = context()->input()->mouse_position();
                data::x = x - (box().width / 2);
                data::y = y;
            }

            auto& theme = this->state()->screen()->theme<WidgetTheme>();
            data::set<data::BorderHorizontalColor>(theme.wg_border_horizontal());
            data::set<data::BorderVerticalColor>(theme.wg_border_vertical());
            data::set<data::FocusedColor>(theme.wg_hbg_button());
            data::set<data::BarColor>(
                style::dynavar<[](const auto& value)
                               { return value.extend(charset::box_top_bar); }>(
                    theme.wg_border_horizontal()
                )
            );

            theme_picker::create_at(
                traverse(), TreeState::make<TreeState>(nullptr, traverse().asp(), context()), true
            );

            (at("logic") / "main" / "themes")->setstate(Element::Display, true);
            (at("logic") / "main" / "builder")->setstate(Element::Display, false);

            _update_theme_list();

            VirtualFlowBox::_state_change_impl(state, value);
        }
        else
            VirtualFlowBox::_state_change_impl(state, value);
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
        // auto ptr = (at("logic")/"main"/"themes").as<dx::DemandScrollBox>();
        // ptr->set<dx::DemandScrollBox::Length>(data::themes.size());
        // ptr->reset();
    }

    void ThemeSelectorWindow::submit(const std::string& name)
    {
        if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ThemeSelectorWindow>(shared_from_this()), name
            );
        close();
    }
    void ThemeSelectorWindow::submit()
    {
        if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ThemeSelectorWindow>(shared_from_this()), _accents
            );
        close();
    }

    void ThemeSelector::_state_change_impl(State state, bool value)
    {
        if (state == State::Created && value)
        {
            auto& theme = this->state()->screen()->theme<WidgetTheme>();
            data::set<data::ContainerBorder>(true);
            data::set<data::BorderHorizontalColor>(theme.wg_border_horizontal());
            data::set<data::BorderVerticalColor>(theme.wg_border_vertical());

            theme_picker::create_at(
                traverse(), TreeState::make<TreeState>(nullptr, traverse().asp(), context()), false
            );

            (at("logic") / "main" / "themes")->setstate(Element::Display, true);
            (at("logic") / "main" / "builder")->setstate(Element::Display, false);

            _update_theme_list();

            FlowBox::_state_change_impl(state, value);
        }
        else
            FlowBox::_state_change_impl(state, value);
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
        // auto ptr = (at("logic")/"main"/"themes").as<dx::DemandScrollBox>();
        // ptr->set<dx::DemandScrollBox::Length>(data::themes.size());
        // ptr->reset();
    }

    void ThemeSelector::submit(const std::string& name)
    {
        if (data::on_submit != nullptr)
            data::on_submit(std::static_pointer_cast<ThemeSelector>(shared_from_this()), name);
    }
    void ThemeSelector::submit()
    {
        if (data::on_submit != nullptr)
            data::on_submit(std::static_pointer_cast<ThemeSelector>(shared_from_this()), _accents);
    }
} // namespace d2::ex
