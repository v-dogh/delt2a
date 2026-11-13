# `d2::Position`

**Kind:** struct
**Declared in:** `core/types/d2_vtypes.hpp:7`

## Friends

### `operator+`

```cpp
Position operator+
```

Source: `core/types/d2_vtypes.hpp:47`

### `operator-`

```cpp
Position operator-
```

Source: `core/types/d2_vtypes.hpp:52`

### `operator*`

```cpp
Position operator*
```

Source: `core/types/d2_vtypes.hpp:57`

### `operator*`

```cpp
Position operator*
```

Source: `core/types/d2_vtypes.hpp:62`

### `operator/`

```cpp
Position operator/
```

Source: `core/types/d2_vtypes.hpp:67`


## Functions

### `operator+`

```cpp
Position d2::Position::operator+() const noexcept
```

Source: `core/types/d2_vtypes.hpp:12`

### `operator-`

```cpp
Position d2::Position::operator-() const noexcept
```

Source: `core/types/d2_vtypes.hpp:16`

### `operator+=`

```cpp
Position & d2::Position::operator+=(Position rhs) noexcept
```

Source: `core/types/d2_vtypes.hpp:21`

### `operator-=`

```cpp
auto & d2::Position::operator-=(Position rhs) noexcept
```

Source: `core/types/d2_vtypes.hpp:28`

### `operator*=`

```cpp
auto & d2::Position::operator*=(int scalar) noexcept
```

Source: `core/types/d2_vtypes.hpp:34`

### `operator/=`

```cpp
auto & d2::Position::operator/=(int scalar) noexcept
```

Source: `core/types/d2_vtypes.hpp:40`

### `operator<=>`

```cpp
auto d2::Position::operator<=>(const Position &) const =default
```

Source: `core/types/d2_vtypes.hpp:73`


## Variables

### `x`

```cpp
int d2::Position::x
```

Source: `core/types/d2_vtypes.hpp:9`

### `y`

```cpp
int d2::Position::y
```

Source: `core/types/d2_vtypes.hpp:10`
