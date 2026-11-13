# `d2::sys::SystemImageLoader`

**Kind:** class
**Declared in:** `core/mods/d2_image_loader.hpp:19`

**Bases:** `d2::sys::ModuleDecl< SystemImageLoader, "ImageLoader", Access::TSafe >`

## Functions

### `decode`

```cpp
virtual image::Raster::ptr d2::sys::SystemImageLoader::decode(std::span< const unsigned char > bytes, image::LoadOptions options={}) const =0
```

Source: `core/mods/d2_image_loader.hpp:23`

### `load`

```cpp
image::Raster::ptr d2::sys::SystemImageLoader::load(const std::filesystem::path &path, image::LoadOptions options={}) const
```

Source: `core/mods/d2_image_loader.hpp:24`

### `load_async`

```cpp
mt::future< image::Raster::ptr > d2::sys::SystemImageLoader::load_async(std::filesystem::path path, image::LoadOptions options={})
```

Source: `core/mods/d2_image_loader.hpp:25`
