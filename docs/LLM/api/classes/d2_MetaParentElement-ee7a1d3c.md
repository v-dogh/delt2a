# `d2::MetaParentElement`

**Kind:** class
**Declared in:** `core/tree/d2_tree_parent.hpp:162`

**Bases:** `d2::ParentElement`

## Functions

### `_index_of_impl`

```cpp
virtual int d2::MetaParentElement::_index_of_impl(id) const override
```

Source: `core/tree/d2_tree_parent.hpp:165`

### `_empty_impl`

```cpp
virtual bool d2::MetaParentElement::_empty_impl() const override
```

Source: `core/tree/d2_tree_parent.hpp:170`

### `_exists_impl`

```cpp
virtual bool d2::MetaParentElement::_exists_impl(const std::string &name) const override
```

Source: `core/tree/d2_tree_parent.hpp:174`

### `_exists_impl`

```cpp
virtual bool d2::MetaParentElement::_exists_impl(ptr ptr) const override
```

Source: `core/tree/d2_tree_parent.hpp:178`

### `_at_impl`

```cpp
virtual TreeIter d2::MetaParentElement::_at_impl(id id) const override
```

Source: `core/tree/d2_tree_parent.hpp:183`

### `_create_impl`

```cpp
virtual TreeIter d2::MetaParentElement::_create_impl(ptr ptr) override
```

Source: `core/tree/d2_tree_parent.hpp:187`

### `_override_impl`

```cpp
virtual TreeIter d2::MetaParentElement::_override_impl(ptr ptr) override
```

Source: `core/tree/d2_tree_parent.hpp:191`

### `_create_after_impl`

```cpp
virtual TreeIter d2::MetaParentElement::_create_after_impl(ptr p, id after) override
```

Source: `core/tree/d2_tree_parent.hpp:195`

### `_override_after_impl`

```cpp
virtual TreeIter d2::MetaParentElement::_override_after_impl(ptr p, id after) override
```

Source: `core/tree/d2_tree_parent.hpp:199`

### `_remove_impl`

```cpp
virtual bool d2::MetaParentElement::_remove_impl(id id) override
```

Source: `core/tree/d2_tree_parent.hpp:204`

### `_extract_impl`

```cpp
virtual ptr d2::MetaParentElement::_extract_impl(id id) override
```

Source: `core/tree/d2_tree_parent.hpp:208`

### `_clear_impl`

```cpp
virtual void d2::MetaParentElement::_clear_impl() override
```

Source: `core/tree/d2_tree_parent.hpp:212`

### `foreach_internal`

```cpp
virtual void d2::MetaParentElement::foreach_internal(foreach_internal_callback callback) const override
```

Source: `core/tree/d2_tree_parent.hpp:216`

### `foreach`

```cpp
virtual void d2::MetaParentElement::foreach(foreach_callback callback) const override
```

Source: `core/tree/d2_tree_parent.hpp:217`
