# `d2::style::DependencyBase::StateData`

**Kind:** struct
**Declared in:** `core/tree/d2_theme.hpp:31`

## Variables

### `owner`

```cpp
const DependencyBase* d2::style::DependencyBase::StateData::owner
```

Source: `core/tree/d2_theme.hpp:33`

### `recursion_ctr`

```cpp
std::size_t d2::style::DependencyBase::StateData::recursion_ctr
```

Source: `core/tree/d2_theme.hpp:34`

### `next_generation`

```cpp
std::uint32_t d2::style::DependencyBase::StateData::next_generation
```

Source: `core/tree/d2_theme.hpp:35`

### `destroying`

```cpp
bool d2::style::DependencyBase::StateData::destroying
```

Source: `core/tree/d2_theme.hpp:36`

### `dependencies`

```cpp
std::vector<Deps> d2::style::DependencyBase::StateData::dependencies
```

Source: `core/tree/d2_theme.hpp:38`

### `free_slots`

```cpp
std::vector<std::uint32_t> d2::style::DependencyBase::StateData::free_slots
```

Source: `core/tree/d2_theme.hpp:39`

### `pending_free`

```cpp
std::vector<std::uint32_t> d2::style::DependencyBase::StateData::pending_free
```

Source: `core/tree/d2_theme.hpp:40`

### `pending_destroy`

```cpp
std::vector<Deps> d2::style::DependencyBase::StateData::pending_destroy
```

Source: `core/tree/d2_theme.hpp:41`
