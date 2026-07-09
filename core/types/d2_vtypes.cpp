#include "d2_vtypes.hpp"

namespace d2
{
    // Background

    PixelBackground::scalar PixelBackground::to_scalar(const PixelBackground& px)
    {
        scalar result = 0x00;
        result |= scalar(px.r);
        result |= scalar(px.g) << 8;
        result |= scalar(px.b) << 16;
        result |= scalar(px.a) << 24;
        return result;
    }
    PixelBackground PixelBackground::from_scalar(const scalar& px)
    {
        PixelBackground bg{};
        bg.r = component(px);
        bg.g = component(px >> 8);
        bg.b = component(px >> 16);
        bg.a = component(px >> 24);
        return bg;
    }

    PixelForeground PixelBackground::extend(value_type v, component style) const
    {
        return PixelForeground{
            .r = r,
            .g = g,
            .b = b,
            .a = a,
            .style = style,
            .v = v,
        };
    }
    PixelBackground PixelBackground::alpha(float value) const
    {
        auto cpy = PixelBackground(*this);
        cpy.a = value * 255;
        return cpy;
    }
    PixelBackground PixelBackground::alpha(component value) const
    {
        auto cpy = PixelBackground(*this);
        cpy.a = value;
        return cpy;
    }
    PixelBackground PixelBackground::stylize(component style) const
    {
        return PixelForeground(*this).stylize(style);
    }
    PixelBackground PixelBackground::invert() const
    {
        return PixelBackground{
            static_cast<component>(255 - r),
            static_cast<component>(255 - g),
            static_cast<component>(255 - b),
            a
        };
    }
    PixelBackground PixelBackground::mask(const PixelBackground& px, float progress) const
    {
        return PixelBackground{
            .r = static_cast<component>(std::clamp<int>(r + progress * (px.r - r), 0, 255)),
            .g = static_cast<component>(std::clamp<int>(g + progress * (px.g - g), 0, 255)),
            .b = static_cast<component>(std::clamp<int>(b + progress * (px.b - b), 0, 255)),
            .a = static_cast<component>(std::clamp<int>(a + progress * (px.a - a), 0, 255)),
        };
    }

    PixelForeground PixelBackground::operator|(component flag) const
    {
        return extend(' ', flag);
    }

    PixelBackground::operator PixelForeground() const
    {
        return PixelForeground{
            .r = r,
            .g = g,
            .b = b,
            .a = a,
        };
    }
    PixelBackground::operator PixelBase() const
    {
        return PixelBase{.r = r, .g = g, .b = b, .a = a, .af = 255};
    }

    PixelBackground& PixelBackground::operator=(const PixelForeground& copy)
    {
        r = copy.r;
        g = copy.g;
        b = copy.b;
        a = copy.a;
        return *this;
    }
    PixelBackground& PixelBackground::operator=(const PixelBase& copy)
    {
        r = copy.r;
        g = copy.g;
        b = copy.b;
        a = copy.a;
        return *this;
    }
    PixelForeground& PixelForeground::operator=(const PixelBase& copy)
    {
        r = copy.rf;
        g = copy.gf;
        b = copy.bf;
        a = copy.af;
        v = copy.v;
        style = copy.style;
        return *this;
    }

    // Foreground

    PixelForeground PixelForeground::alpha(float value) const
    {
        auto cpy = PixelForeground(*this);
        cpy.a = value * 255;
        return cpy;
    }
    PixelForeground PixelForeground::alpha(component value) const
    {
        auto cpy = PixelForeground(*this);
        cpy.a = value;
        return cpy;
    }
    PixelForeground PixelForeground::stylize(component style) const
    {
        auto cpy = PixelForeground(*this);
        cpy.style = style;
        return cpy;
    }
    PixelForeground PixelForeground::extend(value_type v, component style) const
    {
        return {
            .r = r,
            .g = g,
            .b = b,
            .a = a,
            .style = style,
            .v = v,
        };
    }
    PixelForeground PixelForeground::invert() const
    {
        return PixelForeground{
            static_cast<component>(255 - r),
            static_cast<component>(255 - g),
            static_cast<component>(255 - b),
            a,
            style,
            v
        };
    }
    PixelForeground PixelForeground::mask(const PixelForeground& px, float progress) const
    {
        return PixelForeground{
            .r = static_cast<component>(std::clamp<int>(r + progress * (px.r - r), 0, 255) / 2),
            .g = static_cast<component>(std::clamp<int>(g + progress * (px.g - g), 0, 255) / 2),
            .b = static_cast<component>(std::clamp<int>(b + progress * (px.b - b), 0, 255) / 2),
            .a = static_cast<component>(std::clamp<int>(a + progress * (px.a - a), 0, 255) / 2),
            .style = static_cast<component>(style | px.style),
            .v = v
        };
    }

