#include "d2_default_theme.hpp"
#include <d2_chardef.hpp>
#include <d2_colors.hpp>
#include <d2_locale.hpp>
#include <d2_pixel.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace d2::ex
{
    using component = px::component;

    namespace
    {
        static component clamp_channel(float value)
        {
            if (value <= 0.0f)
                return 0;

            if (value >= 255.0f)
                return 255;

            return static_cast<component>(value + 0.5f);
        }

        static px::background rgb(component r, component g, component b)
        {
            return px::background{.r = r, .g = g, .b = b};
        }

        static px::foreground to_foreground(px::background color)
        {
            return px::foreground{
                .r = color.r,
                .g = color.g,
                .b = color.b,
            };
        }

        static px::combined to_combined(px::background bg, px::background fg, value_type ch)
        {
            return px::combined{
                .r = bg.r,
                .g = bg.g,
                .b = bg.b,

                .rf = fg.r,
                .gf = fg.g,
                .bf = fg.b,

                .v = ch,
            };
        }

        static px::combined to_border_horizontal(px::background fg)
        {
            return to_combined(d2::colors::w::transparent, fg, charset::box_border_horizontal);
        }

        static px::combined to_border_vertical(px::background fg)
        {
            return to_combined(d2::colors::w::transparent, fg, charset::box_border_vertical);
        }

        static px::background from_tint(float r, float g, float b, px::background tint)
        {
            return px::background{
                .r = clamp_channel(static_cast<float>(tint.r) * r),
                .g = clamp_channel(static_cast<float>(tint.g) * g),
                .b = clamp_channel(static_cast<float>(tint.b) * b),
            };
        }

        static px::background scale_tint(px::background tint, float amount)
        {
            return from_tint(amount, amount, amount, tint);
        }

        static px::background mix(px::background a, px::background b, float t)
        {
            t = std::clamp(t, 0.0f, 1.0f);

            auto lerp = [t](component x, component y)
            {
                return clamp_channel(
                    static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t
                );
            };

            return px::background{
                .r = lerp(a.r, b.r),
                .g = lerp(a.g, b.g),
                .b = lerp(a.b, b.b),
            };
        }

        static px::background accent_at(std::vector<px::background> const& accents, std::size_t i)
        {
            return accents[i % accents.size()];
        }

        static px::background average_accent(std::vector<px::background> const& accents)
        {
            std::uint32_t r = 0;
            std::uint32_t g = 0;
            std::uint32_t b = 0;

            for (auto accent : accents)
            {
                r += accent.r;
                g += accent.g;
                b += accent.b;
            }

            auto count = static_cast<std::uint32_t>(accents.size());

            return rgb(
                static_cast<component>(r / count),
                static_cast<component>(g / count),
                static_cast<component>(b / count)
            );
        }

        static float luminance(px::background color)
        {
            auto chan = [](component c)
            {
                float x = static_cast<float>(c) / 255.0f;

                return x <= 0.04045f ? x / 12.92f : std::pow((x + 0.055f) / 1.055f, 2.4f);
            };

            return 0.2126f * chan(color.r) + 0.7152f * chan(color.g) + 0.0722f * chan(color.b);
        }

        static float contrast_ratio(px::background a, px::background b)
        {
            float la = luminance(a);
            float lb = luminance(b);

            if (la < lb)
                std::swap(la, lb);

            return (la + 0.05f) / (lb + 0.05f);
        }

        static px::background black_or_white_on(px::background bg)
        {
            auto black = rgb(0, 0, 0);
            auto white = rgb(255, 255, 255);

            return contrast_ratio(bg, black) >= contrast_ratio(bg, white) ? black : white;
        }

        static px::background
        readable_on(px::background preferred, px::background bg, float min_ratio)
        {
            if (contrast_ratio(preferred, bg) >= min_ratio)
                return preferred;

            auto toward_white = mix(preferred, rgb(255, 255, 255), 0.72f);
            if (contrast_ratio(toward_white, bg) >= min_ratio)
                return toward_white;

            auto toward_black = mix(preferred, rgb(0, 0, 0), 0.72f);
            if (contrast_ratio(toward_black, bg) >= min_ratio)
                return toward_black;

            return black_or_white_on(bg);
        }
    } // namespace

    void DefaultTheme::from(std::vector<px::background> accents)
    {
        if (accents.empty())
        {
            auto black = rgb(0, 0, 0);
            auto white = rgb(255, 255, 255);

            get<Surface>() = black;
            get<SurfaceAlt>() = rgb(24, 24, 24);
            get<SurfaceRaised>() = rgb(36, 36, 36);
            get<SurfaceSunken>() = rgb(12, 12, 12);
            get<SurfaceOverlay>() = rgb(48, 48, 48);
            get<SurfaceDisabled>() = rgb(28, 28, 28);

            get<SurfaceContrast>() = white;

            get<BorderHoriz>() = to_border_horizontal(black);
            get<BorderVert>() = to_border_vertical(black);

            get<BorderMutedHoriz>() = to_border_horizontal(rgb(160, 160, 160));
            get<BorderMutedVert>() = to_border_vertical(rgb(160, 160, 160));

            get<BorderStrongHoriz>() = to_border_horizontal(white);
            get<BorderStrongVert>() = to_border_vertical(white);

            get<BorderFocusHoriz>() = to_border_horizontal(white);
            get<BorderFocusVert>() = to_border_vertical(white);

            get<BorderContrastHoriz>() = to_border_horizontal(white);
            get<BorderContrastVert>() = to_border_vertical(white);

            get<Text>() = to_foreground(white);
            get<TextMuted>() = to_foreground(rgb(200, 200, 200));
            get<TextDisabled>() = to_foreground(rgb(110, 110, 110));
            get<TextInverse>() = to_foreground(black);
            get<TextAccent>() = to_foreground(white);
            get<TextContrast>() = to_foreground(black);

            get<Accent>() = white;
            get<AccentHover>() = rgb(220, 220, 220);
            get<AccentActive>() = rgb(180, 180, 180);
            get<AccentMuted>() = rgb(90, 90, 90);
            get<AccentText>() = to_foreground(black);

            get<Cursor>() = to_combined(black, white, '_');

            get<Selection>() = white;
            get<SelectionInactive>() = rgb(90, 90, 90);
            get<SelectionText>() = to_foreground(black);

            get<Info>() = white;
            get<InfoText>() = to_foreground(black);

            get<Success>() = white;
            get<SuccessText>() = to_foreground(black);

            get<Warning>() = white;
            get<WarningText>() = to_foreground(black);

            get<Error>() = white;
            get<ErrorText>() = to_foreground(black);

            get<Highlight>() = rgb(55, 55, 55);
            get<HighlightInactive>() = rgb(35, 35, 35);

            return;
        }

        auto base = accent_at(accents, 0);
        auto accent_a = accent_at(accents, accents.size() >= 2 ? 1 : 0);
        auto accent_b = accent_at(accents, accents.size() >= 3 ? 2 : 0);
        auto focus = accent_at(accents, accents.size() >= 4 ? 3 : 1);
        auto contrast = accent_at(accents, accents.size() >= 5 ? 4 : 0);

        auto info_seed = accent_at(accents, accents.size() >= 6 ? 5 : 1);
        auto success_seed = accent_at(accents, accents.size() >= 7 ? 6 : 2);
        auto warning_seed = accent_at(accents, accents.size() >= 8 ? 7 : 3);
        auto error_seed = accent_at(accents, accents.size() >= 9 ? 8 : 4);

        auto avg = average_accent(accents);

        auto from_base = [base](float r, float g, float b) { return from_tint(r, g, b, base); };

        auto from_a = [accent_a](float r, float g, float b)
        { return from_tint(r, g, b, accent_a); };

        auto from_b = [accent_b](float r, float g, float b)
        { return from_tint(r, g, b, accent_b); };

        auto from_focus = [focus](float r, float g, float b) { return from_tint(r, g, b, focus); };

        auto from_contrast = [contrast](float r, float g, float b)
        { return from_tint(r, g, b, contrast); };

        auto surface =
            accents.size() >= 3 ? from_base(0.22f, 0.26f, 0.30f) : from_base(0.25f, 0.30f, 0.35f);

        auto surface_alt = accents.size() >= 2 ? mix(from_base(0.16f, 0.18f, 0.22f),
                                                     from_a(0.18f, 0.20f, 0.25f),
                                                     accents.size() >= 3 ? 0.35f : 0.30f)
                                               : from_base(0.18f, 0.20f, 0.25f);

        auto surface_raised =
            accents.size() >= 2
                ? mix(from_base(0.30f, 0.35f, 0.40f), from_a(0.24f, 0.28f, 0.32f), 0.25f)
                : from_base(0.30f, 0.35f, 0.40f);

        auto surface_sunken = mix(from_base(0.12f, 0.14f, 0.18f), scale_tint(avg, 0.12f), 0.35f);

        auto surface_overlay =
            accents.size() >= 2
                ? mix(from_base(0.34f, 0.39f, 0.44f), from_a(0.28f, 0.32f, 0.36f), 0.30f)
                : from_base(0.34f, 0.39f, 0.44f);

        auto surface_disabled = mix(surface, surface_sunken, 0.55f);

        auto surface_contrast = from_contrast(0.92f, 0.92f, 0.92f);

        auto border_raw =
            accents.size() >= 3
                ? mix(from_a(0.90f, 0.90f, 0.90f), from_base(0.80f, 0.80f, 0.80f), 0.40f)
                : mix(from_base(0.90f, 0.90f, 0.90f), from_a(0.95f, 0.95f, 0.95f), 0.60f);

        auto border = readable_on(border_raw, surface, 3.0f);
        auto border_muted = readable_on(mix(border, surface, 0.42f), surface, 2.2f);

        auto border_strong = readable_on(
            mix(from_base(1.00f, 1.00f, 1.00f), from_a(1.00f, 1.00f, 1.00f), 0.60f), surface, 4.5f
        );

        auto border_focus = readable_on(from_focus(0.95f, 0.95f, 0.95f), surface, 4.5f);

        auto border_contrast =
            readable_on(from_contrast(0.24f, 0.24f, 0.24f), surface_contrast, 4.5f);

        auto text = readable_on(from_base(0.95f, 0.95f, 0.95f), surface, 4.5f);
        auto text_muted = readable_on(mix(text, surface, 0.36f), surface, 3.0f);
        auto text_disabled = readable_on(mix(text, surface, 0.62f), surface, 2.0f);

        auto text_inverse = readable_on(from_base(0.18f, 0.18f, 0.18f), text, 4.5f);

        auto text_accent = readable_on(
            accents.size() >= 2 ? from_a(0.95f, 0.95f, 0.95f) : from_base(0.95f, 0.95f, 0.95f),
            surface,
            4.5f
        );

        auto text_contrast =
            readable_on(from_contrast(0.22f, 0.22f, 0.22f), surface_contrast, 4.5f);

        auto accent =
            accents.size() >= 2 ? from_a(0.55f, 0.55f, 0.55f) : from_base(0.50f, 0.50f, 0.50f);

        auto accent_hover =
            accents.size() >= 2 ? from_a(0.75f, 0.75f, 0.75f) : from_base(0.75f, 0.75f, 0.75f);

        auto accent_active =
            accents.size() >= 3 ? from_b(0.90f, 0.90f, 0.90f) : from_a(0.95f, 0.95f, 0.95f);

        auto accent_muted = mix(accent, surface, 0.42f);

        auto accent_text = readable_on(
            accents.size() >= 3 ? from_b(0.25f, 0.25f, 0.25f) : from_base(0.95f, 0.95f, 0.95f),
            accent,
            4.5f
        );

        auto cursor_fg = readable_on(
            accents.size() >= 3 ? from_b(0.95f, 0.95f, 0.95f) : from_a(0.95f, 0.95f, 0.95f),
            surface_sunken,
            4.5f
        );

        auto selection = readable_on(mix(accent_active, surface, 0.18f), surface, 3.0f);
        auto selection_inactive = mix(selection, surface_alt, 0.50f);

        auto selection_text = readable_on(from_base(0.16f, 0.16f, 0.16f), selection, 4.5f);

        auto info = readable_on(scale_tint(info_seed, 0.72f), surface, 3.0f);
        auto success = readable_on(scale_tint(success_seed, 0.72f), surface, 3.0f);
        auto warning = readable_on(scale_tint(warning_seed, 0.82f), surface, 3.0f);
        auto error = readable_on(scale_tint(error_seed, 0.82f), surface, 3.0f);

        auto info_text = readable_on(scale_tint(info_seed, 0.20f), info, 4.5f);
        auto success_text = readable_on(scale_tint(success_seed, 0.20f), success, 4.5f);
        auto warning_text = readable_on(scale_tint(warning_seed, 0.20f), warning, 4.5f);
        auto error_text = readable_on(scale_tint(error_seed, 0.20f), error, 4.5f);

        auto highlight = mix(accent_hover, surface, 0.28f);
        auto highlight_inactive = mix(highlight, surface_alt, 0.52f);

        get<Surface>() = surface;
        get<SurfaceAlt>() = surface_alt;
        get<SurfaceRaised>() = surface_raised;
        get<SurfaceSunken>() = surface_sunken;
        get<SurfaceOverlay>() = surface_overlay;
        get<SurfaceDisabled>() = surface_disabled;

        get<SurfaceContrast>() = surface_contrast;

        get<BorderHoriz>() = to_border_horizontal(border);
        get<BorderVert>() = to_border_vertical(border);

        get<BorderMutedHoriz>() = to_border_horizontal(border_muted);
        get<BorderMutedVert>() = to_border_vertical(border_muted);

        get<BorderStrongHoriz>() = to_border_horizontal(border_strong);
        get<BorderStrongVert>() = to_border_vertical(border_strong);

        get<BorderFocusHoriz>() = to_border_horizontal(border_focus);
        get<BorderFocusVert>() = to_border_vertical(border_focus);

        get<BorderContrastHoriz>() = to_border_horizontal(border_contrast);
        get<BorderContrastVert>() = to_border_vertical(border_contrast);

        get<Text>() = to_foreground(text);
        get<TextMuted>() = to_foreground(text_muted);
        get<TextDisabled>() = to_foreground(text_disabled);
        get<TextInverse>() = to_foreground(text_inverse);
        get<TextAccent>() = to_foreground(text_accent);
        get<TextContrast>() = to_foreground(text_contrast);

        get<Accent>() = accent;
        get<AccentHover>() = accent_hover;
        get<AccentActive>() = accent_active;
        get<AccentMuted>() = accent_muted;
        get<AccentText>() = to_foreground(accent_text);

        get<Cursor>() = to_combined(surface_sunken, cursor_fg, '_');

        get<Selection>() = selection;
        get<SelectionInactive>() = selection_inactive;
        get<SelectionText>() = to_foreground(selection_text);

        get<Info>() = info;
        get<InfoText>() = to_foreground(info_text);

        get<Success>() = success;
        get<SuccessText>() = to_foreground(success_text);

        get<Warning>() = warning;
        get<WarningText>() = to_foreground(warning_text);

        get<Error>() = error;
        get<ErrorText>() = to_foreground(error_text);

        get<Highlight>() = highlight;
        get<HighlightInactive>() = highlight_inactive;
    }
} // namespace d2::ex
