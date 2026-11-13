# `d2::lsx::Expression`

**Kind:** class
**Declared in:** `core/lsx/d2_solver_types.hpp:194`

## Functions

### `reduce`

```cpp
static constexpr Expression d2::lsx::Expression::reduce(const Expression &lhs, const Expression &rhs, Constraint op)
```

Source: `core/lsx/d2_solver_types.hpp:243`

### `from`

```cpp
static constexpr Expression d2::lsx::Expression::from(Variable lhs, Variable rhs, int sign)
```

Source: `core/lsx/d2_solver_types.hpp:304`

### `from`

```cpp
static constexpr Expression d2::lsx::Expression::from(const Expression &lhs, Variable rhs, int sign)
```

Source: `core/lsx/d2_solver_types.hpp:312`

### `from`

```cpp
static constexpr Expression d2::lsx::Expression::from(Variable lhs, const Expression &rhs, int sign)
```

Source: `core/lsx/d2_solver_types.hpp:321`

### `from`

```cpp
static constexpr Expression d2::lsx::Expression::from(const Expression &lhs, result_type cf)
```

Source: `core/lsx/d2_solver_types.hpp:325`

### `from`

```cpp
static constexpr Expression d2::lsx::Expression::from(result_type cf, const Expression &rhs, int sign)
```

Source: `core/lsx/d2_solver_types.hpp:337`

### `from`

```cpp
static constexpr Expression d2::lsx::Expression::from(Variable var)
```

Source: `core/lsx/d2_solver_types.hpp:341`

### `type`

```cpp
Constraint d2::lsx::Expression::type() const
```

Source: `core/lsx/d2_solver_types.hpp:349`

### `vars`

```cpp
std::span< const Variable > d2::lsx::Expression::vars() const
```

Source: `core/lsx/d2_solver_types.hpp:354`

### `locals`

```cpp
std::span< const Variable > d2::lsx::Expression::locals() const
```

Source: `core/lsx/d2_solver_types.hpp:358`

### `globals`

```cpp
std::span< const Variable > d2::lsx::Expression::globals() const
```

Source: `core/lsx/d2_solver_types.hpp:362`

### `constant`

```cpp
result_type d2::lsx::Expression::constant() const
```

Source: `core/lsx/d2_solver_types.hpp:367`

### `print`

```cpp
std::string d2::lsx::Expression::print() const
```

Source: `core/lsx/d2_solver_types.hpp:372`
