#include <d2_std.hpp>
#include <elements/d2_box.hpp>
#include <elements/d2_text.hpp>
#include <elements/d2_input.hpp>
#include <elements/d2_multiline_input.hpp>

struct EditorState : d2::TreeState
{
    using TreeState::TreeState;
    d2::Signals::Handle quit{};
};
using editor = d2::Tree<EditorState, d2::dx::Box,
    [](d2::TreeCtx<d2::dx::Box, EditorState> ctx)
    {
        using namespace d2::dx;
        ctx.state()->quit = ctx.context()->listen<d2::sys::screen::Event::KeyInput>(
            [weak = std::weak_ptr<d2::IOContext>(ctx.context())](d2::in::InputFrame& frame)
            {
                if (frame.map().current[std::size_t(d2::in::special::LeftControl)] &&
                    frame.active(d2::in::key('q'), d2::in::mode::Press))
                    if (auto context = weak.lock())
                        context->stop();
            });
        ctx.elem([](d2::TreeCtx<Text> ctx)
        {
            dstyle(Value, "Click an input to edit. Shift selects; Ctrl+A selects all.\n"
                          "MultiInput: Ctrl+Z / Ctrl+Y undo / redo. Ctrl+Q exits.");
            dstyle(X, d2::Unit(1));
            dstyle(Y, d2::Unit(0));
            dstyle(Width, d2::Unit(65));
            dstyle(TextAlignment, Text::Alignment::Left);
            dstyle(ForegroundColor, d2::px::foreground(255, 255, 255));
        });
        ctx.elem([](d2::TreeCtx<Input> ctx)
        {
            dstyle(X, d2::Unit(1));
            dstyle(Y, d2::Unit(3));
            dstyle(Width, d2::Unit(40));
            dstyle(Pre, "> ");
            dstyle(Hint, "Single-line input");
            dstyle(InputOptions, Input::DrawPtr | Input::AutoPtrColor);
            dstyle(ForegroundColor, d2::px::foreground(255, 255, 255));
            dstyle(BackgroundColor, d2::px::background(30, 30, 30));
        });
        ctx.elem([](d2::TreeCtx<MultiInput> ctx)
        {
            dstyle(X, d2::Unit(1));
            dstyle(Y, d2::Unit(6));
            dstyle(Width, d2::Unit(40));
            dstyle(Height, d2::Unit(6));
            dstyle(Hint, "Multi-line input");
            dstyle(InputOptions, MultiInput::DrawPtr | MultiInput::AutoPtrColor);
            dstyle(EnableScrollHoriz, true);
            dstyle(ForegroundColor, d2::px::foreground(255, 255, 255));
            dstyle(BackgroundColor, d2::px::background(30, 30, 30));
        });
    }>;

int main()
{
    d2::IOContext::make<d2::sys::input, d2::sys::output, d2::sys::screen>()
        ->run<d2::NamedTree<"Editing", editor>>();
}
