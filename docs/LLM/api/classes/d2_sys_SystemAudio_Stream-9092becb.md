# `d2::sys::SystemAudio::Stream`

**Kind:** class
**Declared in:** `core/mods/d2_audio.hpp:47`

## Functions

### `mix`

```cpp
float d2::sys::SystemAudio::Stream::mix(float a, float b, float prog=0.5f)
```

Source: `core/mods/d2_audio.hpp:56`

### `Stream`

```cpp
d2::sys::SystemAudio::Stream::Stream(std::span< float > data, std::size_t offset, std::size_t total, const FormatInfo *format)
```

Source: `core/mods/d2_audio.hpp:58`

### `Stream`

```cpp
d2::sys::SystemAudio::Stream::Stream(const Stream &)=default
```

Source: `core/mods/d2_audio.hpp:66`

### `Stream`

```cpp
d2::sys::SystemAudio::Stream::Stream(Stream &&)=default
```

Source: `core/mods/d2_audio.hpp:67`

### `apply`

```cpp
void d2::sys::SystemAudio::Stream::apply(auto &&func, std::size_t channel, float start=0.f, float end=1.f)
```

Source: `core/mods/d2_audio.hpp:69`

### `apply`

```cpp
void d2::sys::SystemAudio::Stream::apply(auto &&func, float start=0.f, float end=1.f)
```

Source: `core/mods/d2_audio.hpp:78`

### `push`

```cpp
void d2::sys::SystemAudio::Stream::push(std::span< float > in)
```

Source: `core/mods/d2_audio.hpp:108`

### `push_expand`

```cpp
void d2::sys::SystemAudio::Stream::push_expand(std::span< float > in)
```

Source: `core/mods/d2_audio.hpp:109`

### `push`

```cpp
void d2::sys::SystemAudio::Stream::push(std::span< float > in, float mul, float shift=0.f)
```

Source: `core/mods/d2_audio.hpp:111`

### `push_expand`

```cpp
void d2::sys::SystemAudio::Stream::push_expand(std::span< float > in, float mul, float shift=0.f)
```

Source: `core/mods/d2_audio.hpp:112`

### `stop`

```cpp
void d2::sys::SystemAudio::Stream::stop()
```

Source: `core/mods/d2_audio.hpp:114`

### `is_stopped`

```cpp
bool d2::sys::SystemAudio::Stream::is_stopped()
```

Source: `core/mods/d2_audio.hpp:115`

### `info`

```cpp
const SystemAudio::FormatInfo & d2::sys::SystemAudio::Stream::info() const
```

Source: `core/mods/d2_audio.hpp:117`

### `size`

```cpp
std::size_t d2::sys::SystemAudio::Stream::size() const
```

Source: `core/mods/d2_audio.hpp:118`

### `chunk`

```cpp
std::size_t d2::sys::SystemAudio::Stream::chunk() const
```

Source: `core/mods/d2_audio.hpp:119`

### `at`

```cpp
float & d2::sys::SystemAudio::Stream::at(std::size_t idx, std::size_t channel) const
```

Source: `core/mods/d2_audio.hpp:121`

### `at`

```cpp
float & d2::sys::SystemAudio::Stream::at(float idx, std::size_t channel) const
```

Source: `core/mods/d2_audio.hpp:122`

### `operator=`

```cpp
Stream & d2::sys::SystemAudio::Stream::operator=(const Stream &)=default
```

Source: `core/mods/d2_audio.hpp:124`

### `operator=`

```cpp
Stream & d2::sys::SystemAudio::Stream::operator=(Stream &&)=default
```

Source: `core/mods/d2_audio.hpp:125`
