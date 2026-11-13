# `d2::dx::DemandScrollBox`

**Kind:** class
**Declared in:** `elements/d2_demand_scrollbox.hpp:91`

**Bases:** `d2::style::UniversalAccessInterfaceProto< Elem, Base, Interface, Interfaces... >`, `d2::dx::impl::ContainerHelper< DemandScrollBox >`

## Friends

### `EntrySeed`

```cpp
friend class EntrySeed
```

Source: `elements/d2_demand_scrollbox.hpp:103`

### `ContainerHelper< DemandScrollBox >`

```cpp
friend class ContainerHelper< DemandScrollBox >
```

Source: `elements/d2_demand_scrollbox.hpp:103`


## Functions

### `_update_view`

```cpp
void d2::dx::DemandScrollBox::_update_view()
```

Source: `elements/d2_demand_scrollbox.hpp:130`

### `_signal_write_impl`

```cpp
void d2::dx::DemandScrollBox::_signal_write_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_demand_scrollbox.hpp:132`

### `_provides_input_impl`

```cpp
bool d2::dx::DemandScrollBox::_provides_input_impl() const override
```

Source: `elements/d2_demand_scrollbox.hpp:133`

### `_state_change_impl`

```cpp
void d2::dx::DemandScrollBox::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_demand_scrollbox.hpp:134`

### `_event_impl`

```cpp
void d2::dx::DemandScrollBox::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_demand_scrollbox.hpp:135`

### `_get_border_impl`

```cpp
int d2::dx::DemandScrollBox::_get_border_impl(BorderType type, cptr elem) const override
```

Source: `elements/d2_demand_scrollbox.hpp:137`

### `_layout_impl`

```cpp
Unit d2::dx::DemandScrollBox::_layout_impl(enum Element::Layout type) const override
```

Source: `elements/d2_demand_scrollbox.hpp:138`

### `_layout_for_impl`

```cpp
void d2::dx::DemandScrollBox::_layout_for_impl(enum Element::Layout type, cptr ptr) const override
```

Source: `elements/d2_demand_scrollbox.hpp:139`

### `_frame_impl`

```cpp
void d2::dx::DemandScrollBox::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_demand_scrollbox.hpp:141`

### `scrollbar`

```cpp
TreeIter< VerticalSlider > d2::dx::DemandScrollBox::scrollbar() const
```

Source: `elements/d2_demand_scrollbox.hpp:143`

### `scroll_to`

```cpp
void d2::dx::DemandScrollBox::scroll_to(int offset)
```

Source: `elements/d2_demand_scrollbox.hpp:144`

### `reset`

```cpp
void d2::dx::DemandScrollBox::reset()
```

Source: `elements/d2_demand_scrollbox.hpp:145`

### `foreach`

```cpp
void d2::dx::DemandScrollBox::foreach(foreach_callback callback) const noexcept override
```

Source: `elements/d2_demand_scrollbox.hpp:147`

### `foreach_internal`

```cpp
void d2::dx::DemandScrollBox::foreach_internal(foreach_internal_callback callback) const override
```

Source: `elements/d2_demand_scrollbox.hpp:148`


## Typedefs

### `seed`

```cpp
using d2::dx::DemandScrollBox::seed = EntrySeed
```

Source: `elements/d2_demand_scrollbox.hpp:105`

### `data`

```cpp
using d2::dx::DemandScrollBox::data = style::UAIE< MetaParentElement, DemandScrollBox, style::ILayout, style::IContainer, style::IColors, style::IDemandScrollBox >
```

Source: `elements/d2_demand_scrollbox.hpp:106`


## Variables

### `_top_index`

```cpp
std::size_t d2::dx::DemandScrollBox::_top_index
```

Source: `elements/d2_demand_scrollbox.hpp:116`

### `_top_inset`

```cpp
int d2::dx::DemandScrollBox::_top_inset
```

Source: `elements/d2_demand_scrollbox.hpp:117`

### `_view_base`

```cpp
int d2::dx::DemandScrollBox::_view_base
```

Source: `elements/d2_demand_scrollbox.hpp:118`

### `_zero_start`

```cpp
int d2::dx::DemandScrollBox::_zero_start
```

Source: `elements/d2_demand_scrollbox.hpp:119`

### `_height_cache`

```cpp
std::vector<int> d2::dx::DemandScrollBox::_height_cache
```

Source: `elements/d2_demand_scrollbox.hpp:121`

### `_avg_height_cnt`

```cpp
std::size_t d2::dx::DemandScrollBox::_avg_height_cnt
```

Source: `elements/d2_demand_scrollbox.hpp:122`

### `_avg_height`

```cpp
double d2::dx::DemandScrollBox::_avg_height
```

Source: `elements/d2_demand_scrollbox.hpp:123`

### `_scrollbar`

```cpp
std::shared_ptr<VerticalSlider> d2::dx::DemandScrollBox::_scrollbar
```

Source: `elements/d2_demand_scrollbox.hpp:125`

### `_view`

```cpp
std::vector<d2::Element::ptr> d2::dx::DemandScrollBox::_view
```

Source: `elements/d2_demand_scrollbox.hpp:126`

### `_prev_offset`

```cpp
int d2::dx::DemandScrollBox::_prev_offset
```

Source: `elements/d2_demand_scrollbox.hpp:127`

### `_offset`

```cpp
int d2::dx::DemandScrollBox::_offset
```

Source: `elements/d2_demand_scrollbox.hpp:128`
