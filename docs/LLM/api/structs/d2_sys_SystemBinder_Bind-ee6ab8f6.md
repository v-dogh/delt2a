# `d2::sys::SystemBinder::Bind`

**Kind:** struct
**Declared in:** `core/mods/d2_binder.hpp:30`

## Enums

### `DepMode`

```cpp
enum DepMode
```

Source: `core/mods/d2_binder.hpp:32`

Values: `Weak`, `Strong`

### `DepCond`

```cpp
enum DepCond
```

Source: `core/mods/d2_binder.hpp:37`

Values: `Visible`, `Active`, `Focused`, `Hovered`

### `FocusPolicy`

```cpp
enum FocusPolicy
```

Source: `core/mods/d2_binder.hpp:44`

Values: `RespectInput`, `IgnoreInput`, `RequireInput`

### `ConsumePolicy`

```cpp
enum ConsumePolicy
```

Source: `core/mods/d2_binder.hpp:50`

Values: `OnMatch`, `Never`


## Variables

### `sequences`

```cpp
std::vector<key_list> d2::sys::SystemBinder::Bind::sequences
```

Source: `core/mods/d2_binder.hpp:55`

### `whitelist`

```cpp
control_list d2::sys::SystemBinder::Bind::whitelist
```

Source: `core/mods/d2_binder.hpp:56`

### `blacklist`

```cpp
control_list d2::sys::SystemBinder::Bind::blacklist
```

Source: `core/mods/d2_binder.hpp:57`

### `enable_whitelist`

```cpp
bool d2::sys::SystemBinder::Bind::enable_whitelist
```

Source: `core/mods/d2_binder.hpp:58`

### `enable_blacklist`

```cpp
bool d2::sys::SystemBinder::Bind::enable_blacklist
```

Source: `core/mods/d2_binder.hpp:59`

### `description`

```cpp
std::string d2::sys::SystemBinder::Bind::description
```

Source: `core/mods/d2_binder.hpp:60`

### `display_name`

```cpp
std::string d2::sys::SystemBinder::Bind::display_name
```

Source: `core/mods/d2_binder.hpp:61`

### `enabled`

```cpp
bool d2::sys::SystemBinder::Bind::enabled
```

Source: `core/mods/d2_binder.hpp:62`

### `focus_policy`

```cpp
FocusPolicy d2::sys::SystemBinder::Bind::focus_policy
```

Source: `core/mods/d2_binder.hpp:63`

### `consume_policy`

```cpp
ConsumePolicy d2::sys::SystemBinder::Bind::consume_policy
```

Source: `core/mods/d2_binder.hpp:64`

### `dependencies`

```cpp
std::vector<std::tuple<std::weak_ptr<Element>, DepMode, DepCond> > d2::sys::SystemBinder::Bind::dependencies
```

Source: `core/mods/d2_binder.hpp:66`

### `sequential_delay`

```cpp
std::chrono::milliseconds d2::sys::SystemBinder::Bind::sequential_delay
```

Source: `core/mods/d2_binder.hpp:67`

### `hold_threshold`

```cpp
std::chrono::milliseconds d2::sys::SystemBinder::Bind::hold_threshold
```

Source: `core/mods/d2_binder.hpp:68`
