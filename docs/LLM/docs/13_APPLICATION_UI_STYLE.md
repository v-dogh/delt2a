# 13. Application UI composition style

This is the **app-authoring** companion to [11_STYLE_GUIDE.md](11_STYLE_GUIDE.md), which mostly describes conventions for contributing to the *library itself*. The examples consulted include `examples/e0-hello/ui.hpp`, `examples/e1-counter/ui.hpp`, `examples/manual.cpp`, and `examples/editing.cpp`.

**Status legend:** **Observed** means the retained examples/API use it. **Recommended** is an opinionated default for consistent agent-generated apps, not a compiler or repository mandate. The library does **not** impose a universal visual design language, spacing scale, or color palette.

## Default shape of an application

1. **Observed:** Define a compile-time `d2::Tree<State, RootWidget, BuildLambda>`; give the callback a typed `d2::TreeCtx<...> ctx`. Register the tree with `d2::NamedTree<"Name", Tree>` in `IOContext::run`.
2. **Observed:** Declare the visual hierarchy with nested `ctx.elem([](d2::TreeCtx<Widget> ctx) { ... });`, with exactly the widget type you want in each callback argument. Use named children (`ctx.elem("name", ...)`) when stable lookup or replacement is useful; don't name everything reflexively.
3. **Observed:** Configure element appearance/geometry/interaction in its own builder using one `dstyle(Property, value)` statement per property. The macro depends on the identifier `ctx`.
4. **Observed:** Express terminal layout with `d2::Unit`/literal suffixes (`0.0_center`, `1.0_relative`, `1.0_auto`, `20.0_px`), rather than manually computing absolute terminal coordinates for every element.
5. **Observed:** For derived display state, use a named `ctx.dep<T>("name")` and `ctx.subscribe(...)` or a compatible direct property binding. Subscriptions fire immediately with the current value.
6. **Observed:** Configure typed callbacks such as `dstyle(OnSubmit, [](d2::TreeIter<Button> button) { ... });`; don't invent untyped event names or `.onClick` style APIs.
7. **Recommended:** Keep a top-level builder focused on screen structure. Extract repeatable *style and local builder behavior* to `d2::Stylesheet` + `dapply`. For large, independently managed subtrees, consider `ctx.embed` or separate named trees, not a single thousand-line builder.
8. **Recommended:** Use the tree's `State` type for app-level persistent/RAII-owned resources and dependencies for values that should cause UI updates. Avoid retaining UI elements via global `shared_ptr` by default.
9. **Recommended:** Gather property calls in a predictable order: content, geometry, colors/visuals, then callback wiring. Existing examples vary, so preserve local order when modifying a file.
10. **Recommended:** Make event handlers small and delegate substantial work to named helpers/services. Work launched from background callbacks must synchronize UI mutations and check that target elements still exist.

## Idiomatic small-screen example

The following illustrates both the source-native layout and a *recommended* separation between reusable presentation and stateful interaction. It is a reference pattern, not an exact verbatim upstream example.

```cpp
#include <d2_std.hpp>
#include <elements/d2_std.hpp>

using StatusText = d2::Stylesheet<
    []<typename Widget>(d2::TreeCtx<Widget> ctx)
    {
        dstyle(X, 0.0_center);
        dstyle(Y, 1.0_relative);
        dstyle(ForegroundColor, d2::colors::w::white);
    }
>;

using Dashboard = d2::Tree<
    d2::TreeState,
    d2::dx::FlowBox,
    [](d2::TreeCtx<d2::dx::FlowBox> ctx)
    {
        using namespace d2::dx;

        auto& count = ctx.dep<int>("count");
        count = 0;

        ctx.elem("count", [&](d2::TreeCtx<Text> ctx)
        {
            dapply(StatusText);

            ctx.subscribe(count,
                [label = ctx.ptr()](d2::TreeIter<Text>, int value)
                {
                    if (label)
                    {
                        label->set<Text::Value>(
                            std::format("Count: {}", value));
                    }
                }
            ).release();
        });

        ctx.elem("increment", [&](d2::TreeCtx<Button> ctx)
        {
            dstyle(Value, "Increment");
            dstyle(OnSubmit, [&count](d2::TreeIter<Button>)
            {
                ++count;
            });
        });
    }
>;

int main()
{
    d2::IOContext::make<
        d2::sys::input,
        d2::sys::output,
        d2::sys::screen
    >()->run<d2::NamedTree<"Dashboard", Dashboard>>();
}
```

`release()` on the dependency subscription follows the retained element-associated pattern in `e1-counter`. A global signal subscription differs: keep its `Signals::Handle` alive (often in `TreeState`). A subscription callback can run *during tree construction*, so establish required widget styles and initialize dependencies first. Ensure any deferred work captures owned data or weak handles, not dangling builder-frame references.

## Structure vs. behavior vs. appearance

| Concern | Preferred home | Why |
| --- | --- | --- |
| Screen hierarchy | `Tree` builder and nested `ctx.elem` | Typed retained layout |
| One widget's initial properties | Its `TreeCtx<Widget>` lambda | Easy to read and modify locally |
| Repeated presentation | `Stylesheet` / `dapply` | Avoid duplicate color/geometry code |
| Theme-derived appearance | Theme variables / `dstylev` | Supports theme changes and consistent palettes |
| Reactive model state | `ctx.dep<T>` and tree state | Separates source-of-truth values from display |
| UI callbacks | `OnSubmit`, `ctx.onv`, screen listeners | Typed event contracts and explicit lifetimes |
| Complex non-UI logic | Named functions/classes/services | Keeps layout lambda inspectable |
| Background I/O | Runtime modules + context synchronization | Avoids UI access on the wrong thread |

## Formatting that an agent should generate

- Use 4 spaces, Allman-like braces, and explicit `TreeCtx<Widget> ctx` on widget lambdas. Source examples also use an expanded `ctx.elem( ... );` style; either is acceptable and local consistency wins.
- Prefer one property-setting call per statement, without wrapping each in a helper or packing multiple calls onto one line.
- Keep trivial callbacks inline; move long bodies to named helpers once they hide the tree structure.
- Use `using namespace d2::dx;` inside a local builder scope if it improves readability. Avoid broad namespace imports in new public headers.
- Reserve `dapply` for genuine reuse; avoid a stylesheet abstraction for a single `dstyle(Value, ...)` call.
- Do not invent CSS classes, HTML-like markup, `.appendChild`, `.onClick`, or an immediate redraw loop. They are not part of the documented UI model.

## What is deliberately **not** codified

No fixed visual theme, padding scale, typography hierarchy, screen grid system, forced MVC/MVVM architecture, or universally optimal file splitting was inferred from these small examples. An LLM should ask only for genuine product requirements and otherwise make pragmatic, source-compatible choices. The **API/ownership conventions** above are stronger constraints than any aesthetic preference.
