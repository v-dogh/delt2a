#include "check.hpp"
#include <elements/d2_text.hpp>
#include <elements/d2_box.hpp>
#include <core/mods/d2_ext.hpp>
#include <core/screen/d2_screen.hpp>

using namespace d2;

struct Root : dx::Box
{
    Root() : Box("root", nullptr) { override_dimensions(20, 8); }
    void attach(const Element::ptr& child, Position pos, BoundingBox bounds)
    {
        _elements.push_back(child);
        _setparent_of(child);
        child->override_position(pos.x, pos.y);
        child->override_dimensions(bounds.width, bounds.height);
    }
    void detach(const Element::ptr& child) { std::erase(_elements, child); }
};
struct Label : dx::Text
{
    Label(std::string value) : Text("text", nullptr)
    {
        data::text = std::move(value);
        data::alignment = Alignment::Left;
        data::foreground_color.a = 255;
    }
    void value(std::string value)
    {
        data::text = std::move(value);
        _signal_write(WriteType::Style, Value);
    }
    void disable() { enable_selection = false; }
    void z(char value) { data::zindex = value; }
    void paint(PixelBuffer::View view) { _frame_impl(view); }
    Unit measured(Layout axis) const { return _layout_impl(axis); }
};

int main()
{
    auto root = std::make_shared<Root>();
    auto first = std::make_shared<Label>("hello world");
    auto second = std::make_shared<Label>("next");
    root->attach(first, {0, 0}, {6, 2});
    root->attach(second, {0, 3}, {10, 1});
    TextSelection selection;
    CHECK(selection.begin(root, {1, 0}));
    CHECK(selection.extend({3, 3}));
    selection.finish();
    CHECK(selection.value() == "ello world\nnex");
    CHECK((first->text_selection() == text::Range{1, 11}));
    CHECK((second->text_selection() == text::Range{0, 3}));
    CHECK(selection.begin(root, {3, 3}));
    CHECK(selection.extend({1, 0}));
    CHECK(selection.value() == "ello world\nnex");
    PixelBuffer painted(6, 2);
    first->paint(PixelBuffer::View(&painted, 0, 0, 6, 2));
    CHECK(painted.at(0, 0).v == 'h' && painted.at(0, 1).v == 'w');
    CHECK(painted.at(1, 0).r == 170 && painted.at(0, 0).r != 170);
    first->override_dimensions(12, 1);
    CHECK(selection.refresh() && selection.value() == "ello world\nnex");
    first->value("changed");
    CHECK(!selection.refresh() && selection.empty());
    CHECK(!second->text_selection());

    selection.select_all(root);
    CHECK(selection.value() == "changed\nnext");
    root->detach(second);
    CHECK(!selection.refresh());
    root->attach(second, {0, 3}, {10, 1});
    selection.select_all(root);
    second->setstate(Element::Display, false);
    CHECK(!selection.refresh());
    second->setstate(Element::Display, true);
    second->override_dimensions(10, 1);
    second->override_position(0, 3);

    auto overlay = std::make_shared<Label>("cover");
    overlay->disable();
    overlay->z(10);
    root->attach(overlay, {0, 0}, {5, 1});
    CHECK(!selection.begin(root, {1, 0}));
    root->detach(overlay);
    CHECK(selection.begin(root, {1, 0}));
    CHECK(selection.extend({100, 100}));
    CHECK(selection.value() == "hanged\nnext");
    selection.clear();
    CHECK(!first->text_selection() && !second->text_selection());

    auto context = std::make_shared<IOContext>(mt::ConcurrentPool::make<>(), nullptr);
    auto screen_events = context->connect<sys::screen::EventManifest>();
    std::shared_ptr<sys::SystemSelector> selector = sys::SystemModule::make<sys::SystemSelector>(context);
    selector->load();
    CHECK(selector->status() == sys::SystemModule::Status::Ok);
    selector->selection().select_all(root);
    CHECK(!selector->copy());
    context->signal<sys::screen::Event::TreeSwap>(std::string("one"), std::string("two"));
    CHECK(selector->value().empty());
    std::string selected;
    auto listener = context->listen<sys::selector::Event::Selected>(
        [&](std::string value, Position, Position) { selected = std::move(value); });
    CHECK(selector->selection().begin(root, {1, 0}));
    auto frame = in::InputFrame::make();
    auto view = in::internal::InputFrameView::from(frame.get());
    view.set(in::mouse::Left);
    view.swap();
    view.set(in::mouse::Left, false);
    view.set_mouse_position({3, 3});
    context->signal<sys::screen::Event::MouseInput>(ref(*frame));
    CHECK(selected == "hanged\nnex");
    CHECK(!selector->selection().dragging());
    selector->unload();
    CHECK(!first->text_selection());
    selector->load();
    CHECK(selector->status() == sys::SystemModule::Status::Ok);
    selector.reset();
    context->signal<sys::screen::Event::Update>();

#if D2_LOCALE_MODE == UNICODE
    auto unicode = std::make_shared<Label>("e\xcc\x81界z");
    auto unicode_root = std::make_shared<Root>();
    unicode_root->attach(unicode, {}, {5, 1});
    CHECK(selection.begin(unicode_root, {0, 0}));
    CHECK(selection.extend({2, 0}));
    CHECK(selection.value() == "e\xcc\x81界");
    CHECK(unicode->measured(Element::Layout::Width).raw() == 4);
#endif
}
