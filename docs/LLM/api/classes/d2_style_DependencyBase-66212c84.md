# `d2::style::DependencyBase`

**Kind:** class
**Declared in:** `core/tree/d2_theme.hpp:19`

## Functions

### `_sync`

```cpp
template<typename Func>
static std::invoke_result_t< Func > d2::style::DependencyBase::_sync(Func &&func)
```

Source: `core/tree/d2_theme.hpp:83`

### `_sync_async`

```cpp
template<typename Func>
static void d2::style::DependencyBase::_sync_async(Func &&func)
```

Source: `core/tree/d2_theme.hpp:89`

### `DependencyBase`

```cpp
d2::style::DependencyBase::DependencyBase()=default
```

Source: `core/tree/d2_theme.hpp:112`

### `DependencyBase`

```cpp
d2::style::DependencyBase::DependencyBase(const DependencyBase &)
```

Source: `core/tree/d2_theme.hpp:113`

### `DependencyBase`

```cpp
d2::style::DependencyBase::DependencyBase(DependencyBase &&)=delete
```

Source: `core/tree/d2_theme.hpp:114`

### `~DependencyBase`

```cpp
d2::style::DependencyBase::~DependencyBase()
```

Source: `core/tree/d2_theme.hpp:115`

### `_ensure_state`

```cpp
std::shared_ptr< DependencyBase::StateData > d2::style::DependencyBase::_ensure_state() const
```

Source: `core/tree/d2_theme.hpp:117`

### `_next_generation`

```cpp
std::uint32_t d2::style::DependencyBase::_next_generation() const
```

Source: `core/tree/d2_theme.hpp:118`

### `_make_handle`

```cpp
DependencyBase::Handle d2::style::DependencyBase::_make_handle(std::uint32_t index) const
```

Source: `core/tree/d2_theme.hpp:120`

### `_insert_slot`

```cpp
DependencyBase::Handle d2::style::DependencyBase::_insert_slot(Deps dependency) const
```

Source: `core/tree/d2_theme.hpp:121`

### `_close_slot_local`

```cpp
void d2::style::DependencyBase::_close_slot_local(std::uint32_t index, std::uint32_t generation) const
```

Source: `core/tree/d2_theme.hpp:123`

### `_cleanup_local`

```cpp
void d2::style::DependencyBase::_cleanup_local() const
```

Source: `core/tree/d2_theme.hpp:124`

### `_flush_pending_local`

```cpp
void d2::style::DependencyBase::_flush_pending_local() const
```

Source: `core/tree/d2_theme.hpp:125`

### `_destroy_state_local`

```cpp
void d2::style::DependencyBase::_destroy_state_local() const
```

Source: `core/tree/d2_theme.hpp:126`

### `_destroy`

```cpp
virtual void d2::style::DependencyBase::_destroy(Deps dependency) const =0
```

Source: `core/tree/d2_theme.hpp:128`


## Variables

### `_state`

```cpp
std::shared_ptr<StateData> d2::style::DependencyBase::_state
```

Source: `core/tree/d2_theme.hpp:97`
