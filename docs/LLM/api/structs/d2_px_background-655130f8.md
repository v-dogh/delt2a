# `d2::px::background`

**Kind:** struct
**Declared in:** `core/types/d2_vtypes.hpp:266`

## Functions

### `from_scalar`

```cpp
static constexpr background d2::px::background::from_scalar(scalar value) noexcept
```

Source: `core/types/d2_vtypes.hpp:273`

### `to_scalar`

```cpp
static constexpr scalar d2::px::background::to_scalar(background value) noexcept
```

Source: `core/types/d2_vtypes.hpp:293`

### `to_scalar`

```cpp
scalar d2::px::background::to_scalar() const noexcept
```

Source: `core/types/d2_vtypes.hpp:283`

### `with_alpha`

```cpp
background d2::px::background::with_alpha(component value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:298`

### `with_alpha`

```cpp
background d2::px::background::with_alpha(float value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:305`

### `alpha`

```cpp
background d2::px::background::alpha(component value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:310`

### `alpha`

```cpp
background d2::px::background::alpha(float value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:314`

### `inverted`

```cpp
background d2::px::background::inverted() const noexcept
```

Source: `core/types/d2_vtypes.hpp:319`

### `invert`

```cpp
background d2::px::background::invert() const noexcept
```

Source: `core/types/d2_vtypes.hpp:329`

### `mix`

```cpp
background d2::px::background::mix(background to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:335`

### `mask`

```cpp
background d2::px::background::mask(background to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:346`

### `with_value`

```cpp
foreground d2::px::background::with_value(value_type value=' ', style flags=style::None) const noexcept
```

Source: `core/types/d2_vtypes.hpp:352`

### `extend`

```cpp
foreground d2::px::background::extend(value_type value=' ', style flags=style::None) const noexcept
```

Source: `core/types/d2_vtypes.hpp:354`

### `with_style`

```cpp
foreground d2::px::background::with_style(style flags) const noexcept
```

Source: `core/types/d2_vtypes.hpp:355`

### `stylize`

```cpp
foreground d2::px::background::stylize(style flags) const noexcept
```

Source: `core/types/d2_vtypes.hpp:356`

### `operator foreground`

```cpp
d2::px::background::operator foreground() const noexcept
```

Source: `core/types/d2_vtypes.hpp:358`

### `operator combined`

```cpp
d2::px::background::operator combined() const noexcept
```

Source: `core/types/d2_vtypes.hpp:359`

### `operator<=>`

```cpp
auto d2::px::background::operator<=>(const background &) const =default
```

Source: `core/types/d2_vtypes.hpp:361`


## Variables

### `r`

```cpp
component d2::px::background::r
```

Source: `core/types/d2_vtypes.hpp:268`

### `g`

```cpp
component d2::px::background::g
```

Source: `core/types/d2_vtypes.hpp:269`

### `b`

```cpp
component d2::px::background::b
```

Source: `core/types/d2_vtypes.hpp:270`

### `a`

```cpp
component d2::px::background::a
```

Source: `core/types/d2_vtypes.hpp:271`
