# `d2::TreeCtx< Type, d2::TreeState >`

**Kind:** struct
**Declared in:** `core/tree/d2_tree_construct.hpp:663`

**Bases:** `d2::TreeCtxBase`, `d2::TreeCtx< Type::root, Type::state >`

## Functions

### `TreeCtxBase`

```cpp
TreeCtxBase(d2::TreeIter<> ptr)
```

Source: `core/tree/d2_tree_construct.hpp:272`

### `TreeCtxBase`

```cpp
TreeCtxBase(const TreeCtxBase &)=default
```

Source: `core/tree/d2_tree_construct.hpp:272`

### `TreeCtxBase`

```cpp
TreeCtxBase(TreeCtxBase &&)=default
```

Source: `core/tree/d2_tree_construct.hpp:272`

### `TreeCtx`

```cpp
d2::TreeCtx< Type, d2::TreeState >::TreeCtx(d2::TreeIter< Other > ptr)
```

Source: `core/tree/d2_tree_construct.hpp:275`

### `ptr`

```cpp
TreeIter< Type > d2::TreeCtx< Type, d2::TreeState >::ptr() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:279`

### `state`

```cpp
tree_state_ptr< d2::TreeState > d2::TreeCtx< Type, d2::TreeState >::state() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:283`

### `clone`

```cpp
void d2::TreeCtx< Type, d2::TreeState >::clone(TreeIter< Type > ptr, auto &&callback) const
```

Source: `core/tree/d2_tree_construct.hpp:288`

### `anchor`

```cpp
auto d2::TreeCtx< Type, d2::TreeState >::anchor(std::string name="", Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:294`

### `animate`

```cpp
Animation::ptr d2::TreeCtx< Type, d2::TreeState >::animate(std::chrono::milliseconds time, Func &&callback, Argv &&... args)
```

Source: `core/tree/d2_tree_construct.hpp:311`

### `animate_twoway`

```cpp
Element::EventListener d2::TreeCtx< Type, d2::TreeState >::animate_twoway(std::chrono::milliseconds time, Argv &&... args)
```

Source: `core/tree/d2_tree_construct.hpp:339`

### `animate_toggle`

```cpp
Element::EventListener d2::TreeCtx< Type, d2::TreeState >::animate_toggle(std::chrono::milliseconds time, Argv &&... args)
```

Source: `core/tree/d2_tree_construct.hpp:377`

### `embed`

```cpp
auto d2::TreeCtx< Type, d2::TreeState >::embed(const std::string &name, Func &&callback, std::tuple< Arge... > arge={}, std::tuple< Args... > args={}) const
```

Source: `core/tree/d2_tree_construct.hpp:420`

### `elem`

```cpp
auto d2::TreeCtx< Type, d2::TreeState >::elem(Func &&callback, Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:469`

### `branch`

```cpp
void d2::TreeCtx< Type, d2::TreeState >::branch(Var &var, std::pair< typename Var::value_type, Branch >... branches)
```

Source: `core/tree/d2_tree_construct.hpp:506`

### `persist`

```cpp
void d2::TreeCtx< Type, d2::TreeState >::persist(bool preserve_position=true, std::vector< std::string > whitelist={})
```

Source: `core/tree/d2_tree_construct.hpp:555`

### `apply`

```cpp
void d2::TreeCtx< Type, d2::TreeState >::apply(Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:603`

### `operator->`

```cpp
TreeIter< Type > d2::TreeCtx< Type, d2::TreeState >::operator->() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:608`

### `operator TreeCtx< OtherRoot, OtherState >`

```cpp
d2::TreeCtx< Type, d2::TreeState >::operator TreeCtx< OtherRoot, OtherState >() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:614`


## Typedefs

### `tree_type`

```cpp
using d2::TreeCtx< Type, d2::TreeState >::tree_type = Type
```

Source: `core/tree/d2_tree_construct.hpp:665`

### `element_type`

```cpp
using d2::TreeCtx< Type, d2::TreeState >::element_type =
```

Source: `core/tree/d2_tree_construct.hpp:269`

### `state_type`

```cpp
using d2::TreeCtx< Type, d2::TreeState >::state_type =
```

Source: `core/tree/d2_tree_construct.hpp:270`