    PixelForeground::operator PixelBackground() const
    {
        return PixelBackground{
            .r = r,
            .g = g,
            .b = b,
            .a = a,
        };
    }
    PixelForeground::operator PixelBase() const
    {
        return PixelBase{.rf = r, .gf = g, .bf = b, .a = 0, .af = a, .style = style, .v = v};
    }

    PixelForeground PixelForeground::operator|(component style) const
    {
        PixelForeground copy = *this;
        copy.style |= style;
        return copy;
    }

    PixelForeground& PixelForeground::operator=(const PixelBackground& copy)
    {
        r = copy.r;
        g = copy.g;
        b = copy.b;
        a = copy.a;
        return *this;
    }

    // Base

    PixelBase PixelBase::combine(const PixelForeground& fg, const PixelBackground& bg)
    {
        PixelBase px;
        px.r = bg.r;
        px.g = bg.g;
        px.b = bg.b;
        px.rf = fg.r;
        px.gf = fg.g;
        px.bf = fg.b;
        px.a = bg.a;
        px.af = fg.a;
        px.v = fg.v;
        px.style = fg.style;
        return px;
    }
    PixelBase PixelBase::interpolate(const PixelBase& from, const PixelBase& to, float progress)
    {
        PixelBase value{};
        value.r = from.r + progress * (to.r - from.r);
        value.g = from.g + progress * (to.g - from.g);
        value.b = from.b + progress * (to.b - from.b);
        value.a = from.a + progress * (to.a - from.a);
        value.rf = from.rf + progress * (to.rf - from.rf);
        value.gf = from.gf + progress * (to.gf - from.gf);
        value.bf = from.bf + progress * (to.bf - from.bf);
        value.af = from.af + progress * (to.af - from.af);
        return value;
    }

    PixelBase PixelBase::cform(std::uint64_t len)
    {
        PixelBase px;
        *reinterpret_cast<std::uint64_t*>(&px.r) = len;
        px.style = Style::Reserved;
        return px;
    }
    std::size_t PixelBase::is_cform(const PixelBase& px)
    {
        if (px.style == Style::Reserved)
            return *reinterpret_cast<const std::uint64_t*>(&px.r);
        return 0;
    }

    void PixelBase::blend(const PixelBase& src)
    {
        constexpr auto inv = 1.f / 255.f;

        auto do_blend = [](component source, component dest, float alpha)
        { return static_cast<component>(source * alpha + dest * (1.f - alpha) + 0.5f); };

        const auto special_case = false; // (src.v == ' ' && src.a < a);
        const auto src_af = special_case ? src.a : src.af;
        if (src_af == 0 && src.a == 0)
            return;

        if (src_af == 255) [[likely]]
        {
            rf = src.rf;
            gf = src.gf;
            bf = src.bf;
            af = 255;
        }
        else if (v == ' ')
        {
            const auto alpha_src = src_af * inv;
            rf = do_blend(src.rf, r, alpha_src);
            gf = do_blend(src.gf, g, alpha_src);
            bf = do_blend(src.bf, b, alpha_src);
            af = 255;
        }
        else if (src.af != 0)
        {
            const auto alpha_src = src_af * inv;
            const auto alpha_dest = af * inv;
            rf = do_blend(src.rf, rf, alpha_src);
            gf = do_blend(src.gf, gf, alpha_src);
            bf = do_blend(src.bf, bf, alpha_src);
            af =
                static_cast<component>((alpha_src + alpha_dest * (1.f - alpha_src)) * 255.f + 0.5f);
        }

        if (src.a == 255) [[likely]]
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
            r = do_blend(src.r, r, alpha_src);
            g = do_blend(src.g, g, alpha_src);
            b = do_blend(src.b, b, alpha_src);
            a = static_cast<component>((alpha_src + alpha_dest * (1.f - alpha_src)) * 255.f + 0.5f);
        }

