# `d2::image::Scene`

**Kind:** class
**Declared in:** `core/types/d2_image.hpp:48`

## Functions

### `place`

```cpp
void d2::image::Scene::place(Raster::ptr raster, Rect destination, Rect clip, Rect source={})
```

Source: `core/types/d2_image.hpp:53`

### `erase`

```cpp
void d2::image::Scene::erase(Rect area)
```

Source: `core/types/d2_image.hpp:54`

### `append`

```cpp
void d2::image::Scene::append(const Scene &source, Rect source_view, Position destination, Rect clip)
```

Source: `core/types/d2_image.hpp:55`

### `clear`

```cpp
void d2::image::Scene::clear()
```

Source: `core/types/d2_image.hpp:56`

### `empty`

```cpp
bool d2::image::Scene::empty() const
```

Source: `core/types/d2_image.hpp:57`

### `placements`

```cpp
const std::vector< Placement > & d2::image::Scene::placements() const
```

Source: `core/types/d2_image.hpp:58`

### `operator==`

```cpp
bool d2::image::Scene::operator==(const Scene &) const =default
```

Source: `core/types/d2_image.hpp:59`
