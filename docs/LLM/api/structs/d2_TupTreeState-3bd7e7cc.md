# `d2::TupTreeState`

**Kind:** struct
**Declared in:** `core/tree/d2_tree_state.hpp:76`

**Bases:** `d2::TreeState`

## Functions

### `TupTreeState`

```cpp
d2::TupTreeState< Argv >::TupTreeState(std::shared_ptr< ParentElement > rptr, std::shared_ptr< ParentElement > coreptr, std::shared_ptr< IOContext > ctx, Argv... args)
```

Source: `core/tree/d2_tree_state.hpp:79`

### `TreeState`

```cpp
d2::TreeState::TreeState(std::shared_ptr< ParentElement > rptr, std::shared_ptr< ParentElement > coreptr, std::shared_ptr< IOContext > ctx)
```

Source: `core/tree/d2_tree_state.hpp:87`

### `TreeState`

```cpp
d2::TreeState::TreeState(const TreeState &)=delete
```

Source: `core/tree/d2_tree_state.hpp:87`

### `TreeState`

```cpp
d2::TreeState::TreeState(TreeState &&)=default
```

Source: `core/tree/d2_tree_state.hpp:87`


## Variables

### `params`

```cpp
std::tuple<Argv...> d2::TupTreeState< Argv >::params
```

Source: `core/tree/d2_tree_state.hpp:78`
