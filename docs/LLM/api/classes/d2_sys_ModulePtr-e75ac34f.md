# `d2::sys::ModulePtr`

**Kind:** class
**Declared in:** `core/io/d2_module.hpp:134`

## Functions

### `ModulePtr`

```cpp
d2::sys::ModulePtr< Module >::ModulePtr(std::nullptr_t)
```

Source: `core/io/d2_module.hpp:139`

### `ModulePtr`

```cpp
d2::sys::ModulePtr< Module >::ModulePtr()=default
```

Source: `core/io/d2_module.hpp:140`

### `ModulePtr`

```cpp
d2::sys::ModulePtr< Module >::ModulePtr(const ModulePtr &)=default
```

Source: `core/io/d2_module.hpp:141`

### `ModulePtr`

```cpp
d2::sys::ModulePtr< Module >::ModulePtr(ModulePtr &&)=default
```

Source: `core/io/d2_module.hpp:142`

### `ModulePtr`

```cpp
d2::sys::ModulePtr< Module >::ModulePtr(std::shared_ptr< Module > ptr)
```

Source: `core/io/d2_module.hpp:143`

### `ptr`

```cpp
std::shared_ptr< Module > d2::sys::ModulePtr< Module >::ptr() const
```

Source: `core/io/d2_module.hpp:145`

### `id`

```cpp
std::uintptr_t d2::sys::ModulePtr< Module >::id() const
```

Source: `core/io/d2_module.hpp:154`

### `operator->`

```cpp
std::shared_ptr< Module > d2::sys::ModulePtr< Module >::operator->() const
```

Source: `core/io/d2_module.hpp:159`

### `operator bool`

```cpp
d2::sys::ModulePtr< Module >::operator bool() const noexcept
```

Source: `core/io/d2_module.hpp:164`

### `operator==`

```cpp
bool d2::sys::ModulePtr< Module >::operator==(const ModulePtr &ptr) const noexcept
```

Source: `core/io/d2_module.hpp:169`

### `operator!=`

```cpp
bool d2::sys::ModulePtr< Module >::operator!=(const ModulePtr &ptr) const noexcept
```

Source: `core/io/d2_module.hpp:173`

### `operator=`

```cpp
ModulePtr & d2::sys::ModulePtr< Module >::operator=(const ModulePtr &)=default
```

Source: `core/io/d2_module.hpp:178`

### `operator=`

```cpp
ModulePtr & d2::sys::ModulePtr< Module >::operator=(ModulePtr &&)=default
```

Source: `core/io/d2_module.hpp:179`
