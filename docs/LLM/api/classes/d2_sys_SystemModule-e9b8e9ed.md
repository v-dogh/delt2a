# `d2::sys::SystemModule`

**Kind:** class
**Declared in:** `core/io/d2_module.hpp:51`

**Bases:** `std::enable_shared_from_this< SystemModule >`

## Enums

### `Status`

```cpp
enum Status
```

Source: `core/io/d2_module.hpp:54`

Values: `Ok`, `Degraded`, `Failure`, `Offline`, `Loading`

### `Compat`

```cpp
enum Compat
```

Source: `core/io/d2_module.hpp:62`

Values: `Native`, `Generic`, `Unknown`, `Failed`


## Functions

### `_mark_safe`

```cpp
void d2::sys::SystemModule::_mark_safe(std::thread::id id=std::this_thread::get_id())
```

Source: `core/io/d2_module.hpp:88`

### `_transition`

```cpp
void d2::sys::SystemModule::_transition(bool loading, std::function< Status()> callback)
```

Source: `core/io/d2_module.hpp:89`

### `_stat`

```cpp
void d2::sys::SystemModule::_stat(Status status)
```

Source: `core/io/d2_module.hpp:90`

### `_pre_load_impl`

```cpp
SystemModule::Status d2::sys::SystemModule::_pre_load_impl()
```

Source: `core/io/d2_module.hpp:92`

### `_post_unload_impl`

```cpp
SystemModule::Status d2::sys::SystemModule::_post_unload_impl()
```

Source: `core/io/d2_module.hpp:93`

### `_load_impl`

```cpp
SystemModule::Status d2::sys::SystemModule::_load_impl()
```

Source: `core/io/d2_module.hpp:95`

### `_unload_impl`

```cpp
SystemModule::Status d2::sys::SystemModule::_unload_impl()
```

Source: `core/io/d2_module.hpp:96`

### `compatible`

```cpp
SystemModule::Compat d2::sys::SystemModule::compatible()
```

Source: `core/io/d2_module.hpp:98`

### `make`

```cpp
template<typename Module>
static std::unique_ptr< Module > d2::sys::SystemModule::make(std::weak_ptr< IOContext > ctx)
```

Source: `core/io/d2_module.hpp:100`

### `SystemModule`

```cpp
d2::sys::SystemModule::SystemModule(std::weak_ptr< IOContext > ptr, ModPreset preset, std::size_t static_usage)
```

Source: `core/io/d2_module.hpp:105`

### `~SystemModule`

```cpp
virtual d2::sys::SystemModule::~SystemModule()=default
```

Source: `core/io/d2_module.hpp:106`

### `info`

```cpp
virtual ModInfo d2::sys::SystemModule::info() const =0
```

Source: `core/io/d2_module.hpp:108`

### `preset`

```cpp
SystemModule::ModPreset d2::sys::SystemModule::preset() const
```

Source: `core/io/d2_module.hpp:109`

### `load_type`

```cpp
Load d2::sys::SystemModule::load_type() const noexcept
```

Source: `core/io/d2_module.hpp:110`

### `access`

```cpp
Access d2::sys::SystemModule::access() const noexcept
```

Source: `core/io/d2_module.hpp:111`

### `static_usage`

```cpp
std::size_t d2::sys::SystemModule::static_usage() const noexcept
```

Source: `core/io/d2_module.hpp:112`

### `dependencies`

```cpp
std::span< const std::type_index > d2::sys::SystemModule::dependencies() const noexcept
```

Source: `core/io/d2_module.hpp:114`

### `dependency_names`

```cpp
std::span< const std::string > d2::sys::SystemModule::dependency_names() const noexcept
```

Source: `core/io/d2_module.hpp:115`

### `context`

```cpp
std::shared_ptr< IOContext > d2::sys::SystemModule::context() const noexcept
```

Source: `core/io/d2_module.hpp:116`

### `status`

```cpp
SystemModule::Status d2::sys::SystemModule::status()
```

Source: `core/io/d2_module.hpp:118`

### `load`

```cpp
void d2::sys::SystemModule::load()
```

Source: `core/io/d2_module.hpp:119`

### `unload`

```cpp
void d2::sys::SystemModule::unload()
```

Source: `core/io/d2_module.hpp:120`

### `accessible`

```cpp
bool d2::sys::SystemModule::accessible() const
```

Source: `core/io/d2_module.hpp:121`

### `as`

```cpp
template<typename Type>
Type * d2::sys::SystemModule::as()
```

Source: `core/io/d2_module.hpp:123`
