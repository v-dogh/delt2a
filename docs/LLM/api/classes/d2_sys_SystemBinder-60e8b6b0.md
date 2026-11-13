# `d2::sys::SystemBinder`

**Kind:** class
**Declared in:** `core/mods/d2_binder.hpp:22`

**Bases:** `d2::sys::Module< SystemBinder, "Binder", Access::TUnsafe, Load::Lazy, sys::SystemScreen >`

## Enums

### `ConflictType`

```cpp
enum ConflictType
```

Source: `core/mods/d2_binder.hpp:70`

Values: `Whitelist`, `Blacklist`, `Collision`, `Failure`


## Functions

### `~SystemBinder`

```cpp
d2::sys::SystemBinder::~SystemBinder() override
```

Source: `core/mods/d2_binder.hpp:110`

### `bind`

```cpp
std::optional< SystemBinder::BindError > d2::sys::SystemBinder::bind(const std::string &name, Bind value, callback_type callback)
```

Source: `core/mods/d2_binder.hpp:112`

### `rebind`

```cpp
std::optional< SystemBinder::BindError > d2::sys::SystemBinder::rebind(const std::string &name, Bind value)
```

Source: `core/mods/d2_binder.hpp:113`

### `check`

```cpp
const SystemBinder::Bind & d2::sys::SystemBinder::check(const std::string &name) const noexcept
```

Source: `core/mods/d2_binder.hpp:114`

### `unbind`

```cpp
void d2::sys::SystemBinder::unbind(const std::string &name)
```

Source: `core/mods/d2_binder.hpp:115`

### `freeze`

```cpp
void d2::sys::SystemBinder::freeze(const std::string &name)
```

Source: `core/mods/d2_binder.hpp:116`

### `unfreeze`

```cpp
void d2::sys::SystemBinder::unfreeze(const std::string &name)
```

Source: `core/mods/d2_binder.hpp:117`

### `list`

```cpp
std::vector< SystemBinder::Bind > d2::sys::SystemBinder::list() const
```

Source: `core/mods/d2_binder.hpp:118`


## Typedefs

### `key_list`

```cpp
using d2::sys::SystemBinder::key_list = std::vector<std::pair<in::keytype, in::mode>>
```

Source: `core/mods/d2_binder.hpp:27`

### `control_list`

```cpp
using d2::sys::SystemBinder::control_list = absl::flat_hash_set<std::pair<in::keytype, std::optional<in::mode>>>
```

Source: `core/mods/d2_binder.hpp:28`

### `callback_type`

```cpp
using d2::sys::SystemBinder::callback_type = std::function<void()>
```

Source: `core/mods/d2_binder.hpp:29`
