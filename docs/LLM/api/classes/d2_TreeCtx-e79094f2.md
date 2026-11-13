# `d2::TreeCtx`

**Kind:** class
**Declared in:** `core/tree/d2_tree_construct.hpp:266`

**Bases:** `d2::TreeCtxBase`

## Functions

### `TreeCtx`

```cpp
template<typename Other>
d2::TreeCtx< Parent, State >::TreeCtx(d2::TreeIter< Other > ptr)
```

Source: `core/tree/d2_tree_construct.hpp:275`

### `TreeCtx`

```cpp
template<typename Other>
d2::TreeCtx< Parent, State >::TreeCtx(std::shared_ptr< Other > ptr)
```

Source: `core/tree/d2_tree_construct.hpp:277`

### `ptr`

```cpp
TreeIter< Parent > d2::TreeCtx< Parent, State >::ptr() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:279`

### `state`

```cpp
tree_state_ptr< State > d2::TreeCtx< Parent, State >::state() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:283`

### `clone`

```cpp
template<typename Type>
void d2::TreeCtx< Parent, State >::clone(TreeIter< Type > ptr, auto &&callback) const
```

Source: `core/tree/d2_tree_construct.hpp:288`

### `anchor`

```cpp
template<typename Type, typename... Argv>
auto d2::TreeCtx< Parent, State >::anchor(std::string name="", Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:294`

### `animate`

```cpp
template<template< typename, Element::property > typename Interpolator, Element::property Prop, typename Func, typename... Argv>
Animation::ptr d2::TreeCtx< Parent, State >::animate(std::chrono::milliseconds time, Func &&callback, Argv &&... args)
```

Source: `core/tree/d2_tree_construct.hpp:311`

### `animate`

```cpp
template<template< typename, Element::property > typename Interpolator, Element::property Prop, typename... Argv>
Animation::ptr d2::TreeCtx< Parent, State >::animate(std::chrono::milliseconds time, Argv &&... args)
```

Source: `core/tree/d2_tree_construct.hpp:327`

### `animate_twoway`

```cpp
template<template< typename, Element::property > typename Interpolator, Element::property Prop, typename... Argv>
Element::EventListener d2::TreeCtx< Parent, State >::animate_twoway(std::chrono::milliseconds time, Argv &&... args)
```

Source: `core/tree/d2_tree_construct.hpp:339`

### `animate_twoway`

```cpp
template<template< typename, Element::property > typename InterpolatorTo, template< typename, Element::property > typename InterpolatorFrom, Element::property Prop, typename... Argv>
Element::EventListener d2::TreeCtx< Parent, State >::animate_twoway(std::chrono::milliseconds time_to, std::chrono::milliseconds time_from, Argv &&... args)
```

Source: `core/tree/d2_tree_construct.hpp:351`

### `animate_toggle`

```cpp
template<template< typename, Element::property > typename Interpolator, Element::property Prop, typename... Argv>
Element::EventListener d2::TreeCtx< Parent, State >::animate_toggle(std::chrono::milliseconds time, Argv &&... args)
```

Source: `core/tree/d2_tree_construct.hpp:377`

### `animate_toggle`

```cpp
template<template< typename, Element::property > typename InterpolatorTo, template< typename, Element::property > typename InterpolatorFrom, Element::property Prop, typename... Argv>
Element::EventListener d2::TreeCtx< Parent, State >::animate_toggle(std::chrono::milliseconds time_to, std::chrono::milliseconds time_from, Argv &&... args)
```

Source: `core/tree/d2_tree_construct.hpp:389`

### `embed`

```cpp
template<typename Func, typename... Arge, typename... Args>
auto d2::TreeCtx< Parent, State >::embed(const std::string &name, Func &&callback, std::tuple< Arge... > arge={}, std::tuple< Args... > args={}) const
```

Source: `core/tree/d2_tree_construct.hpp:420`

### `embed`

```cpp
template<typename Func, typename... Arge, typename... Args>
auto d2::TreeCtx< Parent, State >::embed(Func &&callback, std::tuple< Arge... > arge={}, std::tuple< Args... > args={}) const
```

