#ifndef D2_WIDGET_THEME_BASE_H
#define D2_WIDGET_THEME_BASE_H

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "../d2_screen.hpp"
#include "../d2_dsl.hpp"

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

	namespace impl
	{
		D2_STYLESHEET_BEGIN(button_react)
            D2_STYLE(BackgroundColor, D2_VAR(WidgetTheme, wg_bg_button()))
            D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_fg_button()))
            D2_INTERPOLATE_TWOWAY_AUTO(Hovered, 500, Linear, BackgroundColor, D2_VAR(WidgetTheme, wg_hbg_button()));
            D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, BackgroundColor, D2_VAR(WidgetTheme, wg_pbg_button()));
            D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, ForegroundColor, D2_VAR(WidgetTheme, wg_pfg_button()));
            D2_STYLE(FocusedColor, D2_VAR(WidgetTheme, wg_hbg_button()))
		D2_STYLESHEET_END(button_react)
	}
}

#endif // D2_WIDGET_THEME_BASE_H
