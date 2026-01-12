#include "d2_exit_prompt.hpp"
#include "d2_widget_theme_base.hpp"
#include "../elements/d2_text.hpp"
#include "../elements/d2_button.hpp"
#include "../elements/d2_flow_box.hpp"
#include "../d2_colors.hpp"

namespace d2::ctm
{
    using namespace dx;
    D2_TREE_DEFINE(exit_prompt)
        D2_STYLE(ZIndex, 126)
        D2_STYLE(Width, 1.0_pc)
        D2_STYLE(Height, 1.0_pc)
        D2_STYLE(BackgroundColor, d2::colors::w::black.alpha(0.7f))
        D2_STYLE(ForegroundColor, d2::colors::w::black.alpha(0.7f))
        D2_ELEM(Box)
            D2_STYLE(X, 0.0_center)
            D2_STYLE(Y, 0.0_center)
            D2_STYLE(Width, 32.0_px)
            D2_STYLE(Height, 4.0_px)
            D2_STYLE(BackgroundColor, d2::colors::w::transparent)
            D2_STYLE(ContainerBorder, true)
            D2_STYLE(BorderHorizontalColor, D2_DYNAVAR(WidgetTheme, wg_border_horizontal(),
                value.extend(charset::box_border_horizontal).balpha(0.f)
            ))
            D2_STYLE(BorderVerticalColor, D2_DYNAVAR(WidgetTheme, wg_border_vertical(),
                value.extend(charset::box_border_vertical).balpha(0.f)
            ))
            D2_ELEM(Text)
                D2_STYLE(Value, "Exit? Are You Sure?")
                D2_STYLE(X, 0.0_center)
                D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
            D2_ELEM_END
            D2_ELEM(FlowBox)
                D2_STYLE(X, 0.0_center)
                D2_STYLE(Y, 1.0_px)
                D2_ELEM(Button)
                    D2_STYLE(Value, "CONFIRM")
                    D2_STYLE(X, 0.0_relative)
                    D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
                    D2_STYLE(FocusedColor, colors::r::red.alpha(0.8f))
                    D2_STYLE(OnSubmit, [](TreeIter ptr) {
                        ptr->screen()->stop_blocking();
                    })
                    D2_INTERP_TWOWAY_AUTO(Hovered, 500, Linear, ForegroundColor, colors::r::red);
                D2_ELEM_END
                D2_ELEM(Button)
                    D2_STYLE(Value, "CANCEL")
                    D2_STYLE(ForegroundColor, D2_VAR(WidgetTheme, wg_text()))
                    D2_STYLE(BackgroundColor, d2::colors::w::transparent)
                    D2_STYLE(FocusedColor, colors::g::green.alpha(0.8f))
                    D2_STYLE(X, 1.0_relative)
                    D2_STYLE(OnSubmit, [](TreeIter ptr) {
                        auto state = ptr->state();
                        state->root()->remove(state->core());
                    })
                    D2_INTERP_TWOWAY_AUTO(Hovered, 500, Linear, ForegroundColor, colors::g::green);
                D2_ELEM_END
            D2_ELEM_END
        D2_ELEM_END
    D2_TREE_DEFINITION_END
}
