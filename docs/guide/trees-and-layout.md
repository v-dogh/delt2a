# Trees and layout

[Documentation index](../index.md) · [Widget catalogue](widgets.md)

## Construct once, update retained objects

`Tree<State, Root, Build, Args...>` specifies a state type, root element and construction callable. `TreeForward` supports construction in a named class. `TreeCtx<Element, State>` gives access to the current element, application state, screen and context. `ctx.elem(lambda)` infers the child type from the lambda's `TreeCtx<T>` argument and returns a `TreeIter<T>`.

```cpp
auto label = ctx.elem([](d2::TreeCtx<d2::dx::Text> ctx)
{
    dstyle(Value, "Ready");
    dstyle(Width, d2::Unit(20));
});
label->set<d2::dx::Text::Value>("Working");
```

Use `ctx.anchor<T>()` when one child needs to reference another before construction. The overload of `ctx.elem(anchor, lambda)` fills that anchor. `ctx.apply<Stylesheet>()` applies reusable construction/style code. Branch/persistence helpers support conditional subtrees and carrying selected elements between screen trees; see their declarations in `core/tree/d2_tree_construct.hpp` and the larger examples before using them for ownership-sensitive flows.

`ParentElement::create` adds children. `Element::remove()` detaches an element, while `extract()` returns the detached shared pointer for reuse. A removed element can remain alive while application references exist. Prefer weak references in callbacks owned by the same tree. Do not retain references to construction lambda stack variables.

## Geometry

Positions and dimensions use terminal cells. `X`, `Y`, `Width`, `Height` accept `Unit`; they are resolved against the parent, screen and container behavior.

| Unit | Meaning | Example |
| --- | --- | --- |
| `Px` | Cells | `Unit(12)` or `12.0_px` |
| `Pc` | Fraction of parent extent | `Unit(0.5f, Unit::Pc)` or `0.5_pc` |
| `Pv` | Fraction of viewport extent | `Unit(1.f, Unit::Pv)` or `1.0_pv` |
| `Auto` | Element/container computes size or alignment | `Unit(Unit::Auto)` or `0.0_auto` |
| `Relative` modifier | Container-relative positioning | `0.0_relative` |
| `Inverted` modifier | Measure from the opposite edge | `0.0_pxi` |
| `Center` modifier | Center relative to available geometry | `0.0_center` |

Literal suffixes live in `d2`. Fractions use `1.0` for the full extent, not `100`. `HDUnit` and `VDUnit` annotate horizontal/vertical interpretation. Adjusted units account for cell aspect ratio; ordinary cell units remain preferable when exact character alignment matters.

A `Box` supplies a bounded parent and clipping. `FlowBox` lays out relative children; `ScrollBox` adds a vertical viewport and scrollbar, and `ScrollFlowBox` combines both. Set explicit dimensions for image widgets and scroll viewports. Text can measure its content, but wrapping depends on the actual allocated width.

Borders and padding reduce usable content geometry. The `IContainer` properties select border sides, widths, characters and padding. `position()`/`box()` give resolved outer geometry; internal variants describe content geometry. `position_screen_space()` translates to the terminal. Layout override functions are for layout implementations/tests, not the normal styling path.

## Painting and state

`ZIndex` controls sibling paint ordering. Children are clipped by parent/view geometry; image and text content follow that same order. `Element::Display` controls visibility. `Swapped`, `Created`, `Embedded` and input-related states represent different lifecycle or interaction conditions; hiding is not destruction.

Use setters to invalidate cached layout/style. Direct writes to inherited style storage bypass invalidation and bindings. A custom element reports geometry through `_layout_impl` and draws through `_frame_impl`; see [extension rules](extending.md).

`TreeIter` supports typed `as<T>()`/`asp<T>()`, `ptr()`/`weak()`, traversal and path navigation. A path lookup can fail; validate the result before accessing it. Paths and element names are application-level identifiers rather than permanent ownership handles.
