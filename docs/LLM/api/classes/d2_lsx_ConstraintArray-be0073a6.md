# `d2::lsx::ConstraintArray`

**Kind:** class
**Declared in:** `core/lsx/d2_solver.hpp:122`

## Enums

### `Status`

```cpp
enum Status
```

Source: `core/lsx/d2_solver.hpp:126`

Values: `NotFound`, `Unsolvable`, `Ok`


## Functions

### `status_to_string`

```cpp
std::string d2::lsx::ConstraintArray::status_to_string(Status status)
```

Source: `core/lsx/d2_solver.hpp:164`

### `ConstraintArray`

```cpp
d2::lsx::ConstraintArray::ConstraintArray()=default
```

Source: `core/lsx/d2_solver.hpp:166`

### `ConstraintArray`

```cpp
d2::lsx::ConstraintArray::ConstraintArray(const ConstraintArray &)=default
```

Source: `core/lsx/d2_solver.hpp:167`

### `ConstraintArray`

```cpp
d2::lsx::ConstraintArray::ConstraintArray(ConstraintArray &&)=default
```

Source: `core/lsx/d2_solver.hpp:168`

### `status`

```cpp
ConstraintArray::Status d2::lsx::ConstraintArray::status(result_type value) const
```

Source: `core/lsx/d2_solver.hpp:171`

### `remove_constraint`

```cpp
bool d2::lsx::ConstraintArray::remove_constraint(handle handle)
```

Source: `core/lsx/d2_solver.hpp:174`

### `set_constraint`

```cpp
ConstraintArray::handle d2::lsx::ConstraintArray::set_constraint(const Expression &expr, Strength strength=Strength::Required)
```

Source: `core/lsx/d2_solver.hpp:175`

### `set_constraint`

```cpp
ConstraintArray::handle d2::lsx::ConstraintArray::set_constraint(const Expression &expr, float strength)
```

Source: `core/lsx/d2_solver.hpp:176`

### `get`

```cpp
result_type d2::lsx::ConstraintArray::get(Variable var) const
```

Source: `core/lsx/d2_solver.hpp:178`

### `get_or`

```cpp
result_type d2::lsx::ConstraintArray::get_or(Variable var, auto &&func, result_type fallback) const
```

Source: `core/lsx/d2_solver.hpp:179`

### `get_or`

```cpp
std::optional< result_type > d2::lsx::ConstraintArray::get_or(Variable var, auto &&func) const
```

Source: `core/lsx/d2_solver.hpp:193`

### `start_batch`

```cpp
void d2::lsx::ConstraintArray::start_batch()
```

Source: `core/lsx/d2_solver.hpp:208`

### `end_batch`

```cpp
void d2::lsx::ConstraintArray::end_batch()
```

Source: `core/lsx/d2_solver.hpp:209`

### `batch`

```cpp
void d2::lsx::ConstraintArray::batch(auto &&func)
```

Source: `core/lsx/d2_solver.hpp:214`

### `clear`

```cpp
void d2::lsx::ConstraintArray::clear()
```

Source: `core/lsx/d2_solver.hpp:220`

### `print`

```cpp
std::string d2::lsx::ConstraintArray::print() const
```

Source: `core/lsx/d2_solver.hpp:222`

### `operator=`

```cpp
ConstraintArray & d2::lsx::ConstraintArray::operator=(const ConstraintArray &)=default
```

Source: `core/lsx/d2_solver.hpp:224`

### `operator=`

```cpp
ConstraintArray & d2::lsx::ConstraintArray::operator=(ConstraintArray &&)=default
```

Source: `core/lsx/d2_solver.hpp:225`


## Typedefs

### `handle`

```cpp
using d2::lsx::ConstraintArray::handle = impl::ConstraintHandle
```

Source: `core/lsx/d2_solver.hpp:125`
