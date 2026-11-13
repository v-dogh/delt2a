# 05. Standard element API catalogue

The standard widget header aggregator is `#include <elements/d2_std.hpp>`. All ordinary widgets below are in `d2::dx`, except graph types in `d2::dx::graph`. Every widget exposes *only* the UAI property families it inherits. `ILayout` supplies `X/Y/Width/Height/ZIndex` when present.

**How to use this document:** the tables name property identifiers, not necessarily field types. Check each header's `D2_UAI_FIELDS` and `D2_UAI_LINK` when constructing a value or building generic components. See [properties and themes](04_PROPERTIES_STYLES_THEMES.md).

## Containers and layout

| Widget / header | Typical purpose | API and details |
| --- | --- | --- |
| `Box` / `d2_box.hpp` | General parent | Layout, container border/corners, colors; ordinary nested `ctx.elem` children |
| `FlowBox` / `d2_flow_box.hpp` | Automatic flow | Flow-style layout; prefer for sequences of labels/buttons that should size naturally |
| `ScrollFlowBox` / `d2_flow_box.hpp` | Flow + scrolling | Flow layout with scrolling behavior; inspect specialized layout rules |
| `VirtualFlowBox` / `d2_flow_box.hpp` | Flow variant | Specialized flow container; inspect source when deriving |
| `ScrollBox` / `d2_scrollbox.hpp` | Scrollable ordinary box | `scrollbar()`, `internal_height()`, `window_height()`, `scroll_offset()`, `scroll_to(int)`, `sync()` |
| `DemandScrollBox` / `d2_demand_scrollbox.hpp` | Large, lazy lists | `Preload`, `Length`, `StaticHeightHint`, `PaddingHorizontal`, `PaddingVertical`, `SwapOut`, `HideSlider`, `FetchCallback`; `scrollbar()`, `scroll_to(...)`, `reset()` |
| `VirtualBox` / `d2_draggable_box.hpp` | Movable/resizable window | `Title`, `BarColor`, `BarHeight`, `ResizePointValue`, `VBoxOptions`; `close()` |

### `DemandScrollBox` is a virtualized container, not a generic `Box`

`FetchCallback` has type `std::function<void(d2::dx::EntrySeed&)>`. For each requested row, use `seed.create<Element>(constructor_args...)` once. `seed.index()` supplies the item index; `seed.parent()` returns the scroll box handle. A seed may only create one element. Source directly throws `std::logic_error` on a second `create` call. For nontrivial row styling, configure the returned element explicitly or use a suitable typed `TreeCtx`.

```cpp
ctx.elem([](d2::TreeCtx<d2::dx::DemandScrollBox> ctx)
{
    dstyle(Length, std::size_t{10000});
    dstyle(Preload, std::size_t{3});
    dstyle(FetchCallback, [](d2::dx::EntrySeed& seed)
    {
        auto row = seed.create<d2::dx::Text>();
        row->set<d2::dx::Text::Value>(
            std::format("Row {}", seed.index()));
    });
});
```

`VirtualBox::VBoxOptions` bit flags: `Resizable`, `Draggable`, `Minimizable`, `Auto`, `Freezed`, `Static`. They are defined in the style interface, not as independent global constants.

## Text, buttons, interaction

| Widget | Additional property IDs | Methods / callback contract |
| --- | --- | --- |
| `Text` | `Value`, `TextAlignment`, `TextOptions`, `TextPreserveWordBoundaries`, `TextHandleNewlines`, `EnableSelection`, `HighlightMask`; colors | Selection/text geometry helpers; ordinary display of text |
| `Button` | Text, colors, container, `FocusedColor`, `OnSubmit` | `OnSubmit`: `void(d2::TreeIter<d2::dx::Button>)` |
| `Checkbox` | `Value` (`bool`), `ValueOn`, `ValueOff`, `ColorOnForeground`, `ColorOnBackground`, `ColorOffForeground`, `ColorOffBackground`, `FocusedColor`, `OnSubmit` | `OnSubmit`: `void(TreeIter<Checkbox>,bool)` |
| `Dropdown` | `DefaultValue`, `SelectedForeground`, `SelectedBackground`, `OptionForeground`, `OptionBackground`, `ShowChoice`, `Options`, `OnSubmit` | `choose(int)`, `reset()`, `reset(const string&)`, `index()`, `value()`; `OnSubmit`: `void(TreeIter<Dropdown>,string,std::size_t)` |
| `Switch` | `Options`, `EnabledForegroundColor`, `EnabledBackgroundColor`, `DisabledForegroundColor`, `DisabledBackgroundColor`, `OnChange`, `OnChangeValues`, `SeparatorColor`, `DisableSeparator`, `ExpandBackground`, `PreColor`, `PostColor`, `SelectPre`, `SelectPost`, `TextAlignment` | `reset(int=-1)`, `reset(string)`, `choice()`, `index()`; `OnChange`: `(TreeIter<Switch>, int new, int old)`; `OnChangeValues`: `(TreeIter<Switch>, string new, string old)` |
| `VerticalSwitch` | Switch properties plus `SliderBackgroundColor`, `SliderColor`, `LeftConnector`, `RightConnector` | Vertically oriented switch, includes inherited choice/reset |

### Button and checkbox example

