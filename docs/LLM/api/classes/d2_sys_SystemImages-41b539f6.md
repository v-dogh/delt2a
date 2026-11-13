# `d2::sys::SystemImages`

**Kind:** class
**Declared in:** `core/mods/d2_images.hpp:8`

**Bases:** `d2::sys::ModuleDecl< SystemImages, "Images", Access::TUnsafe, SystemOutput >`

## Functions

### `_load_impl`

```cpp
SystemImages::Status d2::sys::SystemImages::_load_impl() override
```

Source: `core/mods/d2_images.hpp:25`

### `_unload_impl`

```cpp
SystemImages::Status d2::sys::SystemImages::_unload_impl() override
```

Source: `core/mods/d2_images.hpp:26`

### `_repaint_on_change`

```cpp
virtual bool d2::sys::SystemImages::_repaint_on_change() const =0
```

Source: `core/mods/d2_images.hpp:27`

### `_fallback_metrics`

```cpp
virtual BoundingBox d2::sys::SystemImages::_fallback_metrics() const
```

Source: `core/mods/d2_images.hpp:28`

### `_encode`

```cpp
virtual std::string d2::sys::SystemImages::_encode(const image::Composite &image, std::span< const pixel > cells, int columns)=0
```

Source: `core/mods/d2_images.hpp:29`

### `_erase`

```cpp
virtual std::string d2::sys::SystemImages::_erase() const =0
```

Source: `core/mods/d2_images.hpp:30`

### `configure`

```cpp
void d2::sys::SystemImages::configure(BoundingBox cell_pixels)
```

Source: `core/mods/d2_images.hpp:33`

### `prepare`

```cpp
SystemImages::Plan d2::sys::SystemImages::prepare(std::span< pixel > cells, BoundingBox viewport, const image::Scene &scene, BoundingBox cell_pixels)
```

Source: `core/mods/d2_images.hpp:34`

### `reset`

```cpp
SystemImages::Plan d2::sys::SystemImages::reset()
```

Source: `core/mods/d2_images.hpp:36`

### `invalidate`

```cpp
void d2::sys::SystemImages::invalidate() noexcept
```

Source: `core/mods/d2_images.hpp:37`
