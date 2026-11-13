# `d2::Element::EventListenerState`

**Kind:** class
**Declared in:** `core/tree/d2_tree_element.hpp:196`

**Bases:** `std::enable_shared_from_this< EventListenerState >`

## Enums

### `Mode`

```cpp
enum Mode
```

Source: `core/tree/d2_tree_element.hpp:201`

Values: `Active`, `Muted`, `Destroyed`

### `Dep`

```cpp
enum Dep
```

Source: `core/tree/d2_tree_element.hpp:207`

Values: `On`, `Off`, `Any`


## Functions

### `EventListenerState`

```cpp
d2::Element::EventListenerState::EventListenerState()=default
```

Source: `core/tree/d2_tree_element.hpp:221`

### `EventListenerState`

```cpp
d2::Element::EventListenerState::EventListenerState(EventListenerState &&)=delete
```

Source: `core/tree/d2_tree_element.hpp:222`

### `EventListenerState`

```cpp
d2::Element::EventListenerState::EventListenerState(const EventListenerState &)=default
```

Source: `core/tree/d2_tree_element.hpp:223`

### `EventListenerState`

```cpp
template<typename Func>
d2::Element::EventListenerState::EventListenerState(Element::ptr ptr, std::size_t idx, Dep value, State ev, Func callback)
```

Source: `core/tree/d2_tree_element.hpp:225`

### `state`

```cpp
Element::EventListenerState::Mode d2::Element::EventListenerState::state() const
```

Source: `core/tree/d2_tree_element.hpp:233`

### `event`

```cpp
Element::State d2::Element::EventListenerState::event() const
```

Source: `core/tree/d2_tree_element.hpp:234`

### `value`

```cpp
Element::EventListenerState::Dep d2::Element::EventListenerState::value() const
```

Source: `core/tree/d2_tree_element.hpp:235`

### `index`

```cpp
std::size_t d2::Element::EventListenerState::index() const
```

Source: `core/tree/d2_tree_element.hpp:236`

### `setstate`

```cpp
void d2::Element::EventListenerState::setstate(Mode state)
```

Source: `core/tree/d2_tree_element.hpp:238`

### `unmute`

```cpp
void d2::Element::EventListenerState::unmute()
```

Source: `core/tree/d2_tree_element.hpp:240`

### `mute`

```cpp
void d2::Element::EventListenerState::mute()
```

Source: `core/tree/d2_tree_element.hpp:241`

### `destroy`

```cpp
void d2::Element::EventListenerState::destroy()
```

Source: `core/tree/d2_tree_element.hpp:242`

### `invoke`

```cpp
template<typename... Argv>
void d2::Element::EventListenerState::invoke(Argv &&... args)
```

Source: `core/tree/d2_tree_element.hpp:244`

### `operator bool`

```cpp
d2::Element::EventListenerState::operator bool() const
```

Source: `core/tree/d2_tree_element.hpp:246`

### `operator==`

```cpp
bool d2::Element::EventListenerState::operator==(std::nullptr_t) const
```

Source: `core/tree/d2_tree_element.hpp:251`

### `operator!=`

```cpp
bool d2::Element::EventListenerState::operator!=(std::nullptr_t) const
```

Source: `core/tree/d2_tree_element.hpp:255`

### `operator=`

```cpp
EventListenerState & d2::Element::EventListenerState::operator=(EventListenerState &&)=default
```

Source: `core/tree/d2_tree_element.hpp:260`

### `operator=`

```cpp
EventListenerState & d2::Element::EventListenerState::operator=(const EventListenerState &)=default
```

Source: `core/tree/d2_tree_element.hpp:261`


## Typedefs

### `ptr`

```cpp
using d2::Element::EventListenerState::ptr = std::shared_ptr<EventListenerState>
```

Source: `core/tree/d2_tree_element.hpp:199`

### `wptr`

```cpp
using d2::Element::EventListenerState::wptr = std::weak_ptr<EventListenerState>
```

Source: `core/tree/d2_tree_element.hpp:200`
