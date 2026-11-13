# `d2::style`

**Kind:** namespace
**Declared in:** `core/tree/d2_styles.hpp:17`

## Enums

### `DepQuery`

```cpp
enum DepQuery
```

Source: `core/tree/d2_theme.hpp:13`

Values: `Invoke`, `Destroy`


## Functions

### `D2_UAI_INTERFACE`

```cpp
d2::style::D2_UAI_INTERFACE(Selectable, D2_UAI_OPTS(), D2_UAI_FIELDS(px::background highlight_mask{.r=170,.g=170,.b=170}), D2_UAI_PROPS(EnableSelection, HighlightMask), D2_UAI_PROP(HighlightMask, highlight_mask, Style))
```

Source: `core/tree/d2_styles.hpp:825`

### `D2_UAI_OPTS`

```cpp
IVSelectable d2::style::D2_UAI_OPTS()
```

Source: `core/tree/d2_styles.hpp:927`

### `D2_UAI_FIELDS`

```cpp
IVSelectable d2::style::D2_UAI_FIELDS(px::background background_color{.a=0};px::foreground foreground_color{.a=0};)
```

Source: `core/tree/d2_styles.hpp:928`

### `D2_UAI_PROPS`

```cpp
IVSelectable d2::style::D2_UAI_PROPS(BackgroundColor, ForegroundColor)
```

Source: `core/tree/d2_styles.hpp:931`

### `D2_UAI_FIELDS`

```cpp
IVSelectable d2::style::D2_UAI_FIELDS(px::background focused_color{.r=150,.g=150,.b=150};)
```

Source: `core/tree/d2_styles.hpp:939`

### `D2_UAI_PROPS`

```cpp
IVSelectable d2::style::D2_UAI_PROPS(FocusedColor)
```

Source: `core/tree/d2_styles.hpp:940`

### `D2_UAI_INTERFACE`

```cpp
IVSelectable d2::style::D2_UAI_INTERFACE(Text, D2_UAI_OPTS(enum class Alignment{ Left, Center, Right, };enum TextOption{ PreserveWordBoundaries=1<< 0, HandleNewlines=1<< 1, Paragraph=PreserveWordBoundaries|HandleNewlines };), D2_UAI_FIELDS(string text{};Alignment alignment{Alignment::Center};unsigned char text_options{Paragraph};), D2_UAI_PROPS(Value, TextAlignment, TextOptions, TextPreserveWordBoundaries, TextHandleNewlines), D2_UAI_LINK(D2_UAI_PROP(Value, text, Style) D2_UAI_PROP(TextAlignment, alignment, Style) D2_UAI_PROP(TextOptions, text_options, Style) D2_UAI_PROP(TextPreserveWordBoundaries, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&text_options, PreserveWordBoundaries)), Style) D2_UAI_PROP(TextHandleNewlines, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&text_options, HandleNewlines)), Style))) template< typename Type
```

Source: `core/tree/d2_styles.hpp:944`

### `dynavar`

```cpp
template<auto Func, typename Type>
auto d2::style::dynavar(Type &dep)
```

Source: `core/tree/d2_theme.hpp:675`

### `D2_UAI_INTERFACE`

```cpp
d2::style::D2_UAI_INTERFACE(RangeGraph, D2_UAI_OPTS(), D2_UAI_FIELDS(pixel max_color{.a=0,.v='.'};pixel min_color{.a=0,.v='.'};int resolution{0};bool automatic_resolution{true};bool high_resolution{true};bool invert{false};), D2_UAI_PROPS(MaxColor, MinColor, Resolution, AutomaticResolution, HighResolution, Invert), D2_UAI_PROP(Invert, invert, Style))) D2_UAI_INTERFACE(NodeWaveGraph
```

Source: `elements/d2_graph.hpp:12`

### `D2_UAI_FIELDS`

