# 04. Units, properties, styles, themes and reactive state

**Source:** `core/types/d2_element_units.hpp`, `core/tree/d2_styles.hpp`, `d2_styles_base.hpp`, `d2_theme.hpp`, `d2_tree_construct.hpp`, `extras/d2_default_*`.

## Typed Universal Access Interface (UAI)

Widget style properties are **compile-time typed identifiers**, assembled from `style::I...` interfaces. A widget usually derives from `style::UAI<Widget, style::ILayout, style::IColors, ...>`. To know what `Widget::Property` accepts, look at its `D2_UAI_INTERFACE` definitions and the widget's `UAI` list.

### Shared property families

| Interface | Properties |
| --- | --- |
| `style::ILayout` | `X`, `Y`, `Width`, `Height`, `ZIndex` |
| `style::IColors` | `BackgroundColor`, `ForegroundColor` |
| `style::IContainer` | `BorderWidth`, `BorderHorizontalColor`, `BorderVerticalColor`, `CornerTopLeft`, `CornerTopRight`, `CornerBottomLeft`, `CornerBottomRight`, `ContainerOptions`, `ContainerBorder`, `ContainerDisableLeft`, `ContainerDisableRight`, `ContainerDisableTop`, `ContainerDisableBottom`, `ContainerTopFix`, `ContainerOverrideCorners` |
| `style::IKeyboardNav` | `FocusedColor` |
| `style::IText` | `Value`, `TextAlignment`, `TextOptions`, `TextPreserveWordBoundaries`, `TextHandleNewlines` |
| `style::ISelectable` | `EnableSelection`, `HighlightMask` |
| `style::IResponsive<Widget, Args...>` | `OnSubmit` (takes `TreeIter<Widget>, Args...`) |

An element only has the subset of interfaces it explicitly includes. For example, `Text::Value` exists, but `Box::Value` is not a universal base property. `Color` fields are strongly typed `px::foreground`/`px::background` or pixel structures, not CSS strings.

### Style-setting forms

```cpp
// Inside a callback whose parameter is named 'ctx':
dstyle(Value, "Ready");
dstyle(Width, 30.0_px);
dstyle(X, 0.0_center);

// Outside the macro or from a retained handle:
label->set<d2::dx::Text::Value>("Updated");
auto old_value = label->get<d2::dx::Text::Value>();
```

`dstylev(Property, ThemeEnumValue)` sets a property from a theme variable. `dstyledv(Property, ThemeEnumValue, expression)` computes a transformed theme-dependent binding whose expression refers to the local name `value`. `dmstyle(element, Property, value)` sets a property on another typed element handle. `dapply(Stylesheet, args...)` invokes `TreeCtx::apply`.

**Mutation rule:** assign through `set` or a tracked dependency, not by editing `getref`'s underlying field. UAI's source forwards cross-thread reads/writes to the context for synchronization. Some raw references bypass that pathway.

## Units and positioning

`d2::Unit` expresses a numeric magnitude and unit/positioning modifiers. Literal suffix examples:

| Literal | Concept |
| --- | --- |
| `12.0_px` / `12.0_pxi` | terminal-cell quantity (`Px`) |
| `25.0_pv` | percent of viewport (`Pv`) |
| `50.0_pc` | percent of parent (`Pc`) |
| `1.0_auto` | automatic content-driven sizing (`Auto`) |
| `0.0_center` | centered position modifier |
| `1.0_relative` | relative positioning |
| `...` | variants with `Inverted`, `Adjusted` and combined forms also exist |

Prefer established expressions from repository examples, such as `0.0_center`, `1.0_relative`, `1.0_auto`, and verify unfamiliar suffixes in `d2_element_units.hpp` instead of inventing `_vw` or CSS `%` syntax. Width/Height and X/Y can interpret units differently based on parent layout. `FlowBox` uses its own flow positioning behavior.

## Dependencies: named, typed, reactive values

```cpp
// In a tree builder:
auto& count = ctx.dep<int>("count");
count = 0;

ctx.elem([&](d2::TreeCtx<d2::dx::Text> ctx)
{
    dstyle(Value, "0");
    ctx.subscribe(count,
        [=](d2::TreeIter<d2::dx::Text>, int next)
        {
            ctx->set<d2::dx::Text::Value>(std::to_string(next));
        }
    ).release();
});
```

