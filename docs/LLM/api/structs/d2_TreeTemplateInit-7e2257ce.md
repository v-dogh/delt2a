# `d2::TreeTemplateInit`

**Kind:** struct
**Declared in:** `core/tree/d2_tree.hpp:10`

## Functions

### `build_sub`

```cpp
template<typename... Argv>
static auto d2::TreeTemplateInit< State, Tree, Root >::build_sub(const std::string &name, TreeState::ptr state, d2::Element::pptr root, Argv &&... args)
```

Source: `core/tree/d2_tree.hpp:17`

### `build`

```cpp
template<typename... Argv>
static auto d2::TreeTemplateInit< State, Tree, Root >::build(IOContext::ptr ctx, Argv &&... args)
```

Source: `core/tree/d2_tree.hpp:31`


## Typedefs

### `root`

```cpp
using d2::TreeTemplateInit< State, Tree, Root >::root = Root
```

Source: `core/tree/d2_tree.hpp:13`

### `state`

```cpp
using d2::TreeTemplateInit< State, Tree, Root >::state = State
```

Source: `core/tree/d2_tree.hpp:14`
