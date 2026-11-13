# `d2::TreeState`

**Kind:** class
**Declared in:** `core/tree/d2_tree_state.hpp:12`

**Bases:** `std::enable_shared_from_this< TreeState >`

## Functions

### `make`

```cpp
template<typename Type, typename... Argv>
static auto d2::TreeState::make(std::shared_ptr< ParentElement > rptr, std::shared_ptr< ParentElement > coreptr, std::shared_ptr< IOContext > ctx, Argv &&... args)
```

Source: `core/tree/d2_tree_state.hpp:23`

### `TreeState`

```cpp
d2::TreeState::TreeState(std::shared_ptr< ParentElement > rptr, std::shared_ptr< ParentElement > coreptr, std::shared_ptr< IOContext > ctx)
```

Source: `core/tree/d2_tree_state.hpp:33`

### `TreeState`

```cpp
d2::TreeState::TreeState(const TreeState &)=delete
```

Source: `core/tree/d2_tree_state.hpp:38`

### `TreeState`

```cpp
d2::TreeState::TreeState(TreeState &&)=default
```

Source: `core/tree/d2_tree_state.hpp:39`

### `~TreeState`

```cpp
virtual d2::TreeState::~TreeState()=default
```

Source: `core/tree/d2_tree_state.hpp:40`

### `construct`

```cpp
virtual void d2::TreeState::construct()
```

Source: `core/tree/d2_tree_state.hpp:42`

### `swap_in`

```cpp
virtual void d2::TreeState::swap_in()
```

Source: `core/tree/d2_tree_state.hpp:43`

### `swap_out`

```cpp
virtual void d2::TreeState::swap_out()
```

Source: `core/tree/d2_tree_state.hpp:44`

### `update`

```cpp
virtual void d2::TreeState::update()
```

Source: `core/tree/d2_tree_state.hpp:45`

### `set_context`

```cpp
void d2::TreeState::set_context(std::shared_ptr< IOContext > ctx)
```

Source: `core/tree/d2_tree_state.hpp:47`

### `set_root`

```cpp
void d2::TreeState::set_root(std::shared_ptr< ParentElement > ptr)
```

Source: `core/tree/d2_tree_state.hpp:48`

### `set_core`

```cpp
void d2::TreeState::set_core(std::shared_ptr< ParentElement > ptr)
```

Source: `core/tree/d2_tree_state.hpp:49`

### `context`

```cpp
std::shared_ptr< IOContext > d2::TreeState::context() const
```

Source: `core/tree/d2_tree_state.hpp:51`

### `root`

```cpp
std::shared_ptr< ParentElement > d2::TreeState::root() const
```

Source: `core/tree/d2_tree_state.hpp:52`

### `root_state`

```cpp
std::shared_ptr< TreeState > d2::TreeState::root_state() const
```

Source: `core/tree/d2_tree_state.hpp:53`

### `core`

```cpp
std::shared_ptr< ParentElement > d2::TreeState::core() const
```

Source: `core/tree/d2_tree_state.hpp:54`

### `screen`

```cpp
sys::module< sys::SystemScreen > d2::TreeState::screen() const
```

Source: `core/tree/d2_tree_state.hpp:55`

### `as`

```cpp
template<typename Type>
std::shared_ptr< const Type > d2::TreeState::as() const
```

Source: `core/tree/d2_tree_state.hpp:57`

### `as`

```cpp
template<typename Type>
std::shared_ptr< Type > d2::TreeState::as()
```

Source: `core/tree/d2_tree_state.hpp:64`

### `operator=`

```cpp
TreeState & d2::TreeState::operator=(const TreeState &)=delete
```

Source: `core/tree/d2_tree_state.hpp:72`

### `operator=`

```cpp
TreeState & d2::TreeState::operator=(TreeState &&)=default
```

Source: `core/tree/d2_tree_state.hpp:73`


## Typedefs

### `ptr`

```cpp
using d2::TreeState::ptr = std::shared_ptr<TreeState>
```

Source: `core/tree/d2_tree_state.hpp:16`
