# `d2::sys::net::SystemWS`

**Kind:** class
**Declared in:** `core/mods/d2_ws.hpp:8`

**Bases:** `d2::sys::ModuleDecl< SystemWS, "WebSockets", d2::sys::Access::TSafe >`

## Enums

### `State`

```cpp
enum State
```

Source: `core/mods/d2_ws.hpp:11`

Values: `Created`, `Connecting`, `Open`, `Closing`, `Closed`, `Failed`, `Reconnecting`

### `Type`

```cpp
enum Type
```

Source: `core/mods/d2_ws.hpp:21`

Values: `Text`, `Binary`


## Functions

### `_connect_impl`

```cpp
virtual Client::ptr d2::sys::net::SystemWS::_connect_impl(Connection connection)=0
```

Source: `core/mods/d2_ws.hpp:60`

### `connect`

```cpp
SystemWS::Client::ptr d2::sys::net::SystemWS::connect(Connection connection)
```

Source: `core/mods/d2_ws.hpp:62`
