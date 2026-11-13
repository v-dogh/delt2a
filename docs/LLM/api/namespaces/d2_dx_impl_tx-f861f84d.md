# `d2::dx::impl::tx`

**Kind:** namespace
**Declared in:** `elements/d2_element_utils.cpp:4`

## Functions

### `_text_bounding_box`

```cpp
BoundingBox d2::dx::impl::tx::_text_bounding_box(const string &value)
```

Source: `elements/d2_element_utils.cpp:6`

### `_paragraph_bounding_box`

```cpp
BoundingBox d2::dx::impl::tx::_paragraph_bounding_box(const string &value, int width, int height)
```

Source: `elements/d2_element_utils.cpp:10`

### `_render_text_simple`

```cpp
void d2::dx::impl::tx::_render_text_simple(const string &value, pixel color, style::IZText::Alignment alignment, Position pos, BoundingBox box, PixelBuffer::View buffer)
```

Source: `elements/d2_element_utils.cpp:33`

### `_render_text`

```cpp
void d2::dx::impl::tx::_render_text(const string &value, pixel color, style::IZText::Alignment alignment, Position pos, BoundingBox box, PixelBuffer::View buffer)
```

Source: `elements/d2_element_utils.cpp:38`

### `_render_paragraph`

```cpp
void d2::dx::impl::tx::_render_paragraph(const string &value, pixel color, style::IZText::Alignment alignment, Position pos, BoundingBox box, PixelBuffer::View buffer)
```

Source: `elements/d2_element_utils.cpp:43`
