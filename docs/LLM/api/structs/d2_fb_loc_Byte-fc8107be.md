# `d2::fb::loc::Byte`

**Kind:** struct
**Declared in:** `core/platform/d2_fragmented_buffer.hpp:57`

**Bases:** `d2::fb::Locale`

## Functions

### `source`

```cpp
void d2::fb::loc::Byte::source(std::string_view str) override
```

Source: `core/platform/d2_fragmented_buffer.hpp:63`

### `jump`

```cpp
void d2::fb::loc::Byte::jump(std::size_t off) override
```

Source: `core/platform/d2_fragmented_buffer.hpp:64`

### `seek_last`

```cpp
void d2::fb::loc::Byte::seek_last() override
```

Source: `core/platform/d2_fragmented_buffer.hpp:65`

### `seek`

```cpp
void d2::fb::loc::Byte::seek(int pos) override
```

Source: `core/platform/d2_fragmented_buffer.hpp:66`

### `value`

```cpp
Locale::Value d2::fb::loc::Byte::value() override
```

Source: `core/platform/d2_fragmented_buffer.hpp:67`

### `offset`

```cpp
std::size_t d2::fb::loc::Byte::offset() const override
```

Source: `core/platform/d2_fragmented_buffer.hpp:68`

### `is_end`

```cpp
bool d2::fb::loc::Byte::is_end() const override
```

Source: `core/platform/d2_fragmented_buffer.hpp:69`

### `is_endline`

```cpp
bool d2::fb::loc::Byte::is_endline() const override
```

Source: `core/platform/d2_fragmented_buffer.hpp:70`

### `iterate`

```cpp
void d2::fb::loc::Byte::iterate(const std::function< bool(Value)> &callback) const override
```

Source: `core/platform/d2_fragmented_buffer.hpp:71`
