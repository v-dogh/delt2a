# `d2::dx::Text`

**Kind:** class
**Declared in:** `elements/d2_text.hpp:9`

**Bases:** `d2::style::UniversalAccessInterfaceProto< Element, Base, Interface, Interfaces... >`, `d2::dx::impl::TextHelper< Text >`

## Friends

### `UnitUpdateHelper`

```cpp
friend class UnitUpdateHelper
```

Source: `elements/d2_text.hpp:13`

### `TextHelper`

```cpp
friend class TextHelper
```

Source: `elements/d2_text.hpp:14`


## Functions

### `_options`

```cpp
text::Options d2::dx::Text::_options(int width) const
```

Source: `elements/d2_text.hpp:19`

### `_layout_impl`

```cpp
Unit d2::dx::Text::_layout_impl(Element::Layout type) const override
```

Source: `elements/d2_text.hpp:21`

### `_frame_impl`

```cpp
void d2::dx::Text::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_text.hpp:22`

### `_signal_write_impl`

```cpp
void d2::dx::Text::_signal_write_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_text.hpp:23`

### `Text`

```cpp
d2::dx::Text::Text(const std::string &name, TreeState::ptr state)
```

Source: `elements/d2_text.hpp:25`

### `text_layout`

```cpp
const text::Layout * d2::dx::Text::text_layout() const override
```

Source: `elements/d2_text.hpp:26`

### `select_text`

```cpp
void d2::dx::Text::select_text(std::optional< text::Range > range) override
```

Source: `elements/d2_text.hpp:27`


## Typedefs

### `data`

```cpp
using d2::dx::Text::data = style::UAI<Text, style::ILayout, style::IText, style::IColors, style::ISelectable>
```

Source: `elements/d2_text.hpp:15`


## Variables

### `_text_layout`

```cpp
text::Layout d2::dx::Text::_text_layout
```

Source: `elements/d2_text.hpp:17`

### `_measurement`

```cpp
text::Layout d2::dx::Text::_measurement
```

Source: `elements/d2_text.hpp:18`
