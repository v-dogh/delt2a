# `d2::style::DependencyBase::Handle`

**Kind:** class
**Declared in:** `core/tree/d2_theme.hpp:44`

## Enums

### `State`

```cpp
enum State
```

Source: `core/tree/d2_theme.hpp:47`

Values: `Inactive`, `Active`


## Functions

### `Handle`

```cpp
d2::style::DependencyBase::Handle::Handle()=default
```

Source: `core/tree/d2_theme.hpp:64`

### `Handle`

```cpp
d2::style::DependencyBase::Handle::Handle(const Handle &)=delete
```

Source: `core/tree/d2_theme.hpp:65`

### `Handle`

```cpp
d2::style::DependencyBase::Handle::Handle(Handle &&other) noexcept
```

Source: `core/tree/d2_theme.hpp:66`

### `~Handle`

```cpp
d2::style::DependencyBase::Handle::~Handle()
```

Source: `core/tree/d2_theme.hpp:71`

### `close`

```cpp
void d2::style::DependencyBase::Handle::close()
```

Source: `core/tree/d2_theme.hpp:73`

### `release`

```cpp
void d2::style::DependencyBase::Handle::release() noexcept
```

Source: `core/tree/d2_theme.hpp:74`

### `state`

```cpp
DependencyBase::Handle::State d2::style::DependencyBase::Handle::state() const noexcept
```

Source: `core/tree/d2_theme.hpp:75`

### `operator=`

```cpp
Handle & d2::style::DependencyBase::Handle::operator=(const Handle &)=delete
```

Source: `core/tree/d2_theme.hpp:77`

### `operator=`

```cpp
DependencyBase::Handle & d2::style::DependencyBase::Handle::operator=(Handle &&other) noexcept
```

Source: `core/tree/d2_theme.hpp:78`

### `operator bool`

```cpp
d2::style::DependencyBase::Handle::operator bool() const noexcept
```

Source: `core/tree/d2_theme.hpp:80`
