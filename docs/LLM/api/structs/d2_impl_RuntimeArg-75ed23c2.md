# `d2::impl::RuntimeArg`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:142`

## Functions

### `make`

```cpp
template<typename Type>
static RuntimeArg d2::impl::RuntimeArg::make() noexcept
```

Source: `core/io/d2_signal_handler.hpp:149`

### `operator==`

```cpp
bool d2::impl::RuntimeArg::operator==(const RuntimeArg &other) const noexcept
```

Source: `core/io/d2_signal_handler.hpp:161`

### `operator!=`

```cpp
bool d2::impl::RuntimeArg::operator!=(const RuntimeArg &other) const noexcept
```

Source: `core/io/d2_signal_handler.hpp:162`


## Variables

### `type`

```cpp
const std::type_info* d2::impl::RuntimeArg::type
```

Source: `core/io/d2_signal_handler.hpp:144`

### `lvalue_ref`

```cpp
bool d2::impl::RuntimeArg::lvalue_ref
```

Source: `core/io/d2_signal_handler.hpp:145`

### `is_const`

```cpp
bool d2::impl::RuntimeArg::is_const
```

Source: `core/io/d2_signal_handler.hpp:146`

### `is_volatile`

```cpp
bool d2::impl::RuntimeArg::is_volatile
```

Source: `core/io/d2_signal_handler.hpp:147`
