# `d2::fb::Locale::Value`

**Kind:** struct
**Declared in:** `core/platform/d2_fragmented_buffer.hpp:20`

## Functions

### `Value`

```cpp
d2::fb::Locale::Value::Value()=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:27`

### `Value`

```cpp
d2::fb::Locale::Value::Value(std::string_view str, std::size_t width, std::size_t height)
```

Source: `core/platform/d2_fragmented_buffer.hpp:28`

### `Value`

```cpp
d2::fb::Locale::Value::Value(const Value &)=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:32`

### `Value`

```cpp
d2::fb::Locale::Value::Value(Value &&)=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:33`

### `value`

```cpp
std::string_view d2::fb::Locale::Value::value() const
```

Source: `core/platform/d2_fragmented_buffer.hpp:35`

### `width`

```cpp
std::size_t d2::fb::Locale::Value::width() const
```

Source: `core/platform/d2_fragmented_buffer.hpp:37`

### `height`

```cpp
std::size_t d2::fb::Locale::Value::height() const
```

Source: `core/platform/d2_fragmented_buffer.hpp:38`

### `operator=`

```cpp
Value & d2::fb::Locale::Value::operator=(const Value &)=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:40`

### `operator=`

```cpp
Value & d2::fb::Locale::Value::operator=(Value &&)=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:41`
