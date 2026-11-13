# 10. Source-grounded implementation recipes

These recipes are intentionally small and use public APIs seen in `README.md`, `examples/manual.cpp`, `examples/e1-counter/ui.hpp`, `examples/editing.cpp`, and public headers. For full compilable examples, include `<d2_std.hpp>`, `<elements/d2_std.hpp>` and the C++ standard headers used below.

## Recipe A. Counter with reactive label and buttons

This models the existing `examples/e1-counter/ui.hpp`: a retained dependency is the source of truth; the view subscribes to it; button callbacks mutate it.

```cpp
using CounterApp = d2::Tree<
    d2::TreeState,
    d2::dx::FlowBox,
    [](d2::TreeCtx<d2::dx::FlowBox> ctx)
    {
        using namespace d2::dx;
        auto& count = ctx.dep<int>("count");
        count = 0;

        ctx.elem([&](d2::TreeCtx<Text> ctx)
        {
            dstyle(Value, "Count: 0");
            ctx.subscribe(count,
                [=](d2::TreeIter<Text>, int value)
                {
                    ctx->set<Text::Value>(std::format("Count: {}", value));
                }
            ).release();
        });

        ctx.elem([&](d2::TreeCtx<Button> ctx)
        {
            dstyle(Value, "+");
            dstyle(OnSubmit, [&count](d2::TreeIter<Button>) { ++count; });
        });

        ctx.elem([&](d2::TreeCtx<Button> ctx)
        {
            dstyle(Value, "Reset");
            dstyle(OnSubmit, [&count](d2::TreeIter<Button>) { count = 0; });
        });
    }
>;
```

The callback to `subscribe` is invoked synchronously with the initial count. `count` resides in the tree's dependency store, not on the builder's stack. Verify lifetime before using this capture pattern across screen switching.

## Recipe B. Reusable stylesheet with parameters

```cpp
using BoxTitle = d2::Stylesheet<
    []<typename T>(d2::TreeCtx<T> ctx, std::string_view text)
    {
        dstyle(Value, std::string(text));
        dstyle(X, 0.0_center);
        dstyle(ForegroundColor, d2::colors::w::white);
    },
    std::string_view
>;

// Inside TreeCtx<Text> callback:
dapply(BoxTitle, "Configuration");
```

Do not apply this to `Box`, which does not have `Value`; its `Value` expression is compiled against the concrete stylesheet target.

## Recipe C. Typed event shortcut with a state-owned RAII handle

Source pattern from `examples/editing.cpp`:

```cpp
struct EditorState : d2::TreeState
{
    using TreeState::TreeState;
    d2::Signals::Handle quit{};
};

// Inside a TreeCtx<Root,EditorState> builder:
ctx.state()->quit =
    ctx.context()->listen<d2::sys::screen::Event::KeyInput>(
        [weak = std::weak_ptr<d2::IOContext>(ctx.context())]
        (d2::in::InputFrame& frame)
        {
            if (frame.map().current[
                    std::size_t(d2::in::special::LeftControl)] &&
                frame.active(d2::in::key('q'), d2::in::mode::Press))
            {
                if (auto runtime = weak.lock())
                    runtime->stop();
            }
        });
```

This is a better pattern than a discarded signal `Handle`: the `TreeState` member retains it for the intended lifecycle. The source uses a weak context capture to prevent accidental retention cycles.

## Recipe D. Single and multiline editors

```cpp
ctx.elem([](d2::TreeCtx<d2::dx::Input> ctx)
{
    dstyle(X, d2::Unit(1));
    dstyle(Y, d2::Unit(3));
    dstyle(Width, d2::Unit(40));
    dstyle(Pre, "> ");
    dstyle(Hint, "One line");
    dstyle(InputOptions,
        d2::dx::Input::DrawPtr | d2::dx::Input::AutoPtrColor);
});

ctx.elem([](d2::TreeCtx<d2::dx::MultiInput> ctx)
{
    dstyle(X, d2::Unit(1));
    dstyle(Y, d2::Unit(6));
    dstyle(Width, d2::Unit(40));
    dstyle(Height, d2::Unit(8));
    dstyle(Hint, "Multiple lines");
    dstyle(EnableScrollHoriz, true);
});
```

