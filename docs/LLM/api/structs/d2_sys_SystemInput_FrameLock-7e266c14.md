# `d2::sys::SystemInput::FrameLock`

**Kind:** struct
**Declared in:** `core/mods/d2_core.hpp:15`

## Functions

### `FrameLock`

```cpp
d2::sys::SystemInput::FrameLock::FrameLock(FrameLock &&copy)
```

Source: `core/mods/d2_core.hpp:22`

### `FrameLock`

```cpp
d2::sys::SystemInput::FrameLock::FrameLock(const FrameLock &)=delete
```

Source: `core/mods/d2_core.hpp:23`

### `FrameLock`

```cpp
d2::sys::SystemInput::FrameLock::FrameLock(SystemInput *ptr, bool consume_mask=false)
```

Source: `core/mods/d2_core.hpp:24`

### `~FrameLock`

```cpp
d2::sys::SystemInput::FrameLock::~FrameLock()
```

Source: `core/mods/d2_core.hpp:25`

### `operator=`

```cpp
SystemInput::FrameLock & d2::sys::SystemInput::FrameLock::operator=(FrameLock &&copy)
```

Source: `core/mods/d2_core.hpp:27`

### `operator=`

```cpp
FrameLock & d2::sys::SystemInput::FrameLock::operator=(const FrameLock &)=delete
```

Source: `core/mods/d2_core.hpp:28`
