#pragma once

#include <core/platform/d2_locale.hpp>

namespace d2
{
    struct Box
    {
        int width{0};
        int height{0};
    };
    struct Position
    {
        int x{0};
        int y{0};
    };

    // Pixel stuff

    struct PixelBase;
    struct PixelBackground;
    struct PixelForeground;

    struct PixelBackground
    {
        using component = std::uint8_t;
        using scalar = std::uint32_t;

        component r{0};
        component g{0};
        component b{0};
        component a{255};

        static scalar to_scalar(const PixelBackground& px);
        static PixelBackground from_scalar(const scalar& px);

        PixelForeground extend(value_type v = ' ', component style = 0x00) const;
        PixelBackground alpha(float value) const;
        PixelBackground alpha(component value) const;
        PixelBackground invert() const;
        PixelBackground mask(const PixelBackground& px, float progress = 0.5f) const;
        PixelBackground stylize(component style) const;

        PixelForeground operator|(component flag) const;

        operator PixelBase() const;
        operator PixelForeground() const;

        PixelBackground& operator=(const PixelBackground&) = default;
        PixelBackground& operator=(const PixelForeground& copy);
        PixelBackground& operator=(const PixelBase& copy);

        auto operator<=>(const PixelBackground&) const = default;
    };
    struct PixelForeground
    {
        enum Style : std::uint8_t
        {
            Bold = 1 << 0,
            Underline = 1 << 1,
            Italic = 1 << 2,
            Strikethrough = 1 << 3,
            Blink = 1 << 4,
            Inverse = 1 << 5,
            Hidden = 1 << 6,
            Reserved = 1 << 7
        };

        using component = std::uint8_t;

        component r{255};
        component g{255};
        component b{255};
        component a{255};
        component style{0x00};
        value_type v{' '};

        PixelForeground alpha(float value) const;
        PixelForeground alpha(component value) const;
        PixelForeground extend(value_type v, component style = 0x00) const;
        PixelForeground invert() const;
        PixelForeground stylize(component style) const;
        PixelForeground mask(const PixelForeground& px, float progress = 0.5f) const;

        operator PixelBase() const;
        operator PixelBackground() const;

        PixelForeground operator|(component style) const;

        PixelForeground& operator=(const PixelForeground&) = default;
        PixelForeground& operator=(const PixelBackground& copy);
        PixelForeground& operator=(const PixelBase& copy);

        auto operator<=>(const PixelForeground&) const = default;
    };
    struct PixelBase
    {
        using Style = PixelForeground::Style;
        using value_type = ::d2::value_type;
        using component = std::uint8_t;

        component r{0};
        component g{0};
        component b{0};
        component rf{255};
        component gf{255};
        component bf{255};
        component a{255};
        component af{255};
        component style{0x00};
        value_type v{' '};

        static PixelBase combine(const PixelForeground& fg, const PixelBackground& bg);
        static PixelBase interpolate(const PixelBase& from, const PixelBase& to, float progress);
        static PixelBase cform(std::uint64_t len);
        static std::size_t is_cform(const PixelBase& px);

        void blend(const PixelBase& src);

        bool compare_colors(const PixelBase& px) const;

        PixelBase alpha(float v, float vf) const;
        PixelBase alpha(component v, component vf) const;
        PixelBase falpha(float value) const;
        PixelBase falpha(component value) const;
        PixelBase balpha(float value) const;
        PixelBase balpha(component value) const;
        PixelBase extend(value_type ch) const;
        PixelBase stylize(component style) const;

        PixelBase mask(const PixelBase& px) const;
        PixelBase mask_foreground(const PixelBase& px) const;
        PixelBase mask_background(const PixelBase& px) const;

        PixelBase invert() const;

        operator PixelBackground() const;
        operator PixelForeground() const;

        PixelBase& operator=(const PixelBase&) = default;
        PixelBase& operator=(const PixelBackground& copy);
        PixelBase& operator=(const PixelForeground& copy);

        auto operator<=>(const PixelBase&) const = default;
    };

    // A single RGBAV value
    // RGBA standard color components and V locale independent value
    using Pixel = PixelBase;

    // Aliases
    namespace px
    {
        using style = Pixel::Style;
        using component = Pixel::component;
        using combined = Pixel;
        using background = PixelBackground;
        using foreground = PixelForeground;
    } // namespace px
} // namespace d2
