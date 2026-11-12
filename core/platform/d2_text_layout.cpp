#include "core/platform/d2_text_layout.hpp"
#include <core/platform/d2_platform_string.hpp>
#include <algorithm>
#include <stdexcept>
#if D2_LOCALE_MODE == UNICODE
#include <uni_algo/conv.h>
#include "generated/d2_unicode_width.hpp"
#endif

namespace d2::text
{
    namespace
    {
#if D2_LOCALE_MODE == UNICODE
        template<std::size_t N>
        bool contains(const std::array<std::pair<char32_t, char32_t>, N>& ranges, char32_t cp)
        {
            const auto it = std::lower_bound(ranges.begin(), ranges.end(), cp,
                [](const auto& range, char32_t value) { return range.second < value; });
            return it != ranges.end() && it->first <= cp;
        }
#endif
        bool line_break(std::string_view str)
        {
            return str == "\n" || str == "\r" || str == "\r\n" ||
                   str == "\xe2\x80\xa8" || str == "\xe2\x80\xa9";
        }
    }

    int columns(std::string_view grapheme)
    {
        if (grapheme.empty())
            return 0;
#if D2_LOCALE_MODE == ASCII
        return grapheme.size() == 1 && grapheme[0] >= 32 && grapheme[0] <= 126 ? 1 : -1;
#else
        if (!una::is_valid_utf8(grapheme))
            return -1;
        int width = 0;
        bool emoji = false, presentation = false, text = false, vs16 = false;
        for (auto cp : grapheme | una::views::utf8)
        {
            const una::codepoint::prop prop(cp);
            if (prop.Control())
                return -1;
            text |= cp == 0xfe0e;
            vs16 |= cp == 0xfe0f;
            emoji |= contains(unicode::emoji, cp);
            presentation |= contains(unicode::emoji_presentation, cp);
            if (!prop.General_Category_M() && !prop.Format())
                width = std::max(width, contains(unicode::wide, cp) ? 2 : 1);
        }
        if (!text && (presentation || (emoji && vs16)))
            width = 2;
        return width;
#endif
    }

    Layout::Layout() { _build(); }
    Layout::Layout(std::string_view source, Options options) { reset(source, options); }

    void Layout::reset(std::string_view source, Options options)
    {
        if (options.width < 0 || options.tab_width <= 0)
            throw std::invalid_argument("Invalid text layout dimensions");
        if (!_lines.empty() && _source == source && _options == options)
            return;
        if (_source != source)
        {
            _source = source;
            ++_revision;
        }
        _options = options;
        _build();
    }

