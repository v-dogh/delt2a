# `d2::lsx::Variable`

**Kind:** class
**Declared in:** `core/lsx/d2_solver_types.hpp:121`

**Bases:** `d2::lsx::VariableIndex`

## Functions

### `Variable`

```cpp
d2::lsx::Variable::Variable()
```

Source: `core/lsx/d2_solver_types.hpp:126`

### `Variable`

```cpp
d2::lsx::Variable::Variable(VariableIndex idx, result_type coefficient=1.f)
```

Source: `core/lsx/d2_solver_types.hpp:128`

### `Variable`

```cpp
d2::lsx::Variable::Variable(result_type coefficient)
```

Source: `core/lsx/d2_solver_types.hpp:130`

### `coefficient`

```cpp
result_type d2::lsx::Variable::coefficient() const
```

Source: `core/lsx/d2_solver_types.hpp:133`

### `operator[]`

```cpp
Variable d2::lsx::Variable::operator[](std::size_t idx) const
```

Source: `core/lsx/d2_solver_types.hpp:138`
