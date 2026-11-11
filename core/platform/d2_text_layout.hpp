#pragma once

#include <core/types/d2_pixel.hpp>
#include <climits>
#include <optional>
#include <string>
#include <vector>

namespace d2::text
{
    enum class Alignment { Left, Center, Right };
    enum class Wrap { None, Grapheme, Word };
    enum class Affinity { Upstream, Downstream };

    struct Range
    {
        std::size_t begin{0};
        std::size_t end{0};
        auto operator<=>(const Range&) const = default;
    };
    struct Options
    {
        int width{INT_MAX};
        int tab_width{4};
        Wrap wrap{Wrap::None};
        Alignment alignment{Alignment::Left};
        bool newlines{true};
        auto operator<=>(const Options&) const = default;
    };
    struct Glyph
    {
        Range bytes{};
        Position position{};
        int columns{1};
        bool newline{false};
        bool tab{false};
        bool blank{false};
        bool space{false};
        bool replacement{false};
    };
    struct Line
    {
        Range graphemes{};
        int columns{0};
        int offset{0};
        bool hard_break{false};
    };

    int columns(std::string_view grapheme);

    class Layout
    {
    private:
        std::string _source{};
        Options _options{};
        std::vector<Glyph> _glyphs{};
        std::vector<Line> _lines{};
        BoundingBox _bounds{};
        std::size_t _revision{0};

        void _build();
    public:
        Layout();
        explicit Layout(std::string_view source, Options options = {});

        void reset(std::string_view source, Options options = {});
        std::string_view source() const noexcept { return _source; }
        const Options& options() const noexcept { return _options; }
        const std::vector<Glyph>& glyphs() const noexcept { return _glyphs; }
        const std::vector<Line>& lines() const noexcept { return _lines; }
        BoundingBox bounds() const noexcept { return _bounds; }
        std::size_t size() const noexcept { return _glyphs.size(); }
        std::size_t revision() const noexcept { return _revision; }

        std::size_t byte_offset(std::size_t grapheme) const;
        std::size_t grapheme_at(std::size_t byte, Affinity affinity = Affinity::Upstream) const;
        std::string_view extract(Range range) const;
        std::size_t hit_test(Position position) const;
        Position caret(std::size_t grapheme, Affinity affinity = Affinity::Downstream) const;

        void draw(PixelBuffer::View buffer, pixel color, Position origin = {},
                  std::optional<Range> selection = {}, px::background highlight = {},
                  std::optional<std::size_t> caret_gap = {}) const;
    };

    bool continuation(const pixel& cell) noexcept;
    pixel display_cell(std::span<const pixel> cells, std::size_t index, std::size_t width);
} // namespace d2::text