    void Layout::_build()
    {
        _glyphs.clear();
        _lines.clear();
        _bounds = {};
        std::size_t byte = 0;
        for (auto it = StringIterator(_source); !it.is_end(); ++it)
        {
            auto str = it.current();
            std::size_t size = str.size();
#if D2_LOCALE_MODE == ASCII
            if (str == "\r" && byte + 1 < _source.size() && _source[byte + 1] == '\n')
            {
                size = 2;
                ++it;
            }
#endif
            Glyph glyph{.bytes = {byte, byte + size}};
            glyph.newline = _options.newlines && line_break(str);
            glyph.tab = str == "\t";
            glyph.blank = !_options.newlines && line_break(str);
            glyph.space = !glyph.newline && (it.is_space() || line_break(str));
            if (str == "\xc2\xa0" || str == "\xe2\x80\xaf" || str == "\xe2\x80\x87")
                glyph.space = false;
            const auto width = columns(str);
            glyph.columns = glyph.newline ? 0 : (glyph.blank ? 1 : std::max(1, width));
            glyph.replacement = !glyph.newline && !glyph.tab && !glyph.blank && width <= 0;
            _glyphs.push_back(glyph);
            byte += size;
        }

        std::size_t begin = 0;
        do
        {
            auto end = begin;
            auto last_space = begin;
            int width = 0;
            while (end < size() && !_glyphs[end].newline)
            {
                const auto& glyph = _glyphs[end];
                const auto count = glyph.tab ? _options.tab_width - width % _options.tab_width
                                             : glyph.columns;
                if (_options.wrap != Wrap::None && _options.width > 0 && end > begin &&
                    count > _options.width - width)
                {
                    if (_options.wrap == Wrap::Word && last_space > begin)
                        end = last_space;
                    break;
                }
                if (width > INT_MAX - count)
                    throw std::length_error("Text line exceeds coordinate range");
                width += count;
                if (glyph.space)
                    last_space = end + 1;
                ++end;
            }
            Line line{.graphemes = {begin, end}};
            line.hard_break = end < size() && _glyphs[end].newline;
            if (_lines.size() >= INT_MAX)
                throw std::length_error("Text exceeds coordinate range");
            for (auto i = begin; i < end; ++i)
            {
                auto& glyph = _glyphs[i];
                if (glyph.tab)
                    glyph.columns = _options.tab_width - line.columns % _options.tab_width;
                glyph.position = {line.columns, static_cast<int>(_lines.size())};
                line.columns += glyph.columns;
            }
            if (_options.width != INT_MAX && line.columns < _options.width)
            {
                if (_options.alignment == Alignment::Center)
                    line.offset = (_options.width - line.columns) / 2;
                else if (_options.alignment == Alignment::Right)
                    line.offset = _options.width - line.columns;
            }
            for (auto i = begin; i < end; ++i)
                _glyphs[i].position.x += line.offset;
            if (line.hard_break)
                _glyphs[end].position = {line.offset + line.columns, static_cast<int>(_lines.size())};
            _bounds.width = std::max(_bounds.width, line.columns);
            _lines.push_back(line);
            begin = end + line.hard_break;
            if (end == size())
                break;
        } while (begin <= size());
        _bounds.height = static_cast<int>(_lines.size());
    }

    std::size_t Layout::byte_offset(std::size_t grapheme) const
    {
        if (grapheme > size())
            throw std::out_of_range("Text grapheme offset");
        return grapheme == size() ? _source.size() : _glyphs[grapheme].bytes.begin;
    }
    std::size_t Layout::grapheme_at(std::size_t byte, Affinity affinity) const
    {
        if (byte > _source.size())
            throw std::out_of_range("Text byte offset");
        const auto it = std::lower_bound(_glyphs.begin(), _glyphs.end(), byte,
            [](const Glyph& glyph, std::size_t pos) { return glyph.bytes.end <= pos; });
        auto index = static_cast<std::size_t>(it - _glyphs.begin());
        if (it != _glyphs.end() && byte > it->bytes.begin && affinity == Affinity::Downstream)
            ++index;
        return index;
    }
    std::string_view Layout::extract(Range range) const
    {
        if (range.begin > range.end)
            std::swap(range.begin, range.end);
        const auto begin = byte_offset(range.begin);
        return std::string_view(_source).substr(begin, byte_offset(range.end) - begin);
    }
    std::size_t Layout::hit_test(Position position) const
    {
        if (position.y < 0)
            return 0;
        if (position.y >= static_cast<int>(_lines.size()))
            return size();
        const auto& line = _lines[position.y];
        for (auto i = line.graphemes.begin; i < line.graphemes.end; ++i)
        {
            const auto& glyph = _glyphs[i];
            if (position.x <= glyph.position.x)
                return i;
            if (position.x < glyph.position.x + glyph.columns)
                return position.x - glyph.position.x < (glyph.columns + 1) / 2 ? i : i + 1;
        }
        return line.graphemes.end;
    }
    Position Layout::caret(std::size_t grapheme, Affinity affinity) const
    {
        byte_offset(grapheme);
        for (std::size_t i = 0; i < _lines.size(); ++i)
        {
            const auto& line = _lines[i];
            if (grapheme < line.graphemes.end)
                return _glyphs[grapheme].position;
            if (grapheme == line.graphemes.end &&
                (line.hard_break || affinity == Affinity::Upstream || i + 1 == _lines.size()))
                return {line.offset + line.columns, static_cast<int>(i)};
        }
        return {};
    }

