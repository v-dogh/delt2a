# `d2::style::UniversalAccessInterface`

**Kind:** class
**Declared in:** `core/tree/d2_styles.hpp:108`

**Bases:** `Chain`, `Interface< impl::ResolveChain< Chain >::offset >`, `Interfaces< impl::OffsetHelper< impl::ResolveChain< Chain >::offset, Interfaces, Interface, Interfaces... >::offset >`

## Friends

### `impl::ResolveChain`

```cpp
template<typename>
friend class impl::ResolveChain
```

Source: `core/tree/d2_styles.hpp:464`

### `UniversalAccessInterface`

```cpp
template<typename, typename, D2_UAI_INTERFACE_TEMPL, D2_UAI_INTERFACE_TEMPL...>
friend class UniversalAccessInterface
```

Source: `core/tree/d2_styles.hpp:466`


## Functions

### `_has_interface_own_impl`

```cpp
void * d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_has_interface_own_impl(std::size_t id)
```

Source: `core/tree/d2_styles.hpp:366`

### `_initialize_impl`

```cpp
virtual void d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_initialize_impl(bool force) override
```

Source: `core/tree/d2_styles.hpp:370`

### `_clear_anims_impl`

```cpp
virtual void d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_clear_anims_impl(uai_property prop) override
```

Source: `core/tree/d2_styles.hpp:389`

### `_signal_base_impl`

```cpp
virtual void d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_signal_base_impl(Element::write_flag type, property prop) override
```

Source: `core/tree/d2_styles.hpp:393`

### `_register_dep_bind`

```cpp
virtual void d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_register_dep_bind(property prop, DependencyHandle handle) override
```

Source: `core/tree/d2_styles.hpp:398`

### `_deregister_dep_bind`

```cpp
virtual void d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_deregister_dep_bind(property prop) override
```

Source: `core/tree/d2_styles.hpp:404`

### `_set_dynamic_impl`

```cpp
virtual void d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_set_dynamic_impl(property prop, bool value) override
```

Source: `core/tree/d2_styles.hpp:411`

### `_test_dynamic_impl`

```cpp
virtual bool d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_test_dynamic_impl(property prop) override
```

Source: `core/tree/d2_styles.hpp:423`

### `_has_interface_impl`

```cpp
virtual void * d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_has_interface_impl(std::size_t id) override
```

Source: `core/tree/d2_styles.hpp:432`

### `_context_impl`

```cpp
virtual std::shared_ptr< IOContext > d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_context_impl() const override
```

Source: `core/tree/d2_styles.hpp:455`

### `_handle_impl`

```cpp
virtual std::weak_ptr< void > d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::_handle_impl() override
```

Source: `core/tree/d2_styles.hpp:459`

### `interface`

```cpp
template<D2_UAI_INTERFACE_TEMPL Type>
auto & d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::interface()
```

Source: `core/tree/d2_styles.hpp:470`

### `set`

```cpp
template<property Property, bool Temporary = false, typename Type>
auto & d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::set(Type &&value)
```

Source: `core/tree/d2_styles.hpp:490`

### `get`

```cpp
template<property Property>
auto d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::get()
```

Source: `core/tree/d2_styles.hpp:510`

### `getref`

```cpp
template<property Property>
auto * d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::getref()
```

Source: `core/tree/d2_styles.hpp:531`

### `apply_get`

```cpp
template<property Property, typename Func>
auto d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::apply_get(Func &&func)
```

Source: `core/tree/d2_styles.hpp:551`

### `apply_set`

```cpp
template<property Property, typename Func>
auto d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::apply_set(Func &&func)
```

Source: `core/tree/d2_styles.hpp:560`


## Typedefs

### `property`

```cpp
using d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::property = uai_property
```

Source: `core/tree/d2_styles.hpp:119`

### `uai_chain`

```cpp
using d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::uai_chain = UniversalAccessInterface
```

Source: `core/tree/d2_styles.hpp:120`

### `type_of`

```cpp
template<property Property, typename Self = UniversalAccessInterface>
using d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::type_of = decltype(std::declval<Self&>().template get<Property>())
```

Source: `core/tree/d2_styles.hpp:529`
