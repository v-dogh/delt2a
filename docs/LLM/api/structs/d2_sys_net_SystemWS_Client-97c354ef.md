# `d2::sys::net::SystemWS::Client`

**Kind:** struct
**Declared in:** `core/mods/d2_ws.hpp:27`

## Functions

### `send`

```cpp
virtual void d2::sys::net::SystemWS::Client::send(std::string_view msg, Type type=Type::Text)=0
```

Source: `core/mods/d2_ws.hpp:31`

### `close`

```cpp
virtual void d2::sys::net::SystemWS::Client::close(std::uint16_t code=1000, std::string_view reason="")=0
```

Source: `core/mods/d2_ws.hpp:32`

### `state`

```cpp
virtual State d2::sys::net::SystemWS::Client::state()=0
```

Source: `core/mods/d2_ws.hpp:33`

### `~Client`

```cpp
virtual d2::sys::net::SystemWS::Client::~Client()=default
```

Source: `core/mods/d2_ws.hpp:35`


## Typedefs

### `ptr`

```cpp
using d2::sys::net::SystemWS::Client::ptr = std::shared_ptr<Client>
```

Source: `core/mods/d2_ws.hpp:29`
