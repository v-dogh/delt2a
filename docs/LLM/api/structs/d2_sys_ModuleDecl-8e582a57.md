# `d2::sys::ModuleDecl`

**Kind:** struct
**Declared in:** `core/io/d2_module.hpp:213`

**Bases:** `d2::sys::SystemModule`, `d2::sys::ModuleLocalGet< Base >`

## Friends

### `ModuleImpl`

```cpp
template<typename B, Load L, typename... D>
friend struct ModuleImpl
```

Source: `core/io/d2_module.hpp:220`


## Functions

### `D2_TAG_MODULE_VALUE`

```cpp
d2::sys::ModuleDecl< Base, Name, Ac, Deps >::D2_TAG_MODULE_VALUE(Name.view())
```

Source: `core/io/d2_module.hpp:215`

### `info`

```cpp
virtual ModInfo d2::sys::ModuleDecl< Base, Name, Ac, Deps >::info() const override final
```

Source: `core/io/d2_module.hpp:228`

### `ptr`

```cpp
template<typename Type = Base>
module< const Type > d2::sys::ModuleDecl< Base, Name, Ac, Deps >::ptr() const noexcept
```

Source: `core/io/d2_module.hpp:233`

### `ptr`

```cpp
template<typename Type = Base>
module< Type > d2::sys::ModuleDecl< Base, Name, Ac, Deps >::ptr() noexcept
```

Source: `core/io/d2_module.hpp:239`

### `SystemModule`

```cpp
d2::sys::SystemModule::SystemModule(std::weak_ptr< IOContext > ptr, ModPreset preset, std::size_t static_usage)
```

Source: `core/io/d2_module.hpp:222`

### `module_info`

```cpp
static ModInfo d2::sys::ModuleDecl< Base, Name, Ac, Deps >::module_info()
```

Source: `core/io/d2_module.hpp:224`
