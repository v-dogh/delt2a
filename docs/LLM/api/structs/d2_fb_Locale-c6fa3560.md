# `d2::fb::Locale`

**Kind:** struct
**Declared in:** `core/platform/d2_fragmented_buffer.hpp:18`

## Functions

### `~Locale`

```cpp
virtual d2::fb::Locale::~Locale()=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:44`

### `source`

```cpp
virtual void d2::fb::Locale::source(std::string_view str)=0
```

Source: `core/platform/d2_fragmented_buffer.hpp:45`

### `jump`

```cpp
virtual void d2::fb::Locale::jump(std::size_t off)=0
```

Source: `core/platform/d2_fragmented_buffer.hpp:46`

### `seek_last`

```cpp
virtual void d2::fb::Locale::seek_last()=0
```

Source: `core/platform/d2_fragmented_buffer.hpp:47`

### `seek`

```cpp
virtual void d2::fb::Locale::seek(int pos)=0
```

Source: `core/platform/d2_fragmented_buffer.hpp:48`

### `value`

```cpp
virtual Value d2::fb::Locale::value()=0
```

Source: `core/platform/d2_fragmented_buffer.hpp:49`

### `offset`

```cpp
virtual std::size_t d2::fb::Locale::offset() const =0
```

Source: `core/platform/d2_fragmented_buffer.hpp:50`

### `is_end`

```cpp
virtual bool d2::fb::Locale::is_end() const =0
```

Source: `core/platform/d2_fragmented_buffer.hpp:51`

### `is_endline`

```cpp
virtual bool d2::fb::Locale::is_endline() const =0
```

Source: `core/platform/d2_fragmented_buffer.hpp:52`

### `iterate`

```cpp
virtual void d2::fb::Locale::iterate(const std::function< bool(Value)> &callback) const =0
```

Source: `core/platform/d2_fragmented_buffer.hpp:53`
