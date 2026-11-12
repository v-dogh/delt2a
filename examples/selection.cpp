#include <d2_std.hpp>
#include <elements/d2_box.hpp>
#include <elements/d2_text.hpp>

struct DocumentState : d2::TreeState
{
    using TreeState::TreeState;
    d2::Signals::Handle quit{};
};

using document = d2::Tree<DocumentState, d2::dx::Box,
    [](d2::TreeCtx<d2::dx::Box, DocumentState> ctx)
    {
        using namespace d2::dx;
        ctx.state()->quit = ctx.context()->listen<d2::sys::screen::Event::KeyInput>(
            [weak = std::weak_ptr<d2::IOContext>(ctx.context())](d2::in::InputFrame& frame)
            {
                const auto keys = frame.map();
                if (keys.current[std::size_t(d2::in::special::LeftControl)] &&
                    frame.active(d2::in::key('q'), d2::in::mode::Press))
                    if (auto context = weak.lock())
                        context->stop();
            });
        ctx.elem([](d2::TreeCtx<Text> ctx)
        {
            dstyle(Value, "Drag across these paragraphs. Ctrl+C copies; Escape clears; Ctrl+Q exits.\n"
                          "Soft wrapping does not insert newlines into the copied text.");
            dstyle(X, d2::Unit(1));
            dstyle(Y, d2::Unit(1));
            dstyle(Width, d2::Unit(32));
            dstyle(TextAlignment, Text::Alignment::Left);
            dstyle(ForegroundColor, d2::px::foreground(255, 255, 255));
        });
        ctx.elem([](d2::TreeCtx<Text> ctx)
        {
#if D2_LOCALE_MODE == UNICODE
            dstyle(Value, "Combining: e\xcc\x81. Wide: 界. Emoji: 👩‍💻 🇵🇱.\n"
                          "Selection endpoints stay on grapheme boundaries.");
#else
            dstyle(Value, "ASCII mode uses the same layout and selection API.");
#endif
            dstyle(X, d2::Unit(1));
            dstyle(Y, d2::Unit(8));
            dstyle(Width, d2::Unit(32));
            dstyle(TextAlignment, Text::Alignment::Left);
            dstyle(ForegroundColor, d2::px::foreground(255, 255, 255));
        });
    }>;

int main()
{
    d2::IOContext::make<d2::sys::input, d2::sys::output, d2::sys::screen,
                       d2::sys::clipboard, d2::sys::selector>()
        ->run<d2::NamedTree<"Selection", document>>();
}
