# `d2::ExtendedCodePage`

**Kind:** class
**Declared in:** `core/platform/d2_extended_page.hpp:56`

## Functions

### `activate_thread`

```cpp
void d2::ExtendedCodePage::activate_thread()
```

Source: `core/platform/d2_extended_page.hpp:69`

### `deactivate_thread`

```cpp
void d2::ExtendedCodePage::deactivate_thread()
```

Source: `core/platform/d2_extended_page.hpp:70`

### `capacity`

```cpp
static constexpr std::size_t d2::ExtendedCodePage::capacity() noexcept
```

Source: `core/platform/d2_extended_page.hpp:78`

### `ExtendedCodePage`

```cpp
d2::ExtendedCodePage::ExtendedCodePage()=default
```

Source: `core/platform/d2_extended_page.hpp:72`

### `ExtendedCodePage`

```cpp
d2::ExtendedCodePage::ExtendedCodePage(const ExtendedCodePage &)=delete
```

Source: `core/platform/d2_extended_page.hpp:73`

### `ExtendedCodePage`

```cpp
d2::ExtendedCodePage::ExtendedCodePage(ExtendedCodePage &&)=default
```

Source: `core/platform/d2_extended_page.hpp:74`

### `is_activated`

```cpp
bool d2::ExtendedCodePage::is_activated() const
```

Source: `core/platform/d2_extended_page.hpp:76`

### `is_extended`

```cpp
bool d2::ExtendedCodePage::is_extended(standard_value_type value) const
```

Source: `core/platform/d2_extended_page.hpp:77`

### `size`

```cpp
std::size_t d2::ExtendedCodePage::size() const noexcept
```

Source: `core/platform/d2_extended_page.hpp:79`

### `try_write`

```cpp
std::optional< standard_value_type > d2::ExtendedCodePage::try_write(std::string_view value)
```

Source: `core/platform/d2_extended_page.hpp:80`

### `write`

```cpp
standard_value_type d2::ExtendedCodePage::write(std::string_view value)
```

Source: `core/platform/d2_extended_page.hpp:81`

### `read`

```cpp
std::string_view d2::ExtendedCodePage::read(standard_value_type value) const
```

Source: `core/platform/d2_extended_page.hpp:82`

### `clear`

```cpp
void d2::ExtendedCodePage::clear()
```

Source: `core/platform/d2_extended_page.hpp:83`

### `activate`

```cpp
void d2::ExtendedCodePage::activate()
```

Source: `core/platform/d2_extended_page.hpp:84`

### `deactivate`

```cpp
void d2::ExtendedCodePage::deactivate()
```

Source: `core/platform/d2_extended_page.hpp:85`

### `operator=`

```cpp
ExtendedCodePage & d2::ExtendedCodePage::operator=(const ExtendedCodePage &)=delete
```

Source: `core/platform/d2_extended_page.hpp:87`

### `operator=`

```cpp
ExtendedCodePage & d2::ExtendedCodePage::operator=(ExtendedCodePage &&)=default
```

Source: `core/platform/d2_extended_page.hpp:88`
