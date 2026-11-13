# `d2::sys::SystemOutput`

**Kind:** class
**Declared in:** `core/mods/d2_core.hpp:86`

**Bases:** `d2::sys::ModuleDecl< SystemOutput, "Output", Access::TUnsafe >`

## Functions

### `clear_images`

```cpp
virtual void d2::sys::SystemOutput::clear_images()
```

Source: `core/mods/d2_core.hpp:90`

### `write_frame`

```cpp
void d2::sys::SystemOutput::write_frame(PixelBuffer::View frame)
```

Source: `core/mods/d2_core.hpp:91`

### `write`

```cpp
virtual void d2::sys::SystemOutput::write(std::span< const pixel > buffer, std::size_t width, std::size_t height, const image::Scene &images)
```

Source: `core/mods/d2_core.hpp:92`

### `delta_size`

```cpp
virtual std::size_t d2::sys::SystemOutput::delta_size()=0
```

Source: `core/mods/d2_core.hpp:98`

### `swapframe_size`

```cpp
virtual std::size_t d2::sys::SystemOutput::swapframe_size()=0
```

Source: `core/mods/d2_core.hpp:99`

### `write`

```cpp
virtual void d2::sys::SystemOutput::write(std::span< const pixel > buffer, std::size_t width, std::size_t height)=0
```

Source: `core/mods/d2_core.hpp:102`
