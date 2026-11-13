# `d2::style::DependencyBase::InvokeGuard`

**Kind:** struct
**Declared in:** `core/tree/d2_theme.hpp:99`

## Functions

### `InvokeGuard`

```cpp
d2::style::DependencyBase::InvokeGuard::InvokeGuard(const DependencyBase &self)
```

Source: `core/tree/d2_theme.hpp:104`

### `InvokeGuard`

```cpp
d2::style::DependencyBase::InvokeGuard::InvokeGuard(const InvokeGuard &)=delete
```

Source: `core/tree/d2_theme.hpp:105`

### `InvokeGuard`

```cpp
d2::style::DependencyBase::InvokeGuard::InvokeGuard(const DependencyBase &self, std::shared_ptr< StateData > state)
```

Source: `core/tree/d2_theme.hpp:106`

### `~InvokeGuard`

```cpp
d2::style::DependencyBase::InvokeGuard::~InvokeGuard()
```

Source: `core/tree/d2_theme.hpp:107`

### `operator=`

```cpp
InvokeGuard & d2::style::DependencyBase::InvokeGuard::operator=(const InvokeGuard &)=delete
```

Source: `core/tree/d2_theme.hpp:109`


## Variables

### `self`

```cpp
const DependencyBase& d2::style::DependencyBase::InvokeGuard::self
```

Source: `core/tree/d2_theme.hpp:101`

### `state`

```cpp
std::shared_ptr<StateData> d2::style::DependencyBase::InvokeGuard::state
```

Source: `core/tree/d2_theme.hpp:102`
