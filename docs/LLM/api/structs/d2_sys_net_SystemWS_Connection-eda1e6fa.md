# `d2::sys::net::SystemWS::Connection`

**Kind:** struct
**Declared in:** `core/mods/d2_ws.hpp:41`

## Variables

### `url`

```cpp
std::string d2::sys::net::SystemWS::Connection::url
```

Source: `core/mods/d2_ws.hpp:43`

### `ping_interval`

```cpp
std::chrono::seconds d2::sys::net::SystemWS::Connection::ping_interval
```

Source: `core/mods/d2_ws.hpp:45`

### `timeout`

```cpp
std::chrono::seconds d2::sys::net::SystemWS::Connection::timeout
```

Source: `core/mods/d2_ws.hpp:46`

### `reconnect`

```cpp
bool d2::sys::net::SystemWS::Connection::reconnect
```

Source: `core/mods/d2_ws.hpp:48`

### `reconnect_min_delay`

```cpp
std::chrono::milliseconds d2::sys::net::SystemWS::Connection::reconnect_min_delay
```

Source: `core/mods/d2_ws.hpp:49`

### `reconnect_max_delay`

```cpp
std::chrono::milliseconds d2::sys::net::SystemWS::Connection::reconnect_max_delay
```

Source: `core/mods/d2_ws.hpp:50`

### `reconnect_max_attempts`

```cpp
std::size_t d2::sys::net::SystemWS::Connection::reconnect_max_attempts
```

Source: `core/mods/d2_ws.hpp:51`

### `on_setup`

```cpp
std::function<void(RequestBuilder&)> d2::sys::net::SystemWS::Connection::on_setup
```

Source: `core/mods/d2_ws.hpp:53`

### `on_open`

```cpp
std::function<void(Client::ptr)> d2::sys::net::SystemWS::Connection::on_open
```

Source: `core/mods/d2_ws.hpp:54`

### `on_message`

```cpp
std::function<void(std::string_view, Type, Client::ptr)> d2::sys::net::SystemWS::Connection::on_message
```

Source: `core/mods/d2_ws.hpp:55`

### `on_error`

```cpp
std::function<void(std::string)> d2::sys::net::SystemWS::Connection::on_error
```

Source: `core/mods/d2_ws.hpp:56`

### `on_close`

```cpp
std::function<void(std::uint16_t, std::string_view)> d2::sys::net::SystemWS::Connection::on_close
```

Source: `core/mods/d2_ws.hpp:57`
