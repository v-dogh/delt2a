# `d2::dx::Dropdown`

**Kind:** class
**Declared in:** `elements/d2_dropdown.hpp:47`

**Bases:** `style:: UAI< Dropdown, style::ILayout, style::IColors, style::IContainer, style::IDropdown, style::IKeyboardNav, style::IResponsive< Dropdown, string, std::size_t >::type >`, `d2::dx::impl::TextHelper< Dropdown >`, `d2::dx::impl::ContainerHelper< Dropdown >`

## Friends

### `ContainerHelper< Dropdown >`

```cpp
friend class ContainerHelper< Dropdown >
```

Source: `elements/d2_dropdown.hpp:11`

### `TextHelper< Dropdown >`

```cpp
friend class TextHelper< Dropdown >
```

Source: `elements/d2_dropdown.hpp:11`


## Functions

### `_border_width`

```cpp
int d2::dx::Dropdown::_border_width()
```

Source: `elements/d2_dropdown.hpp:78`

### `_layout_impl`

```cpp
Unit d2::dx::Dropdown::_layout_impl(Element::Layout type) const override
```

Source: `elements/d2_dropdown.hpp:80`

### `_provides_input_impl`

```cpp
bool d2::dx::Dropdown::_provides_input_impl() const override
```

Source: `elements/d2_dropdown.hpp:81`

### `_state_change_impl`

```cpp
void d2::dx::Dropdown::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_dropdown.hpp:82`

### `_event_impl`

```cpp
void d2::dx::Dropdown::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_dropdown.hpp:83`

### `_frame_impl`

```cpp
void d2::dx::Dropdown::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_dropdown.hpp:84`

### `_signal_write_impl`

```cpp
void d2::dx::Dropdown::_signal_write_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_dropdown.hpp:86`

### `choose`

```cpp
void d2::dx::Dropdown::choose(int idx)
```

Source: `elements/d2_dropdown.hpp:88`

### `reset`

```cpp
void d2::dx::Dropdown::reset()
```

Source: `elements/d2_dropdown.hpp:89`

### `reset`

```cpp
void d2::dx::Dropdown::reset(const string &value)
```

Source: `elements/d2_dropdown.hpp:90`

### `index`

```cpp
int d2::dx::Dropdown::index()
```

Source: `elements/d2_dropdown.hpp:91`

### `value`

```cpp
string d2::dx::Dropdown::value()
```

Source: `elements/d2_dropdown.hpp:92`


## Typedefs

### `data`

```cpp
using d2::dx::Dropdown::data = style:: UAI<Dropdown, style::ILayout, style::IColors, style::IContainer, style::IDropdown, style::IKeyboardNav, style::IResponsive<Dropdown, string, std::size_t>::type>
```

Source: `elements/d2_dropdown.hpp:61`

### `opts`

```cpp
using d2::dx::Dropdown::opts = std::vector<string>
```

Source: `elements/d2_dropdown.hpp:70`


## Variables

### `_perfect_dimensions`

```cpp
BoundingBox d2::dx::Dropdown::_perfect_dimensions
```

Source: `elements/d2_dropdown.hpp:72`

### `_cell_height`

```cpp
int d2::dx::Dropdown::_cell_height
```

Source: `elements/d2_dropdown.hpp:73`

### `_soft_selected`

```cpp
int d2::dx::Dropdown::_soft_selected
```

Source: `elements/d2_dropdown.hpp:74`

### `_selected`

```cpp
int d2::dx::Dropdown::_selected
```

Source: `elements/d2_dropdown.hpp:75`

### `_opened`

```cpp
bool d2::dx::Dropdown::_opened
```

Source: `elements/d2_dropdown.hpp:76`
