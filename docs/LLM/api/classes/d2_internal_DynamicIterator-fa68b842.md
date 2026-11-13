# `d2::internal::DynamicIterator`

**Kind:** class
**Declared in:** `core/tree/d2_tree_element_frwd.hpp:117`

## Functions

### `make`

```cpp
template<typename Adaptor, typename... Argv>
static auto d2::internal::DynamicIterator::make(std::weak_ptr< ParentElement > ptr, Argv &&... args)
```

Source: `core/tree/d2_tree_element_frwd.hpp:152`

### `DynamicIterator`

```cpp
d2::internal::DynamicIterator::DynamicIterator()=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:159`

### `DynamicIterator`

```cpp
d2::internal::DynamicIterator::DynamicIterator(std::nullptr_t)
```

Source: `core/tree/d2_tree_element_frwd.hpp:160`

### `DynamicIterator`

```cpp
d2::internal::DynamicIterator::DynamicIterator(const DynamicIterator &copy)
```

Source: `core/tree/d2_tree_element_frwd.hpp:161`

### `DynamicIterator`

```cpp
d2::internal::DynamicIterator::DynamicIterator(DynamicIterator &&)=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:163`

### `DynamicIterator`

```cpp
d2::internal::DynamicIterator::DynamicIterator(std::weak_ptr< ParentElement > ptr, std::unique_ptr< DynamicIteratorAdaptor > adaptor)
```

Source: `core/tree/d2_tree_element_frwd.hpp:164`

### `increment`

```cpp
void d2::internal::DynamicIterator::increment(int cnt=1)
```

Source: `core/tree/d2_tree_element_frwd.hpp:167`

### `decrement`

```cpp
void d2::internal::DynamicIterator::decrement(int cnt=1)
```

Source: `core/tree/d2_tree_element_frwd.hpp:168`

### `is_begin`

```cpp
bool d2::internal::DynamicIterator::is_begin() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:170`

### `is_end`

```cpp
bool d2::internal::DynamicIterator::is_end() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:171`

### `is_null`

```cpp
bool d2::internal::DynamicIterator::is_null() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:172`

### `is_equal`

```cpp
bool d2::internal::DynamicIterator::is_equal(DynamicIterator it) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:173`

### `value`

```cpp
TreeIter d2::internal::DynamicIterator::value() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:175`

### `operator->`

```cpp
TreeIter d2::internal::DynamicIterator::operator->() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:177`

### `operator*`

```cpp
Element & d2::internal::DynamicIterator::operator*() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:178`

### `operator==`

```cpp
bool d2::internal::DynamicIterator::operator==(const DynamicIterator &other) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:180`

### `operator!=`

```cpp
bool d2::internal::DynamicIterator::operator!=(const DynamicIterator &other) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:181`

### `operator=`

```cpp
DynamicIterator & d2::internal::DynamicIterator::operator=(const DynamicIterator &copy)
```

Source: `core/tree/d2_tree_element_frwd.hpp:183`

### `operator=`

```cpp
DynamicIterator & d2::internal::DynamicIterator::operator=(DynamicIterator &&)=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:184`
