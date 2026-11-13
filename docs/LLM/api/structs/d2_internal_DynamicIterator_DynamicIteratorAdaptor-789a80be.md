# `d2::internal::DynamicIterator::DynamicIteratorAdaptor`

**Kind:** struct
**Declared in:** `core/tree/d2_tree_element_frwd.hpp:120`

## Functions

### `DynamicIteratorAdaptor`

```cpp
d2::internal::DynamicIterator::DynamicIteratorAdaptor::DynamicIteratorAdaptor()=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:122`

### `DynamicIteratorAdaptor`

```cpp
d2::internal::DynamicIterator::DynamicIteratorAdaptor::DynamicIteratorAdaptor(const DynamicIteratorAdaptor &)=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:123`

### `DynamicIteratorAdaptor`

```cpp
d2::internal::DynamicIterator::DynamicIteratorAdaptor::DynamicIteratorAdaptor(DynamicIteratorAdaptor &&)=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:124`

### `~DynamicIteratorAdaptor`

```cpp
virtual d2::internal::DynamicIterator::DynamicIteratorAdaptor::~DynamicIteratorAdaptor()=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:125`

### `increment`

```cpp
void d2::internal::DynamicIterator::DynamicIteratorAdaptor::increment(int cnt, std::shared_ptr< ParentElement > elem)
```

Source: `core/tree/d2_tree_element_frwd.hpp:127`

### `decrement`

```cpp
void d2::internal::DynamicIterator::DynamicIteratorAdaptor::decrement(int cnt, std::shared_ptr< ParentElement > elem)
```

Source: `core/tree/d2_tree_element_frwd.hpp:132`

### `value`

```cpp
virtual TreeIter d2::internal::DynamicIterator::DynamicIteratorAdaptor::value(std::shared_ptr< ParentElement > elem) const =0
```

Source: `core/tree/d2_tree_element_frwd.hpp:138`

### `increment`

```cpp
virtual void d2::internal::DynamicIterator::DynamicIteratorAdaptor::increment(std::shared_ptr< ParentElement > elem)=0
```

Source: `core/tree/d2_tree_element_frwd.hpp:139`

### `decrement`

```cpp
virtual void d2::internal::DynamicIterator::DynamicIteratorAdaptor::decrement(std::shared_ptr< ParentElement > elem)=0
```

Source: `core/tree/d2_tree_element_frwd.hpp:140`

### `is_null`

```cpp
virtual bool d2::internal::DynamicIterator::DynamicIteratorAdaptor::is_null(std::shared_ptr< ParentElement > elem) const =0
```

Source: `core/tree/d2_tree_element_frwd.hpp:141`

### `is_begin`

```cpp
virtual bool d2::internal::DynamicIterator::DynamicIteratorAdaptor::is_begin(std::shared_ptr< ParentElement > elem) const =0
```

Source: `core/tree/d2_tree_element_frwd.hpp:142`

### `is_end`

```cpp
virtual bool d2::internal::DynamicIterator::DynamicIteratorAdaptor::is_end(std::shared_ptr< ParentElement > elem) const =0
```

Source: `core/tree/d2_tree_element_frwd.hpp:143`

### `is_equal`

```cpp
virtual bool d2::internal::DynamicIterator::DynamicIteratorAdaptor::is_equal(DynamicIteratorAdaptor *adapter, std::shared_ptr< ParentElement > elem) const =0
```

Source: `core/tree/d2_tree_element_frwd.hpp:144`

### `clone`

```cpp
virtual std::unique_ptr< DynamicIteratorAdaptor > d2::internal::DynamicIterator::DynamicIteratorAdaptor::clone() const =0
```

Source: `core/tree/d2_tree_element_frwd.hpp:145`
