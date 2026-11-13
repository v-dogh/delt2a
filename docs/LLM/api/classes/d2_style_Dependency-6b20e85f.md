# `d2::style::Dependency`

**Kind:** class
**Declared in:** `core/tree/d2_theme.hpp:132`

**Bases:** `d2::style::DependencyBase`

## Friends

### `Dependency`

```cpp
template<typename Other>
friend class Dependency
```

Source: `core/tree/d2_theme.hpp:298`


## Functions

### `Dependency`

```cpp
d2::style::Dependency< Type >::Dependency()=default
```

Source: `core/tree/d2_theme.hpp:300`

### `Dependency`

```cpp
d2::style::Dependency< Type >::Dependency(const Dependency &copy)
```

Source: `core/tree/d2_theme.hpp:301`

### `Dependency`

```cpp
d2::style::Dependency< Type >::Dependency(Dependency &&)=delete
```

Source: `core/tree/d2_theme.hpp:305`

### `~Dependency`

```cpp
d2::style::Dependency< Type >::~Dependency()
```

Source: `core/tree/d2_theme.hpp:306`

### `subscribe`

```cpp
template<typename Data, typename Elem, typename Func>
Handle d2::style::Dependency< Type >::subscribe(std::weak_ptr< Elem > handle, Data &&def, Func &&callback) const
```

Source: `core/tree/d2_theme.hpp:318`

### `subscribe`

```cpp
template<typename Elem, typename Func>
Handle d2::style::Dependency< Type >::subscribe(std::weak_ptr< Elem > handle, Func &&callback) const
```

Source: `core/tree/d2_theme.hpp:357`

### `subscribe_base`

```cpp
Handle d2::style::Dependency< Type >::subscribe_base(std::weak_ptr< void > handle, void *base, manager_callback func) const
```

Source: `core/tree/d2_theme.hpp:361`

### `cleanup`

```cpp
void d2::style::Dependency< Type >::cleanup() const
```

Source: `core/tree/d2_theme.hpp:366`

### `apply`

```cpp
void d2::style::Dependency< Type >::apply() const
```

Source: `core/tree/d2_theme.hpp:370`

### `set`

```cpp
template<typename Value>
void d2::style::Dependency< Type >::set(Value &&value)
```

Source: `core/tree/d2_theme.hpp:375`

### `set_async`

```cpp
template<typename Value>
void d2::style::Dependency< Type >::set_async(Value &&value)
```

Source: `core/tree/d2_theme.hpp:380`

### `value`

```cpp
Type d2::style::Dependency< Type >::value() const
```

Source: `core/tree/d2_theme.hpp:394`

### `unlink`

```cpp
void d2::style::Dependency< Type >::unlink()
```

Source: `core/tree/d2_theme.hpp:399`

### `link`

```cpp
void d2::style::Dependency< Type >::link(Dependency< Type > &provider)
```

Source: `core/tree/d2_theme.hpp:403`

### `link`

```cpp
template<typename Other>
void d2::style::Dependency< Type >::link(Dependency< Other > &provider, Type(*callback)(const Other &))
```

Source: `core/tree/d2_theme.hpp:426`

### `operator Value`

```cpp
template<typename Value>
d2::style::Dependency< Type >::operator Value() const
```

Source: `core/tree/d2_theme.hpp:461`

### `operator+`

```cpp
auto d2::style::Dependency< Type >::operator+() const
```

Source: `core/tree/d2_theme.hpp:479`

### `operator-`

```cpp
auto d2::style::Dependency< Type >::operator-() const
```

Source: `core/tree/d2_theme.hpp:484`

### `operator~`

```cpp
auto d2::style::Dependency< Type >::operator~() const
```

Source: `core/tree/d2_theme.hpp:489`

### `operator!`

```cpp
auto d2::style::Dependency< Type >::operator!() const
```

Source: `core/tree/d2_theme.hpp:494`

### `operator++`

```cpp
auto d2::style::Dependency< Type >::operator++()
```

Source: `core/tree/d2_theme.hpp:500`

### `operator--`

```cpp
auto d2::style::Dependency< Type >::operator--()
```

Source: `core/tree/d2_theme.hpp:512`

### `operator++`

```cpp
auto d2::style::Dependency< Type >::operator++(int)
```

Source: `core/tree/d2_theme.hpp:524`

### `operator--`

```cpp
auto d2::style::Dependency< Type >::operator--(int)
```

Source: `core/tree/d2_theme.hpp:536`

### `operator[]`

```cpp
template<class Index>
auto d2::style::Dependency< Type >::operator[](Index &&index) const
```

Source: `core/tree/d2_theme.hpp:578`

### `operator()`

```cpp
template<class... Args>
auto d2::style::Dependency< Type >::operator()(Args &&... args) const
```

Source: `core/tree/d2_theme.hpp:586`

### `operator=`

```cpp
Dependency & d2::style::Dependency< Type >::operator=(const Dependency &copy)
```

Source: `core/tree/d2_theme.hpp:597`

### `operator=`

```cpp
Dependency & d2::style::Dependency< Type >::operator=(Dependency &&)=delete
```

Source: `core/tree/d2_theme.hpp:602`

### `operator=`

```cpp
template<typename Value>
Dependency & d2::style::Dependency< Type >::operator=(Value &&value)
```

Source: `core/tree/d2_theme.hpp:606`


## Typedefs

### `Query`

```cpp
using d2::style::Dependency< Type >::Query = DepQuery
```

Source: `core/tree/d2_theme.hpp:136`

### `value_type`

```cpp
using d2::style::Dependency< Type >::value_type = Type
```

Source: `core/tree/d2_theme.hpp:137`

### `manager_callback`

```cpp
using d2::style::Dependency< Type >::manager_callback = bool (*)(std::shared_ptr<void>, void*, const Type&, Query)
```

Source: `core/tree/d2_theme.hpp:138`

### `Handle`

```cpp
using d2::style::Dependency< Type >::Handle = DependencyBase::Handle
```

Source: `core/tree/d2_theme.hpp:139`
