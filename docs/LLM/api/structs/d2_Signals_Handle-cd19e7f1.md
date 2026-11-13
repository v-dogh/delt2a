# `d2::Signals::Handle`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:785`

## Functions

### `Handle`

```cpp
d2::Signals::Handle::Handle()=default
```

Source: `core/io/d2_signal_handler.hpp:793`

### `Handle`

```cpp
d2::Signals::Handle::Handle(const Handle &)=delete
```

Source: `core/io/d2_signal_handler.hpp:794`

### `Handle`

```cpp
d2::Signals::Handle::Handle(Handle &&other) noexcept
```

Source: `core/io/d2_signal_handler.hpp:795`

### `Handle`

```cpp
d2::Signals::Handle::Handle(std::shared_ptr< HandleState > ptr, std::shared_ptr< SignalStorage > storage)
```

Source: `core/io/d2_signal_handler.hpp:796`

### `Handle`

```cpp
d2::Signals::Handle::Handle(std::nullptr_t)
```

Source: `core/io/d2_signal_handler.hpp:797`

### `~Handle`

```cpp
d2::Signals::Handle::~Handle()
```

Source: `core/io/d2_signal_handler.hpp:798`

### `mute`

```cpp
void d2::Signals::Handle::mute()
```

Source: `core/io/d2_signal_handler.hpp:800`

### `unmute`

```cpp
void d2::Signals::Handle::unmute()
```

Source: `core/io/d2_signal_handler.hpp:801`

### `release`

```cpp
void d2::Signals::Handle::release()
```

Source: `core/io/d2_signal_handler.hpp:802`

### `close`

```cpp
void d2::Signals::Handle::close()
```

Source: `core/io/d2_signal_handler.hpp:803`

### `state`

```cpp
Signals::Handle::State d2::Signals::Handle::state() const
```

Source: `core/io/d2_signal_handler.hpp:804`

### `operator=`

```cpp
Handle & d2::Signals::Handle::operator=(const Handle &)=delete
```

Source: `core/io/d2_signal_handler.hpp:806`

### `operator=`

```cpp
Signals::Handle & d2::Signals::Handle::operator=(Handle &&other) noexcept
```

Source: `core/io/d2_signal_handler.hpp:807`

### `operator==`

```cpp
bool d2::Signals::Handle::operator==(std::nullptr_t) const
```

Source: `core/io/d2_signal_handler.hpp:809`

### `operator!=`

```cpp
bool d2::Signals::Handle::operator!=(std::nullptr_t) const
```

Source: `core/io/d2_signal_handler.hpp:810`


## Typedefs

### `State`

```cpp
using d2::Signals::Handle::State = HandleState::State
```

Source: `core/io/d2_signal_handler.hpp:788`
