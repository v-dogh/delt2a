#include "d2_exit_prompt.hpp"
#include "d2_popup_theme_base.hpp"
#include "../elements/d2_text.hpp"
#include "../elements/d2_button.hpp"
#include "../d2_colors.hpp"

namespace d2::ctm
{
	using namespace dx;
	D2_TREE_DEFINE(exit_prompt)
		D2_ELEM_NESTED(Box)
			D2_STYLE(ZIndex, 126)
			D2_STYLE(Width, 1.0_pc)
			D2_STYLE(Height, 1.0_pc)
			D2_STYLE(BackgroundColor, d2::colors::w::black.alpha(0.7f))
			D2_STYLE(ForegroundColor, d2::colors::w::black.alpha(0.7f))
		D2_UELEM_NESTED_BODY
			D2_ELEM_NESTED(Box)
				D2_STYLE(X, 0.0_center)
				D2_STYLE(Y, 0.0_center)
				D2_STYLE(Width, 32.0_px)
				D2_STYLE(Height, 5.0_px)
				D2_STYLE(BackgroundColor, d2::colors::w::transparent)
				D2_STYLE(ContainerBorder, true)
				D2_STYLE(BorderHorizontalColor, D2_DYNAVAR(PopupTheme, pt_border_horizontal(),
					value.extend('+').balpha(0)
				))
				D2_STYLE(BorderVerticalColor, D2_DYNAVAR(PopupTheme, pt_border_vertical(),
					value.extend('+').balpha(0)
				))
			D2_UELEM_NESTED_BODY
				D2_ELEM(Text)
					D2_STYLE(Value, "Exit? Are You Sure?")
					D2_STYLE(X, 0.0_center)
					D2_STYLE(Y, 0.0_center)
					D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
				D2_ELEM_END
				D2_ELEM_NESTED(Box)
					D2_STYLE(X, 0.0_center)
					D2_STYLE(Y, 1.0_center)
				D2_UELEM_NESTED_BODY
					D2_ELEM(Button)
						D2_STYLE(Value, "CONFIRM")
						D2_STYLE(X, 0.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(FocusedColor, colors::r::red.alpha(0.8f))
						D2_STYLE(OnSubmit, [](Element::TraversalWrapper ptr) {
							ptr->screen()->stop_blocking();
						})
						D2_INTERPOLATE_TWOWAY_AUTO(Hovered, 500, Linear, ForegroundColor, colors::r::red);
					D2_ELEM_END
					D2_ELEM(Button)
						D2_STYLE(Value, "CANCEL")
						D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
						D2_STYLE(BackgroundColor, d2::colors::w::transparent)
						D2_STYLE(FocusedColor, colors::g::green.alpha(0.8f))
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(OnSubmit, [](Element::TraversalWrapper ptr) {
							// lol (go up to root of this tree)
							// idk why cant i just implement IDs
							const auto root = ptr.up(3);
							root
								.up().asp()
								->remove(root.as());
						})
						D2_INTERPOLATE_TWOWAY_AUTO(Hovered, 500, Linear, ForegroundColor, colors::g::green);
					D2_ELEM_END
				D2_ELEM_NESTED_END
			D2_ELEM_NESTED_END
		D2_ELEM_NESTED_END
	D2_TREE_DEFINITION_END
}
