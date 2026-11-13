# `d2::sys::net::SystemHTTP::Response`

**Kind:** struct
**Declared in:** `core/mods/d2_http.hpp:25`

## Functions

### `status`

```cpp
virtual int d2::sys::net::SystemHTTP::Response::status()=0
```

Source: `core/mods/d2_http.hpp:27`

### `get`

```cpp
virtual std::optional< std::string_view > d2::sys::net::SystemHTTP::Response::get(std::string_view header)=0
```

Source: `core/mods/d2_http.hpp:28`

### `body`

```cpp
virtual std::string_view d2::sys::net::SystemHTTP::Response::body()=0
```

Source: `core/mods/d2_http.hpp:29`

### `~Response`

```cpp
virtual d2::sys::net::SystemHTTP::Response::~Response()=default
```

Source: `core/mods/d2_http.hpp:30`
