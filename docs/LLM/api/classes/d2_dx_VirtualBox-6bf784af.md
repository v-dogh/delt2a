# `d2::dx::VirtualBox`

**Kind:** class
**Declared in:** `elements/d2_draggable_box.hpp:47`

**Bases:** `d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces... >`

## Functions

### `_is_being_resized`

```cpp
bool d2::dx::VirtualBox::_is_being_resized() const
```

Source: `elements/d2_draggable_box.hpp:57`

### `_layout_impl`

```cpp
Unit d2::dx::VirtualBox::_layout_impl(enum Element::Layout type) const override
```

Source: `elements/d2_draggable_box.hpp:59`

### `_provides_input_impl`

```cpp
bool d2::dx::VirtualBox::_provides_input_impl() const override
```

Source: `elements/d2_draggable_box.hpp:60`

### `_get_border_impl`

```cpp
int d2::dx::VirtualBox::_get_border_impl(BorderType type, Element::cptr elem) const override
```

Source: `elements/d2_draggable_box.hpp:61`

### `_state_change_impl`

```cpp
void d2::dx::VirtualBox::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_draggable_box.hpp:62`

### `_event_impl`

```cpp
void d2::dx::VirtualBox::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_draggable_box.hpp:63`

### `_frame_impl`

```cpp
void d2::dx::VirtualBox::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_draggable_box.hpp:64`

### `close`

```cpp
void d2::dx::VirtualBox::close()
```

Source: `elements/d2_draggable_box.hpp:66`


## Typedefs

### `data`

```cpp
using d2::dx::VirtualBox::data = style::UAIC<Box, VirtualBox, style::IDraggableBox, style::IKeyboardNav>
```

Source: `elements/d2_draggable_box.hpp:51`


## Variables

### `offset_`

```cpp
std::pair<int, int> d2::dx::VirtualBox::offset_
```

Source: `elements/d2_draggable_box.hpp:54`

### `minimized_`

```cpp
bool d2::dx::VirtualBox::minimized_
```

Source: `elements/d2_draggable_box.hpp:55`
