# `d2::dx::Image`

**Kind:** class
**Declared in:** `elements/d2_image.hpp:30`

**Bases:** `d2::style::UniversalAccessInterfaceProto< Element, Base, Interface, Interfaces... >`, `d2::dx::impl::ContainerHelper< Image >`

## Friends

### `ContainerHelper< Image >`

```cpp
friend class ContainerHelper< Image >
```

Source: `elements/d2_image.hpp:45`


## Functions

### `load`

```cpp
mt::co_future< bool > d2::dx::Image::load(std::filesystem::path path, image::LoadOptions options={}, const std::string &loader={})
```

Source: `elements/d2_image.hpp:56`

### `cancel_load`

```cpp
void d2::dx::Image::cancel_load()
```

Source: `elements/d2_image.hpp:58`


## Typedefs

### `data`

```cpp
using d2::dx::Image::data = style::UAI< Image, style::ILayout, style::IContainer, style::IColors, style::IImage >
```

Source: `elements/d2_image.hpp:48`
