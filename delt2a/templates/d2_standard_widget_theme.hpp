#ifndef D2_STANDARD_WIDGET_THEME_HPP
#define D2_STANDARD_WIDGET_THEME_HPP

#include "d2_widget_theme_base.hpp"
#include "../d2_colors.hpp"

namespace d2::ctm
{
    struct StandardTheme : d2::style::Theme, d2::ctm::WidgetTheme
    {
        D2_DEPENDENCY(d2::px::background, primary_background)
        D2_DEPENDENCY(d2::px::background, secondary_background)
        D2_DEPENDENCY(d2::px::combined, border_vertical)
        D2_DEPENDENCY(d2::px::combined, border_horizontal)
        D2_DEPENDENCY(d2::px::foreground, text)
        D2_DEPENDENCY(d2::px::combined, text_ptr)
        D2_DEPENDENCY(d2::px::background, button)
        D2_DEPENDENCY(d2::px::foreground, button_foreground)
        D2_DEPENDENCY(d2::px::foreground, button_react_foreground)
        D2_DEPENDENCY(d2::px::background, button_hover)
        D2_DEPENDENCY(d2::px::background, button_press)

        D2_DEPENDENCY_LINK(wg_bg_primary, primary_background)
        D2_DEPENDENCY_LINK(wg_bg_secondary, secondary_background)
        D2_DEPENDENCY_LINK(wg_border_vertical, border_vertical)
        D2_DEPENDENCY_LINK(wg_border_horizontal, border_horizontal)

        D2_DEPENDENCY_LINK(wg_text, text)
        D2_DEPENDENCY_LINK(wg_text_ptr, text_ptr)

        D2_DEPENDENCY_LINK(wg_bg_button, button)
        D2_DEPENDENCY_LINK(wg_fg_button, button_foreground)
        D2_DEPENDENCY_LINK(wg_pbg_button, button_press)
        D2_DEPENDENCY_LINK(wg_pfg_button, button_react_foreground)
        D2_DEPENDENCY_LINK(wg_hbg_button, button_hover)

        void accents(
            d2::px::background bgp,
            d2::px::background bgs,
            d2::px::combined bdh,
            d2::px::combined bdv,
            d2::px::foreground txt,
            d2::px::foreground crs,
            d2::px::background bt,
            d2::px::background bth,
            d2::px::background btp,
            d2::px::foreground btf,
            d2::px::foreground btrf
            )
        {
            primary_background = bgp;
            secondary_background = bgs;
            border_horizontal = bdh;
            border_vertical = bdv;
            text = txt;
            text_ptr = crs;
            button = bt;
            button_hover = bth;
            button_press = btp;
            button_foreground = btf;
            button_react_foreground = btrf;
        }

        static void accent_dynamic(StandardTheme* theme, d2::px::background tint)
        {
            auto from_tint_manual = [](float r, float g, float b, d2::px::background tint)
            {
                return d2::px::background{
                    .r = static_cast<std::uint8_t>(tint.r * r),
                    .g = static_cast<std::uint8_t>(tint.g * g),
                    .b = static_cast<std::uint8_t>(tint.b * b),
                };
            };
            auto from_tint = [&tint, &from_tint_manual](float r, float g, float b)
            {
                return from_tint_manual(r, g, b, tint);
            };

            theme->accents(
                from_tint(0.25f, 0.3f, 0.35f),
                from_tint(0.18f, 0.2f, 0.25f),
                from_tint(0.95f, 0.95f, 0.95f).extend(charset::box_border_horizontal),
                from_tint(0.95f, 0.95f, 0.95f).extend(charset::box_border_vertical),
                from_tint(0.95f, 0.95f, 0.95f),
                from_tint(0.95f, 0.95f, 0.95f).extend('_'),
                from_tint(0.5f, 0.5f, 0.5f),
                from_tint(0.75f, 0.75f, 0.75f),
                from_tint(0.95f, 0.95f, 0.95f),
                from_tint(0.95f, 0.95f, 0.95f),
                from_tint(0.25f, 0.25f, 0.25f)
                );
        }
        static void accent_default(StandardTheme* theme)
        {
            accent_dynamic(theme, d2::colors::b::slate_blue);
        }
    };
}

#endif // D2_STANDARD_WIDGET_THEME_HPP