Source: `core/tree/d2_tree_construct.hpp:448`

### `embed`

```cpp
template<typename Type, typename... Args, typename... Arge>
auto d2::TreeCtx< Parent, State >::embed(const std::string &name, std::tuple< Arge... > arge={}, std::tuple< Args... > args={}) const
```

Source: `core/tree/d2_tree_construct.hpp:453`

### `embed`

```cpp
template<typename Type, typename... Arge, typename... Args>
auto d2::TreeCtx< Parent, State >::embed(std::tuple< Arge... > arge={}, std::tuple< Args... > args={}) const
```

Source: `core/tree/d2_tree_construct.hpp:460`

### `elem`

```cpp
template<typename Func, typename... Argv>
auto d2::TreeCtx< Parent, State >::elem(Func &&callback, Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:469`

### `elem`

```cpp
template<typename Func, typename... Argv>
auto d2::TreeCtx< Parent, State >::elem(std::string name, Func &&callback, Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:482`

### `elem`

```cpp
template<typename Type, typename Func>
auto d2::TreeCtx< Parent, State >::elem(Element::eptr< Type > ptr, Func &&callback) const
```

Source: `core/tree/d2_tree_construct.hpp:495`

### `branch`

```cpp
template<typename Var, typename... Branch>
void d2::TreeCtx< Parent, State >::branch(Var &var, std::pair< typename Var::value_type, Branch >... branches)
```

Source: `core/tree/d2_tree_construct.hpp:506`

### `branch`

```cpp
template<typename Type, typename... Branch>
void d2::TreeCtx< Parent, State >::branch(const std::string &tree, const std::string &name, std::pair< Type, Branch >... branches)
```

Source: `core/tree/d2_tree_construct.hpp:535`

### `branch`

```cpp
template<typename Type, typename... Value, typename... Branch>
void d2::TreeCtx< Parent, State >::branch(const std::string &name, std::pair< Type, Branch >... branches)
```

Source: `core/tree/d2_tree_construct.hpp:543`

### `branch`

```cpp
template<auto Var, typename... Value, typename... Branch>
void d2::TreeCtx< Parent, State >::branch(std::pair< typename style::ThemeRegistry< decltype(Var)>::type, Branch >... branches)
```

Source: `core/tree/d2_tree_construct.hpp:550`

### `persist`

```cpp
void d2::TreeCtx< Parent, State >::persist(bool preserve_position=true, std::vector< std::string > whitelist={})
```

Source: `core/tree/d2_tree_construct.hpp:555`

### `apply`

```cpp
template<typename Style, typename... Argv>
void d2::TreeCtx< Parent, State >::apply(Argv &&... args) const
```

Source: `core/tree/d2_tree_construct.hpp:603`

### `operator->`

```cpp
TreeIter< Parent > d2::TreeCtx< Parent, State >::operator->() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:608`

### `operator TreeCtx< OtherRoot, OtherState >`

```cpp
template<typename OtherRoot, typename OtherState>
d2::TreeCtx< Parent, State >::operator TreeCtx< OtherRoot, OtherState >() const noexcept
```

Source: `core/tree/d2_tree_construct.hpp:614`

### `TreeCtxBase`

```cpp
d2::TreeCtxBase::TreeCtxBase(d2::TreeIter<> ptr)
```

Source: `core/tree/d2_tree_construct.hpp:272`

### `TreeCtxBase`

```cpp
d2::TreeCtxBase::TreeCtxBase(const TreeCtxBase &)=default
```

Source: `core/tree/d2_tree_construct.hpp:272`

### `TreeCtxBase`

```cpp
d2::TreeCtxBase::TreeCtxBase(TreeCtxBase &&)=default
```

Source: `core/tree/d2_tree_construct.hpp:272`


## Typedefs

### `element_type`

```cpp
using d2::TreeCtx< Parent, State >::element_type = Parent
```

Source: `core/tree/d2_tree_construct.hpp:269`

### `state_type`

```cpp
using d2::TreeCtx< Parent, State >::state_type = State
```

Source: `core/tree/d2_tree_construct.hpp:270`
