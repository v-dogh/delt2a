# `d2::sys`

**Kind:** namespace
**Declared in:** `core/io/d2_context.hpp:351`

## Enums

### `Load`

```cpp
enum Load
```

Source: `core/io/d2_module.hpp:19`

Values: `Immediate`, `Lazy`

### `Access`

```cpp
enum Access
```

Source: `core/io/d2_module.hpp:24`

Values: `TSafe`, `TUnsafe`


## Functions

### `_convert_sample`

```cpp
template<SystemAudio::Format>
float d2::sys::_convert_sample(const unsigned char *sample)
```

Source: `core/mods/d2_audio.cpp:113`

### `_convert_sample< SystemAudio::Format::PCM16 >`

```cpp
float d2::sys::_convert_sample< SystemAudio::Format::PCM16 >(const unsigned char *sample)
```

Source: `core/mods/d2_audio.cpp:115`

### `_convert_sample< SystemAudio::Format::PCM32 >`

```cpp
float d2::sys::_convert_sample< SystemAudio::Format::PCM32 >(const unsigned char *sample)
```

Source: `core/mods/d2_audio.cpp:119`

### `_convert_sample< SystemAudio::Format::Float32 >`

```cpp
float d2::sys::_convert_sample< SystemAudio::Format::Float32 >(const unsigned char *sample)
```

Source: `core/mods/d2_audio.cpp:124`

### `_convert_sample< SystemAudio::Format::Float64 >`

```cpp
float d2::sys::_convert_sample< SystemAudio::Format::Float64 >(const unsigned char *sample)
```

Source: `core/mods/d2_audio.cpp:129`

### `_convert_from`

```cpp
template<SystemAudio::Format Fmt>
void d2::sys::_convert_from(std::span< float > out, std::span< const unsigned char > data, SystemAudio::FormatInfo format, SystemAudio::FormatInfo expected)
```

Source: `core/mods/d2_audio.cpp:135`

### `flip`

```cpp
template<std::size_t First, std::size_t Len, std::size_t Count>
void d2::sys::flip(std::bitset< Count > &bs)
```

Source: `core/screen/d2_screen.cpp:10`


## Typedefs

### `module`

```cpp
template<typename Type>
using d2::sys::module = ModulePtr<Type>
```

Source: `core/io/d2_module.hpp:181`

### `audio`

```cpp
using d2::sys::audio = SystemAudio
```

Source: `core/mods/d2_audio.hpp:403`

### `binder`

```cpp
using d2::sys::binder = SystemBinder
```

Source: `core/mods/d2_binder.hpp:120`

### `input`

```cpp
using d2::sys::input = SystemInput
```

Source: `core/mods/d2_core.hpp:111`

### `output`

```cpp
using d2::sys::output = SystemOutput
```

Source: `core/mods/d2_core.hpp:112`

### `screen`

```cpp
using d2::sys::screen = SystemScreen
```

Source: `core/mods/d2_core.hpp:113`

### `clipboard`

```cpp
using d2::sys::clipboard = SystemClipboard
```

Source: `core/mods/d2_ext.hpp:110`

### `notify`

```cpp
using d2::sys::notify = SystemNotify
```

Source: `core/mods/d2_ext.hpp:111`

### `selector`

```cpp
using d2::sys::selector = SystemSelector
```

Source: `core/mods/d2_ext.hpp:112`

### `image_loader`

```cpp
using d2::sys::image_loader = SystemImageLoader
```

Source: `core/mods/d2_image_loader.hpp:27`

### `images`

```cpp
using d2::sys::images = SystemImages
```

Source: `core/mods/d2_images.hpp:39`


## Variables

### `is_abstract`

```cpp
template<typename Module>
bool d2::sys::is_abstract
```

Source: `core/io/d2_module.hpp:251`
