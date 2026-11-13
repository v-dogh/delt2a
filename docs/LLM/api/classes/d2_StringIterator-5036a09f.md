# `d2::StringIterator`

**Kind:** class
**Declared in:** `core/platform/d2_platform_string.hpp:14`

## Functions

### `distance`

```cpp
std::size_t d2::StringIterator::distance(const StringIterator &first, const StringIterator &second)
```

Source: `core/platform/d2_platform_string.hpp:20`

### `StringIterator`

```cpp
d2::StringIterator::StringIterator(StringIterator &)=default
```

Source: `core/platform/d2_platform_string.hpp:22`

### `StringIterator`

```cpp
d2::StringIterator::StringIterator(const StringIterator &)=default
```

Source: `core/platform/d2_platform_string.hpp:23`

### `StringIterator`

```cpp
d2::StringIterator::StringIterator(string_view str)
```

Source: `core/platform/d2_platform_string.hpp:24`

### `StringIterator`

```cpp
d2::StringIterator::StringIterator(string_view str, string_view::iterator it)
```

Source: `core/platform/d2_platform_string.hpp:25`

### `begin`

```cpp
StringIterator d2::StringIterator::begin() const
```

Source: `core/platform/d2_platform_string.hpp:27`

### `end`

```cpp
StringIterator d2::StringIterator::end() const
```

Source: `core/platform/d2_platform_string.hpp:28`

### `increment`

```cpp
bool d2::StringIterator::increment()
```

Source: `core/platform/d2_platform_string.hpp:30`

### `current`

```cpp
string_view d2::StringIterator::current() const
```

Source: `core/platform/d2_platform_string.hpp:31`

### `is_end`

```cpp
bool d2::StringIterator::is_end() const
```

Source: `core/platform/d2_platform_string.hpp:33`

### `is_space`

```cpp
bool d2::StringIterator::is_space() const
```

Source: `core/platform/d2_platform_string.hpp:34`

### `is_endline`

```cpp
bool d2::StringIterator::is_endline() const
```

Source: `core/platform/d2_platform_string.hpp:35`

### `operator=`

```cpp
StringIterator & d2::StringIterator::operator=(const StringIterator &)=default
```

Source: `core/platform/d2_platform_string.hpp:37`

### `operator=`

```cpp
StringIterator & d2::StringIterator::operator=(StringIterator &&)=default
```

Source: `core/platform/d2_platform_string.hpp:38`

### `operator==`

```cpp
bool d2::StringIterator::operator==(const StringIterator &copy) const
```

Source: `core/platform/d2_platform_string.hpp:40`

### `operator++`

```cpp
StringIterator & d2::StringIterator::operator++()
```

Source: `core/platform/d2_platform_string.hpp:41`

### `operator++`

```cpp
StringIterator d2::StringIterator::operator++(int)
```

Source: `core/platform/d2_platform_string.hpp:42`

### `operator*`

```cpp
string_view d2::StringIterator::operator*() const
```

Source: `core/platform/d2_platform_string.hpp:43`
