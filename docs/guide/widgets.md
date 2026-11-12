# Widget catalogue

[Documentation index](../index.md) · [Style properties](../reference/styles.md)

All names below are in `d2::dx`. Include `<elements/d2_std.hpp>` for the catalogue, or the listed header under `elements/`. Common layout, color, container and text properties are inherited where the header's `data`/UAI declaration includes those interfaces. Set dimensions explicitly when a widget has no meaningful intrinsic size.

| Element | Header | Purpose and principal controls |
| --- | --- | --- |
| `Box` | `d2_box.hpp` | Parent with layout, borders, padding, colors and child clipping |
| `FlowBox`, `ScrollFlowBox`, `VirtualFlowBox` | `d2_flow_box.hpp` | Container-managed relative child placement |
| `VirtualBox` | `d2_draggable_box.hpp` | Window-like draggable/resizable container with title and option flags |
| `ScrollBox` | `d2_scrollbox.hpp` | Vertical viewport; `scroll_to`, `scroll_offset`, `internal_height`, `window_height`, `scrollbar`, `sync` |
| `DemandScrollBox` | `d2_demand_scrollbox.hpp` | On-demand entries; `Length`, `FetchCallback`, `Preload`, height hint, padding, `SwapOut`, `scroll_to`, `reset` |
| `Text` | `d2_text.hpp` | Display text with wrapping, alignment, selection geometry and highlight styles |
| `Input` | `d2_input.hpp` | Single-line grapheme editor; value, cursor, selection, insertion, deletion and submit callback |
| `MultiInput` | `d2_multiline_input.hpp` | Multiline grapheme editor with viewport, selection, undo/redo and configurable history limits |
| `Button` | `d2_button.hpp` | Text/button appearance with `OnSubmit` callback |
| `Checkbox` | `d2_checkbox.hpp` | Checked state and glyph/color styling |
| `Switch`, `VerticalSwitch` | `d2_switch.hpp` | Horizontal or vertical toggle with state-change callbacks |
| `Slider`, `VerticalSlider` | `d2_slider.hpp` | Directional range/value controls and change callbacks |
| `Dropdown` | `d2_dropdown.hpp` | Choice list, selection, window and submit behavior |
| `Image` | `d2_image.hpp` | Shared RGBA source/crop and asynchronous `load`; requires an output renderer to display graphics |
| `Matrix` | `d2_matrix.hpp` | Character-pixel model display through screen-managed `MatrixModel` resources |
| `graph::CyclicVerticalBar`, `graph::NodeWaveGraph` | `d2_graph.hpp` | Rolling bar data (`push`, `clear`, `rescale`) and editable wave nodes |
| `Separator`, `VerticalSeparator` | `d2_separator.hpp` | Styled separator with orientation and line character controls |

## Editors versus display text

`Input` and `MultiInput` handle their own editing selection. `Text` exposes geometry and selection styling but needs the optional `sys::selector` module for mouse-driven generic selection across elements. Clipboard integration is a separate optional module. This distinction is intentional; see [text and selection](text-and-selection.md) for grapheme indexes, cursor movement, wrapping and keyboard behavior.

## Loading an image

```cpp
auto loaded = picture->load("assets/logo.png");
// In a coroutine:
bool applied = co_await loaded;
```

Call `load` on the active UI thread with a registered image loader. The widget remains usable while loading. `false` means the target was destroyed, superseded, explicitly cancelled or moved to another context; decode/I/O failures are exceptions. See [images](images.md) for the complete ownership and completion contract.

## Demand-loaded lists

Set `Length` and provide `FetchCallback` accepting `EntrySeed&`. The seed's `index()` identifies the item, and `create<Element>(...)` creates exactly one entry. Do not retain a seed after its callback returns or use it twice. Use a `StaticHeightHint` when item heights are predictable, and `Preload` to create a margin around the visible range. `SwapOut` determines whether entries leaving the viewport can be discarded. Application state should live outside short-lived entry elements.

## Extras

`extras/d2_default_theme.hpp` and `d2_default_styles.hpp` provide ready-made styling. `extras/d2_loading_indicators.hpp` contains loading decorations. `extras/d2_debug_box.hpp` exposes a diagnostic view using screen/scheduler statistics. Extras require elements and can be excluded independently with `D2_EXCLUDE_LIBRARY_EXTRAS=ON`.

Widget-specific behavior outside the editor/image paths has varying regression coverage. The generated API reference exposes exact signatures; the [limitations page](../reference/limitations.md) separates supported contracts from incomplete subsystems.
