# `d2::lsx::impl::ConstraintHandle`

**Kind:** struct
**Declared in:** `core/lsx/d2_solver.hpp:96`

## Functions

### `ConstraintHandle`

```cpp
d2::lsx::impl::ConstraintHandle::ConstraintHandle()=default
```

Source: `core/lsx/d2_solver.hpp:102`

### `ConstraintHandle`

```cpp
d2::lsx::impl::ConstraintHandle::ConstraintHandle(std::nullptr_t)
```

Source: `core/lsx/d2_solver.hpp:103`

### `ConstraintHandle`

```cpp
d2::lsx::impl::ConstraintHandle::ConstraintHandle(ConstraintHandle &&)=default
```

Source: `core/lsx/d2_solver.hpp:104`

### `ConstraintHandle`

```cpp
d2::lsx::impl::ConstraintHandle::ConstraintHandle(const ConstraintHandle &)=default
```

Source: `core/lsx/d2_solver.hpp:105`

### `ConstraintHandle`

```cpp
d2::lsx::impl::ConstraintHandle::ConstraintHandle(VariableIndex low, VariableIndex high=VariableIndex::Type::Constant)
```

Source: `core/lsx/d2_solver.hpp:106`

### `operator==`

```cpp
bool d2::lsx::impl::ConstraintHandle::operator==(std::nullptr_t)
```

Source: `core/lsx/d2_solver.hpp:112`

### `operator=`

```cpp
ConstraintHandle & d2::lsx::impl::ConstraintHandle::operator=(const ConstraintHandle &)=default
```

Source: `core/lsx/d2_solver.hpp:117`

### `operator=`

```cpp
ConstraintHandle & d2::lsx::impl::ConstraintHandle::operator=(ConstraintHandle &&)=default
```

Source: `core/lsx/d2_solver.hpp:118`