        if ((v == ' ' && src_af > 0) || src_af >= af) [[unlikely]]
        {
            v = src.v;
            style = src.style;
        }
    }

    bool PixelBase::compare_colors(const PixelBase& px) const
    {
        return r == px.r && g == px.g && b == px.b && rf == px.rf && gf == px.gf && bf == px.bf &&
               style == px.style;
    }

    PixelBase PixelBase::alpha(float v, float vf) const
    {
        auto cpy = PixelBase(*this);
        cpy.a = v * 255;
        cpy.af = vf * 255;
        return cpy;
    }
    PixelBase PixelBase::falpha(float value) const
    {
        auto cpy = PixelBase(*this);
        cpy.af = value * 255;
        return cpy;
    }
    PixelBase PixelBase::balpha(float value) const
    {
        auto cpy = PixelBase(*this);
        cpy.a = value * 255;
        return cpy;
    }

    PixelBase PixelBase::alpha(component v, component vf) const
    {
        auto cpy = PixelBase(*this);
        cpy.a = v;
        cpy.af = vf;
        return cpy;
    }
    PixelBase PixelBase::falpha(component value) const
    {
        auto cpy = PixelBase(*this);
        cpy.af = value;
        return cpy;
    }
    PixelBase PixelBase::balpha(component value) const
    {
        auto cpy = PixelBase(*this);
        cpy.a = value;
        return cpy;
    }

    PixelBase PixelBase::stylize(component style) const
    {
        auto cpy = PixelBase(*this);
        cpy.style = style;
        return cpy;
    }
    PixelBase PixelBase::extend(value_type ch) const
    {
        auto cpy = PixelBase(*this);
        cpy.v = ch;
        return cpy;
    }

    PixelBase PixelBase::mask(const PixelBase& px) const
    {
        return PixelBase{
            .r = static_cast<component>((r + px.r) / 2),
            .g = static_cast<component>((g + px.g) / 2),
            .b = static_cast<component>((b + px.b) / 2),
            .rf = static_cast<component>((rf + px.rf) / 2),
            .gf = static_cast<component>((gf + px.gf) / 2),
            .bf = static_cast<component>((bf + px.bf) / 2),
            .a = static_cast<component>((a + px.a) / 2),
            .af = static_cast<component>((af + px.af) / 2),
        };
    }
    PixelBase PixelBase::mask_foreground(const PixelBase& px) const
    {
        return PixelBase{
            .r = r,
            .g = g,
            .b = b,
            .rf = static_cast<component>((rf + px.rf) / 2),
            .gf = static_cast<component>((gf + px.gf) / 2),
            .bf = static_cast<component>((bf + px.bf) / 2),
            .a = a,
            .af = static_cast<component>((af + px.af) / 2),
        };
    }
    PixelBase PixelBase::mask_background(const PixelBase& px) const
    {
        return PixelBase{
            .r = static_cast<component>((r + px.r) / 2),
            .g = static_cast<component>((g + px.g) / 2),
            .b = static_cast<component>((b + px.b) / 2),
            .rf = rf,
            .gf = gf,
            .bf = bf,
            .a = static_cast<component>((a + px.a) / 2),
            .af = af,
        };
    }

    PixelBase PixelBase::invert() const
    {
        return PixelBase{
            static_cast<component>(255 - r),
            static_cast<component>(255 - g),
            static_cast<component>(255 - b),
            a,
            static_cast<component>(255 - rf),
            static_cast<component>(255 - gf),
            static_cast<component>(255 - bf),
            af
        };
    }

    PixelBase::operator PixelBackground() const
    {
        return {
            .r = r,
            .g = g,
            .b = b,
            .a = a,
        };
    }
    PixelBase::operator PixelForeground() const
    {
        return {.r = rf, .g = gf, .b = bf, .a = af, .v = v};
    }

    PixelBase& PixelBase::operator=(const PixelBackground& copy)
    {
        r = copy.r;
        g = copy.g;
        b = copy.b;
        a = copy.a;
        return *this;
    }
    PixelBase& PixelBase::operator=(const PixelForeground& copy)
    {
        rf = copy.r;
        gf = copy.g;
        bf = copy.b;
        af = copy.a;
        v = copy.v;
        return *this;
    }
} // namespace d2
