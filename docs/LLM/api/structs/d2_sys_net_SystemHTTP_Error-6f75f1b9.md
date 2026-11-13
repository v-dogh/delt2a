# `d2::sys::net::SystemHTTP::Error`

**Kind:** struct
**Declared in:** `core/mods/d2_http.hpp:64`

## Enums

### `Code`

```cpp
enum Code
```

Source: `core/mods/d2_http.hpp:66`

Values: `InvalidRequest`, `Unavailable`, `Cancelled`, `Timeout`, `Transport`, `Callback`, `Limit`


## Variables

### `code`

```cpp
Code d2::sys::net::SystemHTTP::Error::code
```

Source: `core/mods/d2_http.hpp:67`

### `message`

```cpp
std::string d2::sys::net::SystemHTTP::Error::message
```

Source: `core/mods/d2_http.hpp:68`

### `cause`

```cpp
std::exception_ptr d2::sys::net::SystemHTTP::Error::cause
```

Source: `core/mods/d2_http.hpp:69`

### `native_code`

```cpp
std::optional<int> d2::sys::net::SystemHTTP::Error::native_code
```

Source: `core/mods/d2_http.hpp:70`
