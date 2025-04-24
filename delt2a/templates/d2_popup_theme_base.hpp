#ifndef D2_POPUP_THEME_BASE_HPP
#define D2_POPUP_THEME_BASE_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "../d2_screen.hpp"
#include "../d2_dsl.hpp"

namespace d2::templ
{
	struct PopupTheme
	{
	private:
		template<typename Type>
		using dependency = style::Theme::Dependency<Type>;
	public:
		virtual dependency<px::background>& pt_bg_primary() = 0;
		virtual dependency<px::background>& pt_bg_secondary() = 0;
		virtual dependency<px::combined>& pt_border_vertical() = 0;
		virtual dependency<px::combined>& pt_border_horizontal() = 0;

		virtual dependency<px::foreground>& pt_text() = 0;
		virtual dependency<px::combined>& pt_text_ptr() = 0;

		virtual dependency<px::background>& pt_bg_button() = 0;
		virtual dependency<px::foreground>& pt_fg_button() = 0;
		virtual dependency<px::background>& pt_pbg_button() = 0;
		virtual dependency<px::foreground>& pt_pfg_button() = 0;
		virtual dependency<px::background>& pt_hbg_button() = 0;
	};

	namespace impl
	{
		D2_STYLESHEET_BEGIN(button_react)
			D2_STYLE(BackgroundColor, D2_VAR(PopupTheme, pt_bg_button()))
			D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_fg_button()))
			D2_INTERPOLATE_TWOWAY_AUTO(Hovered, 500, Linear, BackgroundColor, D2_VAR(PopupTheme, pt_hbg_button()));
			D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, BackgroundColor, D2_VAR(PopupTheme, pt_pbg_button()));
			D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, ForegroundColor, D2_VAR(PopupTheme, pt_pfg_button()));
			D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
		D2_STYLESHEET_END(button_react)
	}
}

#endif // D2_POPUP_THEME_BASE_HPP