- `style::Dependency<T>` supports `set`, `set_async`, `value`, arithmetic update operators, `link`, `unlink`, `subscribe`, `apply`, `cleanup`.
- `ctx.dep<T>("name")` gets the current tree's named dependency; `ctx.dep<T>("OtherTree","name")` accesses another registered tree dependency.
- **Subscription initialization is eager:** the dependency's implementation invokes the callback with its current value during subscription, before returning its handle. Ensure the element exists/has expected basic styles first.
- `subscribe` returns `style::DependencyHandle`, which manages cancellation. A handle owned by a temporary normally ends its subscription at end of statement. Repository examples use `.release()` for element-associated subscriptions; another correct pattern is `ctx.declare<style::DependencyHandle>() = ctx.subscribe(...);` to tie the handle to element lifetime.
- Direct dependency binding of a compatible style property is supported by UAI; setting a plain value later can replace the reactive binding. Use a subscription where you need formatting/transformation, control flow, or side effects.
- `ctx.branch(dep, std::pair{key, [](TreeCtx<...>) { ... }}, ...)` handles conditional subtree reconstruction.

When a dependency callback outlives a local scope, inspect captures carefully. Do not capture ephemeral stack variables by reference in callbacks without a provably longer owner.

## Reusable stylesheets

```cpp
using AccentText = d2::Stylesheet<
    []<typename T>(d2::TreeCtx<T> ctx)
    {
        dstyle(X, 0.0_center);
        dstyle(ForegroundColor, d2::colors::w::white);
    }
>;

// Inside a compatible widget callback:
dapply(AccentText);
```

`Stylesheet<Callback, ArgTypes...>` also supports parameters: the callback accepts `TreeCtx<Widget>` followed by `ArgTypes...`, and `dapply(Name, args...)` supplies them. This is source-native reuse instead of creating a second styling DSL.

## Themes

Themes are collections of named typed dependencies, accessed using compile-time enumerators through `ctx.var<ThemeEnum>()`, `dstylev`, `dstyledv`. A default example is in `extras/d2_default_theme.hpp`, stylesheets in `extras/d2_default_styles.hpp` (`d2::ex::df`). Instantiate/register a concrete theme using the theme APIs (`style::Theme::make<...>` / named themes), pass it into `run(...)`, or use screen theme facilities.

A concrete default-theme initialization, matching the available factory and `run` signature:

```cpp
#include <extras/d2_default_theme.hpp>
#include <extras/d2_default_styles.hpp>

// Use the same App tree as in the quickstart.
auto theme = d2::style::Theme::make<d2::ex::DefaultTheme>();
d2::IOContext::make<d2::sys::input, d2::sys::output, d2::sys::screen>()
    ->run<d2::NamedTree<"App", App>>(theme);

// Inside a TreeCtx<dx::Text> callback, with the theme registered:
// dapply(d2::ex::df::text);
// dstylev(ForegroundColor, d2::ex::DefaultTheme::Text);
```

`ex::DefaultTheme::Text`, `TextMuted`, `Surface`, `Accent`, `BorderHoriz`, etc. are `DefaultThemeDeps::Deps` entries, and its `ThemeRegistry` specialization is declared in the default-theme header.

Unlike static constants, theme variables can remain reactive. `ctx.dynavar` produces a derived value from a variable through a compile-time callback; this is useful for resolved colors. Theme accessors require the type/enum to be registered with `ThemeRegistry`, not an arbitrary enum.

## Extension author checklist

1. Select existing interfaces (`ILayout`, `IColors`, `IText`, etc.).
2. Declare any new interface via `D2_UAI_INTERFACE(Name, D2_UAI_OPTS(...), D2_UAI_FIELDS(...), D2_UAI_PROPS(...), D2_UAI_LINK(...))` in the style layer.
3. Construct widget `style::UAI<Widget,...>` with that interface; expose `using data = ...; using data::data;` as in standard widgets.
4. Set precise `D2_UAI_PROP(..., field, WriteType)` flags so changing a property correctly invalidates style/layout/frame.
5. Verify every property maps to the intended backing field and has appropriate invalidation behavior.

Do not copy an invented property ID: IDs are produced through the UAI interface chain machinery in `d2_styles.hpp`.
