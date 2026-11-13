# `d2::Animation`

**Kind:** class
**Declared in:** `core/screen/d2_animate.hpp:102`

**Bases:** `std::enable_shared_from_this< Animation >`

## Functions

### `_progress`

```cpp
float d2::Animation::_progress() const
```

Source: `core/screen/d2_animate.hpp:124`

### `_start_impl`

```cpp
virtual void d2::Animation::_start_impl()
```

Source: `core/screen/d2_animate.hpp:125`

### `_update_impl`

```cpp
virtual float d2::Animation::_update_impl(std::shared_ptr< Element > ptr, float progress)=0
```

Source: `core/screen/d2_animate.hpp:126`

### `_keep_alive_impl`

```cpp
virtual bool d2::Animation::_keep_alive_impl(float progress)
```

Source: `core/screen/d2_animate.hpp:127`

### `Animation`

```cpp
d2::Animation::Animation(std::chrono::milliseconds ms, std::shared_ptr< Element > ptr, style::uai_property prop)
```

Source: `core/screen/d2_animate.hpp:132`

### `~Animation`

```cpp
virtual d2::Animation::~Animation()=default
```

Source: `core/screen/d2_animate.hpp:135`

### `target`

```cpp
std::pair< std::weak_ptr< Element >, style::uai_property > d2::Animation::target() const
```

Source: `core/screen/d2_animate.hpp:137`

### `stop`

```cpp
void d2::Animation::stop()
```

Source: `core/screen/d2_animate.hpp:139`

### `mute`

```cpp
void d2::Animation::mute()
```

Source: `core/screen/d2_animate.hpp:140`

### `unmute`

```cpp
void d2::Animation::unmute()
```

Source: `core/screen/d2_animate.hpp:141`

### `start`

```cpp
void d2::Animation::start()
```

Source: `core/screen/d2_animate.hpp:142`

### `keep_alive`

```cpp
bool d2::Animation::keep_alive()
```

Source: `core/screen/d2_animate.hpp:143`

### `update`

```cpp
std::chrono::milliseconds d2::Animation::update()
```

Source: `core/screen/d2_animate.hpp:144`

### `duration`

```cpp
std::chrono::milliseconds d2::Animation::duration() const noexcept
```

Source: `core/screen/d2_animate.hpp:145`


## Typedefs

### `ptr`

```cpp
using d2::Animation::ptr = std::shared_ptr<Animation>
```

Source: `core/screen/d2_animate.hpp:112`
