# `d2::MatrixModel`

**Kind:** class
**Declared in:** `core/utils/d2_model.hpp:9`

**Bases:** `d2::PixelBuffer`

## Enums

### `ModelType`

```cpp
enum ModelType
```

Source: `core/utils/d2_model.hpp:13`

Values: `Raw`, `Visual`

### `ModelHeader`

```cpp
enum ModelHeader
```

Source: `core/utils/d2_model.hpp:18`

Values: `Endian`, `Compressed`, `Extended`


## Functions

### `make`

```cpp
MatrixModel::ptr d2::MatrixModel::make(int width=0, int height=0)
```

Source: `core/utils/d2_model.hpp:28`

### `load_model`

```cpp
void d2::MatrixModel::load_model(const std::string &path, ModelType type)
```

Source: `core/utils/d2_model.hpp:30`

### `save_model`

```cpp
void d2::MatrixModel::save_model(const std::string &path, ModelType type)
```

Source: `core/utils/d2_model.hpp:31`

### `PixelBuffer`

```cpp
d2::PixelBuffer::PixelBuffer()=default
```

Source: `core/utils/d2_model.hpp:26`

### `PixelBuffer`

```cpp
d2::PixelBuffer::PixelBuffer(int w, int h)
```

Source: `core/utils/d2_model.hpp:26`

### `PixelBuffer`

```cpp
d2::PixelBuffer::PixelBuffer(const PixelBuffer &)=default
```

Source: `core/utils/d2_model.hpp:26`

### `PixelBuffer`

```cpp
d2::PixelBuffer::PixelBuffer(PixelBuffer &&)=default
```

Source: `core/utils/d2_model.hpp:26`


## Typedefs

### `ptr`

```cpp
using d2::MatrixModel::ptr = std::shared_ptr<MatrixModel>
```

Source: `core/utils/d2_model.hpp:25`
