# `d2::TreeIter`

**Kind:** class
**Declared in:** `core/tree/d2_tree_element_frwd.hpp:14`

## Functions

### `TreeIter`

```cpp
d2::TreeIter< Type >::TreeIter()=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:23`

### `TreeIter`

```cpp
d2::TreeIter< Type >::TreeIter(std::nullptr_t)
```

Source: `core/tree/d2_tree_element_frwd.hpp:24`

### `TreeIter`

```cpp
d2::TreeIter< Type >::TreeIter(const TreeIter &)=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:25`

### `TreeIter`

```cpp
d2::TreeIter< Type >::TreeIter(TreeIter &&)=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:26`

### `TreeIter`

```cpp
d2::TreeIter< Type >::TreeIter(std::weak_ptr< Type > ptr)
```

Source: `core/tree/d2_tree_element_frwd.hpp:27`

### `TreeIter`

```cpp
template<typename Other>
d2::TreeIter< Type >::TreeIter(TreeIter< Other > ptr)
```

Source: `core/tree/d2_tree_element_frwd.hpp:28`

### `TreeIter`

```cpp
template<typename Other>
d2::TreeIter< Type >::TreeIter(std::shared_ptr< Other > ptr)
```

Source: `core/tree/d2_tree_element_frwd.hpp:29`

### `TreeIter`

```cpp
template<typename Other>
d2::TreeIter< Type >::TreeIter(std::weak_ptr< Other > ptr)
```

Source: `core/tree/d2_tree_element_frwd.hpp:30`

### `shared`

```cpp
std::shared_ptr< Type > d2::TreeIter< Type >::shared() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:41`

### `weak`

```cpp
std::weak_ptr< Type > d2::TreeIter< Type >::weak() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:42`

### `is_type`

```cpp
template<typename Other>
bool d2::TreeIter< Type >::is_type() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:45`

### `is_type_of`

```cpp
bool d2::TreeIter< Type >::is_type_of(TreeIter<> other) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:51`

### `as`

```cpp
template<typename Other = Element>
TreeIter< Other > d2::TreeIter< Type >::as() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:58`

### `asp`

```cpp
TreeIter< ParentElement > d2::TreeIter< Type >::asp() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:65`

### `up`

```cpp
TreeIter< ParentElement > d2::TreeIter< Type >::up() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:67`

### `up`

```cpp
TreeIter< ParentElement > d2::TreeIter< Type >::up(std::size_t cnt) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:71`

### `up`

```cpp
TreeIter< ParentElement > d2::TreeIter< Type >::up(const std::string &name) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:77`

### `foreach`

```cpp
void d2::TreeIter< Type >::foreach(foreach_callback callback) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:89`

### `operator==`

```cpp
bool d2::TreeIter< Type >::operator==(const TreeIter &other) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:91`

### `operator!=`

```cpp
bool d2::TreeIter< Type >::operator!=(const TreeIter &other) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:96`

### `operator->`

```cpp
std::shared_ptr< Type > d2::TreeIter< Type >::operator->() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:98`

### `operator*`

```cpp
Type & d2::TreeIter< Type >::operator*() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:99`

### `operator/`

```cpp
TreeIter d2::TreeIter< Type >::operator/(const std::string &path) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:101`

### `operator^`

```cpp
TreeIter< ParentElement > d2::TreeIter< Type >::operator^(std::size_t cnt) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:102`

### `operator^`

```cpp
TreeIter< ParentElement > d2::TreeIter< Type >::operator^(const std::string &name) const
```

Source: `core/tree/d2_tree_element_frwd.hpp:103`

### `operator std::shared_ptr< Element >`

```cpp
d2::TreeIter< Type >::operator std::shared_ptr< Element >() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:105`

### `operator std::weak_ptr< Element >`

```cpp
d2::TreeIter< Type >::operator std::weak_ptr< Element >() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:106`

### `operator std::shared_ptr< Type >`

```cpp
d2::TreeIter< Type >::operator std::shared_ptr< Type >() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:108`

### `operator std::weak_ptr< Type >`

```cpp
d2::TreeIter< Type >::operator std::weak_ptr< Type >() const
```

Source: `core/tree/d2_tree_element_frwd.hpp:109`

### `operator=`

```cpp
TreeIter & d2::TreeIter< Type >::operator=(const TreeIter &)=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:111`

### `operator=`

```cpp
TreeIter & d2::TreeIter< Type >::operator=(TreeIter &&)=default
```

Source: `core/tree/d2_tree_element_frwd.hpp:112`


## Typedefs

### `type`

```cpp
using d2::TreeIter< Type >::type = Type
```

Source: `core/tree/d2_tree_element_frwd.hpp:18`

### `foreach_callback`

```cpp
using d2::TreeIter< Type >::foreach_callback = std::function<bool(TreeIter<>)>
```

Source: `core/tree/d2_tree_element_frwd.hpp:19`
