# `d2::sys::SystemInput`

**Kind:** class
**Declared in:** `core/mods/d2_core.hpp:12`

**Bases:** `d2::sys::ModuleDecl< SystemInput, "Input", Access::TSafe >`

## Functions

### `_lock_frame_impl`

```cpp
virtual void d2::sys::SystemInput::_lock_frame_impl()
```

Source: `core/mods/d2_core.hpp:52`

### `_unlock_frame_impl`

```cpp
virtual void d2::sys::SystemInput::_unlock_frame_impl()
```

Source: `core/mods/d2_core.hpp:53`

### `_frame_impl`

```cpp
virtual in::InputFrame::ptr d2::sys::SystemInput::_frame_impl()=0
```

Source: `core/mods/d2_core.hpp:54`

### `_begincycle_impl`

```cpp
virtual void d2::sys::SystemInput::_begincycle_impl()=0
```

Source: `core/mods/d2_core.hpp:55`

### `_endcycle_impl`

```cpp
virtual void d2::sys::SystemInput::_endcycle_impl()=0
```

Source: `core/mods/d2_core.hpp:56`

### `_worker_impl`

```cpp
virtual MainWorker::ptr d2::sys::SystemInput::_worker_impl()=0
```

Source: `core/mods/d2_core.hpp:57`

### `frame`

```cpp
SystemInput::FramePtr d2::sys::SystemInput::frame()
```

Source: `core/mods/d2_core.hpp:61`

### `worker`

```cpp
MainWorker::ptr d2::sys::SystemInput::worker()
```

Source: `core/mods/d2_core.hpp:62`

### `begincycle`

```cpp
void d2::sys::SystemInput::begincycle()
```

Source: `core/mods/d2_core.hpp:64`

### `endcycle`

```cpp
void d2::sys::SystemInput::endcycle()
```

Source: `core/mods/d2_core.hpp:65`

### `enable`

```cpp
void d2::sys::SystemInput::enable()
```

Source: `core/mods/d2_core.hpp:67`

### `disable`

```cpp
void d2::sys::SystemInput::disable()
```

Source: `core/mods/d2_core.hpp:68`

### `had_event`

```cpp
bool d2::sys::SystemInput::had_event(in::Event ev)
```

Source: `core/mods/d2_core.hpp:70`

### `had_pulse`

```cpp
bool d2::sys::SystemInput::had_pulse()
```

Source: `core/mods/d2_core.hpp:71`

### `mouse_position`

```cpp
Position d2::sys::SystemInput::mouse_position()
```

Source: `core/mods/d2_core.hpp:73`

### `screen_capacity`

```cpp
BoundingBox d2::sys::SystemInput::screen_capacity()
```

Source: `core/mods/d2_core.hpp:74`

### `screen_size`

```cpp
BoundingBox d2::sys::SystemInput::screen_size()
```

Source: `core/mods/d2_core.hpp:75`

### `active`

```cpp
bool d2::sys::SystemInput::active(in::Mouse mouse, in::Mode mode)
```

Source: `core/mods/d2_core.hpp:77`

### `active`

```cpp
bool d2::sys::SystemInput::active(in::Special mod, in::Mode mode)
```

Source: `core/mods/d2_core.hpp:78`

### `active`

```cpp
bool d2::sys::SystemInput::active(in::keytype key, in::Mode mode)
```

Source: `core/mods/d2_core.hpp:79`

### `sequence`

```cpp
std::string_view d2::sys::SystemInput::sequence()
```

Source: `core/mods/d2_core.hpp:81`
