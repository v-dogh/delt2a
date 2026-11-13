# `d2::TextSelection`

**Kind:** class
**Declared in:** `core/tree/d2_text_select.hpp:27`

## Functions

### `TextSelection`

```cpp
d2::TextSelection::TextSelection()=default
```

Source: `core/tree/d2_text_select.hpp:53`

### `TextSelection`

```cpp
d2::TextSelection::TextSelection(const TextSelection &)=delete
```

Source: `core/tree/d2_text_select.hpp:54`

### `operator=`

```cpp
TextSelection & d2::TextSelection::operator=(const TextSelection &)=delete
```

Source: `core/tree/d2_text_select.hpp:55`

### `~TextSelection`

```cpp
d2::TextSelection::~TextSelection()
```

Source: `core/tree/d2_text_select.hpp:56`

### `begin`

```cpp
bool d2::TextSelection::begin(const std::shared_ptr< Element > &root, Position position)
```

Source: `core/tree/d2_text_select.hpp:58`

### `extend`

```cpp
bool d2::TextSelection::extend(Position position)
```

Source: `core/tree/d2_text_select.hpp:59`

### `finish`

```cpp
void d2::TextSelection::finish() noexcept
```

Source: `core/tree/d2_text_select.hpp:60`

### `dragging`

```cpp
bool d2::TextSelection::dragging() const noexcept
```

Source: `core/tree/d2_text_select.hpp:61`

### `refresh`

```cpp
bool d2::TextSelection::refresh()
```

Source: `core/tree/d2_text_select.hpp:62`

### `empty`

```cpp
bool d2::TextSelection::empty() const noexcept
```

Source: `core/tree/d2_text_select.hpp:63`

### `value`

```cpp
std::string d2::TextSelection::value()
```

Source: `core/tree/d2_text_select.hpp:64`

### `select_all`

```cpp
void d2::TextSelection::select_all(const std::shared_ptr< Element > &root)
```

Source: `core/tree/d2_text_select.hpp:65`

### `clear`

```cpp
void d2::TextSelection::clear()
```

Source: `core/tree/d2_text_select.hpp:66`
