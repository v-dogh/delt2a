# `d2::PixelBuffer::View`

**Kind:** class
**Declared in:** `core/types/d2_pixel.hpp:24`

## Functions

### `View`

```cpp
d2::PixelBuffer::View::View()=default
```

Source: `core/types/d2_pixel.hpp:33`

### `View`

```cpp
d2::PixelBuffer::View::View(std::nullptr_t)
```

Source: `core/types/d2_pixel.hpp:34`

### `View`

```cpp
d2::PixelBuffer::View::View(View &&)=default
```

Source: `core/types/d2_pixel.hpp:35`

### `View`

```cpp
d2::PixelBuffer::View::View(const View &)=default
```

Source: `core/types/d2_pixel.hpp:36`

### `View`

```cpp
d2::PixelBuffer::View::View(PixelBuffer *ptr, int x, int y, int width, int height)
```

Source: `core/types/d2_pixel.hpp:37`

### `View`

```cpp
d2::PixelBuffer::View::View(const View &ptr, int x, int y, int width, int height)
```

Source: `core/types/d2_pixel.hpp:45`

### `data`

```cpp
std::span< const pixel > d2::PixelBuffer::View::data() const
```

Source: `core/types/d2_pixel.hpp:56`

### `clear`

```cpp
void d2::PixelBuffer::View::clear()
```

Source: `core/types/d2_pixel.hpp:58`

### `fill`

```cpp
void d2::PixelBuffer::View::fill(pixel px)
```

Source: `core/types/d2_pixel.hpp:59`

### `fill`

```cpp
void d2::PixelBuffer::View::fill(pixel px, int x, int y, int width, int height)
```

Source: `core/types/d2_pixel.hpp:60`

### `fill_blend`

```cpp
void d2::PixelBuffer::View::fill_blend(pixel px)
```

Source: `core/types/d2_pixel.hpp:61`

### `fill_blend`

```cpp
void d2::PixelBuffer::View::fill_blend(pixel px, int x, int y, int width, int height)
```

Source: `core/types/d2_pixel.hpp:62`

### `compressed`

```cpp
bool d2::PixelBuffer::View::compressed() const
```

Source: `core/types/d2_pixel.hpp:64`

### `empty`

```cpp
bool d2::PixelBuffer::View::empty() const
```

Source: `core/types/d2_pixel.hpp:65`

### `width`

```cpp
int d2::PixelBuffer::View::width() const
```

Source: `core/types/d2_pixel.hpp:67`

### `height`

```cpp
int d2::PixelBuffer::View::height() const
```

Source: `core/types/d2_pixel.hpp:68`

### `xpos`

```cpp
int d2::PixelBuffer::View::xpos() const
```

Source: `core/types/d2_pixel.hpp:69`

### `ypos`

```cpp
int d2::PixelBuffer::View::ypos() const
```

Source: `core/types/d2_pixel.hpp:70`

### `at`

```cpp
const pixel & d2::PixelBuffer::View::at(int x, int y) const
```

Source: `core/types/d2_pixel.hpp:72`

### `at`

```cpp
pixel & d2::PixelBuffer::View::at(int x, int y)
```

Source: `core/types/d2_pixel.hpp:73`

### `at`

```cpp
const pixel & d2::PixelBuffer::View::at(int c) const
```

Source: `core/types/d2_pixel.hpp:75`

### `at`

```cpp
pixel & d2::PixelBuffer::View::at(int c)
```

Source: `core/types/d2_pixel.hpp:76`

### `inscribe`

```cpp
void d2::PixelBuffer::View::inscribe(int x, int y, const View &sub)
```

Source: `core/types/d2_pixel.hpp:78`

### `image`

```cpp
void d2::PixelBuffer::View::image(image::Raster::ptr source, image::Rect destination, image::Rect crop={})
```

Source: `core/types/d2_pixel.hpp:79`

### `image_scene`

```cpp
image::Scene d2::PixelBuffer::View::image_scene() const
```

Source: `core/types/d2_pixel.hpp:80`

### `has_images`

```cpp
bool d2::PixelBuffer::View::has_images() const
```

Source: `core/types/d2_pixel.hpp:81`

### `unpack`

```cpp
std::vector< pixel > d2::PixelBuffer::View::unpack() const
```

Source: `core/types/d2_pixel.hpp:82`

### `operator=`

```cpp
View & d2::PixelBuffer::View::operator=(const View &)=default
```

Source: `core/types/d2_pixel.hpp:84`

### `operator=`

```cpp
View & d2::PixelBuffer::View::operator=(View &&)=default
```

Source: `core/types/d2_pixel.hpp:85`
