# `d2::sys::SystemScreen::TreeData`

**Kind:** struct
**Declared in:** `core/screen/d2_screen.hpp:66`

## Variables

### `rebuild`

```cpp
std::function<void(TreeIter<ParentElement>, TreeState::ptr)> d2::sys::SystemScreen::TreeData::rebuild
```

Source: `core/screen/d2_screen.hpp:68`

### `animations`

```cpp
absl::node_hash_map<std::pair<Element*, style::uai_property>, AnimationState> d2::sys::SystemScreen::TreeData::animations
```

Source: `core/screen/d2_screen.hpp:70`

### `state`

```cpp
TreeState::ptr d2::sys::SystemScreen::TreeData::state
```

Source: `core/screen/d2_screen.hpp:71`

### `tags`

```cpp
TreeTags d2::sys::SystemScreen::TreeData::tags
```

Source: `core/screen/d2_screen.hpp:72`

### `deps`

```cpp
DynamicDependencyManager d2::sys::SystemScreen::TreeData::deps
```

Source: `core/screen/d2_screen.hpp:73`

### `unbuilt`

```cpp
bool d2::sys::SystemScreen::TreeData::unbuilt
```

Source: `core/screen/d2_screen.hpp:74`

### `swapped_out`

```cpp
bool d2::sys::SystemScreen::TreeData::swapped_out
```

Source: `core/screen/d2_screen.hpp:75`
