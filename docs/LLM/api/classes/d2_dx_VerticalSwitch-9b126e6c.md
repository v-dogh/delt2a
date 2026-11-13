# `d2::dx::VerticalSwitch`

**Kind:** class
**Declared in:** `elements/d2_switch.hpp:139`

**Bases:** `d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces... >`

## Functions

### `_signal_write_impl`

```cpp
void d2::dx::VerticalSwitch::_signal_write_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_switch.hpp:150`

### `_state_change_impl`

```cpp
void d2::dx::VerticalSwitch::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_switch.hpp:151`

### `_event_impl`

```cpp
void d2::dx::VerticalSwitch::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_switch.hpp:152`

### `_frame_impl`

```cpp
void d2::dx::VerticalSwitch::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_switch.hpp:153`

### `VerticalSwitch`

```cpp
d2::dx::VerticalSwitch::VerticalSwitch(const std::string &name, TreeState::ptr state)
```

Source: `elements/d2_switch.hpp:155`


## Typedefs

### `data`

```cpp
using d2::dx::VerticalSwitch::data = style::UAIC<Switch, VerticalSwitch, style::IVerticalSwitch>
```

Source: `elements/d2_switch.hpp:142`
