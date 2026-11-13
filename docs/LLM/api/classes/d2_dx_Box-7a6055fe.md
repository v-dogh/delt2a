# `d2::dx::Box`

**Kind:** class
**Declared in:** `elements/d2_box.hpp:10`

**Bases:** `style:: UAIE< VecParentElement, Box, style::ILayout, style::IContainer, style::IColors >`, `d2::dx::impl::ContainerHelper< Box >`

## Friends

### `ContainerHelper< Box >`

```cpp
friend class ContainerHelper< Box >
```

Source: `elements/d2_box.hpp:1`


## Functions

### `_perfect_width`

```cpp
int d2::dx::Box::_perfect_width() const
```

Source: `elements/d2_box.hpp:20`

### `_perfect_height`

```cpp
int d2::dx::Box::_perfect_height() const
```

Source: `elements/d2_box.hpp:21`

### `_signal_write_child_impl`

```cpp
void d2::dx::Box::_signal_write_child_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_box.hpp:24`

### `_get_border_impl`

```cpp
int d2::dx::Box::_get_border_impl(BorderType type, cptr elem) const override
```

Source: `elements/d2_box.hpp:25`

### `_layout_impl`

```cpp
Unit d2::dx::Box::_layout_impl(Element::Layout type) const override
```

Source: `elements/d2_box.hpp:26`

### `_frame_impl`

```cpp
void d2::dx::Box::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_box.hpp:28`

### `_render_start`

```cpp
virtual void d2::dx::Box::_render_start()
```

Source: `elements/d2_box.hpp:30`

### `_object_render`

```cpp
void d2::dx::Box::_object_render(PixelBuffer::View buffer, ptr obj)
```

Source: `elements/d2_box.hpp:31`


## Typedefs

### `data`

```cpp
using d2::dx::Box::data = style::UAIE<VecParentElement, Box, style::ILayout, style::IContainer, style::IColors>
```

Source: `elements/d2_box.hpp:16`
