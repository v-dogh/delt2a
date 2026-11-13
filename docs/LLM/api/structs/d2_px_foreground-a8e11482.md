# `d2::px::foreground`

**Kind:** struct
**Declared in:** `core/types/d2_vtypes.hpp:363`

## Functions

### `as_background`

```cpp
background d2::px::foreground::as_background() const noexcept
```

Source: `core/types/d2_vtypes.hpp:372`

### `with_alpha`

```cpp
foreground d2::px::foreground::with_alpha(component value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:377`

### `with_alpha`

```cpp
foreground d2::px::foreground::with_alpha(float value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:384`

### `alpha`

```cpp
foreground d2::px::foreground::alpha(component value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:389`

### `alpha`

```cpp
foreground d2::px::foreground::alpha(float value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:393`

### `with_value`

```cpp
foreground d2::px::foreground::with_value(value_type value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:398`

### `extend`

```cpp
foreground d2::px::foreground::extend(value_type value, px::style flags=style::None) const noexcept
```

Source: `core/types/d2_vtypes.hpp:406`

### `with_style`

```cpp
foreground d2::px::foreground::with_style(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:413`

### `stylize`

```cpp
foreground d2::px::foreground::stylize(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:420`

### `add_style`

```cpp
foreground d2::px::foreground::add_style(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:425`

### `remove_style`

```cpp
foreground d2::px::foreground::remove_style(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:432`

### `has_style`

```cpp
bool d2::px::foreground::has_style(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:439`

### `inverted`

```cpp
foreground d2::px::foreground::inverted() const noexcept
```

Source: `core/types/d2_vtypes.hpp:444`

### `invert`

```cpp
foreground d2::px::foreground::invert() const noexcept
```

Source: `core/types/d2_vtypes.hpp:456`

### `mix`

```cpp
foreground d2::px::foreground::mix(foreground to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:462`

### `mask`

```cpp
foreground d2::px::foreground::mask(foreground to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:475`

### `operator background`

```cpp
d2::px::foreground::operator background() const noexcept
```

Source: `core/types/d2_vtypes.hpp:480`

### `operator combined`

```cpp
d2::px::foreground::operator combined() const noexcept
```

Source: `core/types/d2_vtypes.hpp:485`

### `operator<=>`

```cpp
auto d2::px::foreground::operator<=>(const foreground &) const =default
```

Source: `core/types/d2_vtypes.hpp:487`


## Variables

### `r`

```cpp
component d2::px::foreground::r
```

Source: `core/types/d2_vtypes.hpp:365`

### `g`

```cpp
component d2::px::foreground::g
```

Source: `core/types/d2_vtypes.hpp:366`

### `b`

```cpp
component d2::px::foreground::b
```

Source: `core/types/d2_vtypes.hpp:367`

### `a`

```cpp
component d2::px::foreground::a
```

Source: `core/types/d2_vtypes.hpp:368`

### `v`

```cpp
value_type d2::px::foreground::v
```

Source: `core/types/d2_vtypes.hpp:369`

### `style`

```cpp
px::style d2::px::foreground::style
```

Source: `core/types/d2_vtypes.hpp:370`
