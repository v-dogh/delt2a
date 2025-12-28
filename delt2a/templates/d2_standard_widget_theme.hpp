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

        static void accent_dynamic_duotone(
            StandardTheme* theme,
            d2::px::background base,
            d2::px::background accent
            )
        {
            auto from_tint_manual = [](float r, float g, float b, d2::px::background tint)
            {
                return d2::px::background{
                    .r = static_cast<std::uint8_t>(tint.r * r),
                    .g = static_cast<std::uint8_t>(tint.g * g),
                    .b = static_cast<std::uint8_t>(tint.b * b),
                };
            };

            auto from_base = [&from_tint_manual, &base](float r, float g, float b)
            {
                return from_tint_manual(r, g, b, base);
            };
            auto from_accent = [&from_tint_manual, &accent](float r, float g, float b)
            {
                return from_tint_manual(r, g, b, accent);
            };

            auto mix = [](d2::px::background a, d2::px::background b, float t)
            {
                auto lerp = [t](std::uint8_t x, std::uint8_t y)
                {
                    return static_cast<std::uint8_t>(
                        static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t
                        );
                };
                return d2::px::background{
                    .r = lerp(a.r, b.r),
                    .g = lerp(a.g, b.g),
                    .b = lerp(a.b, b.b),
                };
            };

            // Backgrounds: mostly base
            auto bg_primary   = from_base(0.25f, 0.3f, 0.35f);
            auto bg_secondary = mix(from_base(0.18f, 0.2f, 0.25f),
                                    from_accent(0.18f, 0.2f, 0.25f),
                                    0.3f); // slight accent in secondary

            // Borders: brightened mix leaning toward accent
            auto border_color = mix(
                from_base(0.9f, 0.9f, 0.9f),
                from_accent(0.95f, 0.95f, 0.95f),
                0.6f
                );

            // Text: bright derived from base, pointer from accent to distinguish
            auto text_fg      = from_base(0.95f, 0.95f, 0.95f);
            auto text_ptr_fg  = from_accent(0.95f, 0.95f, 0.95f);

            // Buttons: use accent as main color family
            auto bt_normal    = from_accent(0.55f, 0.55f, 0.55f);
            auto bt_hover     = from_accent(0.75f, 0.75f, 0.75f);
            auto bt_press     = from_accent(0.95f, 0.95f, 0.95f);

            // Button text: bright from base, react foreground from accent
            auto bt_fg        = from_base(0.95f, 0.95f, 0.95f);
            auto bt_react_fg  = from_accent(0.25f, 0.25f, 0.25f);

            theme->accents(
                bg_primary,
                bg_secondary,
                border_color.extend(charset::box_border_horizontal),
                border_color.extend(charset::box_border_vertical),
                text_fg,
                text_ptr_fg.extend('_'),
                bt_normal,
                bt_hover,
                bt_press,
                bt_fg,
                bt_react_fg
                );
        }
        static void accent_dynamic_tritone(
            StandardTheme* theme,
            d2::px::background base,
            d2::px::background accent_a,
            d2::px::background accent_b
            )
        {
            auto from_tint_manual = [](float r, float g, float b, d2::px::background tint)
            {
                return d2::px::background{
                    .r = static_cast<std::uint8_t>(tint.r * r),
                    .g = static_cast<std::uint8_t>(tint.g * g),
                    .b = static_cast<std::uint8_t>(tint.b * b),
                };
            };

            auto from_base = [&from_tint_manual, &base](float r, float g, float b)
            {
                return from_tint_manual(r, g, b, base);
            };
            auto from_a = [&from_tint_manual, &accent_a](float r, float g, float b)
            {
                return from_tint_manual(r, g, b, accent_a);
            };
            auto from_b = [&from_tint_manual, &accent_b](float r, float g, float b)
            {
                return from_tint_manual(r, g, b, accent_b);
            };

            auto mix = [](d2::px::background a, d2::px::background b, float t)
            {
                auto lerp = [t](std::uint8_t x, std::uint8_t y)
                {
                    return static_cast<std::uint8_t>(
                        static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t
                        );
                };
                return d2::px::background{
                    .r = lerp(a.r, b.r),
                    .g = lerp(a.g, b.g),
                    .b = lerp(a.b, b.b),
                };
            };

            // Base is the "canvas"
            auto bg_primary   = from_base(0.22f, 0.26f, 0.3f);
            auto bg_secondary = mix(
                from_base(0.16f, 0.18f, 0.22f),
                from_a(0.18f, 0.2f, 0.25f),
                0.35f
                );

            // Borders use accent A, slightly mixed with base for readability
            auto border_color = mix(
                from_a(0.9f, 0.9f, 0.9f),
                from_base(0.8f, 0.8f, 0.8f),
                0.4f
                );

            // Text = high-contrast from base, cursor = accent B to stand out
            auto text_fg      = from_base(0.95f, 0.95f, 0.95f);
            auto text_ptr_fg  = from_b(0.95f, 0.95f, 0.95f);

            // Buttons:
            //   normal/hover from accent A,
            //   press from accent B for a stronger "action" feel
            auto bt_normal    = from_a(0.55f, 0.55f, 0.55f);
            auto bt_hover     = from_a(0.75f, 0.75f, 0.75f);
            auto bt_press     = from_b(0.9f, 0.9f, 0.9f);

            // Button text: bright neutral (mix of base + accent B)
            auto bt_fg        = mix(
                from_base(0.95f, 0.95f, 0.95f),
                from_b(0.95f, 0.95f, 0.95f),
                0.3f
                );

            // React foreground: darker accent B
            auto bt_react_fg  = from_b(0.25f, 0.25f, 0.25f);

            theme->accents(
                bg_primary,
                bg_secondary,
                border_color.extend(charset::box_border_horizontal),
                border_color.extend(charset::box_border_vertical),
                text_fg,
                text_ptr_fg.extend('_'),
                bt_normal,
                bt_hover,
                bt_press,
                bt_fg,
                bt_react_fg
                );
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
