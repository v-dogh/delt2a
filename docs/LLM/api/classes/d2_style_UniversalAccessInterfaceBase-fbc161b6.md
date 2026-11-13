# `d2::style::UniversalAccessInterfaceBase`

**Kind:** class
**Declared in:** `core/tree/d2_styles_base.hpp:53`

## Functions

### `_initialize_impl`

```cpp
virtual void d2::style::UniversalAccessInterfaceBase::_initialize_impl(bool force)=0
```

Source: `core/tree/d2_styles_base.hpp:151`

### `_signal_base_impl`

```cpp
virtual void d2::style::UniversalAccessInterfaceBase::_signal_base_impl(element_write_flag, property)=0
```

Source: `core/tree/d2_styles_base.hpp:152`

### `_clear_anims_impl`

```cpp
virtual void d2::style::UniversalAccessInterfaceBase::_clear_anims_impl(uai_property prop)=0
```

Source: `core/tree/d2_styles_base.hpp:153`

### `_set_dynamic_impl`

```cpp
virtual void d2::style::UniversalAccessInterfaceBase::_set_dynamic_impl(property, bool)=0
```

Source: `core/tree/d2_styles_base.hpp:154`

### `_test_dynamic_impl`

```cpp
virtual bool d2::style::UniversalAccessInterfaceBase::_test_dynamic_impl(property)=0
```

Source: `core/tree/d2_styles_base.hpp:155`

### `_has_interface_impl`

```cpp
virtual void * d2::style::UniversalAccessInterfaceBase::_has_interface_impl(std::size_t)=0
```

Source: `core/tree/d2_styles_base.hpp:156`

### `_register_dep_bind`

```cpp
virtual void d2::style::UniversalAccessInterfaceBase::_register_dep_bind(property prop, DependencyHandle handle)=0
```

Source: `core/tree/d2_styles_base.hpp:157`

### `_deregister_dep_bind`

```cpp
virtual void d2::style::UniversalAccessInterfaceBase::_deregister_dep_bind(property prop)=0
```

Source: `core/tree/d2_styles_base.hpp:158`

### `_context_impl`

```cpp
virtual std::shared_ptr< IOContext > d2::style::UniversalAccessInterfaceBase::_context_impl() const =0
```

Source: `core/tree/d2_styles_base.hpp:159`

### `_handle_impl`

```cpp
virtual std::weak_ptr< void > d2::style::UniversalAccessInterfaceBase::_handle_impl()=0
```

Source: `core/tree/d2_styles_base.hpp:160`

### `has_interface`

```cpp
template<D2_UAI_INTERFACE_TEMPL Interface>
bool d2::style::UniversalAccessInterfaceBase::has_interface()
```

Source: `core/tree/d2_styles_base.hpp:162`

### `set_for`

```cpp
template<D2_UAI_INTERFACE_TEMPL Interface, Interface< 0 >::Property Property, bool Temporary = false, typename Type>
auto & d2::style::UniversalAccessInterfaceBase::set_for(Type &&value)
```

Source: `core/tree/d2_styles_base.hpp:173`

### `get_for`

```cpp
template<D2_UAI_INTERFACE_TEMPL Interface, Interface< 0 >::Property Property>
auto d2::style::UniversalAccessInterfaceBase::get_for() const
```

Source: `core/tree/d2_styles_base.hpp:190`

### `getref_for`

```cpp
template<D2_UAI_INTERFACE_TEMPL Interface, Interface< 0 >::Property Property>
auto * d2::style::UniversalAccessInterfaceBase::getref_for()
```

Source: `core/tree/d2_styles_base.hpp:207`

### `apply_get_for`

```cpp
template<D2_UAI_INTERFACE_TEMPL Interface, Interface< 0 >::Property Property, typename Func>
auto d2::style::UniversalAccessInterfaceBase::apply_get_for(Func &&func)
```

Source: `core/tree/d2_styles_base.hpp:217`

### `apply_set_for`

```cpp
template<D2_UAI_INTERFACE_TEMPL Interface, Interface< 0 >::Property Property, typename Func>
auto d2::style::UniversalAccessInterfaceBase::apply_set_for(Func &&func)
```

Source: `core/tree/d2_styles_base.hpp:231`

### `initialize`

```cpp
void d2::style::UniversalAccessInterfaceBase::initialize(bool force=false)
```

Source: `core/tree/d2_styles_base.hpp:254`


## Typedefs

### `property`

```cpp
using d2::style::UniversalAccessInterfaceBase::property = unsigned int
```

Source: `core/tree/d2_styles_base.hpp:57`


## Variables

### `initial_property`

```cpp
auto d2::style::UniversalAccessInterfaceBase::initial_property
```

Source: `core/tree/d2_styles_base.hpp:58`

### `invalid_property`

```cpp
auto d2::style::UniversalAccessInterfaceBase::invalid_property
```

Source: `core/tree/d2_styles_base.hpp:59`
