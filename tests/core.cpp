#include "check.hpp"
#include "core/platform/d2_extended_page.hpp"
#include "core/platform/d2_platform_string.hpp"
#include "core/platform/d2_fragmented_buffer.hpp"
#include "core/types/d2_pixel.hpp"

void unicode()
{
#if D2_LOCALE_MODE == UNICODE
    d2::ExtendedCodePage page;
    auto first = page.write("12345678");
    auto second = page.write("xy");
    CHECK(page.read(first) == "12345678" && page.read(second) == "xy");
    std::string long_grapheme = "a";
    for (int i = 0; i < 1000; ++i)
        long_grapheme += "\xcc\x81";
    CHECK(page.read(page.write(long_grapheme)) == long_grapheme);
    while (page.size() < page.capacity())
    {
        auto text = std::to_string(1000 + page.size());
        auto token = page.write(text);
        CHECK(token != '\t' && token != 127);
        CHECK(page.read(token) == text);
    }
    CHECK(!page.try_write("overflow"));
    throws<std::overflow_error>([&] { page.write("overflow"); });
    CHECK(page.write("xy") == second);
    page.clear();
    CHECK(page.size() == 0);
    CHECK(page.read(page.write("again")) == "again");
    d2::ExtendedCodePage::activate_thread();
    CHECK(d2::global_extended_code_page.is_activated());
    d2::ExtendedCodePage::deactivate_thread();
    CHECK(!d2::global_extended_code_page.is_activated());

    d2::StringIterator text("a\xcc\x81\xf0\x9f\x91\xa9\xe2\x80\x8d\xf0\x9f\x92\xbb!");
    CHECK(d2::StringIterator::distance(text.begin(), text.end()) == 3);
    CHECK(d2::StringIterator::distance(text.end(), text.begin()) == 3);
    d2::StringIterator space("\xe3\x80\x80");
    CHECK(space.is_space());
#else
    d2::StringIterator text("abc");
    CHECK(d2::StringIterator::distance(text.begin(), text.end()) == 3);
#endif
    d2::StringIterator empty("");
    CHECK(empty.is_end());
    CHECK(empty.current().empty());
    CHECK(!empty.increment());
    CHECK(d2::StringIterator::distance(empty.begin(), empty.end()) == 0);
}

void pixels()
{
    d2::PixelBuffer dst(5, 3), src(3, 2);
    CHECK(dst.data().size() == 15);
    d2::pixel mark;
    mark.v = 'X';
    src.fill(mark);
    d2::PixelBuffer::View target(&dst, 2, 1, 2, 1);
    d2::PixelBuffer::View source(&src, 0, 0, 3, 2);
    target.inscribe(0, 0, source);
    CHECK(dst.at(2, 1).v == 'X' && dst.at(3, 1).v == 'X');
    CHECK(dst.at(0, 0).v != 'X' && dst.at(4, 1).v != 'X' && dst.at(2, 2).v != 'X');
    target.clear();
    CHECK(dst.width() == 5 && dst.height() == 3);
    CHECK(dst.at(2, 1).v != 'X');
    src.at(1, 1).v = 'Y';
    src.compress();
    target.inscribe(0, 0, d2::PixelBuffer::View(&src, 1, 1, 1, 1));
    CHECK(dst.at(2, 1).v == 'Y');
    throws<std::logic_error>([&] { src.at(0, 0); });
    throws<std::out_of_range>([&] { dst.at(5, 0); });
    throws<std::invalid_argument>([] { d2::PixelBuffer bad(-1, 2); });
    auto unpacked = d2::PixelBuffer::rle_unpack(src.data());
    CHECK(unpacked.size() == 6 && unpacked[4].v == 'Y');
    std::array<d2::pixel, 1> bad{d2::px::combined::cform(3)};
    throws<std::invalid_argument>([&] { d2::PixelBuffer::rle_unpack(bad); });
    throws<std::invalid_argument>([&] { d2::PixelBuffer::rle_iterator(bad); });
    d2::PixelBuffer alias(4, 1);
    for (int i = 0; i < 4; ++i)
        alias.at(i, 0).v = 'a' + i;
    alias.inscribe(1, 0, d2::PixelBuffer::View(&alias, 0, 0, 3, 1));
    CHECK(alias.at(1, 0).v == 'a' && alias.at(2, 0).v == 'b' && alias.at(3, 0).v == 'c');
}

void fragmented()
{
    static_assert(!std::is_move_constructible_v<d2::fb::FragmentedBuffer>);
    d2::fb::FragmentedBuffer buffer;
    buffer.buf.setup_buffer("abcd");
    buffer.ptr.jump(3);
    buffer.act.insert(1, "X");
    CHECK(buffer.ptr.absolute_offset() == 3);
    buffer.act.insert("Y");
    std::string value;
    buffer.buf.iterate_raw([&](auto data) { value += data; return true; });
    CHECK(value == "aXbYcd");
}

int main()
{
    unicode();
    pixels();
    fragmented();
}
