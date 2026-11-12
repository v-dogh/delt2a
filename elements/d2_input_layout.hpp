#pragma once

#include <core/platform/d2_text_layout.hpp>

namespace d2::dx::detail
{
    struct InputLayout
    {
        text::Layout source{};
        text::Layout masked{};
        bool censor{false};

        void reset(std::string_view value, bool hide, bool newlines)
        {
            const auto revision = source.revision();
            const auto changed = source.options().newlines != newlines;
            source.reset(value, {.newlines = newlines});
            if (hide && (!censor || revision != source.revision() || changed))
            {
                std::string mask;
                for (const auto& glyph : source.glyphs())
                    mask += glyph.newline ? "\n" : "*";
                masked.reset(mask, {.newlines = newlines});
            }
            censor = hide;
        }
        const text::Layout& display() const { return censor ? masked : source; }
        std::size_t hit(Position position, std::optional<std::size_t> gap = {}) const
        {
            if (gap)
            {
                const auto caret = display().caret(*gap);
                if (position.y == caret.y && position.x > caret.x)
                    --position.x;
            }
            return display().hit_test(position);
        }
    };
} // namespace d2::dx::detail
