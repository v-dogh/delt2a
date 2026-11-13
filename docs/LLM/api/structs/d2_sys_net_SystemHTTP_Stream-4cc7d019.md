# `d2::sys::net::SystemHTTP::Stream`

**Kind:** struct
**Declared in:** `core/mods/d2_http.hpp:32`

## Functions

### `write`

```cpp
virtual void d2::sys::net::SystemHTTP::Stream::write(std::string_view data)=0
```

Source: `core/mods/d2_http.hpp:36`

### `finish`

```cpp
virtual void d2::sys::net::SystemHTTP::Stream::finish()=0
```

Source: `core/mods/d2_http.hpp:37`

### `cancel`

```cpp
virtual void d2::sys::net::SystemHTTP::Stream::cancel()=0
```

Source: `core/mods/d2_http.hpp:38`

### `is_open`

```cpp
virtual bool d2::sys::net::SystemHTTP::Stream::is_open()=0
```

Source: `core/mods/d2_http.hpp:39`

### `up_current`

```cpp
virtual std::size_t d2::sys::net::SystemHTTP::Stream::up_current()=0
```

Source: `core/mods/d2_http.hpp:41`

### `up_total`

```cpp
virtual std::size_t d2::sys::net::SystemHTTP::Stream::up_total()=0
```

Source: `core/mods/d2_http.hpp:42`

### `~Stream`

```cpp
virtual d2::sys::net::SystemHTTP::Stream::~Stream()=default
```

Source: `core/mods/d2_http.hpp:44`


## Typedefs

### `ptr`

```cpp
using d2::sys::net::SystemHTTP::Stream::ptr = std::unique_ptr<Stream>
```

Source: `core/mods/d2_http.hpp:34`
