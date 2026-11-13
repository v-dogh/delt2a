# `d2::sys::net::SystemHTTP::Request`

**Kind:** struct
**Declared in:** `core/mods/d2_http.hpp:72`

## Variables

### `method`

```cpp
std::string_view d2::sys::net::SystemHTTP::Request::method
```

Source: `core/mods/d2_http.hpp:74`

### `url`

```cpp
std::string d2::sys::net::SystemHTTP::Request::url
```

Source: `core/mods/d2_http.hpp:75`

### `on_setup`

```cpp
std::function<void(RequestBuilder&)> d2::sys::net::SystemHTTP::Request::on_setup
```

Source: `core/mods/d2_http.hpp:76`

### `on_response`

```cpp
std::function<void(Response&)> d2::sys::net::SystemHTTP::Request::on_response
```

Source: `core/mods/d2_http.hpp:77`

### `on_error`

```cpp
std::function<void(const Error&)> d2::sys::net::SystemHTTP::Request::on_error
```

Source: `core/mods/d2_http.hpp:78`

### `on_chunk`

```cpp
std::function<void(Chunk&)> d2::sys::net::SystemHTTP::Request::on_chunk
```

Source: `core/mods/d2_http.hpp:79`

### `on_headers`

```cpp
std::function<void(Response&)> d2::sys::net::SystemHTTP::Request::on_headers
```

Source: `core/mods/d2_http.hpp:80`

### `deadline`

```cpp
std::optional<std::chrono::steady_clock::time_point> d2::sys::net::SystemHTTP::Request::deadline
```

Source: `core/mods/d2_http.hpp:81`

### `timeout`

```cpp
std::optional<std::chrono::milliseconds> d2::sys::net::SystemHTTP::Request::timeout
```

Source: `core/mods/d2_http.hpp:82`

### `connect_timeout`

```cpp
std::optional<std::chrono::milliseconds> d2::sys::net::SystemHTTP::Request::connect_timeout
```

Source: `core/mods/d2_http.hpp:82`

### `max_response_bytes`

```cpp
std::size_t d2::sys::net::SystemHTTP::Request::max_response_bytes
```

Source: `core/mods/d2_http.hpp:83`

### `max_header_bytes`

```cpp
std::size_t d2::sys::net::SystemHTTP::Request::max_header_bytes
```

Source: `core/mods/d2_http.hpp:83`

### `follow_redirects`

```cpp
bool d2::sys::net::SystemHTTP::Request::follow_redirects
```

Source: `core/mods/d2_http.hpp:84`

### `max_redirects`

```cpp
long d2::sys::net::SystemHTTP::Request::max_redirects
```

Source: `core/mods/d2_http.hpp:85`
