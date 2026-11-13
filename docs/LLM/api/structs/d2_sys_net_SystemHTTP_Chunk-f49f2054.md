# `d2::sys::net::SystemHTTP::Chunk`

**Kind:** struct
**Declared in:** `core/mods/d2_http.hpp:46`

## Functions

### `read`

```cpp
virtual std::string_view d2::sys::net::SystemHTTP::Chunk::read()=0
```

Source: `core/mods/d2_http.hpp:48`

### `cancel`

```cpp
virtual void d2::sys::net::SystemHTTP::Chunk::cancel()=0
```

Source: `core/mods/d2_http.hpp:49`

### `down_current`

```cpp
virtual std::size_t d2::sys::net::SystemHTTP::Chunk::down_current()=0
```

Source: `core/mods/d2_http.hpp:51`

### `down_total`

```cpp
virtual std::size_t d2::sys::net::SystemHTTP::Chunk::down_total()=0
```

Source: `core/mods/d2_http.hpp:52`

### `~Chunk`

```cpp
virtual d2::sys::net::SystemHTTP::Chunk::~Chunk()=default
```

Source: `core/mods/d2_http.hpp:54`
