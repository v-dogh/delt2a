# `d2::VecParentElement::LinearIteratorAdaptor`

**Kind:** struct
**Declared in:** `core/tree/d2_tree_parent.cpp:362`

**Bases:** `d2::internal::DynamicIterator::DynamicIteratorAdaptor`

## Functions

### `LinearIteratorAdaptor`

```cpp
d2::VecParentElement::LinearIteratorAdaptor::LinearIteratorAdaptor()=default
```

Source: `core/tree/d2_tree_parent.cpp:365`

### `LinearIteratorAdaptor`

```cpp
d2::VecParentElement::LinearIteratorAdaptor::LinearIteratorAdaptor(const LinearIteratorAdaptor &)=default
```

Source: `core/tree/d2_tree_parent.cpp:366`

### `LinearIteratorAdaptor`

```cpp
d2::VecParentElement::LinearIteratorAdaptor::LinearIteratorAdaptor(LinearIteratorAdaptor &&)=default
```

Source: `core/tree/d2_tree_parent.cpp:367`

### `LinearIteratorAdaptor`

```cpp
d2::VecParentElement::LinearIteratorAdaptor::LinearIteratorAdaptor(std::vector< ParentElement::ptr >::iterator it)
```

Source: `core/tree/d2_tree_parent.cpp:368`

### `ptr`

```cpp
auto d2::VecParentElement::LinearIteratorAdaptor::ptr(std::shared_ptr< ParentElement > elem) const
```

Source: `core/tree/d2_tree_parent.cpp:372`

### `value`

```cpp
TreeIter d2::VecParentElement::LinearIteratorAdaptor::value(std::shared_ptr< ParentElement > elem) const
```

Source: `core/tree/d2_tree_parent.cpp:377`

### `increment`

```cpp
void d2::VecParentElement::LinearIteratorAdaptor::increment(std::shared_ptr< ParentElement > elem)
```

Source: `core/tree/d2_tree_parent.cpp:381`

### `decrement`

```cpp
void d2::VecParentElement::LinearIteratorAdaptor::decrement(std::shared_ptr< ParentElement > elem)
```

Source: `core/tree/d2_tree_parent.cpp:386`

### `is_null`

```cpp
bool d2::VecParentElement::LinearIteratorAdaptor::is_null(std::shared_ptr< ParentElement > elem) const
```

Source: `core/tree/d2_tree_parent.cpp:391`

### `is_begin`

```cpp
bool d2::VecParentElement::LinearIteratorAdaptor::is_begin(std::shared_ptr< ParentElement > elem) const
```

Source: `core/tree/d2_tree_parent.cpp:395`

### `is_end`

```cpp
bool d2::VecParentElement::LinearIteratorAdaptor::is_end(std::shared_ptr< ParentElement > elem) const
```

Source: `core/tree/d2_tree_parent.cpp:399`

### `is_equal`

```cpp
bool d2::VecParentElement::LinearIteratorAdaptor::is_equal(DynamicIteratorAdaptor *adapter, std::shared_ptr< ParentElement > elem) const
```

Source: `core/tree/d2_tree_parent.cpp:403`

### `clone`

```cpp
std::unique_ptr< DynamicIteratorAdaptor > d2::VecParentElement::LinearIteratorAdaptor::clone() const
```

Source: `core/tree/d2_tree_parent.cpp:407`


## Variables

### `current`

```cpp
std::vector<ParentElement::ptr>::iterator d2::VecParentElement::LinearIteratorAdaptor::current
```

Source: `core/tree/d2_tree_parent.cpp:370`
