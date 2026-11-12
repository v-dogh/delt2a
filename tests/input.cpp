#include "check.hpp"
#include <elements/d2_input.hpp>
#include <elements/d2_multiline_input.hpp>
#include <limits>
#include <random>

using namespace d2;

struct Single : dx::Input
{
    Single() : Input("single", nullptr)
    {
        data::foreground_color.a = 255;
        data::input_options = DrawPtr;
        data::width = Unit(12);
        data::height = Unit(1);
        override_dimensions(12, 1);
    }
    void value(std::string value) { data::text = std::move(value); _reset_ptrs(); }
    const std::string& value() const { return data::text; }
    void prefix(std::string value) { data::pre = std::move(value); }
    void options(unsigned char value) { data::input_options = value; }
    void width(int value) { data::width = Unit(value); override_dimensions(value, 1); }
    void focus() { setstate(Focused, true); }
    void event(in::InputFrame& frame) { _event_impl(frame); }
    void paint(PixelBuffer& pixels) { _frame_impl(PixelBuffer::View(&pixels, 0, 0, pixels.width(), pixels.height())); }
    std::string selected() { return _highlighted_value(); }
    std::size_t hit(int x) { return _hit({x, 0}); }
    int scroll() const { return bound_lower_; }
    void insert(std::string_view value) { _insert(value); }
};
struct Multi : dx::MultiInput
{
    Multi() : MultiInput("multi", nullptr)
    {
        data::foreground_color.a = 255;
        data::input_options = DrawPtr;
        data::width = Unit(12);
        data::height = Unit(4);
        override_dimensions(12, 4);
        data::enable_scrollbar_vertical = false;
    }
    void options(unsigned char value) { data::input_options = value; }
    void size(int width, int height)
    {
        data::width = Unit(width); data::height = Unit(height);
        override_dimensions(width, height);
    }
    void focus() { setstate(Focused, true); }
    void event(in::InputFrame& frame) { _event_impl(frame); }
    void paint(PixelBuffer& pixels) { _frame_impl(PixelBuffer::View(&pixels, 0, 0, pixels.width(), pixels.height())); }
    std::string selected() const { return _highlighted_value(); }
    auto position() const { return _line_col_from_abs(_doc_model(), _ptr_abs); }
    auto scroll() const { return std::pair(_view_x, _view_y); }
    auto hit(int x, int y) const { return _doc_model().text.hit({x, y}); }
};
template<class Widget, class Key>
void key(const std::shared_ptr<Widget>& widget, Key key, bool shift = false, bool ctrl = false)
{
    in::InputFrame frame;
    auto input = in::internal::InputFrameView::from(&frame);
    input.pulse(key);
    if (shift) input.set(in::special::Shift);
    if (ctrl) input.set(in::special::LeftControl);
    input.set_consume(true, std::this_thread::get_id());
    widget->event(frame);
}
template<class Widget>
void type(const std::shared_ptr<Widget>& widget, std::string value, bool shift = false)
{
    in::InputFrame frame;
    auto input = in::internal::InputFrameView::from(&frame);
    input.set_text(std::move(value));
    if (shift) input.set(in::special::Shift);
    widget->event(frame);
}

