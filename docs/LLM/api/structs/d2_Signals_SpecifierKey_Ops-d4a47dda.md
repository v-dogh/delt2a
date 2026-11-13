# `d2::Signals::SpecifierKey::Ops`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:430`

## Variables

### `type`

```cpp
const void* d2::Signals::SpecifierKey::Ops::type
```

Source: `core/io/d2_signal_handler.hpp:432`

### `size`

```cpp
std::size_t d2::Signals::SpecifierKey::Ops::size
```

Source: `core/io/d2_signal_handler.hpp:433`

### `alignment`

```cpp
std::size_t d2::Signals::SpecifierKey::Ops::alignment
```

Source: `core/io/d2_signal_handler.hpp:434`

### `local`

```cpp
bool d2::Signals::SpecifierKey::Ops::local
```

Source: `core/io/d2_signal_handler.hpp:435`

### `hash`

```cpp
std::size_t(*) d2::Signals::SpecifierKey::Ops::hash(const void *)
```

Source: `core/io/d2_signal_handler.hpp:436`

### `equal`

```cpp
bool(*) d2::Signals::SpecifierKey::Ops::equal(const void *, const void *)
```

Source: `core/io/d2_signal_handler.hpp:437`

### `copy`

```cpp
void(*) d2::Signals::SpecifierKey::Ops::copy(void *, const void *)
```

Source: `core/io/d2_signal_handler.hpp:438`

### `move`

```cpp
void(*) d2::Signals::SpecifierKey::Ops::move(void *, void *)
```

Source: `core/io/d2_signal_handler.hpp:439`

### `destroy`

```cpp
void(*) d2::Signals::SpecifierKey::Ops::destroy(void *)
```

Source: `core/io/d2_signal_handler.hpp:440`
