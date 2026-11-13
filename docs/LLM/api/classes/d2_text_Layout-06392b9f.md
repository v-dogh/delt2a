# `d2::text::Layout`

**Kind:** class
**Declared in:** `core/platform/d2_text_layout.hpp:51`

## Functions

### `Layout`

```cpp
d2::text::Layout::Layout()
```

Source: `core/platform/d2_text_layout.hpp:63`

### `Layout`

```cpp
d2::text::Layout::Layout(std::string_view source, Options options={})
```

Source: `core/platform/d2_text_layout.hpp:64`

### `reset`

```cpp
void d2::text::Layout::reset(std::string_view source, Options options={})
```

Source: `core/platform/d2_text_layout.hpp:66`

### `source`

```cpp
std::string_view d2::text::Layout::source() const noexcept
```

Source: `core/platform/d2_text_layout.hpp:67`

### `options`

```cpp
const Options & d2::text::Layout::options() const noexcept
```

Source: `core/platform/d2_text_layout.hpp:68`

### `glyphs`

```cpp
const std::vector< Glyph > & d2::text::Layout::glyphs() const noexcept
```

Source: `core/platform/d2_text_layout.hpp:69`

### `lines`

```cpp
const std::vector< Line > & d2::text::Layout::lines() const noexcept
```

Source: `core/platform/d2_text_layout.hpp:70`

### `bounds`

```cpp
BoundingBox d2::text::Layout::bounds() const noexcept
```

Source: `core/platform/d2_text_layout.hpp:71`

### `size`

```cpp
std::size_t d2::text::Layout::size() const noexcept
```

Source: `core/platform/d2_text_layout.hpp:72`

### `revision`

```cpp
std::size_t d2::text::Layout::revision() const noexcept
```

Source: `core/platform/d2_text_layout.hpp:73`

### `byte_offset`

```cpp
std::size_t d2::text::Layout::byte_offset(std::size_t grapheme) const
```

Source: `core/platform/d2_text_layout.hpp:75`

### `grapheme_at`

```cpp
std::size_t d2::text::Layout::grapheme_at(std::size_t byte, Affinity affinity=Affinity::Upstream) const
```

Source: `core/platform/d2_text_layout.hpp:76`

### `extract`

```cpp
std::string_view d2::text::Layout::extract(Range range) const
```

Source: `core/platform/d2_text_layout.hpp:77`

### `hit_test`

```cpp
std::size_t d2::text::Layout::hit_test(Position position) const
```

Source: `core/platform/d2_text_layout.hpp:78`

### `caret`

```cpp
Position d2::text::Layout::caret(std::size_t grapheme, Affinity affinity=Affinity::Downstream) const
```

Source: `core/platform/d2_text_layout.hpp:79`

### `draw`

```cpp
void d2::text::Layout::draw(PixelBuffer::View buffer, pixel color, Position origin={}, std::optional< Range > selection={}, px::background highlight={}, std::optional< std::size_t > caret_gap={}) const
```

Source: `core/platform/d2_text_layout.hpp:81`