void basic()
{
    auto single = std::make_shared<Single>();
    single->value("abc");
    single->sync();
    CHECK(single->cursor() == 3);
    key(single, in::special::ArrowLeft, true);
    key(single, in::special::ArrowLeft, true);
    CHECK(single->selected() == "bc" && single->cursor() == 1);
    type(single, "X");
    CHECK(single->value() == "aX" && single->cursor() == 2);
    key(single, in::key('a'), false, true);
    CHECK(single->selected() == "aX");
    key(single, in::special::Backspace);
    CHECK(single->value().empty() && single->cursor() == 0);
    type(single, "already Shifted", true);
    CHECK(single->value() == "already Shifted");
    key(single, in::special::Home);
    CHECK(single->cursor() == 0);
    key(single, in::special::End);
    CHECK(single->cursor() == 15);
    single->jump(std::numeric_limits<std::size_t>::max());
    CHECK(single->cursor() == 15);
    single->options(Single::ReadOnly);
    key(single, in::key('a'), false, true);
    type(single, "replace");
    key(single, in::special::Backspace);
    CHECK(single->value() == "already Shifted");

    auto multi = std::make_shared<Multi>();
    multi->insert("abcd\nx\nabcdef");
    multi->jump(4);
    key(multi, in::special::ArrowDown, true);
    CHECK(multi->cursor() == 6 && multi->selected() == "\nx");
    key(multi, in::special::ArrowDown, true);
    CHECK(multi->cursor() == 11 && multi->selected() == "\nx\nabcd");
    type(multi, "!");
    CHECK(multi->value() == "abcd!ef" && multi->cursor() == 5);
    key(multi, in::key('z'), false, true);
    CHECK(multi->value() == "abcd\nx\nabcdef" && multi->cursor() == 11);
    key(multi, in::key('y'), false, true);
    CHECK(multi->value() == "abcd!ef" && multi->cursor() == 5);
    key(multi, in::key('a'), false, true);
    CHECK(multi->selected() == multi->value());
    key(multi, in::special::Delete);
    CHECK(multi->value().empty());
    multi->undo();
    CHECK(multi->value() == "abcd!ef");
    multi->disable_history();
    multi->insert("ignored");
    const auto disabled = multi->value();
    multi->undo();
    CHECK(multi->value() == disabled);
    multi->enable_history();
    multi->insert("tracked");
    multi->undo();
    CHECK(multi->value() == disabled);
    multi->value("a\r\nb\rc\n");
    multi->jump(1);
    key(multi, in::special::Delete);
    CHECK(multi->value() == "ab\rc\n");
    multi->undo();
    CHECK(multi->value() == "a\r\nb\rc\n");
    multi->value("12345");
    multi->jump(4);
    multi->erase_at(1, 2);
    CHECK(multi->value() == "145" && multi->cursor() == 2);
    multi->undo();
    CHECK(multi->value() == "12345" && multi->cursor() == 4);
    multi->redo();
    CHECK(multi->value() == "145" && multi->cursor() == 2);
    multi->value("before");
    multi->jump(2);
    multi->append(" after");
    CHECK(multi->value() == "before after" && multi->cursor() == 2);
    multi->undo();
    CHECK(multi->value() == "before" && multi->cursor() == 2);
    multi->redo();
    CHECK(multi->value() == "before after" && multi->cursor() == 2);
    multi->value("abcdef");
    multi->jump(1);
    multi->insert("!");
    multi->undo();
    multi->insert("?");
    multi->redo();
    CHECK(multi->value() == "a?bcdef");
}

void rendering()
{
    auto single = std::make_shared<Single>();
    single->value("abc");
    single->prefix("> ");
    single->focus();
    single->jump(1);
    PixelBuffer cells(12, 1);
    single->paint(cells);
    CHECK(cells.at(0, 0).v == '>' && cells.at(2, 0).v == 'a');
    CHECK(cells.at(3, 0).v == '_' && cells.at(4, 0).v == 'b');
    CHECK(single->hit(4) == 1 && single->hit(5) == 2);
    single->width(4);
    single->sync();
    PixelBuffer narrow(4, 1);
    single->paint(narrow);
    CHECK(single->scroll() == 2 && narrow.at(3, 0).v == '_');
    single->prefix("prefix longer than view");
    single->paint(narrow);
    single->width(0);
    PixelBuffer empty(0, 0);
    single->paint(empty);

    auto multi = std::make_shared<Multi>();
    multi->value("a\tb\nend");
    multi->focus();
    multi->jump(2);
    PixelBuffer grid(12, 4);
    multi->paint(grid);
    CHECK(grid.at(0, 0).v == 'a' && grid.at(4, 0).v == '_' && grid.at(5, 0).v == 'b');
    CHECK(grid.at(0, 1).v == 'e');
    multi->size(3, 1);
    multi->sync();
    CHECK((multi->scroll() == std::pair<std::size_t, std::size_t>(1, 1)));
    multi->options(Multi::DrawPtr | Multi::PtrOverlap);
    multi->value("abc");
    multi->sync();
    PixelBuffer overlap(3, 1);
    multi->paint(overlap);
    CHECK(overlap.at(2, 0).v == '_');
}

