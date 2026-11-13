# `d2::text`

**Kind:** namespace
**Declared in:** `core/platform/d2_text_layout.cpp:10`

## Enums

### `Alignment`

```cpp
enum Alignment
```

Source: `core/platform/d2_text_layout.hpp:11`

Values: `Left`, `Center`, `Right`

### `Wrap`

```cpp
enum Wrap
```

Source: `core/platform/d2_text_layout.hpp:12`

Values: `None`, `Grapheme`, `Word`

### `Affinity`

```cpp
enum Affinity
```

Source: `core/platform/d2_text_layout.hpp:13`

Values: `Upstream`, `Downstream`


## Functions

### `columns`

```cpp
int d2::text::columns(std::string_view grapheme)
```

Source: `core/platform/d2_text_layout.cpp:30`

### `continuation`

```cpp
bool d2::text::continuation(const pixel &cell) noexcept
```

Source: `core/platform/d2_text_layout.cpp:264`

### `display_cell`

```cpp
pixel d2::text::display_cell(std::span< const pixel > cells, std::size_t index, std::size_t width)
```

Source: `core/platform/d2_text_layout.cpp:268`