To edit `MultiInput` from application code use `append`, `insert`, `erase_at`, `value(string)`, `undo`, `redo`, not direct mutation of its `FragmentedBuffer`. Treat the return from `ctx.elem` as a weak handle.

## Recipe E. Scroll through 10,000 items without constructing all widgets

```cpp
ctx.elem([](d2::TreeCtx<d2::dx::DemandScrollBox> ctx)
{
    dstyle(Length, std::size_t{10000});
    dstyle(StaticHeightHint, d2::VDUnit(1));
    dstyle(FetchCallback, [](d2::dx::EntrySeed& seed)
    {
        auto element = seed.create<d2::dx::Text>();
        element->set<d2::dx::Text::Value>(
            std::format("Item {}", seed.index()));
    });
});
```

A seed may only create one element. If the list data changes, update the `Length` property and invoke its public reset/update methods as appropriate. Ordinary `ctx.elem` children are not the mechanism for indexed virtual rows.

## Recipe F. HTTP request without leaking `string_view`

```cpp
auto runtime = ctx.context();
if (auto http = runtime->sys_if<d2::sys::net::http>())
{
    d2::sys::net::http::Request req;
    req.url = "https://example.com/api";
    req.timeout = std::chrono::seconds{10};
    req.on_response = [weak = std::weak_ptr<d2::IOContext>(runtime)]
        (d2::sys::net::http::Response& response)
    {
        const int status = response.status();
        std::string body(response.body());
        if (auto app = weak.lock())
        {
            app->sync_async([status, body = std::move(body)]
            {
                // Update the relevant live UI state here.
            });
        }
    };
    req.on_error = [](const d2::sys::net::http::Error& error)
    {
        // Handle transport, timeout, callback or limit failure.
    };
    auto stream = http->request(std::move(req));
    // Keep stream if you need cancellation/control.
}
```

The request callback is asynchronous. Do not capture the builder's local references by reference into a network callback; capture an owned result and context/element handles. If `on_chunk` is used, accumulate chunks during streaming instead of reading the final body.

## Recipe G. Cross-tree navigation

```cpp
using Home = d2::Tree<d2::TreeState, d2::dx::Box,
    [](d2::TreeCtx<d2::dx::Box> ctx)
    {
        ctx.elem([](d2::TreeCtx<d2::dx::Button> ctx)
        {
            dstyle(Value, "Open Settings");
            dstyle(OnSubmit, [](d2::TreeIter<d2::dx::Button> b)
            {
                b->screen()->set("Settings");
            });
        });
    }>;

// Register BOTH Home and Settings under their exact NamedTree names in run<...>().
```

A switch changes active tree lifecycle. Consider `TreeState` or explicit `persist()` when state must survive.

## Recipe H. Conditional element state vs conditional subtree

- For inexpensive show/hide, change `Element::Display` with `setstate(Display, bool)`.
- For structurally different children (e.g. different settings form), use `ctx.branch(dependency, cases...)` so old branch children are removed and rebuilt.
- For preserving a widget across tree swaps, use root-level `ctx.persist` rather than treating it as a normal subelement.

## Recipe I. Construct an image without a file decoder

`examples/images.cpp` constructs image data explicitly:

```cpp
std::vector<d2::image::RGBA> pixels(64 * 64);
// Populate RGBA components of each pixel.
auto raster = d2::image::Raster::rgba(64, 64, std::move(pixels));

// Inside TreeCtx<dx::Image> callback, assuming raster is in scope:
dstyle(Source, raster);
```

This bypasses PNG decoding but **not** the need for an image-capable terminal output backend to display an actual inline image.
