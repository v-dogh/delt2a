# `d2::anim::Sequential`

**Kind:** class
**Declared in:** `core/screen/d2_animate.hpp:182`

**Bases:** `d2::Animation`

## Functions

### `_keep_alive_impl`

```cpp
virtual bool d2::anim::Sequential< Type, Property >::_keep_alive_impl(float progress) override
```

Source: `core/screen/d2_animate.hpp:190`

### `_update_impl`

```cpp
virtual float d2::anim::Sequential< Type, Property >::_update_impl(std::shared_ptr< Element > ptr, float progress) override
```

Source: `core/screen/d2_animate.hpp:194`

### `Sequential`

```cpp
d2::anim::Sequential< Type, Property >::Sequential(std::chrono::milliseconds ms, std::shared_ptr< Element > ptr, std::initializer_list< dest_type > stages)
```

Source: `core/screen/d2_animate.hpp:217`

### `Sequential`

```cpp
template<typename Other>
d2::anim::Sequential< Type, Property >::Sequential(std::chrono::milliseconds ms, std::shared_ptr< Element > ptr, Other &&stages)
```

Source: `core/screen/d2_animate.hpp:225`
