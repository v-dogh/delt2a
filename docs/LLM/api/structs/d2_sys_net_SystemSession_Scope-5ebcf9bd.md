# `d2::sys::net::SystemSession::Scope`

**Kind:** struct
**Declared in:** `core/mods/d2_session.hpp:14`

## Functions

### `parse`

```cpp
bool d2::sys::net::SystemSession::Scope::parse(const rule &rule, std::string_view input)
```

Source: `core/mods/d2_session.hpp:24`


## Typedefs

### `rule`

```cpp
using d2::sys::net::SystemSession::Scope::rule = std::variant<std::string, std::regex, std::monostate>
```

Source: `core/mods/d2_session.hpp:16`


## Variables

### `protocol`

```cpp
rule d2::sys::net::SystemSession::Scope::protocol
```

Source: `core/mods/d2_session.hpp:17`

### `method`

```cpp
rule d2::sys::net::SystemSession::Scope::method
```

Source: `core/mods/d2_session.hpp:18`

### `domain`

```cpp
rule d2::sys::net::SystemSession::Scope::domain
```

Source: `core/mods/d2_session.hpp:19`

### `path`

```cpp
rule d2::sys::net::SystemSession::Scope::path
```

Source: `core/mods/d2_session.hpp:20`

### `port`

```cpp
rule d2::sys::net::SystemSession::Scope::port
```

Source: `core/mods/d2_session.hpp:21`

### `short_circuit`

```cpp
bool d2::sys::net::SystemSession::Scope::short_circuit
```

Source: `core/mods/d2_session.hpp:22`
