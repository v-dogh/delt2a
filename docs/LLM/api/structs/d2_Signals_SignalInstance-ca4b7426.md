# `d2::Signals::SignalInstance`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:601`

## Functions

### `SignalInstance`

```cpp
template<typename... Argv>
d2::Signals::SignalInstance::SignalInstance(Argv &&... pack)
```

Source: `core/io/d2_signal_handler.hpp:621`

### `SignalInstance`

```cpp
d2::Signals::SignalInstance::SignalInstance(SignalInstance &&copy) noexcept
```

Source: `core/io/d2_signal_handler.hpp:711`

### `SignalInstance`

```cpp
d2::Signals::SignalInstance::SignalInstance(const SignalInstance &)=delete
```

Source: `core/io/d2_signal_handler.hpp:712`

### `~SignalInstance`

```cpp
d2::Signals::SignalInstance::~SignalInstance()
```

Source: `core/io/d2_signal_handler.hpp:713`

### `apply`

```cpp
template<typename Func, typename... Argv>
void d2::Signals::SignalInstance::apply(void(*acceptor)(void *, Argv...), Func &callback)
```

Source: `core/io/d2_signal_handler.hpp:716`

### `operator=`

```cpp
Signals::SignalInstance & d2::Signals::SignalInstance::operator=(SignalInstance &&copy) noexcept
```

Source: `core/io/d2_signal_handler.hpp:727`

### `operator=`

```cpp
SignalInstance & d2::Signals::SignalInstance::operator=(const SignalInstance &)=delete
```

Source: `core/io/d2_signal_handler.hpp:728`

### `operator==`

```cpp
bool d2::Signals::SignalInstance::operator==(std::nullptr_t) const
```

Source: `core/io/d2_signal_handler.hpp:730`

### `operator!=`

```cpp
bool d2::Signals::SignalInstance::operator!=(std::nullptr_t) const
```

Source: `core/io/d2_signal_handler.hpp:731`
