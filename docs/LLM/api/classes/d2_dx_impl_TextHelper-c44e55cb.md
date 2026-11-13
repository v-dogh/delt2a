# `d2::dx::impl::TextHelper`

**Kind:** class
**Declared in:** `elements/d2_element_utils.hpp:44`

## Functions

### `_text_bounding_box`

```cpp
BoundingBox d2::dx::impl::TextHelper< Base >::_text_bounding_box(const string &value) const
```

Source: `elements/d2_element_utils.hpp:47`

### `_paragraph_bounding_box`

```cpp
BoundingBox d2::dx::impl::TextHelper< Base >::_paragraph_bounding_box(const string &value, int width=INT_MAX, int height=INT_MAX) const
```

Source: `elements/d2_element_utils.hpp:51`

### `_render_text_simple`

```cpp
void d2::dx::impl::TextHelper< Base >::_render_text_simple(const string &value, pixel color, style::IZText::Alignment alignment, Position pos, BoundingBox box, PixelBuffer::View buffer) const
```

Source: `elements/d2_element_utils.hpp:58`

### `_render_text_simple`

```cpp
void d2::dx::impl::TextHelper< Base >::_render_text_simple(const string &value, pixel color, Position pos, BoundingBox box, PixelBuffer::View buffer) const
```

Source: `elements/d2_element_utils.hpp:69`

### `_render_text`

```cpp
void d2::dx::impl::TextHelper< Base >::_render_text(const string &value, pixel color, style::IZText::Alignment alignment, Position pos, BoundingBox box, PixelBuffer::View buffer) const
```

Source: `elements/d2_element_utils.hpp:81`

### `_render_text`

```cpp
void d2::dx::impl::TextHelper< Base >::_render_text(const string &value, pixel color, Position pos, BoundingBox box, PixelBuffer::View buffer) const
```

Source: `elements/d2_element_utils.hpp:92`

### `_render_paragraph`

```cpp
void d2::dx::impl::TextHelper< Base >::_render_paragraph(const string &value, pixel color, style::IZText::Alignment alignment, Position pos, BoundingBox box, PixelBuffer::View buffer) const
```

Source: `elements/d2_element_utils.hpp:102`

### `_render_paragraph`

```cpp
void d2::dx::impl::TextHelper< Base >::_render_paragraph(const string &value, pixel color, Position pos, BoundingBox box, PixelBuffer::View buffer)
```

Source: `elements/d2_element_utils.hpp:113`
