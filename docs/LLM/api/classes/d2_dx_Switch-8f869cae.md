# `d2::dx::Switch`

**Kind:** class
**Declared in:** `elements/d2_switch.hpp:95`

**Bases:** `style:: UAI< Switch, style::ISwitch, style::ILayout, style::IContainer, style::IColors, style::IKeyboardNav >`, `d2::dx::impl::ContainerHelper< Switch >`, `d2::dx::impl::TextHelper< Switch >`

## Friends

### `ContainerHelper< Switch >`

```cpp
friend class ContainerHelper< Switch >
```

Source: `elements/d2_switch.hpp:106`

### `TextHelper< Switch >`

```cpp
friend class TextHelper< Switch >
```

Source: `elements/d2_switch.hpp:106`


## Functions

### `_layout_impl`

```cpp
Unit d2::dx::Switch::_layout_impl(Element::Layout type) const override
```

Source: `elements/d2_switch.hpp:123`

### `_provides_input_impl`

```cpp
bool d2::dx::Switch::_provides_input_impl() const override
```

Source: `elements/d2_switch.hpp:124`

### `_signal_write_impl`

```cpp
void d2::dx::Switch::_signal_write_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_switch.hpp:126`

### `_state_change_impl`

```cpp
void d2::dx::Switch::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_switch.hpp:127`

### `_event_impl`

```cpp
void d2::dx::Switch::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_switch.hpp:128`

### `_frame_impl`

```cpp
void d2::dx::Switch::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_switch.hpp:129`

### `_submit`

```cpp
void d2::dx::Switch::_submit()
```

Source: `elements/d2_switch.hpp:131`

### `reset`

```cpp
void d2::dx::Switch::reset(int idx=-1)
```

Source: `elements/d2_switch.hpp:133`

### `reset`

```cpp
void d2::dx::Switch::reset(string choice)
```

Source: `elements/d2_switch.hpp:134`

### `choice`

```cpp
string d2::dx::Switch::choice()
```

Source: `elements/d2_switch.hpp:135`

### `index`

```cpp
int d2::dx::Switch::index()
```

Source: `elements/d2_switch.hpp:136`


## Typedefs

### `opts`

```cpp
using d2::dx::Switch::opts = std::vector<string>
```

Source: `elements/d2_switch.hpp:109`

### `data`

```cpp
using d2::dx::Switch::data = style:: UAI<Switch, style::ISwitch, style::ILayout, style::IContainer, style::IColors, style::IKeyboardNav>
```

Source: `elements/d2_switch.hpp:110`


## Variables

### `invalid`

```cpp
auto d2::dx::Switch::invalid
```

Source: `elements/d2_switch.hpp:106`

### `_perfect_dimensions`

```cpp
BoundingBox d2::dx::Switch::_perfect_dimensions
```

Source: `elements/d2_switch.hpp:119`

### `_idx`

```cpp
int d2::dx::Switch::_idx
```

Source: `elements/d2_switch.hpp:120`

### `_old`

```cpp
int d2::dx::Switch::_old
```

Source: `elements/d2_switch.hpp:121`
