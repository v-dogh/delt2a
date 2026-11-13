# `d2::image::Placement`

**Kind:** struct
**Declared in:** `core/types/d2_image.hpp:38`

## Functions

### `operator==`

```cpp
bool d2::image::Placement::operator==(const Placement &) const =default
```

Source: `core/types/d2_image.hpp:45`

### `at`

```cpp
bool d2::image::Placement::at(int x, int y) const
```

Source: `core/types/d2_image.hpp:46`


## Variables

### `raster`

```cpp
Raster::ptr d2::image::Placement::raster
```

Source: `core/types/d2_image.hpp:40`

### `source`

```cpp
Rect d2::image::Placement::source
```

Source: `core/types/d2_image.hpp:41`

### `destination`

```cpp
Rect d2::image::Placement::destination
```

Source: `core/types/d2_image.hpp:42`

### `clip`

```cpp
Rect d2::image::Placement::clip
```

Source: `core/types/d2_image.hpp:43`

### `visible`

```cpp
std::vector<unsigned char> d2::image::Placement::visible
```

Source: `core/types/d2_image.hpp:44`
