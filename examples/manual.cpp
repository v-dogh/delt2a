#include <d2_std.hpp>
#include <elements/d2_box.hpp>
#include <elements/d2_text.hpp>
#include <elements/d2_button.hpp>

using namespace d2;
using App = Tree<TreeState, dx::Box, [](TreeCtx<dx::Box> ctx)
{
    ctx.elem([](TreeCtx<dx::Text> ctx)
    {
        dstyle(Value, "Hello from DeltaAscii");
        dstyle(X, Unit(2));
        dstyle(Y, Unit(1));
        dstyle(Width, Unit(30));
        dstyle(ForegroundColor, px::foreground(230, 230, 230));
    });
    ctx.elem([](TreeCtx<dx::Button> ctx)
    {
        dstyle(Value, "Quit");
        dstyle(X, Unit(2));
        dstyle(Y, Unit(3));
        dstyle(Width, Unit(10));
        dstyle(Height, Unit(1));
        dstyle(ForegroundColor, px::foreground(255, 255, 255));
        dstyle(BackgroundColor, px::background(40, 60, 80));
        dstyle(OnSubmit, [](TreeIter<dx::Button> button) { button->context()->stop(); });
    });
}>;

int main()
{
    IOContext::make<sys::input, sys::output, sys::screen>()
        ->run<NamedTree<"Main", App>>();
}
