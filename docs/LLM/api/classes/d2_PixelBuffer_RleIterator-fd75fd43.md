# `d2::PixelBuffer::RleIterator`

**Kind:** class
**Declared in:** `core/types/d2_pixel.hpp:87`

## Functions

### `RleIterator`

```cpp
d2::PixelBuffer::RleIterator::RleIterator()=default
```

Source: `core/types/d2_pixel.hpp:94`

### `RleIterator`

```cpp
d2::PixelBuffer::RleIterator::RleIterator(std::nullptr_t)
```

Source: `core/types/d2_pixel.hpp:95`

### `RleIterator`

```cpp
d2::PixelBuffer::RleIterator::RleIterator(std::span< const px::combined > buffer)
```

Source: `core/types/d2_pixel.hpp:96`

### `RleIterator`

```cpp
d2::PixelBuffer::RleIterator::RleIterator(const RleIterator &)=default
```

Source: `core/types/d2_pixel.hpp:97`

### `RleIterator`

```cpp
d2::PixelBuffer::RleIterator::RleIterator(RleIterator &&)=default
```

Source: `core/types/d2_pixel.hpp:98`

### `value`

```cpp
const px::combined & d2::PixelBuffer::RleIterator::value() const
```

Source: `core/types/d2_pixel.hpp:100`

### `increment`

```cpp
void d2::PixelBuffer::RleIterator::increment()
```

Source: `core/types/d2_pixel.hpp:101`

### `increment`

```cpp
void d2::PixelBuffer::RleIterator::increment(std::size_t cnt)
```

Source: `core/types/d2_pixel.hpp:102`

### `is_end`

```cpp
bool d2::PixelBuffer::RleIterator::is_end() const
```

Source: `core/types/d2_pixel.hpp:103`

### `operator=`

```cpp
RleIterator & d2::PixelBuffer::RleIterator::operator=(const RleIterator &)=default
```

Source: `core/types/d2_pixel.hpp:105`

### `operator=`

```cpp
RleIterator & d2::PixelBuffer::RleIterator::operator=(RleIterator &&)=default
```

Source: `core/types/d2_pixel.hpp:106`
