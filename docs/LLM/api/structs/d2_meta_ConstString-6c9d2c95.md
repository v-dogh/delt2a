# `d2::meta::ConstString`

**Kind:** struct
**Declared in:** `core/utils/d2_meta.hpp:9`

## Functions

### `ConstString`

```cpp
d2::meta::ConstString< Count >::ConstString(const char(&str)[Count])
```

Source: `core/utils/d2_meta.hpp:11`

### `ConstString`

```cpp
d2::meta::ConstString< Count >::ConstString(std::string_view str)
```

Source: `core/utils/d2_meta.hpp:15`

### `view`

```cpp
auto d2::meta::ConstString< Count >::view() const
```

Source: `core/utils/d2_meta.hpp:22`

### `view`

```cpp
auto d2::meta::ConstString< Count >::view()
```

Source: `core/utils/d2_meta.hpp:26`


## Variables

### `data`

```cpp
std::array<char, Count> d2::meta::ConstString< Count >::data
```

Source: `core/utils/d2_meta.hpp:20`
