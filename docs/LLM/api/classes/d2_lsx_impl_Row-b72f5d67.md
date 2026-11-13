# `d2::lsx::impl::Row`

**Kind:** class
**Declared in:** `core/lsx/d2_solver.hpp:32`

## Functions

### `Row`

```cpp
d2::lsx::impl::Row::Row()=default
```

Source: `core/lsx/d2_solver.hpp:39`

### `Row`

```cpp
d2::lsx::impl::Row::Row(const Row &)=default
```

Source: `core/lsx/d2_solver.hpp:40`

### `Row`

```cpp
d2::lsx::impl::Row::Row(Row &&)=default
```

Source: `core/lsx/d2_solver.hpp:41`

### `substitute`

```cpp
void d2::lsx::impl::Row::substitute(Symbol lhs, const Row &rhs)
```

Source: `core/lsx/d2_solver.hpp:44`

### `normalize`

```cpp
void d2::lsx::impl::Row::normalize()
```

Source: `core/lsx/d2_solver.hpp:46`

### `find`

```cpp
std::optional< Symbol > d2::lsx::impl::Row::find(Symbol symbol)
```

Source: `core/lsx/d2_solver.hpp:49`

### `insert`

```cpp
void d2::lsx::impl::Row::insert(Symbol term)
```

Source: `core/lsx/d2_solver.hpp:52`

### `insert`

```cpp
template<typename... Argv>
void d2::lsx::impl::Row::insert(Argv &&... args)
```

Source: `core/lsx/d2_solver.hpp:53`

### `insert_range`

```cpp
void d2::lsx::impl::Row::insert_range(auto &&begin, auto &&end)
```

Source: `core/lsx/d2_solver.hpp:57`

### `erase`

```cpp
void d2::lsx::impl::Row::erase(Symbol term)
```

Source: `core/lsx/d2_solver.hpp:63`

### `reserve`

```cpp
void d2::lsx::impl::Row::reserve(std::size_t count)
```

Source: `core/lsx/d2_solver.hpp:65`

### `contains`

```cpp
bool d2::lsx::impl::Row::contains(Symbol term)
```

Source: `core/lsx/d2_solver.hpp:68`

### `size`

```cpp
std::size_t d2::lsx::impl::Row::size() const
```

Source: `core/lsx/d2_solver.hpp:70`

### `constant`

```cpp
const result_type & d2::lsx::impl::Row::constant() const
```

Source: `core/lsx/d2_solver.hpp:72`

### `constant`

```cpp
result_type & d2::lsx::impl::Row::constant()
```

Source: `core/lsx/d2_solver.hpp:73`

### `foreach`

```cpp
void d2::lsx::impl::Row::foreach(auto &&func) const
```

Source: `core/lsx/d2_solver.hpp:75`

### `foreach`

```cpp
void d2::lsx::impl::Row::foreach(auto &&func)
```

Source: `core/lsx/d2_solver.hpp:81`

### `print`

```cpp
std::string d2::lsx::impl::Row::print() const
```

Source: `core/lsx/d2_solver.hpp:88`

### `operator=`

```cpp
Row & d2::lsx::impl::Row::operator=(const Row &)=default
```

Source: `core/lsx/d2_solver.hpp:90`

### `operator=`

```cpp
Row & d2::lsx::impl::Row::operator=(Row &&)=default
```

Source: `core/lsx/d2_solver.hpp:91`

### `operator*=`

```cpp
Row & d2::lsx::impl::Row::operator*=(result_type cf)
```

Source: `core/lsx/d2_solver.hpp:93`

### `operator/=`

```cpp
Row & d2::lsx::impl::Row::operator/=(result_type cf)
```

Source: `core/lsx/d2_solver.hpp:94`