    void Layout::draw(PixelBuffer::View buffer, pixel color, Position origin,
                      std::optional<Range> selection, px::background highlight,
                      std::optional<std::size_t> caret_gap) const
    {
        if (buffer.empty())
            return;
        if (selection && selection->begin > selection->end)
            std::swap(selection->begin, selection->end);
        const auto gap = caret_gap ? caret(*caret_gap) : Position{-1, -1};
        for (std::size_t i = 0; i < size(); ++i)
        {
            const auto& glyph = _glyphs[i];
            const auto shift = caret_gap && i >= *caret_gap && glyph.position.y == gap.y;
            const auto x = static_cast<long long>(origin.x) + glyph.position.x + shift;
            const auto y = static_cast<long long>(origin.y) + glyph.position.y;
            if (glyph.newline || y < 0 || y >= buffer.height() || x >= buffer.width() ||
                x + glyph.columns <= 0)
                continue;
            const auto full = x >= 0 && x + glyph.columns <= buffer.width() &&
                              static_cast<long long>(glyph.position.x) + glyph.columns + shift <= _options.width;
            auto token = value_type(' ');
            if (full && !glyph.tab && !glyph.blank)
                token = value_type(global_extended_code_page.write(glyph.replacement ? "?" :
                    std::string_view(_source).substr(glyph.bytes.begin, glyph.bytes.end - glyph.bytes.begin)));
            const auto first = std::max(0LL, -x);
            const auto last = std::min({static_cast<long long>(glyph.columns), buffer.width() - x,
                                       static_cast<long long>(_options.width) - glyph.position.x - shift});
            for (auto col = first; col < last; ++col)
            {
                auto paint = color;
                paint.v = token;
                paint.style &= ~px::style::Reserved;
                if (full && col && !glyph.tab && !glyph.blank)
                    paint.style |= px::style::Reserved;
                auto& cell = buffer.at(static_cast<int>(x + col), static_cast<int>(y));
                cell.blend(paint);
                if (selection && i >= selection->begin && i < selection->end)
                    cell.blend(pixel{.r = highlight.r, .g = highlight.g, .b = highlight.b,
                                     .a = highlight.a, .af = 0});
            }
        }
    }

    bool continuation(const pixel& cell) noexcept
    {
        return cell.has_style(px::style::Reserved) && cell.v != ' ';
    }
    pixel display_cell(std::span<const pixel> cells, std::size_t index, std::size_t width)
    {
        if (!width || index >= cells.size())
            throw std::out_of_range("Text display cell coordinates");
        auto cell = cells[index];
        const auto pair = [](pixel first, pixel second)
        {
            if (continuation(first) || !continuation(second))
                return false;
#if D2_LOCALE_MODE == UNICODE
            if (!global_extended_code_page.is_extended(first.v) ||
                columns(global_extended_code_page.read(first.v)) != 2)
                return false;
#else
            return false;
#endif
            second.style &= ~px::style::Reserved;
            return first == second;
        };
        if (continuation(cell))
        {
            if (index % width && pair(cells[index - 1], cell))
                return cell;
            cell.style &= ~px::style::Reserved;
            cell.v = ' ';
        }
#if D2_LOCALE_MODE == UNICODE
        else if (global_extended_code_page.is_extended(cell.v) &&
                 columns(global_extended_code_page.read(cell.v)) == 2 &&
                 (index % width + 1 >= width || index + 1 >= cells.size() ||
                  !pair(cell, cells[index + 1])))
            cell.v = ' ';
#endif
        return cell;
    }
} // namespace d2::text
