# `d2::dx::Button`

**Kind:** class
**Declared in:** `elements/d2_button.hpp:9`

**Bases:** `style:: UAI< Button, style::ILayout, style::IContainer, style::IColors, style::IResponsive< Button >::type, style::IText, style::IKeyboardNav >`, `d2::dx::impl::TextHelper< Button >`, `d2::dx::impl::ContainerHelper< Button >`

## Friends

### `ContainerHelper< Button >`

```cpp
friend class ContainerHelper< Button >
```

Source: `elements/d2_button.hpp:1`

### `TextHelper< Button >`

```cpp
friend class TextHelper< Button >
```

Source: `elements/d2_button.hpp:1`


## Functions

### `_provides_input_impl`

```cpp
bool d2::dx::Button::_provides_input_impl() const override
```

Source: `elements/d2_button.hpp:35`

### `_layout_impl`

```cpp
Unit d2::dx::Button::_layout_impl(enum Element::Layout type) const override
```

Source: `elements/d2_button.hpp:36`

### `_signal_write_impl`

```cpp
void d2::dx::Button::_signal_write_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_button.hpp:38`

### `_state_change_impl`

```cpp
void d2::dx::Button::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_button.hpp:39`

### `_event_impl`

```cpp
void d2::dx::Button::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_button.hpp:40`

### `_frame_impl`

```cpp
void d2::dx::Button::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_button.hpp:42`


## Typedefs

### `data`

```cpp
using d2::dx::Button::data = style:: UAI<Button, style::ILayout, style::IContainer, style::IColors, style::IResponsive<Button>::type, style::IText, style::IKeyboardNav>
```

Source: `elements/d2_button.hpp:23`


## Variables

### `_text_dimensions`

```cpp
BoundingBox d2::dx::Button::_text_dimensions
```

Source: `elements/d2_button.hpp:33`
