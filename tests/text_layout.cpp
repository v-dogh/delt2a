#include "check.hpp"
#include <core/platform/d2_text_layout.hpp>

using namespace d2;

int main()
{
    using namespace text;
    CHECK(sizeof(pixel) == 10);
    Layout empty;
    CHECK(empty.size() == 0 && empty.lines().size() == 1);
    CHECK(empty.hit_test({100, 100}) == 0);
    Layout lines("ab\r\nc\n\n");
    CHECK(lines.size() == 6 && lines.lines().size() == 4);
    CHECK((lines.bounds() == BoundingBox{2, 4}));
    CHECK((lines.caret(3) == Position{0, 1}));
    CHECK(lines.extract({0, lines.size()}) == "ab\r\nc\n\n");
    Layout single("a\nb", {.newlines = false});
    CHECK((single.bounds() == BoundingBox{3, 1}));
    Layout wrap("abc defgh", {.width = 5, .wrap = Wrap::Word});
    CHECK(wrap.lines().size() == 2);
    CHECK(wrap.lines()[0].graphemes.end == 4);
    CHECK(wrap.extract({0, wrap.size()}) == "abc defgh");
    CHECK((wrap.caret(4, Affinity::Upstream) == Position{4, 0}));
    CHECK((wrap.caret(4) == Position{0, 1}));
    Layout long_word("abcdef", {.width = 2, .wrap = Wrap::Word});
    CHECK(long_word.lines().size() == 3);
    Layout zero("abc", {.width = 0, .wrap = Wrap::Word});
    CHECK(zero.size() == 3 && zero.lines().size() == 1);
    Layout tab("a\tb", {.width = 8});
    CHECK(tab.glyphs()[1].columns == 3 && tab.bounds().width == 5);
    CHECK(tab.hit_test({2, 0}) == 1 && tab.hit_test({3, 0}) == 2);
    Layout aligned("ab", {.width = 6, .alignment = Alignment::Right});
    CHECK((aligned.caret(0) == Position{4, 0}));
    CHECK(aligned.hit_test({0, 0}) == 0 && aligned.hit_test({6, 0}) == 2);
    auto revision = aligned.revision();
    aligned.reset("ab", {.width = 8});
    CHECK(aligned.revision() == revision);
    aligned.reset("ac", {.width = 8});
    CHECK(aligned.revision() != revision);
    throws<std::invalid_argument>([] { Layout("x", {.tab_width = 0}); });
    throws<std::out_of_range>([&] { aligned.byte_offset(3); });
    throws<std::out_of_range>([&] { aligned.grapheme_at(3); });

    PixelBuffer target(4, 2);
    Layout("ab\ncd").draw(PixelBuffer::View(&target, 0, 0, 4, 2), pixel{}, {-1, -1});
    CHECK(target.at(0, 0).v == 'd');
    Layout controls(std::string("\x1b\0", 2));
    controls.draw(PixelBuffer::View(&target, 0, 0, 4, 2), pixel{});
    CHECK(target.at(0, 0).v == '?' && target.at(1, 0).v == '?');
#if D2_LOCALE_MODE == UNICODE
    Layout unicode("e\xcc\x81界👩‍💻🇵🇱");
    CHECK(unicode.size() == 4 && unicode.bounds().width == 7);
    CHECK(unicode.byte_offset(1) == 3);
    CHECK(unicode.grapheme_at(2) == 0);
    CHECK(unicode.grapheme_at(2, Affinity::Downstream) == 1);
    CHECK(unicode.extract({0, 1}) == "e\xcc\x81");
    CHECK(unicode.hit_test({1, 0}) == 1 && unicode.hit_test({2, 0}) == 2);
    CHECK(columns("❤") == 1 && columns("❤️") == 2);
    CHECK(columns("👩‍💻") == 2 && columns("🇵🇱") == 2);
    Layout malformed(std::string("\xff", 1));
    CHECK(malformed.glyphs()[0].replacement);
    malformed.draw(PixelBuffer::View(&target, 0, 0, 4, 2), pixel{});
    CHECK(target.at(0, 0).v == '?');

    Layout wide("界x");
    target.fill(pixel{});
    wide.draw(PixelBuffer::View(&target, 0, 0, 4, 2), pixel{}, {}, Range{0, 1}, {.r=30, .g=40, .b=50});
    CHECK(continuation(target.at(1, 0)));
    CHECK(target.at(0, 0).r == 30 && target.at(1, 0).r == 30);
    CHECK(target.at(2, 0).v == 'x');
    CHECK(PixelBuffer::rle_unpack(PixelBuffer::rle_pack(target.data())) ==
          std::vector<pixel>(target.data().begin(), target.data().end()));
    CHECK(display_cell(target.data(), 0, 4).v == target.at(0, 0).v);
    target.at(1, 0).v = 'z';
    CHECK(display_cell(target.data(), 0, 4).v == ' ');
    target.fill(pixel{});
    wide.draw(PixelBuffer::View(&target, 0, 0, 4, 2), pixel{}, {-1, 0});
    CHECK(target.at(0, 0).v == ' ' && target.at(1, 0).v == 'x');
    Layout narrow("界", {.width = 1, .wrap = Wrap::Word});
    narrow.draw(PixelBuffer::View(&target, 0, 0, 4, 2), pixel{});
    CHECK(target.at(0, 0).v == ' ');
#endif
}