```cpp
ctx.elem([](d2::TreeCtx<d2::dx::Button> ctx)
{
    dstyle(Value, "Quit");
    dstyle(OnSubmit, [](d2::TreeIter<d2::dx::Button> button)
    {
        button->context()->stop();
    });
});

ctx.elem([](d2::TreeCtx<d2::dx::Checkbox> ctx)
{
    dstyle(Value, true);
    dstyle(OnSubmit, [](d2::TreeIter<d2::dx::Checkbox>, bool enabled)
    {
        // Consume the new checkbox value.
    });
});
```

## Text editing

| Widget | Property IDs / options | Methods |
| --- | --- | --- |
| `Input` | `Value` from `IText`, `Pre`, `Hint`, `PtrColor`, `InputOptions`, `EnableSelection`, `HighlightMask`, `OnSubmit` | `jump(std::size_t)`, `sync()`, `cursor() const`; submit callback `(TreeIter<Input>, string)` |
| `MultiInput` | `Pre`, `Hint`, `PtrColor`, `InputOptions`, `EnableScrollVert`, `EnableScrollHoriz`, selectable properties; **does not derive from `Input`** | `value() const`, `value(string)`, `clear()`, `undo()`, `redo()`, `sync()`, `is_synced()`, `jump(pos)`, `insert(text)`, `append(text)`, `erase(len)`, `erase_at(pos,len)`, `disable_history()`, `enable_history()`, `cursor()`, `vertical_scrollbar()`, `horizontal_scrollbar()`, `buffer()` |

`InputOptions` is a bitmask enum: `DrawPtr`, `Censor`, `Blink`, `AutoPtrColor`, `PtrOverlap`, `ReadOnly`, with `Auto` and `Protected` presets. Its declaration lives in the style interface in `elements/d2_input.hpp`. **Important type oddity:** `MultiInput`'s UAI includes `style::IResponsive<Input,string>` rather than `IResponsive<MultiInput,string>`; check concrete callbacks/tests before writing code that assumes its `OnSubmit` takes `TreeIter<MultiInput>`.

`MultiInput` uses `fb::FragmentedBuffer`, cursor offsets, viewport and history; it owns internal scrollbars. It has the misspelled `vertical_scollbar()` compatibility method in addition to `vertical_scrollbar()`. For multiline editing, prefer public editing methods to altering `buffer()` directly.

## Sliders

| Widget | Props | Methods/callback |
| --- | --- | --- |
| `Slider` | `Min`, `Max`, `Step`, `SliderColor`, `SliderWidth`, `FocusedColor`, `OnSubmit` | `reset_absolute(float=0)`, `reset_relative(float=0)`, `absolute_value()`, `relative_value()`; `OnSubmit`: `(TreeIter<Slider>, float relative_value, float absolute_value)` |
| `VerticalSlider` | Inherited slider props | Changes alignment and rendering orientation; inherits slider methods |

`Slider::_submit()` in `elements/d2_slider.cpp` explicitly passes **relative value first**, then **absolute value**. The relative position is distinct from `Min`/`Max`-scaled absolute value. `SliderWidth` is a strong unit type (`HDUnit`), not an arbitrary CSS width.

## Graphics and primitives

| Widget | Properties | Methods / objects |
| --- | --- | --- |
| `Image` | `Source` (`image::Raster::ptr`), `SourceRect` (`image::Rect`), layout/container/colors | `load(path, image::LoadOptions={}, loader_name={}) -> mt::co_future<bool>`, `cancel_load()` |
| `Matrix` | `Model` (`MatrixModel::ptr`), `BackgroundMask`, `ForegroundMask`, `MaskOptions` | Pixel-matrix display, model generation/loading/saving via `MatrixModel` |
| `Separator` | `CornerLeft`, `CornerRight`, `OverrideCorners`, `HideBody` | Horizontal separator |
| `VerticalSeparator` | Separator family | Vertical variant |
| `graph::CyclicVerticalBar` | `MaxColor`, `MinColor`, `Resolution`, `AutomaticResolution`, `HighResolution`, `Invert` | `push(float)`, `clear()`, `rescale(float)`, `rescale(float,float)` |
| `graph::NodeWaveGraph` | `Resolution`, `Frequency`, `NodeColorI`, `NodeColorA`, `NodeColorB`, `NodeColorFocused` | `push()`, `push(float start,float end)`, `pop(id)`, `set_parameters(id,NodePair)` |

`NodeWaveGraph::NodePair` has `id`, `start`, `end`, `phase`, `frequency`, `parameters` (`std::vector<float>`) and an interpolator `std::function<float(float,float,float,std::span<const float>)>`. `CyclicVerticalBar` stores a rolling vector of floats and renders within its resolution. Graph colors are `pixel`/foreground structures.

`Matrix`'s `MaskOptions` flags are `ApplyFg`, `ApplyBg`, `InterpFg`, `InterpBg` (declared on its `style::IModel` interface).

## Other exported conveniences

- `d2::ex::df` reusable default styles, default theme in `extras/d2_default_theme.hpp`.
- Loading indicators in `extras/d2_loading_indicators.hpp`.
- `d2::ex::debug` (lowercase) in `extras/d2_debug_box.hpp`, a `TreeForward` debug/profiling panel.

**Source pointers:** each widget has its declaration and corresponding implementation in `elements/d2_<name>.hpp/.cpp`. Most public style properties are defined in those same headers, not in a centralized property enum.
