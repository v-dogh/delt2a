# `d2::Element::EventListener`

**Kind:** class
**Declared in:** `core/tree/d2_tree_element.hpp:264`

## Functions

### `EventListener`

```cpp
d2::Element::EventListener::EventListener()=default
```

Source: `core/tree/d2_tree_element.hpp:269`

### `EventListener`

```cpp
d2::Element::EventListener::EventListener(EventListener &&)=default
```

Source: `core/tree/d2_tree_element.hpp:270`

### `EventListener`

```cpp
d2::Element::EventListener::EventListener(const EventListener &)=default
```

Source: `core/tree/d2_tree_element.hpp:271`

### `EventListener`

```cpp
d2::Element::EventListener::EventListener(EventListenerState::ptr ptr)
```

Source: `core/tree/d2_tree_element.hpp:272`

### `event`

```cpp
State d2::Element::EventListener::event() const
```

Source: `core/tree/d2_tree_element.hpp:274`

### `index`

```cpp
std::size_t d2::Element::EventListener::index() const
```

Source: `core/tree/d2_tree_element.hpp:280`

### `unmute`

```cpp
void d2::Element::EventListener::unmute()
```

Source: `core/tree/d2_tree_element.hpp:287`

### `mute`

```cpp
void d2::Element::EventListener::mute()
```

Source: `core/tree/d2_tree_element.hpp:294`

### `destroy`

```cpp
void d2::Element::EventListener::destroy()
```

Source: `core/tree/d2_tree_element.hpp:301`

### `operator bool`

```cpp
d2::Element::EventListener::operator bool() const
```

Source: `core/tree/d2_tree_element.hpp:309`

### `operator==`

```cpp
bool d2::Element::EventListener::operator==(std::nullptr_t) const
```

Source: `core/tree/d2_tree_element.hpp:314`

### `operator!=`

```cpp
bool d2::Element::EventListener::operator!=(std::nullptr_t) const
```

Source: `core/tree/d2_tree_element.hpp:318`

### `operator=`

```cpp
EventListener & d2::Element::EventListener::operator=(EventListener &&)=default
```

Source: `core/tree/d2_tree_element.hpp:323`

### `operator=`

```cpp
EventListener & d2::Element::EventListener::operator=(const EventListener &)=default
```

Source: `core/tree/d2_tree_element.hpp:324`
