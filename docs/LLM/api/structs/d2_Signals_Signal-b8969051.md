# `d2::Signals::Signal`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:812`

## Functions

### `Signal`

```cpp
d2::Signals::Signal::Signal()=default
```

Source: `core/io/d2_signal_handler.hpp:819`

### `Signal`

```cpp
d2::Signals::Signal::Signal(std::weak_ptr< Signals > ptr, std::shared_ptr< SignalStorage > storage, SignalKey id)
```

Source: `core/io/d2_signal_handler.hpp:820`

### `Signal`

```cpp
d2::Signals::Signal::Signal(std::nullptr_t)
```

Source: `core/io/d2_signal_handler.hpp:823`

### `Signal`

```cpp
d2::Signals::Signal::Signal(const Signal &)=delete
```

Source: `core/io/d2_signal_handler.hpp:824`

### `Signal`

```cpp
d2::Signals::Signal::Signal(Signal &&other) noexcept
```

Source: `core/io/d2_signal_handler.hpp:825`

### `~Signal`

```cpp
d2::Signals::Signal::~Signal()
```

Source: `core/io/d2_signal_handler.hpp:826`

### `disconnect`

```cpp
void d2::Signals::Signal::disconnect()
```

Source: `core/io/d2_signal_handler.hpp:828`

### `operator=`

```cpp
Signal & d2::Signals::Signal::operator=(const Signal &)=delete
```

Source: `core/io/d2_signal_handler.hpp:830`

### `operator=`

```cpp
Signals::Signal & d2::Signals::Signal::operator=(Signal &&other) noexcept
```

Source: `core/io/d2_signal_handler.hpp:831`

### `operator==`

```cpp
bool d2::Signals::Signal::operator==(std::nullptr_t) const
```

Source: `core/io/d2_signal_handler.hpp:833`

### `operator!=`

```cpp
bool d2::Signals::Signal::operator!=(std::nullptr_t) const
```

Source: `core/io/d2_signal_handler.hpp:834`
