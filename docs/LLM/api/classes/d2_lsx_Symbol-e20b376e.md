# `d2::lsx::Symbol`

**Kind:** class
**Declared in:** `core/lsx/d2_solver_types.hpp:143`

**Bases:** `d2::lsx::VariableIndex`

## Functions

### `Symbol`

```cpp
d2::lsx::Symbol::Symbol()=default
```

Source: `core/lsx/d2_solver_types.hpp:148`

### `Symbol`

```cpp
d2::lsx::Symbol::Symbol(const Variable &var)
```

Source: `core/lsx/d2_solver_types.hpp:149`

### `Symbol`

```cpp
d2::lsx::Symbol::Symbol(VariableIndex idx, result_type coefficient=1.f)
```

Source: `core/lsx/d2_solver_types.hpp:151`

### `Symbol`

```cpp
d2::lsx::Symbol::Symbol(result_type coefficient)
```

Source: `core/lsx/d2_solver_types.hpp:153`

### `coefficient`

```cpp
result_type d2::lsx::Symbol::coefficient() const
```

Source: `core/lsx/d2_solver_types.hpp:156`

### `invert`

```cpp
Symbol d2::lsx::Symbol::invert() const
```

Source: `core/lsx/d2_solver_types.hpp:160`

### `print`

```cpp
std::string d2::lsx::Symbol::print() const
```

Source: `core/lsx/d2_solver_types.hpp:165`

### `operator*=`

```cpp
const Symbol & d2::lsx::Symbol::operator*=(result_type cf) const
```

Source: `core/lsx/d2_solver_types.hpp:167`

### `operator/=`

```cpp
const Symbol & d2::lsx::Symbol::operator/=(result_type cf) const
```

Source: `core/lsx/d2_solver_types.hpp:172`

### `operator+=`

```cpp
const Symbol & d2::lsx::Symbol::operator+=(result_type cf) const
```

Source: `core/lsx/d2_solver_types.hpp:177`

### `operator-=`

```cpp
const Symbol & d2::lsx::Symbol::operator-=(result_type cf) const
```

Source: `core/lsx/d2_solver_types.hpp:182`

### `operator=`

```cpp
const Symbol & d2::lsx::Symbol::operator=(result_type cf) const
```

Source: `core/lsx/d2_solver_types.hpp:187`
