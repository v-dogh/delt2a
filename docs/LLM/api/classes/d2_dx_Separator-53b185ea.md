# `d2::dx::Separator`

**Kind:** class
**Declared in:** `elements/d2_separator.hpp:34`

**Bases:** `d2::style::UniversalAccessInterfaceProto< Element, Base, Interface, Interfaces... >`

## Enums

### `Align`

```cpp
enum Align
```

Source: `elements/d2_separator.hpp:46`

Values: `Vertical`, `Horizontal`


## Friends

### `TextHelper`

```cpp
friend class TextHelper
```

Source: `elements/d2_separator.hpp:43`


## Functions

### `_frame_impl`

```cpp
void d2::dx::Separator::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_separator.hpp:52`

### `Separator`

```cpp
d2::dx::Separator::Separator(const std::string &name, TreeState::ptr state)
```

Source: `elements/d2_separator.hpp:54`


## Typedefs

### `data`

```cpp
using d2::dx::Separator::data = style::UAI<Separator, style::ILayout, style::IColors, style::ISeparator>
```

Source: `elements/d2_separator.hpp:44`


## Variables

### `_orientation`

```cpp
enum d2::dx::Separator::Align d2::dx::Separator::_orientation
```

Source: `elements/d2_separator.hpp:50`
