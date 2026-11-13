# `d2::image::Raster`

**Kind:** class
**Declared in:** `core/types/d2_image.hpp:23`

## Functions

### `Raster`

```cpp
d2::image::Raster::Raster(int width, int height, std::vector< RGBA > pixels)
```

Source: `core/types/d2_image.hpp:31`

### `width`

```cpp
int d2::image::Raster::width() const
```

Source: `core/types/d2_image.hpp:34`

### `height`

```cpp
int d2::image::Raster::height() const
```

Source: `core/types/d2_image.hpp:35`

### `pixels`

```cpp
std::span< const RGBA > d2::image::Raster::pixels() const
```

Source: `core/types/d2_image.hpp:36`

### `rgba`

```cpp
Raster::ptr d2::image::Raster::rgba(int width, int height, std::vector< RGBA > pixels)
```

Source: `core/types/d2_image.hpp:32`

### `rgb`

```cpp
Raster::ptr d2::image::Raster::rgb(int width, int height, std::span< const unsigned char > bytes)
```

Source: `core/types/d2_image.hpp:33`


## Typedefs

### `ptr`

```cpp
using d2::image::Raster::ptr = std::shared_ptr<const Raster>
```

Source: `core/types/d2_image.hpp:29`


## Variables

### `max_pixels`

```cpp
std::size_t d2::image::Raster::max_pixels
```

Source: `core/types/d2_image.hpp:30`
