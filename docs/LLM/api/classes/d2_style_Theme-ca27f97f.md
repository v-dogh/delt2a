# `d2::style::Theme`

**Kind:** class
**Declared in:** `core/tree/d2_theme.hpp:613`

**Bases:** `std::enable_shared_from_this< Theme >`

## Functions

### `id_to_code`

```cpp
static std::size_t d2::style::Theme::id_to_code(const std::string &id)
```

Source: `core/tree/d2_theme.hpp:620`

### `make`

```cpp
template<typename Type>
static auto d2::style::Theme::make(std::vector< px::background > accents={})
```

Source: `core/tree/d2_theme.hpp:625`

### `make_named`

```cpp
template<typename Type>
static auto d2::style::Theme::make_named(const std::string &name, std::vector< px::background > accents={})
```

Source: `core/tree/d2_theme.hpp:633`

### `Theme`

```cpp
d2::style::Theme::Theme()=default
```

Source: `core/tree/d2_theme.hpp:641`

### `Theme`

```cpp
d2::style::Theme::Theme(const Theme &)=default
```

Source: `core/tree/d2_theme.hpp:642`

### `Theme`

```cpp
d2::style::Theme::Theme(Theme &&)=default
```

Source: `core/tree/d2_theme.hpp:643`

### `~Theme`

```cpp
virtual d2::style::Theme::~Theme()=default
```

Source: `core/tree/d2_theme.hpp:644`

### `code`

```cpp
std::size_t d2::style::Theme::code() const noexcept
```

Source: `core/tree/d2_theme.hpp:646`

### `as`

```cpp
template<typename Theme>
Theme & d2::style::Theme::as()
```

Source: `core/tree/d2_theme.hpp:651`

### `operator=`

```cpp
Theme & d2::style::Theme::operator=(const Theme &)=default
```

Source: `core/tree/d2_theme.hpp:656`

### `operator=`

```cpp
Theme & d2::style::Theme::operator=(Theme &&)=default
```

Source: `core/tree/d2_theme.hpp:657`


## Typedefs

### `ptr`

```cpp
using d2::style::Theme::ptr = std::shared_ptr<Theme>
```

Source: `core/tree/d2_theme.hpp:616`
