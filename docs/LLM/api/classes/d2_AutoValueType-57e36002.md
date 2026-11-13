# `d2::AutoValueType`

**Kind:** class
**Declared in:** `core/platform/d2_extended_page.hpp:14`

## Functions

### `AutoValueType`

```cpp
d2::AutoValueType::AutoValueType()=default
```

Source: `core/platform/d2_extended_page.hpp:19`

### `AutoValueType`

```cpp
d2::AutoValueType::AutoValueType(standard_value_type ch)
```

Source: `core/platform/d2_extended_page.hpp:20`

### `AutoValueType`

```cpp
d2::AutoValueType::AutoValueType(const std::string &ext)
```

Source: `core/platform/d2_extended_page.hpp:21`

### `AutoValueType`

```cpp
d2::AutoValueType::AutoValueType(const char *ext)
```

Source: `core/platform/d2_extended_page.hpp:22`

### `AutoValueType`

```cpp
d2::AutoValueType::AutoValueType(const AutoValueType &other)=default
```

Source: `core/platform/d2_extended_page.hpp:23`

### `operator=`

```cpp
AutoValueType & d2::AutoValueType::operator=(standard_value_type ch)
```

Source: `core/platform/d2_extended_page.hpp:25`

### `operator=`

```cpp
AutoValueType & d2::AutoValueType::operator=(const AutoValueType &copy)=default
```

Source: `core/platform/d2_extended_page.hpp:26`

### `operator const standard_value_type &`

```cpp
d2::AutoValueType::operator const standard_value_type &() const
```

Source: `core/platform/d2_extended_page.hpp:28`

### `operator standard_value_type &`

```cpp
d2::AutoValueType::operator standard_value_type &()
```

Source: `core/platform/d2_extended_page.hpp:29`

### `operator==`

```cpp
bool d2::AutoValueType::operator==(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:31`

### `operator!=`

```cpp
bool d2::AutoValueType::operator!=(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:32`

### `operator<`

```cpp
bool d2::AutoValueType::operator<(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:33`

### `operator>`

```cpp
bool d2::AutoValueType::operator>(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:34`

### `operator<=`

```cpp
bool d2::AutoValueType::operator<=(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:35`

### `operator>=`

```cpp
bool d2::AutoValueType::operator>=(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:36`

### `operator+`

```cpp
AutoValueType d2::AutoValueType::operator+(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:38`

### `operator-`

```cpp
AutoValueType d2::AutoValueType::operator-(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:39`

### `operator*`

```cpp
AutoValueType d2::AutoValueType::operator*(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:40`

### `operator/`

```cpp
AutoValueType d2::AutoValueType::operator/(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:41`

### `operator%`

```cpp
AutoValueType d2::AutoValueType::operator%(const standard_value_type &copy) const
```

Source: `core/platform/d2_extended_page.hpp:42`

### `operator+=`

```cpp
AutoValueType & d2::AutoValueType::operator+=(const standard_value_type &copy)
```

Source: `core/platform/d2_extended_page.hpp:44`

### `operator-=`

```cpp
AutoValueType & d2::AutoValueType::operator-=(const standard_value_type &copy)
```

Source: `core/platform/d2_extended_page.hpp:45`

### `operator*=`

```cpp
AutoValueType & d2::AutoValueType::operator*=(const standard_value_type &copy)
```

Source: `core/platform/d2_extended_page.hpp:46`

### `operator/=`

```cpp
AutoValueType & d2::AutoValueType::operator/=(const standard_value_type &copy)
```

Source: `core/platform/d2_extended_page.hpp:47`

### `operator%=`

```cpp
AutoValueType & d2::AutoValueType::operator%=(const standard_value_type &copy)
```

Source: `core/platform/d2_extended_page.hpp:48`

### `operator++`

```cpp
AutoValueType & d2::AutoValueType::operator++()
```

Source: `core/platform/d2_extended_page.hpp:50`

### `operator++`

```cpp
AutoValueType d2::AutoValueType::operator++(int)
```

Source: `core/platform/d2_extended_page.hpp:51`

### `operator--`

```cpp
AutoValueType & d2::AutoValueType::operator--()
```

Source: `core/platform/d2_extended_page.hpp:52`

### `operator--`

```cpp
AutoValueType d2::AutoValueType::operator--(int)
```

Source: `core/platform/d2_extended_page.hpp:53`
