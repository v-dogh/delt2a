# `d2::Signals::SpecifierKey`

**Kind:** class
**Declared in:** `core/io/d2_signal_handler.hpp:426`

## Functions

### `SpecifierKey`

```cpp
d2::Signals::SpecifierKey::SpecifierKey() noexcept=default
```

Source: `core/io/d2_signal_handler.hpp:499`

### `SpecifierKey`

```cpp
template<typename Type>
d2::Signals::SpecifierKey::SpecifierKey(Type &&value)
```

Source: `core/io/d2_signal_handler.hpp:503`

### `SpecifierKey`

```cpp
d2::Signals::SpecifierKey::SpecifierKey(const SpecifierKey &copy)
```

Source: `core/io/d2_signal_handler.hpp:541`

### `SpecifierKey`

```cpp
d2::Signals::SpecifierKey::SpecifierKey(SpecifierKey &&copy) noexcept
```

Source: `core/io/d2_signal_handler.hpp:542`

### `~SpecifierKey`

```cpp
d2::Signals::SpecifierKey::~SpecifierKey()
```

Source: `core/io/d2_signal_handler.hpp:543`

### `operator=`

```cpp
Signals::SpecifierKey & d2::Signals::SpecifierKey::operator=(const SpecifierKey &copy)
```

Source: `core/io/d2_signal_handler.hpp:545`

### `operator=`

```cpp
Signals::SpecifierKey & d2::Signals::SpecifierKey::operator=(SpecifierKey &&copy) noexcept
```

Source: `core/io/d2_signal_handler.hpp:546`

### `empty`

```cpp
bool d2::Signals::SpecifierKey::empty() const noexcept
```

Source: `core/io/d2_signal_handler.hpp:548`

### `hash`

```cpp
std::size_t d2::Signals::SpecifierKey::hash() const noexcept
```

Source: `core/io/d2_signal_handler.hpp:549`

### `operator==`

```cpp
bool d2::Signals::SpecifierKey::operator==(const SpecifierKey &other) const
```

Source: `core/io/d2_signal_handler.hpp:551`

### `operator!=`

```cpp
bool d2::Signals::SpecifierKey::operator!=(const SpecifierKey &other) const
```

Source: `core/io/d2_signal_handler.hpp:552`
