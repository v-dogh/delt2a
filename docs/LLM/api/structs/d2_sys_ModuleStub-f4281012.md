# `d2::sys::ModuleStub`

**Kind:** struct
**Declared in:** `core/io/d2_module.hpp:272`

## Enums

### `Query`

```cpp
enum Query
```

Source: `core/io/d2_module.hpp:276`

Values: `Commit`, `Preset`, `Info`


## Functions

### `make`

```cpp
template<typename Module>
static auto d2::sys::ModuleStub::make()
```

Source: `core/io/d2_module.hpp:314`

### `ModuleStub`

```cpp
d2::sys::ModuleStub::ModuleStub()=default
```

Source: `core/io/d2_module.hpp:319`

### `ModuleStub`

```cpp
d2::sys::ModuleStub::ModuleStub(handle_type handle)
```

Source: `core/io/d2_module.hpp:320`

### `ModuleStub`

```cpp
d2::sys::ModuleStub::ModuleStub(const ModuleStub &)=default
```

Source: `core/io/d2_module.hpp:321`

### `ModuleStub`

```cpp
d2::sys::ModuleStub::ModuleStub(ModuleStub &&)=default
```

Source: `core/io/d2_module.hpp:322`

### `commit`

```cpp
void d2::sys::ModuleStub::commit(std::shared_ptr< IOContext > ctx)
```

Source: `core/io/d2_module.hpp:324`

### `ptr`

```cpp
std::shared_ptr< SystemModule > d2::sys::ModuleStub::ptr() const
```

Source: `core/io/d2_module.hpp:325`

### `info`

```cpp
SystemModule::ModInfo d2::sys::ModuleStub::info() const
```

Source: `core/io/d2_module.hpp:326`

### `preset`

```cpp
SystemModule::ModPreset d2::sys::ModuleStub::preset() const
```

Source: `core/io/d2_module.hpp:327`

### `status`

```cpp
SystemModule::Status d2::sys::ModuleStub::status() const
```

Source: `core/io/d2_module.hpp:328`

### `operator=`

```cpp
ModuleStub & d2::sys::ModuleStub::operator=(const ModuleStub &)=default
```

Source: `core/io/d2_module.hpp:330`

### `operator=`

```cpp
ModuleStub & d2::sys::ModuleStub::operator=(ModuleStub &&)=default
```

Source: `core/io/d2_module.hpp:331`

### `operator==`

```cpp
bool d2::sys::ModuleStub::operator==(std::nullptr_t) const noexcept
```

Source: `core/io/d2_module.hpp:333`

### `operator!=`

```cpp
bool d2::sys::ModuleStub::operator!=(std::nullptr_t) const noexcept
```

Source: `core/io/d2_module.hpp:334`


## Typedefs

### `handle_info`

```cpp
using d2::sys::ModuleStub::handle_info = std::variant< SystemModule::ModInfo, SystemModule::ModPreset, std::shared_ptr<SystemModule>, std::monostate >
```

Source: `core/io/d2_module.hpp:283`

### `handle_type`

```cpp
using d2::sys::ModuleStub::handle_type = handle_info (*)(Query, std::shared_ptr<IOContext>)
```

Source: `core/io/d2_module.hpp:289`


## Variables

### `handle`

```cpp
template<typename Module>
handle_type d2::sys::ModuleStub::handle
```

Source: `core/io/d2_module.hpp:292`