#if D2_LOCALE_MODE == UNICODE
void unicode()
{
    const std::string accented = "e\xcc\x81";
    const std::string emoji = "👩‍💻";
    const std::string source = accented + "界" + emoji + "🇵🇱!";
    auto single = std::make_shared<Single>();
    single->value(source);
    single->sync();
    CHECK(single->cursor() == 5);
    key(single, in::special::ArrowLeft);
    key(single, in::special::Backspace);
    CHECK(single->value() == accented + "界" + emoji + "!" && single->cursor() == 3);
    key(single, in::special::ArrowLeft, true);
    CHECK(single->selected() == emoji);
    type(single, "x");
    CHECK(single->value() == accented + "界x!");
    single->value("e!"); single->jump(1);
    type(single, "\xcc\x81");
    CHECK(single->value() == accented + "!" && single->cursor() == 1);
    single->value("👩💻"); single->jump(1);
    type(single, "\xe2\x80\x8d");
    CHECK(single->value() == emoji && single->cursor() == 1);
    single->value(source);
    single->prefix("界");
    single->focus();
    single->jump(2);
    PixelBuffer cells(12, 1);
    single->paint(cells);
    CHECK(text::continuation(cells.at(1, 0)));
    CHECK(text::continuation(cells.at(4, 0)) && cells.at(5, 0).v == '_');
    CHECK(single->hit(3) == 1 && single->hit(4) == 2);
    single->options(Single::Censor | Single::DrawPtr);
    single->paint(cells);
    CHECK(cells.at(2, 0).v == '*' && cells.at(3, 0).v == '*' && cells.at(4, 0).v == '_');
    single->prefix("");
    single->options(Single::DrawPtr | Single::PtrOverlap);
    single->jump(1);
    single->paint(cells);
    CHECK(cells.at(1, 0).v == '_' && !text::continuation(cells.at(2, 0)));

    auto multi = std::make_shared<Multi>();
    multi->value(source + "\r\n界x\nlast");
    multi->jump(2);
    key(multi, in::special::ArrowDown);
    CHECK(multi->cursor() == 8);
    key(multi, in::special::ArrowDown);
    CHECK(multi->cursor() == 12);
    multi->value("e!"); multi->jump(1);
    multi->insert("\xcc\x81");
    CHECK(multi->value() == accented + "!" && multi->cursor() == 1);
    multi->undo();
    CHECK(multi->value() == "e!" && multi->cursor() == 1);
    multi->redo();
    CHECK(multi->value() == accented + "!" && multi->cursor() == 1);
    multi->value("👩💻"); multi->jump(1); multi->insert("\xe2\x80\x8d");
    CHECK(multi->value() == emoji && multi->cursor() == 1);
    multi->undo();
    CHECK(multi->value() == "👩💻" && multi->cursor() == 1);
    multi->redo();
    CHECK(multi->cursor() == 1);
    key(multi, in::special::Backspace);
    CHECK(multi->value().empty());
    multi->undo();
    CHECK(multi->value() == emoji && multi->cursor() == 1);
    multi->value(source + "\n界");
    multi->focus(); multi->jump(2);
    PixelBuffer grid(12, 4);
    multi->paint(grid);
    CHECK(grid.at(3, 0).v == '_' && text::continuation(grid.at(5, 0)));
    multi->options(Multi::Censor | Multi::DrawPtr);
    multi->paint(grid);
    CHECK(grid.at(0, 0).v == '*' && grid.at(2, 0).v == '_' && grid.at(0, 1).v == '*');

    // Repeated edits across shard boundaries, then replay the exact byte history.
    multi->value(std::string(255, 'a') + emoji + accented);
    multi->sync();
    std::vector<std::string> snapshots{multi->value()};
    std::mt19937 random(47);
    const std::vector<std::string> inserts{"界", emoji, "\xcc\x81", "\r\n", "x", "\t", "🇵🇱"};
    for (int i = 0; i < 100; ++i)
    {
        const auto count = text::Layout(multi->value()).size();
        multi->jump(random() % (count + 1));
        if (i % 3 && multi->cursor() < count)
            multi->erase();
        else
            multi->insert(inserts[random() % inserts.size()]);
        snapshots.push_back(multi->value());
    }
    for (int i = 99; i >= 0; --i)
    {
        multi->undo();
        CHECK(multi->value() == snapshots[i]);
    }
    for (int i = 1; i <= 100; ++i)
    {
        multi->redo();
        CHECK(multi->value() == snapshots[i]);
    }
}
#endif

int main()
{
    basic();
    rendering();
#if D2_LOCALE_MODE == UNICODE
    unicode();
#endif
}
