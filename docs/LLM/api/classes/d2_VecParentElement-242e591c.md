# `d2::VecParentElement`

**Kind:** class
**Declared in:** `core/tree/d2_tree_parent.hpp:130`

**Bases:** `d2::ParentElement`

## Functions

### `_index_of_impl`

```cpp
int d2::VecParentElement::_index_of_impl(id id) const override
```

Source: `core/tree/d2_tree_parent.hpp:137`

### `_empty_impl`

```cpp
bool d2::VecParentElement::_empty_impl() const override
```

Source: `core/tree/d2_tree_parent.hpp:139`

### `_exists_impl`

```cpp
bool d2::VecParentElement::_exists_impl(const std::string &name) const override
```

Source: `core/tree/d2_tree_parent.hpp:140`

### `_exists_impl`

```cpp
bool d2::VecParentElement::_exists_impl(ptr ptr) const override
```

Source: `core/tree/d2_tree_parent.hpp:141`

### `_at_impl`

```cpp
TreeIter d2::VecParentElement::_at_impl(id id) const override
```

Source: `core/tree/d2_tree_parent.hpp:143`

### `_create_impl`

```cpp
TreeIter d2::VecParentElement::_create_impl(ptr ptr) override
```

Source: `core/tree/d2_tree_parent.hpp:144`

### `_override_impl`

```cpp
TreeIter d2::VecParentElement::_override_impl(ptr ptr) override
```

Source: `core/tree/d2_tree_parent.hpp:145`

### `_create_after_impl`

```cpp
TreeIter d2::VecParentElement::_create_after_impl(ptr p, id after) override
```

Source: `core/tree/d2_tree_parent.hpp:146`

### `_override_after_impl`

```cpp
TreeIter d2::VecParentElement::_override_after_impl(ptr p, id after) override
```

Source: `core/tree/d2_tree_parent.hpp:147`

### `_remove_impl`

```cpp
bool d2::VecParentElement::_remove_impl(id id) override
```

Source: `core/tree/d2_tree_parent.hpp:148`

### `_extract_impl`

```cpp
VecParentElement::ptr d2::VecParentElement::_extract_impl(id id) override
```

Source: `core/tree/d2_tree_parent.hpp:149`

### `_clear_impl`

```cpp
void d2::VecParentElement::_clear_impl() override
```

Source: `core/tree/d2_tree_parent.hpp:150`

### `_find`

```cpp
std::size_t d2::VecParentElement::_find(const std::string &name) const
```

Source: `core/tree/d2_tree_parent.hpp:152`

### `_find`

```cpp
std::size_t d2::VecParentElement::_find(ptr ptr) const
```

Source: `core/tree/d2_tree_parent.hpp:153`

### `begin`

```cpp
VecParentElement::DynamicIterator d2::VecParentElement::begin() override
```

Source: `core/tree/d2_tree_parent.hpp:157`

### `end`

```cpp
VecParentElement::DynamicIterator d2::VecParentElement::end() override
```

Source: `core/tree/d2_tree_parent.hpp:158`

### `foreach_internal`

```cpp
void d2::VecParentElement::foreach_internal(foreach_internal_callback callback) const override
```

Source: `core/tree/d2_tree_parent.hpp:159`

### `foreach`

```cpp
void d2::VecParentElement::foreach(foreach_callback callback) const override
```

Source: `core/tree/d2_tree_parent.hpp:160`


## Variables

### `_elements`

```cpp
std::vector<ptr> d2::VecParentElement::_elements
```

Source: `core/tree/d2_tree_parent.hpp:135`
