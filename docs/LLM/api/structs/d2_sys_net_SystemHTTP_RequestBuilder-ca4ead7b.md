# `d2::sys::net::SystemHTTP::RequestBuilder`

**Kind:** struct
**Declared in:** `core/mods/d2_http.hpp:56`

## Functions

### `set`

```cpp
virtual void d2::sys::net::SystemHTTP::RequestBuilder::set(std::string_view header, std::string_view value)=0
```

Source: `core/mods/d2_http.hpp:58`

### `body`

```cpp
virtual void d2::sys::net::SystemHTTP::RequestBuilder::body(std::string_view value)=0
```

Source: `core/mods/d2_http.hpp:59`

### `stream`

```cpp
virtual void d2::sys::net::SystemHTTP::RequestBuilder::stream()=0
```

Source: `core/mods/d2_http.hpp:60`

### `~RequestBuilder`

```cpp
virtual d2::sys::net::SystemHTTP::RequestBuilder::~RequestBuilder()=default
```

Source: `core/mods/d2_http.hpp:62`
