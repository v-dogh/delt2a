# `d2::sys::SystemInput::FramePtr`

**Kind:** struct
**Declared in:** `core/mods/d2_core.hpp:31`

## Functions

### `FramePtr`

```cpp
d2::sys::SystemInput::FramePtr::FramePtr(in::InputFrame::ptr ptr, FrameLock lock)
```

Source: `core/mods/d2_core.hpp:37`

### `FramePtr`

```cpp
d2::sys::SystemInput::FramePtr::FramePtr(FramePtr &&)=default
```

Source: `core/mods/d2_core.hpp:38`

### `FramePtr`

```cpp
d2::sys::SystemInput::FramePtr::FramePtr(const FramePtr &)=delete
```

Source: `core/mods/d2_core.hpp:39`

### `get`

```cpp
in::InputFrame * d2::sys::SystemInput::FramePtr::get() const
```

Source: `core/mods/d2_core.hpp:41`

### `operator*`

```cpp
in::InputFrame & d2::sys::SystemInput::FramePtr::operator*() const
```

Source: `core/mods/d2_core.hpp:43`

### `operator->`

```cpp
in::InputFrame * d2::sys::SystemInput::FramePtr::operator->() const
```

Source: `core/mods/d2_core.hpp:44`

### `operator=`

```cpp
FramePtr & d2::sys::SystemInput::FramePtr::operator=(FramePtr &&)=default
```

Source: `core/mods/d2_core.hpp:46`

### `operator=`

```cpp
FramePtr & d2::sys::SystemInput::FramePtr::operator=(const FramePtr &)=delete
```

Source: `core/mods/d2_core.hpp:47`
