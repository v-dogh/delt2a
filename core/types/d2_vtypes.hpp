#pragma once

#include <core/platform/d2_locale.hpp>

namespace d2
{
    struct Position
    {
        int x{0};
        int y{0};

        [[nodiscard]] constexpr Position operator+() const noexcept
        {
            return *this;
        }
        [[nodiscard]] constexpr Position operator-() const noexcept
        {
            return {-x, -y};
        }

        constexpr Position& operator+=(Position rhs) noexcept
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        constexpr auto& operator-=(Position rhs) noexcept
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }
        constexpr auto& operator*=(int scalar) noexcept
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }
        constexpr auto& operator/=(int scalar) noexcept
        {
            x /= scalar;
            y /= scalar;
            return *this;
        }

        [[nodiscard]] friend constexpr Position operator+(Position lhs, Position rhs) noexcept
        {
            lhs += rhs;
            return lhs;
        }
        [[nodiscard]] friend constexpr Position operator-(Position lhs, Position rhs) noexcept
        {
            lhs -= rhs;
            return lhs;
        }
        [[nodiscard]] friend constexpr Position operator*(Position lhs, int scalar) noexcept
        {
            lhs *= scalar;
            return lhs;
        }
        [[nodiscard]] friend constexpr Position operator*(int scalar, Position rhs) noexcept
        {
            rhs *= scalar;
            return rhs;
        }
        [[nodiscard]] friend constexpr Position operator/(Position lhs, int scalar) noexcept
        {
            lhs /= scalar;
            return lhs;
        }

        auto operator<=>(const Position&) const = default;
    };

    struct BoundingBox
    {
        int width{0};
        int height{0};

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return width <= 0 || height <= 0;
        }
        [[nodiscard]] constexpr int area() const noexcept
        {
            return width * height;
        }
        [[nodiscard]] constexpr bool contains(Position p) const noexcept
        {
            return p.x >= 0 && p.y >= 0 && p.x < width && p.y < height;
        }

        constexpr auto& operator+=(BoundingBox rhs) noexcept
        {
            width += rhs.width;
            height += rhs.height;
            return *this;
        }
        constexpr auto& operator-=(BoundingBox rhs) noexcept
        {
            width -= rhs.width;
            height -= rhs.height;
            return *this;
        }
        constexpr auto& operator*=(int scalar) noexcept
        {
            width *= scalar;
            height *= scalar;
            return *this;
        }
        constexpr auto& operator/=(int scalar) noexcept
        {
            width /= scalar;
            height /= scalar;
            return *this;
        }

        [[nodiscard]] friend constexpr BoundingBox
        operator+(BoundingBox lhs, BoundingBox rhs) noexcept
        {
            lhs += rhs;
            return lhs;
        }
        [[nodiscard]] friend constexpr BoundingBox
        operator-(BoundingBox lhs, BoundingBox rhs) noexcept
        {
            lhs -= rhs;
            return lhs;
        }
        [[nodiscard]] friend constexpr BoundingBox operator*(BoundingBox lhs, int scalar) noexcept
        {
            lhs *= scalar;
            return lhs;
        }
        [[nodiscard]] friend constexpr BoundingBox operator*(int scalar, BoundingBox rhs) noexcept
        {
            rhs *= scalar;
            return rhs;
        }
        [[nodiscard]] friend constexpr BoundingBox operator/(BoundingBox lhs, int scalar) noexcept
        {
            lhs /= scalar;
            return lhs;
        }

        auto operator<=>(const BoundingBox&) const = default;
    };

    namespace px
    {
        using component = std::uint8_t;
        using scalar = std::uint32_t;
        using value_type = d2::value_type;

        enum class style : component
        {
            None = 0,
            Bold = 1 << 0,
            Underline = 1 << 1,
            Italic = 1 << 2,
            Strikethrough = 1 << 3,
            Blink = 1 << 4,
            Inverse = 1 << 5,
            Hidden = 1 << 6,
            Reserved = 1 << 7
        };

        [[nodiscard]] constexpr component to_underlying(style value) noexcept
        {
            return static_cast<component>(value);
        }

        [[nodiscard]] constexpr style as_style(component value) noexcept
        {
            return static_cast<style>(value);
        }

        [[nodiscard]] constexpr style operator|(style lhs, style rhs) noexcept
        {
            return as_style(to_underlying(lhs) | to_underlying(rhs));
        }

        [[nodiscard]] constexpr style operator&(style lhs, style rhs) noexcept
        {
            return as_style(to_underlying(lhs) & to_underlying(rhs));
        }

        [[nodiscard]] constexpr style operator^(style lhs, style rhs) noexcept
        {
            return as_style(to_underlying(lhs) ^ to_underlying(rhs));
        }

        [[nodiscard]] constexpr style operator~(style value) noexcept
        {
            return as_style(~to_underlying(value));
        }

        constexpr style& operator|=(style& lhs, style rhs) noexcept
        {
            lhs = lhs | rhs;
            return lhs;
        }

        constexpr style& operator&=(style& lhs, style rhs) noexcept
        {
            lhs = lhs & rhs;
            return lhs;
        }

        constexpr style& operator^=(style& lhs, style rhs) noexcept
        {
            lhs = lhs ^ rhs;
            return lhs;
        }

        [[nodiscard]] constexpr bool any(style flags) noexcept
        {
            return flags != style::None;
        }

        [[nodiscard]] constexpr bool has(style flags, style flag) noexcept
        {
            return any(flags & flag);
        }

        namespace impl
        {
            [[nodiscard]] constexpr float clamp01(float value) noexcept
            {
                return value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;
            }

            [[nodiscard]] constexpr component clamp_byte(int value) noexcept
            {
                return static_cast<component>(value < 0 ? 0 : value > 255 ? 255 : value);
            }

            [[nodiscard]] constexpr component byte_from_unit(float value) noexcept
            {
                return clamp_byte(static_cast<int>(clamp01(value) * 255.0f + 0.5f));
            }

            [[nodiscard]] constexpr component
            lerp(component from, component to, float progress) noexcept
            {
                const float t = clamp01(progress);
                const auto distance = static_cast<int>(to) - static_cast<int>(from);
                return clamp_byte(
                    static_cast<int>(
                        static_cast<float>(from) + static_cast<float>(distance) * t + 0.5f
                    )
                );
            }

            [[nodiscard]] constexpr component
            blend(component source, component dest, float alpha) noexcept
            {
                return clamp_byte(static_cast<int>(source * alpha + dest * (1.0f - alpha) + 0.5f));
            }
        } // namespace impl

        struct foreground;
        struct combined;

        struct background
        {
            component r{0};
            component g{0};
            component b{0};
            component a{255};

            [[nodiscard]] static constexpr background from_scalar(scalar value) noexcept
            {
                return {
                    static_cast<component>(value),
                    static_cast<component>(value >> 8),
                    static_cast<component>(value >> 16),
                    static_cast<component>(value >> 24)
                };
            }

            [[nodiscard]] constexpr scalar to_scalar() const noexcept
            {
                scalar result = 0;
                result |= static_cast<scalar>(r);
                result |= static_cast<scalar>(g) << 8;
                result |= static_cast<scalar>(b) << 16;
                result |= static_cast<scalar>(a) << 24;
                return result;
            }

            [[nodiscard]] static constexpr scalar to_scalar(background value) noexcept
            {
                return value.to_scalar();
            }

            [[nodiscard]] constexpr background with_alpha(component value) const noexcept
            {
                auto copy = *this;
                copy.a = value;
                return copy;
            }

            [[nodiscard]] constexpr background with_alpha(float value) const noexcept
            {
                return with_alpha(impl::byte_from_unit(value));
            }

            [[nodiscard]] constexpr background alpha(component value) const noexcept
            {
                return with_alpha(value);
            }
            [[nodiscard]] constexpr background alpha(float value) const noexcept
            {
                return with_alpha(value);
            }

            [[nodiscard]] constexpr background inverted() const noexcept
            {
                return {
                    static_cast<component>(255 - r),
                    static_cast<component>(255 - g),
                    static_cast<component>(255 - b),
                    a
                };
            }

            [[nodiscard]] constexpr background invert() const noexcept
            {
                return inverted();
            }

            [[nodiscard]] constexpr background
            mix(background to, float progress = 0.5f) const noexcept
            {
                return {
                    impl::lerp(r, to.r, progress),
                    impl::lerp(g, to.g, progress),
                    impl::lerp(b, to.b, progress),
                    impl::lerp(a, to.a, progress)
                };
            }

            [[nodiscard]] constexpr background
            mask(background to, float progress = 0.5f) const noexcept
            {
                return mix(to, progress);
            }

            [[nodiscard]] constexpr foreground
            with_value(value_type value = ' ', style flags = style::None) const noexcept;
            [[nodiscard]] constexpr foreground
            extend(value_type value = ' ', style flags = style::None) const noexcept;
            [[nodiscard]] constexpr foreground with_style(style flags) const noexcept;
            [[nodiscard]] constexpr foreground stylize(style flags) const noexcept;

            [[nodiscard]] constexpr operator foreground() const noexcept;
            [[nodiscard]] constexpr operator combined() const noexcept;

            auto operator<=>(const background&) const = default;
        };
        struct foreground
        {
            component r{255};
            component g{255};
            component b{255};
            component a{255};
            value_type v{' '};
            style style{style::None};

            [[nodiscard]] constexpr background as_background() const noexcept
            {
                return {r, g, b, a};
            }

            [[nodiscard]] constexpr foreground with_alpha(component value) const noexcept
            {
                auto copy = *this;
                copy.a = value;
                return copy;
            }

            [[nodiscard]] constexpr foreground with_alpha(float value) const noexcept
            {
                return with_alpha(impl::byte_from_unit(value));
            }

            [[nodiscard]] constexpr foreground alpha(component value) const noexcept
            {
                return with_alpha(value);
            }
            [[nodiscard]] constexpr foreground alpha(float value) const noexcept
            {
                return with_alpha(value);
            }

            [[nodiscard]] constexpr foreground with_value(value_type value) const noexcept
            {
                auto copy = *this;
                copy.v = value;
                return copy;
            }

            [[nodiscard]] constexpr foreground
            extend(value_type value, px::style flags = style::None) const noexcept
            {
                auto copy = *this;
                copy.v = value;
                copy.style = flags;
                return copy;
            }
            [[nodiscard]] constexpr foreground with_style(px::style value) const noexcept
            {
                auto copy = *this;
                copy.style = value;
                return copy;
            }

            [[nodiscard]] constexpr foreground stylize(px::style value) const noexcept
            {
                return with_style(value);
            }

            [[nodiscard]] constexpr foreground add_style(px::style value) const noexcept
            {
                auto copy = *this;
                copy.style |= value;
                return copy;
            }

            [[nodiscard]] constexpr foreground remove_style(px::style value) const noexcept
            {
                auto copy = *this;
                copy.style &= ~value;
                return copy;
            }

            [[nodiscard]] constexpr bool has_style(px::style value) const noexcept
            {
                return px::has(style, value);
            }

            [[nodiscard]] constexpr foreground inverted() const noexcept
            {
                return {
                    static_cast<component>(255 - r),
                    static_cast<component>(255 - g),
                    static_cast<component>(255 - b),
                    a,
                    v,
                    style
                };
            }

            [[nodiscard]] constexpr foreground invert() const noexcept
            {
                return inverted();
            }

            [[nodiscard]] constexpr foreground
            mix(foreground to, float progress = 0.5f) const noexcept
            {
                return {
                    impl::lerp(r, to.r, progress),
                    impl::lerp(g, to.g, progress),
                    impl::lerp(b, to.b, progress),
                    impl::lerp(a, to.a, progress),
                    v,
                    style | to.style
                };
            }

            [[nodiscard]] constexpr foreground
            mask(foreground to, float progress = 0.5f) const noexcept
            {
                return mix(to, progress);
            }

            [[nodiscard]] constexpr operator background() const noexcept
            {
                return as_background();
            }

            [[nodiscard]] constexpr operator combined() const noexcept;

            auto operator<=>(const foreground&) const = default;
        };
        struct combined
        {
            component r{0};
            component g{0};
            component b{0};
            component a{255};

            component rf{255};
            component gf{255};
            component bf{255};
            component af{255};

            value_type v{' '};
            px::style style{px::style::None};

            [[nodiscard]] static constexpr combined combine(background bg, foreground fg) noexcept
            {
                return {
                    .r = bg.r,
                    .g = bg.g,
                    .b = bg.b,
                    .a = bg.a,
                    .rf = fg.r,
                    .gf = fg.g,
                    .bf = fg.b,
                    .af = fg.a,
                    .v = fg.v,
                    .style = fg.style
                };
            }
            [[nodiscard]] static constexpr combined combine(foreground fg, background bg) noexcept
            {
                return {
                    .r = bg.r,
                    .g = bg.g,
                    .b = bg.b,
                    .a = bg.a,
                    .rf = fg.r,
                    .gf = fg.g,
                    .bf = fg.b,
                    .af = fg.a,
                    .v = fg.v,
                    .style = fg.style
                };
            }

            [[nodiscard]] constexpr background as_background() const noexcept
            {
                return {r, g, b, a};
            }
            [[nodiscard]] constexpr foreground as_foreground() const noexcept
            {
                return {rf, gf, bf, af, v, style};
            }

            [[nodiscard]] constexpr combined with_background(background value) const noexcept
            {
                auto copy = *this;
                copy.r = value.r;
                copy.g = value.g;
                copy.b = value.b;
                copy.a = value.a;
                return copy;
            }

            [[nodiscard]] constexpr combined with_foreground(foreground value) const noexcept
            {
                auto copy = *this;
                copy.rf = value.r;
                copy.gf = value.g;
                copy.bf = value.b;
                copy.af = value.a;
                copy.v = value.v;
                copy.style = value.style;
                return copy;
            }

            [[nodiscard]] constexpr combined with_value(value_type value) const noexcept
            {
                auto copy = *this;
                copy.v = value;
                return copy;
            }

            [[nodiscard]] constexpr combined extend(value_type value) const noexcept
            {
                return with_value(value);
            }

            [[nodiscard]] constexpr combined with_style(px::style value) const noexcept
            {
                auto copy = *this;
                copy.style = value;
                return copy;
            }

            [[nodiscard]] constexpr combined stylize(px::style value) const noexcept
            {
                return with_style(value);
            }

            [[nodiscard]] constexpr combined add_style(px::style value) const noexcept
            {
                auto copy = *this;
                copy.style |= value;
                return copy;
            }

            [[nodiscard]] constexpr combined remove_style(px::style value) const noexcept
            {
                auto copy = *this;
                copy.style &= ~value;
                return copy;
            }

            [[nodiscard]] constexpr bool has_style(px::style value) const noexcept
            {
                return px::has(style, value);
            }

            [[nodiscard]] constexpr combined
            with_alpha(component background_alpha, component foreground_alpha) const noexcept
            {
                auto copy = *this;
                copy.a = background_alpha;
                copy.af = foreground_alpha;
                return copy;
            }

            [[nodiscard]] constexpr combined
            with_alpha(float background_alpha, float foreground_alpha) const noexcept
            {
                return with_alpha(
                    impl::byte_from_unit(background_alpha), impl::byte_from_unit(foreground_alpha)
                );
            }

            [[nodiscard]] constexpr combined
            alpha(component background_alpha, component foreground_alpha) const noexcept
            {
                return with_alpha(background_alpha, foreground_alpha);
            }

            [[nodiscard]] constexpr combined
            alpha(float background_alpha, float foreground_alpha) const noexcept
            {
                return with_alpha(background_alpha, foreground_alpha);
            }

            [[nodiscard]] constexpr combined with_background_alpha(component value) const noexcept
            {
                auto copy = *this;
                copy.a = value;
                return copy;
            }

            [[nodiscard]] constexpr combined with_background_alpha(float value) const noexcept
            {
                return with_background_alpha(impl::byte_from_unit(value));
            }

            [[nodiscard]] constexpr combined balpha(component value) const noexcept
            {
                return with_background_alpha(value);
            }
            [[nodiscard]] constexpr combined balpha(float value) const noexcept
            {
                return with_background_alpha(value);
            }

            [[nodiscard]] constexpr combined with_foreground_alpha(component value) const noexcept
            {
                auto copy = *this;
                copy.af = value;
                return copy;
            }

            [[nodiscard]] constexpr combined with_foreground_alpha(float value) const noexcept
            {
                return with_foreground_alpha(impl::byte_from_unit(value));
            }

            [[nodiscard]] constexpr combined falpha(component value) const noexcept
            {
                return with_foreground_alpha(value);
            }
            [[nodiscard]] constexpr combined falpha(float value) const noexcept
            {
                return with_foreground_alpha(value);
            }

            [[nodiscard]] constexpr combined inverted() const noexcept
            {
                return combine(as_background().inverted(), as_foreground().inverted());
            }

            [[nodiscard]] constexpr combined invert() const noexcept
            {
                return inverted();
            }

            [[nodiscard]] constexpr combined mix(combined to, float progress = 0.5f) const noexcept
            {
                return combine(
                    as_background().mix(to.as_background(), progress),
                    as_foreground().mix(to.as_foreground(), progress)
                );
            }

            [[nodiscard]] constexpr combined mask(combined to, float progress = 0.5f) const noexcept
            {
                return mix(to, progress);
            }

            [[nodiscard]] constexpr combined
            mix_background(combined to, float progress = 0.5f) const noexcept
            {
                return with_background(as_background().mix(to.as_background(), progress));
            }

            [[nodiscard]] constexpr combined
            mask_background(combined to, float progress = 0.5f) const noexcept
            {
                return mix_background(to, progress);
            }

            [[nodiscard]] constexpr combined
            mix_foreground(combined to, float progress = 0.5f) const noexcept
            {
                return with_foreground(as_foreground().mix(to.as_foreground(), progress));
            }

            [[nodiscard]] constexpr combined
            mask_foreground(combined to, float progress = 0.5f) const noexcept
            {
                return mix_foreground(to, progress);
            }

            [[nodiscard]] constexpr bool same_colors(combined other) const noexcept
            {
                return r == other.r && g == other.g && b == other.b && rf == other.rf &&
                       gf == other.gf && bf == other.bf && style == other.style;
            }

            [[nodiscard]] constexpr bool compare_colors(combined other) const noexcept
            {
                return same_colors(other);
            }

            constexpr void blend(combined src) noexcept
            {
                constexpr auto inv = 1.0f / 255.0f;

                const auto src_fa = src.af;

                if (src_fa == 0 && src.a == 0)
                    return;

                if (src_fa == 255)
                {
                    rf = src.rf;
                    gf = src.gf;
                    bf = src.bf;
                    af = 255;
                }
                else if (v == ' ')
                {
                    const auto alpha_src = src_fa * inv;
                    rf = impl::blend(src.rf, r, alpha_src);
                    gf = impl::blend(src.gf, g, alpha_src);
                    bf = impl::blend(src.bf, b, alpha_src);
                    af = 255;
                }
                else if (src.af != 0)
                {
                    const auto alpha_src = src_fa * inv;
                    const auto alpha_dest = af * inv;

                    rf = impl::blend(src.rf, rf, alpha_src);
                    gf = impl::blend(src.gf, gf, alpha_src);
                    bf = impl::blend(src.bf, bf, alpha_src);
                    af = static_cast<component>(
                        (alpha_src + alpha_dest * (1.0f - alpha_src)) * 255.0f + 0.5f
                    );
                }

                if (src.a == 255)
                {
                    r = src.r;
                    g = src.g;
                    b = src.b;
                    a = 255;
                }
                else if (src.a != 0)
                {
                    const auto alpha_src = src.a * inv;
                    const auto alpha_dest = a * inv;

                    r = impl::blend(src.r, r, alpha_src);
                    g = impl::blend(src.g, g, alpha_src);
                    b = impl::blend(src.b, b, alpha_src);
                    a = static_cast<component>(
                        (alpha_src + alpha_dest * (1.0f - alpha_src)) * 255.0f + 0.5f
                    );
                }

                if ((v == ' ' && src_fa > 0) || src_fa >= af)
                {
                    v = src.v;
                    style = src.style;
                }
            }

            [[nodiscard]] constexpr combined blended(combined src) const noexcept
            {
                auto copy = *this;
                copy.blend(src);
                return copy;
            }

            [[nodiscard]] static constexpr combined
            interpolate(combined from, combined to, float progress) noexcept
            {
                return from.mix(to, progress);
            }

            [[nodiscard]] static constexpr combined cform(std::uint64_t len) noexcept
            {
                combined px{};
                px.r = static_cast<component>(len);
                px.g = static_cast<component>(len >> 8);
                px.b = static_cast<component>(len >> 16);
                px.rf = static_cast<component>(len >> 24);
                px.gf = static_cast<component>(len >> 32);
                px.bf = static_cast<component>(len >> 40);
                px.a = static_cast<component>(len >> 48);
                px.af = static_cast<component>(len >> 56);
                px.style = px::style::Reserved;
                px.v = ' ';
                return px;
            }
            [[nodiscard]] constexpr bool is_cform_marker() const noexcept
            {
                return style == px::style::Reserved;
            }
            [[nodiscard]] constexpr std::uint64_t cform_length() const noexcept
            {
                if (!is_cform_marker())
                    return 0;

                std::uint64_t len = 0;
                len |= static_cast<std::uint64_t>(r);
                len |= static_cast<std::uint64_t>(g) << 8;
                len |= static_cast<std::uint64_t>(b) << 16;
                len |= static_cast<std::uint64_t>(rf) << 24;
                len |= static_cast<std::uint64_t>(gf) << 32;
                len |= static_cast<std::uint64_t>(bf) << 40;
                len |= static_cast<std::uint64_t>(a) << 48;
                len |= static_cast<std::uint64_t>(af) << 56;
                return len;
            }
            [[nodiscard]] constexpr std::size_t is_cform() const noexcept
            {
                return static_cast<std::size_t>(cform_length());
            }

            [[nodiscard]] constexpr operator background() const noexcept
            {
                return as_background();
            }
            [[nodiscard]] constexpr operator foreground() const noexcept
            {
                return as_foreground();
            }

            combined& operator=(background value) noexcept
            {
                r = value.r;
                g = value.g;
                b = value.b;
                a = value.a;
                return *this;
            }

            combined& operator=(foreground value) noexcept
            {
                rf = value.r;
                gf = value.g;
                bf = value.b;
                af = value.a;
                v = value.v;
                style = value.style;
                return *this;
            }

            auto operator<=>(const combined&) const = default;
        };

        [[nodiscard]] constexpr foreground
        background::with_value(value_type value, style flags) const noexcept
        {
            return {r, g, b, a, value, flags};
        }

        [[nodiscard]] constexpr foreground
        background::extend(value_type value, style flags) const noexcept
        {
            return with_value(value, flags);
        }

        [[nodiscard]] constexpr foreground background::with_style(style flags) const noexcept
        {
            return with_value(' ', flags);
        }

        [[nodiscard]] constexpr foreground background::stylize(style flags) const noexcept
        {
            return with_style(flags);
        }

        [[nodiscard]] constexpr background::operator foreground() const noexcept
        {
            return with_value();
        }

        [[nodiscard]] constexpr background::operator combined() const noexcept
        {
            return combined::combine(*this, {});
        }

        [[nodiscard]] constexpr foreground::operator combined() const noexcept
        {
            return combined::combine(*this, {});
        }

        [[nodiscard]] constexpr foreground operator|(foreground fg, style flags) noexcept
        {
            return fg.add_style(flags);
        }

        [[nodiscard]] constexpr foreground operator|(foreground fg, component flags) noexcept
        {
            return fg.add_style(as_style(flags));
        }

        [[nodiscard]] constexpr foreground operator|(background bg, style flags) noexcept
        {
            return bg.with_style(flags);
        }

        [[nodiscard]] constexpr foreground operator|(background bg, component flags) noexcept
        {
            return bg.with_style(as_style(flags));
        }

        [[nodiscard]] constexpr combined operator|(combined px, style flags) noexcept
        {
            return px.add_style(flags);
        }

        [[nodiscard]] constexpr combined operator|(combined px, component flags) noexcept
        {
            return px.add_style(as_style(flags));
        }
    } // namespace px
    using pixel = px::combined;
} // namespace d2
