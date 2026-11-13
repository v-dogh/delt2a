# `d2::px::combined`

**Kind:** struct
**Declared in:** `core/types/d2_vtypes.hpp:489`

## Functions

### `combine`

```cpp
static constexpr combined d2::px::combined::combine(background bg, foreground fg) noexcept
```

Source: `core/types/d2_vtypes.hpp:504`

### `combine`

```cpp
static constexpr combined d2::px::combined::combine(foreground fg, background bg) noexcept
```

Source: `core/types/d2_vtypes.hpp:519`

### `interpolate`

```cpp
static constexpr combined d2::px::combined::interpolate(combined from, combined to, float progress) noexcept
```

Source: `core/types/d2_vtypes.hpp:810`

### `cform`

```cpp
static constexpr combined d2::px::combined::cform(std::uint64_t len) noexcept
```

Source: `core/types/d2_vtypes.hpp:815`

### `as_background`

```cpp
background d2::px::combined::as_background() const noexcept
```

Source: `core/types/d2_vtypes.hpp:535`

### `as_foreground`

```cpp
foreground d2::px::combined::as_foreground() const noexcept
```

Source: `core/types/d2_vtypes.hpp:539`

### `with_background`

```cpp
combined d2::px::combined::with_background(background value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:544`

### `with_foreground`

```cpp
combined d2::px::combined::with_foreground(foreground value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:554`

### `with_value`

```cpp
combined d2::px::combined::with_value(value_type value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:566`

### `extend`

```cpp
combined d2::px::combined::extend(value_type value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:573`

### `with_style`

```cpp
combined d2::px::combined::with_style(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:578`

### `stylize`

```cpp
combined d2::px::combined::stylize(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:585`

### `add_style`

```cpp
combined d2::px::combined::add_style(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:590`

### `remove_style`

```cpp
combined d2::px::combined::remove_style(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:597`

### `has_style`

```cpp
bool d2::px::combined::has_style(px::style value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:604`

### `with_alpha`

```cpp
combined d2::px::combined::with_alpha(component background_alpha, component foreground_alpha) const noexcept
```

Source: `core/types/d2_vtypes.hpp:610`

### `with_alpha`

```cpp
combined d2::px::combined::with_alpha(float background_alpha, float foreground_alpha) const noexcept
```

Source: `core/types/d2_vtypes.hpp:619`

### `alpha`

```cpp
combined d2::px::combined::alpha(component background_alpha, component foreground_alpha) const noexcept
```

Source: `core/types/d2_vtypes.hpp:627`

### `alpha`

```cpp
combined d2::px::combined::alpha(float background_alpha, float foreground_alpha) const noexcept
```

Source: `core/types/d2_vtypes.hpp:633`

### `with_background_alpha`

```cpp
combined d2::px::combined::with_background_alpha(component value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:638`

### `with_background_alpha`

```cpp
combined d2::px::combined::with_background_alpha(float value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:645`

### `balpha`

```cpp
combined d2::px::combined::balpha(component value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:650`

### `balpha`

```cpp
combined d2::px::combined::balpha(float value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:654`

### `with_foreground_alpha`

```cpp
combined d2::px::combined::with_foreground_alpha(component value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:659`

### `with_foreground_alpha`

```cpp
combined d2::px::combined::with_foreground_alpha(float value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:666`

### `falpha`

```cpp
combined d2::px::combined::falpha(component value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:671`

### `falpha`

```cpp
combined d2::px::combined::falpha(float value) const noexcept
```

Source: `core/types/d2_vtypes.hpp:675`

### `inverted`

```cpp
combined d2::px::combined::inverted() const noexcept
```

Source: `core/types/d2_vtypes.hpp:680`

### `invert`

```cpp
combined d2::px::combined::invert() const noexcept
```

Source: `core/types/d2_vtypes.hpp:685`

### `mix`

```cpp
combined d2::px::combined::mix(combined to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:690`

### `mask`

```cpp
combined d2::px::combined::mask(combined to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:698`

### `mix_background`

```cpp
combined d2::px::combined::mix_background(combined to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:704`

### `mask_background`

```cpp
combined d2::px::combined::mask_background(combined to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:710`

### `mix_foreground`

```cpp
combined d2::px::combined::mix_foreground(combined to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:716`

### `mask_foreground`

```cpp
combined d2::px::combined::mask_foreground(combined to, float progress=0.5f) const noexcept
```

Source: `core/types/d2_vtypes.hpp:722`

### `same_colors`

```cpp
bool d2::px::combined::same_colors(combined other) const noexcept
```

Source: `core/types/d2_vtypes.hpp:727`

### `compare_colors`

```cpp
bool d2::px::combined::compare_colors(combined other) const noexcept
```

Source: `core/types/d2_vtypes.hpp:733`

### `blend`

```cpp
void d2::px::combined::blend(combined src) noexcept
```

Source: `core/types/d2_vtypes.hpp:738`

### `blended`

```cpp
combined d2::px::combined::blended(combined src) const noexcept
```

Source: `core/types/d2_vtypes.hpp:802`

### `is_cform_marker`

```cpp
bool d2::px::combined::is_cform_marker() const noexcept
```

Source: `core/types/d2_vtypes.hpp:830`

### `cform_length`

```cpp
std::uint64_t d2::px::combined::cform_length() const noexcept
```

Source: `core/types/d2_vtypes.hpp:834`

### `is_cform`

```cpp
std::size_t d2::px::combined::is_cform() const noexcept
```

Source: `core/types/d2_vtypes.hpp:850`

### `operator background`

```cpp
d2::px::combined::operator background() const noexcept
```

Source: `core/types/d2_vtypes.hpp:855`

### `operator foreground`

```cpp
d2::px::combined::operator foreground() const noexcept
```

Source: `core/types/d2_vtypes.hpp:859`

### `operator=`

```cpp
combined & d2::px::combined::operator=(background value) noexcept
```

Source: `core/types/d2_vtypes.hpp:864`

### `operator=`

```cpp
combined & d2::px::combined::operator=(foreground value) noexcept
```

Source: `core/types/d2_vtypes.hpp:873`

### `operator<=>`

```cpp
auto d2::px::combined::operator<=>(const combined &) const =default
```

Source: `core/types/d2_vtypes.hpp:884`


## Variables

### `r`

```cpp
component d2::px::combined::r
```

Source: `core/types/d2_vtypes.hpp:491`

### `g`

```cpp
component d2::px::combined::g
```

Source: `core/types/d2_vtypes.hpp:492`

### `b`

```cpp
component d2::px::combined::b
```

Source: `core/types/d2_vtypes.hpp:493`

### `a`

```cpp
component d2::px::combined::a
```

Source: `core/types/d2_vtypes.hpp:494`

### `rf`

```cpp
component d2::px::combined::rf
```

Source: `core/types/d2_vtypes.hpp:496`

### `gf`

```cpp
component d2::px::combined::gf
```

Source: `core/types/d2_vtypes.hpp:497`

### `bf`

```cpp
component d2::px::combined::bf
```

Source: `core/types/d2_vtypes.hpp:498`

### `af`

```cpp
component d2::px::combined::af
```

Source: `core/types/d2_vtypes.hpp:499`

### `v`

```cpp
value_type d2::px::combined::v
```

Source: `core/types/d2_vtypes.hpp:501`

### `style`

```cpp
px::style d2::px::combined::style
```

Source: `core/types/d2_vtypes.hpp:502`
