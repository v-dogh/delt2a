# `d2::ParentElement`

**Kind:** class
**Declared in:** `core/tree/d2_tree_parent.hpp:8`

**Bases:** `d2::Element`

## Enums

### `BorderType`

```cpp
enum BorderType
```

Source: `core/tree/d2_tree_parent.hpp:16`

Values: `Top`, `Bottom`, `Left`, `Right`


## Functions

### `_layout_for_impl`

```cpp
void d2::ParentElement::_layout_for_impl(enum Layout, cptr) const
```

Source: `core/tree/d2_tree_parent.hpp:25`

### `_get_border_impl`

```cpp
virtual int d2::ParentElement::_get_border_impl(BorderType, cptr) const
```

Source: `core/tree/d2_tree_parent.hpp:27`

### `_signal_context_change_impl`

```cpp
void d2::ParentElement::_signal_context_change_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `core/tree/d2_tree_parent.hpp:32`

### `_state_change_impl`

```cpp
void d2::ParentElement::_state_change_impl(State state, bool value) override
```

Source: `core/tree/d2_tree_parent.hpp:33`

### `_index_of_impl`

```cpp
virtual int d2::ParentElement::_index_of_impl(id) const =0
```

Source: `core/tree/d2_tree_parent.hpp:35`

### `_empty_impl`

```cpp
virtual bool d2::ParentElement::_empty_impl() const =0
```

Source: `core/tree/d2_tree_parent.hpp:37`

### `_exists_impl`

```cpp
virtual bool d2::ParentElement::_exists_impl(const std::string &) const =0
```

Source: `core/tree/d2_tree_parent.hpp:38`

### `_exists_impl`

```cpp
virtual bool d2::ParentElement::_exists_impl(ptr) const =0
```

Source: `core/tree/d2_tree_parent.hpp:39`

### `_at_impl`

```cpp
virtual TreeIter d2::ParentElement::_at_impl(id id) const =0
```

Source: `core/tree/d2_tree_parent.hpp:41`

### `_create_impl`

```cpp
virtual TreeIter d2::ParentElement::_create_impl(ptr ptr)=0
```

Source: `core/tree/d2_tree_parent.hpp:42`

### `_override_impl`

```cpp
virtual TreeIter d2::ParentElement::_override_impl(ptr ptr)=0
```

Source: `core/tree/d2_tree_parent.hpp:43`

### `_create_after_impl`

```cpp
virtual TreeIter d2::ParentElement::_create_after_impl(ptr p, id after)=0
```

Source: `core/tree/d2_tree_parent.hpp:44`

### `_override_after_impl`

```cpp
virtual TreeIter d2::ParentElement::_override_after_impl(ptr p, id after)=0
```

Source: `core/tree/d2_tree_parent.hpp:45`

### `_remove_impl`

```cpp
virtual bool d2::ParentElement::_remove_impl(id id)=0
```

Source: `core/tree/d2_tree_parent.hpp:46`

### `_extract_impl`

```cpp
virtual ptr d2::ParentElement::_extract_impl(id id)=0
```

Source: `core/tree/d2_tree_parent.hpp:47`

### `_clear_impl`

```cpp
virtual void d2::ParentElement::_clear_impl()=0
```

Source: `core/tree/d2_tree_parent.hpp:48`

### `_setparent_of`

```cpp
void d2::ParentElement::_setparent_of(ptr ptr)
```

Source: `core/tree/d2_tree_parent.hpp:50`

### `_insert_setstate`

```cpp
void d2::ParentElement::_insert_setstate(ptr ptr)
```

Source: `core/tree/d2_tree_parent.hpp:51`

### `_extract_setstate`

```cpp
void d2::ParentElement::_extract_setstate(ptr ptr)
```

Source: `core/tree/d2_tree_parent.hpp:52`

### `_remove_setstate`

```cpp
void d2::ParentElement::_remove_setstate(ptr ptr)
```

Source: `core/tree/d2_tree_parent.hpp:53`

### `empty`

```cpp
bool d2::ParentElement::empty() const
```

Source: `core/tree/d2_tree_parent.hpp:60`

### `exists`

```cpp
bool d2::ParentElement::exists(const std::string &name) const
```

Source: `core/tree/d2_tree_parent.hpp:61`

### `exists`

```cpp
bool d2::ParentElement::exists(ptr ptr) const
```

Source: `core/tree/d2_tree_parent.hpp:62`

### `index_of`

```cpp
int d2::ParentElement::index_of(id id)
```

Source: `core/tree/d2_tree_parent.hpp:64`

### `at`

```cpp
template<typename Type>
TreeIter< Type > d2::ParentElement::at(id id)
```

Source: `core/tree/d2_tree_parent.hpp:66`

### `at`

```cpp
TreeIter d2::ParentElement::at(id id) const
```

Source: `core/tree/d2_tree_parent.hpp:70`

### `layout_for`

```cpp
void d2::ParentElement::layout_for(enum Layout layout, cptr elem) const
```

Source: `core/tree/d2_tree_parent.hpp:72`

### `resolve_units`

```cpp
int d2::ParentElement::resolve_units(Unit, cptr) const
```

Source: `core/tree/d2_tree_parent.hpp:74`

### `create`

```cpp
template<typename Type>
TreeIter< Type > d2::ParentElement::create(const std::string &name="")
```

Source: `core/tree/d2_tree_parent.hpp:77`

### `override`

```cpp
template<typename Type>
TreeIter< Type > d2::ParentElement::override(const std::string &name="")
```

Source: `core/tree/d2_tree_parent.hpp:81`

### `create`

```cpp
TreeIter d2::ParentElement::create(ptr ptr)
```

Source: `core/tree/d2_tree_parent.hpp:85`

### `override`

```cpp
TreeIter d2::ParentElement::override(ptr ptr)
```

Source: `core/tree/d2_tree_parent.hpp:86`

### `create_after`

```cpp
template<typename Type>
TreeIter< Type > d2::ParentElement::create_after(id after, const std::string &name="")
```

Source: `core/tree/d2_tree_parent.hpp:88`

### `override_after`

```cpp
template<typename Type>
TreeIter< Type > d2::ParentElement::override_after(id after, const std::string &name="")
```

Source: `core/tree/d2_tree_parent.hpp:98`

### `create_after`

```cpp
TreeIter d2::ParentElement::create_after(id after, ptr p)
```

Source: `core/tree/d2_tree_parent.hpp:107`

### `override_after`

```cpp
TreeIter d2::ParentElement::override_after(id after, ptr p)
```

Source: `core/tree/d2_tree_parent.hpp:108`

### `extract`

```cpp
ParentElement::ptr d2::ParentElement::extract(id id)
```

Source: `core/tree/d2_tree_parent.hpp:110`

### `clear`

```cpp
void d2::ParentElement::clear()
```

Source: `core/tree/d2_tree_parent.hpp:112`

### `remove`

```cpp
void d2::ParentElement::remove(id id)
```

Source: `core/tree/d2_tree_parent.hpp:114`

### `remove_if`

```cpp
bool d2::ParentElement::remove_if(id id)
```

Source: `core/tree/d2_tree_parent.hpp:115`

### `border_for`

```cpp
int d2::ParentElement::border_for(BorderType type, cptr ptr) const
```

Source: `core/tree/d2_tree_parent.hpp:117`

### `begin`

```cpp
virtual DynamicIterator d2::ParentElement::begin()
```

Source: `core/tree/d2_tree_parent.hpp:119`

### `end`

```cpp
virtual DynamicIterator d2::ParentElement::end()
```

Source: `core/tree/d2_tree_parent.hpp:123`

### `foreach_internal`

```cpp
virtual void d2::ParentElement::foreach_internal(foreach_internal_callback callback) const =0
```

Source: `core/tree/d2_tree_parent.hpp:127`

### `foreach`

```cpp
virtual void d2::ParentElement::foreach(foreach_callback callback) const =0
```

Source: `core/tree/d2_tree_parent.hpp:128`

### `Element`

```cpp
d2::Element::Element()=default
```

Source: `core/tree/d2_tree_parent.hpp:55`

### `Element`

```cpp
d2::Element::Element(const std::string &name, TreeState::ptr state)
```

Source: `core/tree/d2_tree_parent.hpp:55`

### `Element`

```cpp
d2::Element::Element(Element &&)=delete
```

Source: `core/tree/d2_tree_parent.hpp:55`

### `Element`

```cpp
d2::Element::Element(const Element &)=delete
```

Source: `core/tree/d2_tree_parent.hpp:55`

### `extract`

```cpp
Element::ptr d2::Element::extract()
```

Source: `core/tree/d2_tree_parent.hpp:57`

### `remove`

```cpp
void d2::Element::remove()
```

Source: `core/tree/d2_tree_parent.hpp:58`

### `resolve_units`

```cpp
int d2::Element::resolve_units(Unit unit) const
```

Source: `core/tree/d2_tree_parent.hpp:75`


## Typedefs

### `DynamicIterator`

```cpp
using d2::ParentElement::DynamicIterator = internal::DynamicIterator
```

Source: `core/tree/d2_tree_parent.hpp:15`

### `id`

```cpp
using d2::ParentElement::id = std::variant<ptr, int, std::string>
```

Source: `core/tree/d2_tree_parent.hpp:23`


## Variables

### `overlap`

```cpp
auto d2::ParentElement::overlap
```

Source: `core/tree/d2_tree_parent.hpp:12`

### `underlap`

```cpp
auto d2::ParentElement::underlap
```

Source: `core/tree/d2_tree_parent.hpp:13`