```cpp
d2::style::D2_UAI_FIELDS(std::chrono::milliseconds resolution{1000};std::chrono::milliseconds frequency{1000};px::foreground node_color_i{px::foreground(colors::g::green)};px::foreground node_color_a{px::foreground(colors::b::blue)};px::foreground node_color_b{px::foreground(colors::r::red)};px::foreground node_color_focused{px::foreground(colors::g::olive)};)
```

Source: `elements/d2_graph.hpp:35`

### `D2_UAI_PROPS`

```cpp
d2::style::D2_UAI_PROPS(Resolution, Frequency, NodeColorI, NodeColorA, NodeColorB, NodeColorFocused)
```

Source: `elements/d2_graph.hpp:43`

### `D2_UAI_INTERFACE`

```cpp
d2::style::D2_UAI_INTERFACE(Switch, D2_UAI_OPTS(using Alignment=IZText::Alignment;), D2_UAI_FIELDS(std::function< void(TreeIter< dx::Switch >, int, int)> on_change{nullptr};std::function< void(TreeIter< dx::Switch >, string, string)> on_change_values{nullptr};std::vector< string > options{};px::background disabled_background_color{d2::colors::w::transparent};px::foreground disabled_foreground_color{};px::background enabled_background_color{.r=255,.g=255,.b=255};px::foreground enabled_foreground_color{.r=0,.g=0,.b=0};pixel separator_color{.a=0,.v='|'};bool disable_separator{false};bool expand_background{true};px::foreground pre_color{};px::foreground post_color{};string select_pre{""};string select_post{""};IZText::Alignment text_alignment{IZText::Alignment::Center};), D2_UAI_PROPS(Options, EnabledForegroundColor, EnabledBackgroundColor, DisabledForegroundColor, DisabledBackgroundColor, OnChange, OnChangeValues, SeparatorColor, DisableSeparator, ExpandBackground, PreColor, PostColor, SelectPre, SelectPost, TextAlignment), D2_UAI_LINK(D2_UAI_PROP(Options, options, Masked) D2_UAI_PROP(EnabledForegroundColor, enabled_foreground_color, Style) D2_UAI_PROP(EnabledBackgroundColor, enabled_background_color, Style) D2_UAI_PROP(OnChange, on_change, 0x00) D2_UAI_PROP(OnChangeValues, on_change_values, 0x00) D2_UAI_PROP(SeparatorColor, separator_color, Style) D2_UAI_PROP(DisableSeparator, disable_separator, Style) D2_UAI_PROP(ExpandBackground, disable_separator, Style) D2_UAI_PROP(PreColor, pre_color, Style))) D2_UAI_INTERFACE(VerticalSwitch
```

Source: `elements/d2_switch.hpp:16`

### `D2_UAI_FIELDS`

```cpp
d2::style::D2_UAI_FIELDS(px::foreground slider_background_color{.v=charset::slider_vertical};pixel slider_color{.r=255,.g=255,.b=255,.v=' '};value_type left_connector{charset::box_border_connector_left};value_type right_connector{charset::box_border_connector_right};)
```

Source: `elements/d2_switch.hpp:79`

### `D2_UAI_PROPS`

```cpp
d2::style::D2_UAI_PROPS(SliderBackgroundColor, SliderColor, LeftConnector, RightConnector)
```

Source: `elements/d2_switch.hpp:85`


## Typedefs

### `UAI`

```cpp
template<typename Base, D2_UAI_INTERFACE_TEMPL Interface, D2_UAI_INTERFACE_TEMPL... Interfaces>
using d2::style::UAI = UniversalAccessInterfaceProto<Element, Base, Interface, Interfaces...>
```

Source: `core/tree/d2_styles.hpp:658`

### `UAIE`

```cpp
template<typename Elem, typename Base, D2_UAI_INTERFACE_TEMPL Interface, D2_UAI_INTERFACE_TEMPL... Interfaces>
using d2::style::UAIE = UniversalAccessInterfaceProto<Elem, Base, Interface, Interfaces...>
```

Source: `core/tree/d2_styles.hpp:665`

### `UAIC`

