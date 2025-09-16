#ifndef D2_WIDGET_THEME_BASE_H
#define D2_WIDGET_THEME_BASE_H

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "../d2_screen.hpp"
#include "../d2_dsl.hpp"
#include "../d2_colors.hpp"

namespace d2::ctm
{
    struct WidgetTheme
	{
	private:
		template<typename Type>
		using dependency = style::Theme::Dependency<Type>;
	public:
        virtual dependency<px::background>& wg_bg_primary() = 0;
        virtual dependency<px::background>& wg_bg_secondary() = 0;
        virtual dependency<px::combined>& wg_border_vertical() = 0;
        virtual dependency<px::combined>& wg_border_horizontal() = 0;

        virtual dependency<px::foreground>& wg_text() = 0;
        virtual dependency<px::combined>& wg_text_ptr() = 0;

        virtual dependency<px::background>& wg_bg_button() = 0;
        virtual dependency<px::foreground>& wg_fg_button() = 0;
        virtual dependency<px::background>& wg_pbg_button() = 0;
        virtual dependency<px::foreground>& wg_pfg_button() = 0;
        virtual dependency<px::background>& wg_hbg_button() = 0;
	};

    D2_STYLESHEET_BEGIN(button_react)
        D2_STYLE(BackgroundColor, D2_VAR(WidgetTheme, wg_bg_button()))
        D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_fg_button()))
        D2_INTERPOLATE_TWOWAY_AUTO(Hovered, 500, Linear, BackgroundColor, D2_VAR(WidgetTheme, wg_hbg_button()));
        D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, BackgroundColor, D2_VAR(WidgetTheme, wg_pbg_button()));
        D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, ForegroundColor, D2_VAR(WidgetTheme, wg_pfg_button()));
        D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(focus_color)
        D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(border_color)
        D2_STYLE(ContainerBorder, true)
        D2_STYLE(BorderHorizontalColor, D2_VAR(WidgetTheme, wg_border_horizontal()))
        D2_STYLE(BorderVerticalColor, D2_VAR(WidgetTheme, wg_border_vertical()))
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(switch_color)
        D2_STYLE(EnabledForegroundColor, D2_VAR(WidgetTheme, wg_hbg_button()))
        D2_STYLE(EnabledBackgroundColor, colors::w::transparent)
        D2_STYLE(DisabledForegroundColor, D2_VAR(WidgetTheme, wg_text()))
        D2_STYLE(DisabledBackgroundColor, colors::w::transparent)
        D2_STYLE(SeparatorColor, D2_VAR(WidgetTheme, wg_border_vertical()))
        D2_STYLE(FocusedColor, D2_DYNAVAR(WidgetTheme, wg_hbg_button(), value.alpha(0.7f)))
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(input_color)
        D2_STYLE(PtrColor, D2_VAR(WidgetTheme, wg_text_ptr()))
        D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
        D2_STYLE(BackgroundColor, colors::w::transparent)
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(text_color)
        D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
    D2_STYLESHEET_END
}

#endif // D2_WIDGET_THEME_BASE_H
