# `d2::internal::ElementView`

**Kind:** struct
**Declared in:** `core/tree/d2_tree_element.hpp:546`

## Functions

### `from`

```cpp
ElementView d2::internal::ElementView::from(Element::ptr ptr)
```

Source: `core/tree/d2_tree_element.hpp:552`

### `signal_context_change_sub`

```cpp
void d2::internal::ElementView::signal_context_change_sub(Element::write_flag type, unsigned int prop, Element::ptr element)
```

Source: `core/tree/d2_tree_element.hpp:554`

### `signal_context_change_sub`

```cpp
void d2::internal::ElementView::signal_context_change_sub(Element::write_flag type, unsigned int prop)
```

Source: `core/tree/d2_tree_element.hpp:557`

### `signal_context_change`

```cpp
void d2::internal::ElementView::signal_context_change(Element::write_flag type, unsigned int prop, Element::ptr element)
```

Source: `core/tree/d2_tree_element.hpp:558`

### `signal_context_change`

```cpp
void d2::internal::ElementView::signal_context_change(Element::write_flag type=Element::WriteType::Masked, unsigned int prop=~0u)
```

Source: `core/tree/d2_tree_element.hpp:561`

### `signal_write`

```cpp
void d2::internal::ElementView::signal_write(Element::write_flag type=Element::WriteType::Masked, unsigned int prop=~0u)
```

Source: `core/tree/d2_tree_element.hpp:564`

### `signal_initialization`

```cpp
void d2::internal::ElementView::signal_initialization(unsigned int prop)
```

Source: `core/tree/d2_tree_element.hpp:567`

### `signal_write_update`

```cpp
void d2::internal::ElementView::signal_write_update(Element::write_flag type) const
```

Source: `core/tree/d2_tree_element.hpp:568`

### `signal_update`

```cpp
void d2::internal::ElementView::signal_update(Element::internal_flag type) const
```

Source: `core/tree/d2_tree_element.hpp:569`

### `register_bind`

```cpp
void d2::internal::ElementView::register_bind(style::uai_property prop, style::DependencyHandle handle)
```

Source: `core/tree/d2_tree_element.hpp:570`

### `deregister_bind`

```cpp
void d2::internal::ElementView::deregister_bind(style::uai_property prop)
```

Source: `core/tree/d2_tree_element.hpp:571`

### `trigger_event`

```cpp
void d2::internal::ElementView::trigger_event(in::InputFrame &frame, bool recursive)
```

Source: `core/tree/d2_tree_element.hpp:572`

### `setparent`

```cpp
void d2::internal::ElementView::setparent(Element::pptr ptr)
```

Source: `core/tree/d2_tree_element.hpp:573`

### `clone_state_if`

```cpp
void d2::internal::ElementView::clone_state_if(Element::ptr parent)
```

Source: `core/tree/d2_tree_element.hpp:574`
