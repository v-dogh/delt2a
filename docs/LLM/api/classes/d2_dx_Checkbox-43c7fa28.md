# `d2::dx::Checkbox`

**Kind:** class
**Declared in:** `elements/d2_checkbox.hpp:43`

**Bases:** `style:: UAI< Checkbox, style::ILayout, style::IColors, style::ICheckbox, style::IKeyboardNav, style::IResponsive< Checkbox, bool >::type >`, `d2::dx::impl::TextHelper< Checkbox >`

## Friends

### `TextHelper`

```cpp
friend class TextHelper
```

Source: `elements/d2_checkbox.hpp:53`


## Functions

### `_submit`

```cpp
void d2::dx::Checkbox::_submit()
```

Source: `elements/d2_checkbox.hpp:65`

### `_layout_impl`

```cpp
Unit d2::dx::Checkbox::_layout_impl(Element::Layout type) const override
```

Source: `elements/d2_checkbox.hpp:67`

### `_provides_input_impl`

```cpp
bool d2::dx::Checkbox::_provides_input_impl() const override
```

Source: `elements/d2_checkbox.hpp:68`

### `_state_change_impl`

```cpp
void d2::dx::Checkbox::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_checkbox.hpp:69`

### `_event_impl`

```cpp
void d2::dx::Checkbox::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_checkbox.hpp:70`

### `_frame_impl`

```cpp
void d2::dx::Checkbox::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_checkbox.hpp:71`

### `_signal_write_impl`

```cpp
void d2::dx::Checkbox::_signal_write_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_checkbox.hpp:73`


## Typedefs

### `data`

```cpp
using d2::dx::Checkbox::data = style:: UAI<Checkbox, style::ILayout, style::IColors, style::ICheckbox, style::IKeyboardNav, style::IResponsive<Checkbox, bool>::type>
```

Source: `elements/d2_checkbox.hpp:54`


## Variables

### `_text_dimensions`

```cpp
BoundingBox d2::dx::Checkbox::_text_dimensions
```

Source: `elements/d2_checkbox.hpp:63`
