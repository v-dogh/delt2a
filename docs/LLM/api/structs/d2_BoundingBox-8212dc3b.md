# `d2::BoundingBox`

**Kind:** struct
**Declared in:** `core/types/d2_vtypes.hpp:76`

## Friends

### `operator+`

```cpp
BoundingBox operator+
```

Source: `core/types/d2_vtypes.hpp:120`

### `operator-`

```cpp
BoundingBox operator-
```

Source: `core/types/d2_vtypes.hpp:126`

### `operator*`

```cpp
BoundingBox operator*
```

Source: `core/types/d2_vtypes.hpp:131`

### `operator*`

```cpp
BoundingBox operator*
```

Source: `core/types/d2_vtypes.hpp:136`

### `operator/`

```cpp
BoundingBox operator/
```

Source: `core/types/d2_vtypes.hpp:141`


## Functions

### `empty`

```cpp
bool d2::BoundingBox::empty() const noexcept
```

Source: `core/types/d2_vtypes.hpp:81`

### `area`

```cpp
int d2::BoundingBox::area() const noexcept
```

Source: `core/types/d2_vtypes.hpp:85`

### `contains`

```cpp
bool d2::BoundingBox::contains(Position p) const noexcept
```

Source: `core/types/d2_vtypes.hpp:89`

### `operator+=`

```cpp
auto & d2::BoundingBox::operator+=(BoundingBox rhs) noexcept
```

Source: `core/types/d2_vtypes.hpp:94`

### `operator-=`

```cpp
auto & d2::BoundingBox::operator-=(BoundingBox rhs) noexcept
```

Source: `core/types/d2_vtypes.hpp:100`

### `operator*=`

```cpp
auto & d2::BoundingBox::operator*=(int scalar) noexcept
```

Source: `core/types/d2_vtypes.hpp:106`

### `operator/=`

```cpp
auto & d2::BoundingBox::operator/=(int scalar) noexcept
```

Source: `core/types/d2_vtypes.hpp:112`

### `operator<=>`

```cpp
auto d2::BoundingBox::operator<=>(const BoundingBox &) const =default
```

Source: `core/types/d2_vtypes.hpp:147`


## Variables

### `width`

```cpp
int d2::BoundingBox::width
```

Source: `core/types/d2_vtypes.hpp:78`

### `height`

```cpp
int d2::BoundingBox::height
```

Source: `core/types/d2_vtypes.hpp:79`
