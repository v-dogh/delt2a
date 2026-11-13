# `d2::PixelBuffer`

**Kind:** class
**Declared in:** `core/types/d2_pixel.hpp:15`

## Enums

### `RleBreak`

```cpp
enum RleBreak
```

Source: `core/types/d2_pixel.hpp:18`

Values: `Continue`, `SkipRest`, `SkipLine`


## Functions

### `rle_pack`

```cpp
std::vector< pixel > d2::PixelBuffer::rle_pack(std::span< const pixel > buffer)
```

Source: `core/types/d2_pixel.hpp:115`

### `rle_unpack`

```cpp
std::vector< pixel > d2::PixelBuffer::rle_unpack(std::span< const pixel > buffer)
```

Source: `core/types/d2_pixel.hpp:116`

### `rle_walk`

```cpp
void d2::PixelBuffer::rle_walk(std::span< const pixel > buffer, std::function< bool(const pixel &)> func)
```

Source: `core/types/d2_pixel.hpp:117`

### `rle_mdwalk`

```cpp
void d2::PixelBuffer::rle_mdwalk(std::span< const pixel > buffer, int width, int height, std::function< RleBreak(int, int, const pixel &)> func)
```

Source: `core/types/d2_pixel.hpp:118`

### `rle_iterator`

```cpp
PixelBuffer::RleIterator d2::PixelBuffer::rle_iterator(std::span< const pixel > buffer)
```

Source: `core/types/d2_pixel.hpp:124`

### `PixelBuffer`

```cpp
d2::PixelBuffer::PixelBuffer()=default
```

Source: `core/types/d2_pixel.hpp:126`

### `PixelBuffer`

```cpp
d2::PixelBuffer::PixelBuffer(int w, int h)
```

Source: `core/types/d2_pixel.hpp:127`

### `PixelBuffer`

```cpp
d2::PixelBuffer::PixelBuffer(const PixelBuffer &)=default
```

Source: `core/types/d2_pixel.hpp:128`

### `PixelBuffer`

```cpp
d2::PixelBuffer::PixelBuffer(PixelBuffer &&)=default
```

Source: `core/types/d2_pixel.hpp:129`

### `data`

```cpp
std::span< const pixel > d2::PixelBuffer::data() const
```

Source: `core/types/d2_pixel.hpp:131`

### `data`

```cpp
std::span< const pixel > d2::PixelBuffer::data()
```

Source: `core/types/d2_pixel.hpp:132`

### `clear`

```cpp
void d2::PixelBuffer::clear()
```

Source: `core/types/d2_pixel.hpp:134`

### `fill`

```cpp
void d2::PixelBuffer::fill(pixel px)
```

Source: `core/types/d2_pixel.hpp:135`

### `fill`

```cpp
void d2::PixelBuffer::fill(pixel px, int x, int y, int width, int height)
```

Source: `core/types/d2_pixel.hpp:136`

### `fill_blend`

```cpp
void d2::PixelBuffer::fill_blend(pixel px)
```

Source: `core/types/d2_pixel.hpp:137`

### `fill_blend`

```cpp
void d2::PixelBuffer::fill_blend(pixel px, int x, int y, int width, int height)
```

Source: `core/types/d2_pixel.hpp:138`

### `empty`

```cpp
bool d2::PixelBuffer::empty() const
```

Source: `core/types/d2_pixel.hpp:140`

### `compressed`

```cpp
bool d2::PixelBuffer::compressed() const
```

Source: `core/types/d2_pixel.hpp:141`

### `set_size`

```cpp
void d2::PixelBuffer::set_size(int w, int h)
```

Source: `core/types/d2_pixel.hpp:143`

### `reset`

```cpp
void d2::PixelBuffer::reset(std::vector< pixel > data, int w, int h)
```

Source: `core/types/d2_pixel.hpp:144`

### `compress`

```cpp
void d2::PixelBuffer::compress()
```

Source: `core/types/d2_pixel.hpp:145`

### `width`

```cpp
int d2::PixelBuffer::width() const
```

Source: `core/types/d2_pixel.hpp:147`

### `height`

```cpp
int d2::PixelBuffer::height() const
```

Source: `core/types/d2_pixel.hpp:148`

### `at`

```cpp
const pixel & d2::PixelBuffer::at(int x, int y) const
```

Source: `core/types/d2_pixel.hpp:150`

### `at`

```cpp
pixel & d2::PixelBuffer::at(int x, int y)
```

Source: `core/types/d2_pixel.hpp:151`

### `at`

```cpp
const pixel & d2::PixelBuffer::at(int c) const
```

Source: `core/types/d2_pixel.hpp:153`

### `at`

```cpp
pixel & d2::PixelBuffer::at(int c)
```

Source: `core/types/d2_pixel.hpp:154`

### `inscribe`

```cpp
void d2::PixelBuffer::inscribe(int xf, int yf, View view)
```

Source: `core/types/d2_pixel.hpp:156`

### `image_scene`

```cpp
const image::Scene & d2::PixelBuffer::image_scene() const
```

Source: `core/types/d2_pixel.hpp:157`

### `operator=`

```cpp
PixelBuffer & d2::PixelBuffer::operator=(const PixelBuffer &)=default
```

Source: `core/types/d2_pixel.hpp:159`

### `operator=`

```cpp
PixelBuffer & d2::PixelBuffer::operator=(PixelBuffer &&)=default
```

Source: `core/types/d2_pixel.hpp:160`


## Variables

### `buffer_`

```cpp
std::vector<pixel> d2::PixelBuffer::buffer_
```

Source: `core/types/d2_pixel.hpp:109`

### `width_`

```cpp
int d2::PixelBuffer::width_
```

Source: `core/types/d2_pixel.hpp:110`

### `height_`

```cpp
int d2::PixelBuffer::height_
```

Source: `core/types/d2_pixel.hpp:111`

### `compressed_`

```cpp
bool d2::PixelBuffer::compressed_
```

Source: `core/types/d2_pixel.hpp:112`

### `images_`

```cpp
image::Scene d2::PixelBuffer::images_
```

Source: `core/types/d2_pixel.hpp:113`
