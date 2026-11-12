# Style property reference

[Documentation index](../index.md) · [Styles guide](../guide/styles-and-events.md)

Generated from public UAI declarations by `tools/style_reference.py`. Use the symbolic property on the concrete element, for example `Text::Value`; IDs depend on its interface chain. Backing fields/defaults below explain the declaration, not a recommendation to mutate storage directly. Setters handle invalidation and dependency bindings.

`D2_UAI_PROP_VAR(...)` denotes an accessor/proxy (often a bit flag). `0x00`/`None` means no automatic redraw flag. Source declarations remain authoritative for callback and inherited field types.

## ISelectable

Source: [d2_styles.hpp](../../core/tree/d2_styles.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `EnableSelection` | `enable_selection` | `Style` |
| `HighlightMask` | `highlight_mask` | `Style` |

Backing storage and defaults:

```cpp
px::background highlight_mask{.r = 170, .g = 170, .b = 170}
```

## ILayout

Source: [d2_styles.hpp](../../core/tree/d2_styles.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `X` | `x` | `LayoutXPos` |
| `Y` | `y` | `LayoutYPos` |
| `Width` | `width` | `Style \| LayoutWidth` |
| `Height` | `height` | `Style \| LayoutHeight` |
| `ZIndex` | `zindex` | `LayoutXPos \| LayoutYPos` |

Backing storage and defaults:

```cpp
HPUnit x{Unit::Auto}; VPUnit y{Unit::Auto}; HDUnit width{Unit::Auto};
VDUnit height{Unit::Auto};
char zindex{0};
```

## IContainer

Source: [d2_styles.hpp](../../core/tree/d2_styles.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `BorderWidth` | `border_width` | `Style` |
| `BorderHorizontalColor` | `border_horizontal_color` | `Style` |
| `BorderVerticalColor` | `border_vertical_color` | `Style` |
| `CornerTopLeft` | `corners.top_left` | `Style` |
| `CornerTopRight` | `corners.top_right` | `Style` |
| `CornerBottomLeft` | `corners.bottom_left` | `Style` |
| `CornerBottomRight` | `corners.bottom_right` | `Style` |
| `ContainerOptions` | `container_options` | `Element::WriteType::Masked` |
| `ContainerBorder` | `D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, EnableBorder))` | `Masked` |
| `ContainerDisableLeft` | `D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderLeft))` | `Masked` |
| `ContainerDisableRight` | `D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderRight))` | `Masked` |
| `ContainerDisableTop` | `D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderTop))` | `Masked` |
| `ContainerDisableBottom` | `D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderBottom))` | `Masked` |
| `ContainerTopFix` | `D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, TopFix))` | `Style` |
| `ContainerOverrideCorners` | `D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, OverrideCorners))` | `Style` |

Backing storage and defaults:

```cpp
HDUnit border_width{ 1 };
pixel border_horizontal_color{ .a = 0, .v = charset::box_border_horizontal };
pixel border_vertical_color{ .a = 0, .v = charset::box_border_vertical };

struct
{
    px::value_type top_left{ charset::box_tl_corner };
    px::value_type top_right{ charset::box_tr_corner };
    px::value_type bottom_left{ charset::box_bl_corner };
    px::value_type bottom_right{ charset::box_br_corner };
} corners;

unsigned char container_options {
    (charset::top_fix * TopFix) |
    (charset::corners * OverrideCorners)
};
```

## IColors

Source: [d2_styles.hpp](../../core/tree/d2_styles.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `BackgroundColor` | `background_color` | `Style` |
| `ForegroundColor` | `foreground_color` | `Style` |

Backing storage and defaults:

```cpp
px::background background_color{.a = 0}; px::foreground foreground_color{.a = 0};
```

## IKeyboardNav

Source: [d2_styles.hpp](../../core/tree/d2_styles.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `FocusedColor` | `focused_color` | `Style` |

Backing storage and defaults:

```cpp
px::background focused_color{.r = 150, .g = 150, .b = 150};
```

## IText

Source: [d2_styles.hpp](../../core/tree/d2_styles.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `Value` | `text` | `Style` |
| `TextAlignment` | `alignment` | `Style` |
| `TextOptions` | `text_options` | `Style` |
| `TextPreserveWordBoundaries` | `D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&text_options, PreserveWordBoundaries))` | `Style` |
| `TextHandleNewlines` | `D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&text_options, HandleNewlines))` | `Style` |

Backing storage and defaults:

```cpp
string text{}; Alignment alignment{Alignment::Center};
unsigned char text_options{Paragraph};
```

## ICheckbox

Source: [d2_checkbox.hpp](../../elements/d2_checkbox.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `ValueOn` | `value_on` | `Masked` |
| `ValueOff` | `value_off` | `Masked` |
| `ColorOnForeground` | `color_on_foreground` | `Style` |
| `ColorOnBackground` | `color_on_background` | `Style` |
| `ColorOffForeground` | `color_off_foreground` | `Style` |
| `ColorOffBackground` | `color_off_background` | `Style` |
| `Value` | `checkbox_value` | `Masked` |

Backing storage and defaults:

```cpp
string value_on{charset::checkbox_true}; string value_off{charset::checkbox_false};
px::foreground color_on_foreground{};
px::background color_on_background{d2::colors::w::transparent};
px::foreground color_off_foreground{};
px::background color_off_background{d2::colors::w::transparent};
bool checkbox_value{false};
```

## IDemandScrollBox

Source: [d2_demand_scrollbox.hpp](../../elements/d2_demand_scrollbox.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `Preload` | `preload` | `0x00` |
| `Length` | `length` | `Style` |
| `StaticHeightHint` | `static_height_hint` | `Style` |
| `PaddingHorizontal` | `padding_horizontal` | `Style` |
| `PaddingVertical` | `padding_vertical` | `Style` |
| `SwapOut` | `swap_out` | `0x00` |
| `HideSlider` | `hide_slider` | `Style` |
| `FetchCallback` | `fetch_callback` | `0x00` |

Backing storage and defaults:

```cpp
std::size_t preload{ 2 };
std::size_t length{ 0 };
d2::VDUnit static_height_hint{ 0 };
d2::HDUnit padding_horizontal{ 0.0_px };
d2::VDUnit padding_vertical{ 0.0_px };
bool swap_out{ true };
bool hide_slider{ false };

std::function<void(dx::EntrySeed&)> fetch_callback{ nullptr };
```

## IDraggableBox

Source: [d2_draggable_box.hpp](../../elements/d2_draggable_box.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `Title` | `title` | `Style` |
| `BarColor` | `bar_color` | `Style` |
| `BarHeight` | `bar_height` | `Style \| LayoutHeight` |
| `ResizePointValue` | `resize_point_value` | `Style` |
| `VBoxOptions` | `vbox_options` | `Style` |

Backing storage and defaults:

```cpp
string title{};
px::foreground bar_color{ .v = charset::box_top_bar };
px::value_type resize_point_value{ charset::box_top_drag_spot };
VDUnit bar_height{ 2.0_px };
unsigned char vbox_options{ Auto };
```

## IDropdown

Source: [d2_dropdown.hpp](../../elements/d2_dropdown.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `DefaultValue` | `default_value` | `Style` |
| `SelectedForeground` | `selected_foreground_color` | `Style` |
| `SelectedBackground` | `selected_background_color` | `Style` |
| `OptionForeground` | `option_foreground_color` | `Style` |
| `OptionBackground` | `option_background_color` | `Style` |
| `ShowChoice` | `show_choice` | `Style` |
| `Options` | `options` | `Style` |

Backing storage and defaults:

```cpp
string default_value{};

px::foreground selected_foreground_color{};
px::background selected_background_color{};

px::foreground option_foreground_color{};
px::background option_background_color{};

bool show_choice{true};

std::vector<string> options{};
```

## IRangeGraph

Source: [d2_graph.hpp](../../elements/d2_graph.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `MaxColor` | `max_color` | `Style` |
| `MinColor` | `min_color` | `Style` |
| `Resolution` | `resolution` | `Style` |
| `AutomaticResolution` | `automatic_resolution` | `Style` |
| `HighResolution` | `high_resolution` | `Style` |
| `Invert` | `invert` | `Style` |

Backing storage and defaults:

```cpp
pixel max_color{.a = 0, .v = '.'}; pixel min_color{.a = 0, .v = '.'};
int resolution{0};
bool automatic_resolution{true};
bool high_resolution{true};
bool invert{false};
```

## INodeWaveGraph

Source: [d2_graph.hpp](../../elements/d2_graph.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `Resolution` | `resolution` | `Style` |
| `Frequency` | `frequency` | `Style` |
| `NodeColorI` | `node_color_i` | `Style` |
| `NodeColorA` | `node_color_a` | `Style` |
| `NodeColorB` | `node_color_b` | `Style` |
| `NodeColorFocused` | `node_color_focused` | `Style` |

Backing storage and defaults:

```cpp
std::chrono::milliseconds resolution{1000};
std::chrono::milliseconds frequency{1000};
px::foreground node_color_i{px::foreground(colors::g::green)};
px::foreground node_color_a{px::foreground(colors::b::blue)};
px::foreground node_color_b{px::foreground(colors::r::red)};
px::foreground node_color_focused{px::foreground(colors::g::olive)};
```

## IImage

Source: [d2_image.hpp](../../elements/d2_image.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `Source` | `source` | `Style` |
| `SourceRect` | `source_rect` | `Style` |

Backing storage and defaults:

```cpp
image::Raster::ptr source{};
image::Rect source_rect{};
```

## IInput

Source: [d2_input.hpp](../../elements/d2_input.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `Pre` | `pre` | `Masked` |
| `Hint` | `hint` | `Masked` |
| `PtrColor` | `ptr_color` | `Masked` |
| `InputOptions` | `input_options` | `Masked` |

Backing storage and defaults:

```cpp
string pre{};
string hint{};
px::foreground ptr_color{ .a = 255, .v = '_' };
unsigned char input_options{ Auto };
```

## IModel

Source: [d2_matrix.hpp](../../elements/d2_matrix.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `Model` | `model` | `Masked` |
| `BackgroundMask` | `background_mask` | `Masked` |
| `ForegroundMask` | `foreground_mask` | `Masked` |
| `MaskOptions` | `mask_options` | `Masked` |

Backing storage and defaults:

```cpp
MatrixModel::ptr model{nullptr}; px::background background_mask{};
px::foreground foreground_mask{};
unsigned char mask_options{0x00};
```

## IMultiInput

Source: [d2_multiline_input.hpp](../../elements/d2_multiline_input.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `EnableScrollVert` | `enable_scrollbar_vertical` | `Masked` |
| `EnableScrollHoriz` | `enable_scrollbar_horizontal` | `Masked` |

Backing storage and defaults:

```cpp
bool enable_scrollbar_horizontal{ false };
bool enable_scrollbar_vertical{ true };
```

## ISeparator

Source: [d2_separator.hpp](../../elements/d2_separator.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `CornerLeft` | `corner_left` | `Style` |
| `CornerRight` | `corner_right` | `Style` |
| `OverrideCorners` | `override_corners` | `Style` |
| `HideBody` | `hide_body` | `Style` |

Backing storage and defaults:

```cpp
value_type corner_left{ '<' };
value_type corner_right{ '>' };
bool override_corners{ false };
bool hide_body{ false };
```

## ISlider

Source: [d2_slider.hpp](../../elements/d2_slider.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `Max` | `max` | `Style` |
| `Min` | `min` | `Style` |
| `Step` | `step` | `None` |
| `SliderColor` | `slider_color` | `Style` |
| `SliderWidth` | `slider_width` | `Style` |

Backing storage and defaults:

```cpp
float min{0.f}; float max{100.f}; float step{1.f};
pixel slider_color{.r = 255, .g = 255, .b = 255, .v = ' '};
HDUnit slider_width{1};
```

## ISwitch

Source: [d2_switch.hpp](../../elements/d2_switch.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `Options` | `options` | `Masked` |
| `EnabledForegroundColor` | `enabled_foreground_color` | `Style` |
| `EnabledBackgroundColor` | `enabled_background_color` | `Style` |
| `DisabledForegroundColor` | `disabled_foreground_color` | `Style` |
| `DisabledBackgroundColor` | `disabled_background_color` | `Style` |
| `OnChange` | `on_change` | `0x00` |
| `OnChangeValues` | `on_change_values` | `0x00` |
| `SeparatorColor` | `separator_color` | `Style` |
| `DisableSeparator` | `disable_separator` | `Style` |
| `ExpandBackground` | `disable_separator` | `Style` |
| `PreColor` | `pre_color` | `Style` |
| `PostColor` | `post_color` | `Style` |
| `SelectPre` | `select_pre` | `Style` |
| `SelectPost` | `select_post` | `Style` |
| `TextAlignment` | `text_alignment` | `Style` |

Backing storage and defaults:

```cpp
std::function<void(TreeIter<dx::Switch>, int, int)> on_change{nullptr};
std::function<void(TreeIter<dx::Switch>, string, string)> on_change_values{nullptr};
std::vector<string> options{};

px::background disabled_background_color{d2::colors::w::transparent};
px::foreground disabled_foreground_color{};
px::background enabled_background_color{.r = 255, .g = 255, .b = 255};
px::foreground enabled_foreground_color{.r = 0, .g = 0, .b = 0};
pixel separator_color{.a = 0, .v = '|'};
bool disable_separator{false};
bool expand_background{true};

px::foreground pre_color{};
px::foreground post_color{};
string select_pre{""};
string select_post{""};

IZText::Alignment text_alignment{IZText::Alignment::Center};
```

## IVerticalSwitch

Source: [d2_switch.hpp](../../elements/d2_switch.hpp)

| Property | Backing field/accessor | Invalidation |
| --- | --- | --- |
| `SliderBackgroundColor` | `slider_background_color` | `Style` |
| `SliderColor` | `slider_color` | `Style` |
| `LeftConnector` | `left_connector` | `Style` |
| `RightConnector` | `right_connector` | `Style` |

Backing storage and defaults:

```cpp
px::foreground slider_background_color{.v = charset::slider_vertical};
pixel slider_color{.r = 255, .g = 255, .b = 255, .v = ' '};
value_type left_connector{charset::box_border_connector_left};
value_type right_connector{charset::box_border_connector_right};
```

## IResponsive

The handwritten `IResponsive<Element, Args...>::type` interface provides `OnSubmit`, a `std::function<void(TreeIter<Element>, Args...)>`. Argument types are selected by each widget (for example a button uses only its iterator, while a slider also reports values). See [d2_styles.hpp](../../core/tree/d2_styles.hpp) and the widget declaration.
