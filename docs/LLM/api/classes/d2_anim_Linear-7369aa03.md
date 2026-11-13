# `d2::anim::Linear`

**Kind:** class
**Declared in:** `core/screen/d2_animate.hpp:151`

**Bases:** `d2::Animation`, `d2::impl::LinearInterpolationWriter< impl::style_type< Type, Property > >`

## Functions

### `_keep_alive_impl`

```cpp
virtual bool d2::anim::Linear< Type, Property >::_keep_alive_impl(float progress) override
```

Source: `core/screen/d2_animate.hpp:157`

### `_update_impl`

```cpp
virtual float d2::anim::Linear< Type, Property >::_update_impl(std::shared_ptr< Element > ptr, float progress) override
```

Source: `core/screen/d2_animate.hpp:161`

### `Linear`

```cpp
d2::anim::Linear< Type, Property >::Linear(std::chrono::milliseconds ms, std::shared_ptr< Element > ptr, dest_type dest)
```

Source: `core/screen/d2_animate.hpp:169`

### `~Linear`

```cpp
virtual d2::anim::Linear< Type, Property >::~Linear()
```

Source: `core/screen/d2_animate.hpp:176`
