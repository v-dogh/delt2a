#ifndef D2_DEFAULT_THEME_HPP
#define D2_DEFAULT_THEME_HPP

#include <d2_pixel.hpp>
#include <d2_theme.hpp>

namespace d2::ex
{
    struct DefaultThemeDeps
    {
        enum Deps
        {
            // Surface
            Surface,
            SurfaceAlt,
            SurfaceRaised,
            SurfaceSunken,
            SurfaceOverlay,
            SurfaceDisabled,

            // Contrast
            SurfaceContrast,

            // Borders
            BorderHoriz,
            BorderVert,
            BorderMutedHoriz,
            BorderMutedVert,
            BorderStrongHoriz,
            BorderStrongVert,
            BorderFocusHoriz,
            BorderFocusVert,
            BorderContrastHoriz,
            BorderContrastVert,

            // Text
            Text,
            TextMuted,
            TextDisabled,
            TextInverse,
            TextAccent,
            TextContrast,

            // Accent
            Accent,
            AccentHover,
            AccentActive,
            AccentMuted,
            AccentText,
            Cursor,

            // Selection
            Selection,
            SelectionInactive,
            SelectionText,

            // Semantic
            Info,
            InfoText,
            Success,
            SuccessText,
            Warning,
            WarningText,
            Error,
            ErrorText,

            // Misc
            Highlight,
            HighlightInactive,
        };
    };

    struct DefaultTheme : style::DeclTheme<
                              DefaultTheme,
                              DefaultThemeDeps,

                              // Surface
                              style::Dep<px::background, "Surface">,
                              style::Dep<px::background, "Surface Alternative">,
                              style::Dep<px::background, "Surface Raised">,
                              style::Dep<px::background, "Surface Sunken">,
                              style::Dep<px::background, "Surface Overlay">,
                              style::Dep<px::background, "Surface Disabled">,

                              // Contrast
                              style::Dep<px::background, "Surface Contrast">,

                              // Borders
                              style::Dep<px::combined, "Horizontal Border">,
                              style::Dep<px::combined, "Vertical Border">,

                              style::Dep<px::combined, "Muted Horizontal Border">,
                              style::Dep<px::combined, "Muted Vertical Border">,

                              style::Dep<px::combined, "Strong Horizontal Border">,
                              style::Dep<px::combined, "Strong Vertical Border">,

                              style::Dep<px::combined, "Focus Horizontal Border">,
                              style::Dep<px::combined, "Focus Vertical Border">,

                              style::Dep<px::combined, "Contrast Horizontal Border">,
                              style::Dep<px::combined, "Contrast Vertical Border">,

                              // Text
                              style::Dep<px::foreground, "Text">,
                              style::Dep<px::foreground, "Muted Text">,
                              style::Dep<px::foreground, "Disabled Text">,
                              style::Dep<px::foreground, "Inverse Text">,
                              style::Dep<px::foreground, "Accent Text">,
                              style::Dep<px::foreground, "Contrast Text">,

                              // Accent
                              style::Dep<px::background, "Accent">,
                              style::Dep<px::background, "Accent Hover">,
                              style::Dep<px::background, "Accent Active">,
                              style::Dep<px::background, "Muted Accent">,
                              style::Dep<px::foreground, "Accent Foreground">,
                              style::Dep<px::combined, "Cursor">,

                              // Selection
                              style::Dep<px::background, "Selection">,
                              style::Dep<px::background, "Inactive Selection">,
                              style::Dep<px::foreground, "Selection Text">,

                              // Semantic
                              style::Dep<px::background, "Info">,
                              style::Dep<px::foreground, "Info Text">,
                              style::Dep<px::background, "Success">,
                              style::Dep<px::foreground, "Success Text">,
                              style::Dep<px::background, "Warning">,
                              style::Dep<px::foreground, "Warning Text">,
                              style::Dep<px::background, "Error">,
                              style::Dep<px::foreground, "Error Text">,

                              // Misc
                              style::Dep<px::background, "Highlight">,
                              style::Dep<px::background, "Inactive Highlight">
                          >
    {
        void from(std::vector<px::background> accents);
    };
} // namespace d2::ex

namespace d2::style
{
    template<> struct ThemeRegistry<ctm::DefaultTheme::Deps>
    {
        using type = ctm::DefaultTheme;
    };
} // namespace d2::style

#endif