```cpp
template<typename Chain, typename Base, D2_UAI_INTERFACE_TEMPL Interface, D2_UAI_INTERFACE_TEMPL... Interfaces>
using d2::style::UAIC = UniversalAccessInterface<Chain, Base, Interface, Interfaces...>
```

Source: `core/tree/d2_styles.hpp:672`

### `UAIB`

```cpp
using d2::style::UAIB = UniversalAccessInterfaceBase
```

Source: `core/tree/d2_styles.hpp:673`

### `InterfaceHelper`

```cpp
template<template< std::size_t > typename Type, std::size_t PropBase, std::size_t Count>
using d2::style::InterfaceHelper = impl::InterfaceHelper<Type, PropBase, Count>
```

Source: `core/tree/d2_styles.hpp:719`

### `IZResponsive`

```cpp
using d2::style::IZResponsive = IResponsive<d2::Element>::type<0>
```

Source: `core/tree/d2_styles.hpp:1008`

### `uai_property`

```cpp
using d2::style::uai_property = unsigned int
```

Source: `core/tree/d2_styles_base.hpp:24`

### `DependencyHandle`

```cpp
using d2::style::DependencyHandle = DependencyBase::Handle
```

Source: `core/tree/d2_theme.hpp:130`


## Variables

### `D2_UAI_INTERFACE`

```cpp
IVSelectable d2::style::D2_UAI_INTERFACE(Text, D2_UAI_OPTS( enum class Alignment{ Left, Center, Right, }; enum TextOption{ PreserveWordBoundaries=1<< 0, HandleNewlines=1<< 1, Paragraph=PreserveWordBoundaries|HandleNewlines };), D2_UAI_FIELDS( string text{};Alignment alignment{Alignment::Center}; unsigned char text_options{Paragraph};), D2_UAI_PROPS( Value, TextAlignment, TextOptions, TextPreserveWordBoundaries, TextHandleNewlines), D2_UAI_LINK( D2_UAI_PROP(Value, text, Style) D2_UAI_PROP(TextAlignment, alignment, Style) D2_UAI_PROP(TextOptions, text_options, Style) D2_UAI_PROP( TextPreserveWordBoundaries, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&text_options, PreserveWordBoundaries)), Style) D2_UAI_PROP( TextHandleNewlines, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&text_options, HandleNewlines)), Style))) template< typename Type
```

Source: `core/tree/d2_styles.hpp:837`

### `D2_UAI_OPTS`

```cpp
d2::style::D2_UAI_OPTS
```

Source: `core/tree/d2_styles.hpp:853`

### `D2_UAI_FIELDS`

```cpp
IVSelectable d2::style::D2_UAI_FIELDS(px::background focused_color{.r=150,.g=150,.b=150};)
```

Source: `core/tree/d2_styles.hpp:867`

### `D2_UAI_PROPS`

```cpp
IVSelectable d2::style::D2_UAI_PROPS(FocusedColor)
```

Source: `core/tree/d2_styles.hpp:885`

### `D2_UAI_LINK`

```cpp
IVSelectable d2::style::D2_UAI_LINK(D2_UAI_PROP(ContainerBorder, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, EnableBorder)), Masked) D2_UAI_PROP(ContainerDisableLeft, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderLeft)), Masked) D2_UAI_PROP(ContainerDisableRight, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderRight)), Masked) D2_UAI_PROP(ContainerDisableTop, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderTop)), Masked) D2_UAI_PROP(ContainerDisableBottom, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderBottom)), Masked) D2_UAI_PROP(ContainerTopFix, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, TopFix)), Style) D2_UAI_PROP(ContainerOverrideCorners, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, OverrideCorners)), Style))) D2_UAI_INTERFACE(Colors
```

Source: `core/tree/d2_styles.hpp:906`

### `D2_UAI_PROP`

```cpp
IVSelectable d2::style::D2_UAI_PROP(ForegroundColor, foreground_color, Style))) D2_UAI_INTERFACE(KeyboardNav
```

Source: `core/tree/d2_styles.hpp:933`
