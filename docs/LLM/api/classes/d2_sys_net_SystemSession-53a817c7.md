# `d2::sys::net::SystemSession`

**Kind:** class
**Declared in:** `core/mods/d2_session.hpp:10`

**Bases:** `d2::sys::Module< SystemSession, "Session", Access::TSafe, Load::Lazy >`

## Functions

### `set_scope`

```cpp
void d2::sys::net::SystemSession::set_scope(std::string_view name, Scope scope, absl::flat_hash_map< std::string, std::string > headers={})
```

Source: `core/mods/d2_session.hpp:38`

### `set_header`

```cpp
void d2::sys::net::SystemSession::set_header(std::string_view name, std::string_view header, std::string value)
```

Source: `core/mods/d2_session.hpp:43`

### `get_header`

```cpp
std::optional< std::string > d2::sys::net::SystemSession::get_header(std::string_view name, std::string_view header)
```

Source: `core/mods/d2_session.hpp:44`

### `reset`

```cpp
void d2::sys::net::SystemSession::reset(std::string_view name)
```

Source: `core/mods/d2_session.hpp:46`

### `reset`

```cpp
void d2::sys::net::SystemSession::reset()
```

Source: `core/mods/d2_session.hpp:47`

### `process`

```cpp
void d2::sys::net::SystemSession::process(std::string_view url, std::string_view method, std::function< void(std::string_view, std::string_view)> callback) const
```

Source: `core/mods/d2_session.hpp:49`
