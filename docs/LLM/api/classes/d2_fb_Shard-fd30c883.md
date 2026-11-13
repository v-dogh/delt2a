# `d2::fb::Shard`

**Kind:** class
**Declared in:** `core/platform/d2_fragmented_buffer.hpp:75`

## Functions

### `Shard`

```cpp
d2::fb::Shard::Shard(std::string_view view)
```

Source: `core/platform/d2_fragmented_buffer.hpp:80`

### `Shard`

```cpp
d2::fb::Shard::Shard(std::string str)
```

Source: `core/platform/d2_fragmented_buffer.hpp:81`

### `Shard`

```cpp
d2::fb::Shard::Shard(std::size_t block)
```

Source: `core/platform/d2_fragmented_buffer.hpp:82`

### `Shard`

```cpp
d2::fb::Shard::Shard(const Shard &)=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:83`

### `Shard`

```cpp
d2::fb::Shard::Shard(Shard &&)=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:84`

### `is_view`

```cpp
bool d2::fb::Shard::is_view() const
```

Source: `core/platform/d2_fragmented_buffer.hpp:86`

### `value`

```cpp
std::string_view d2::fb::Shard::value() const
```

Source: `core/platform/d2_fragmented_buffer.hpp:88`

### `buffer`

```cpp
const std::string & d2::fb::Shard::buffer() const
```

Source: `core/platform/d2_fragmented_buffer.hpp:89`

### `buffer`

```cpp
std::string & d2::fb::Shard::buffer()
```

Source: `core/platform/d2_fragmented_buffer.hpp:90`

### `operator==`

```cpp
bool d2::fb::Shard::operator==(std::nullptr_t) const
```

Source: `core/platform/d2_fragmented_buffer.hpp:92`

### `operator=`

```cpp
Shard & d2::fb::Shard::operator=(const Shard &)=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:94`

### `operator=`

```cpp
Shard & d2::fb::Shard::operator=(Shard &&)=default
```

Source: `core/platform/d2_fragmented_buffer.hpp:95`
