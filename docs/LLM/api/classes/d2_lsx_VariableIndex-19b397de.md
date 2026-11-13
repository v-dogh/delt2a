# `d2::lsx::VariableIndex`

**Kind:** class
**Declared in:** `core/lsx/d2_solver_types.hpp:21`

## Enums

### `Type`

```cpp
enum Type
```

Source: `core/lsx/d2_solver_types.hpp:24`

Values: `Constant`, `Local`, `Global`, `Objective`, `Variable`, `Slack`, `Error`, `Dummy`, `Artificial`


## Functions

### `VariableIndex`

```cpp
d2::lsx::VariableIndex::VariableIndex()
```

Source: `core/lsx/d2_solver_types.hpp:64`

### `VariableIndex`

```cpp
d2::lsx::VariableIndex::VariableIndex(const VariableIndex &)=default
```

Source: `core/lsx/d2_solver_types.hpp:66`

### `VariableIndex`

```cpp
d2::lsx::VariableIndex::VariableIndex(VariableIndex &&)=default
```

Source: `core/lsx/d2_solver_types.hpp:67`

### `VariableIndex`

```cpp
d2::lsx::VariableIndex::VariableIndex(Type type, bool derived=false)
```

Source: `core/lsx/d2_solver_types.hpp:68`

### `VariableIndex`

```cpp
d2::lsx::VariableIndex::VariableIndex(std::uint32_t idx, Type type, bool derived=false)
```

Source: `core/lsx/d2_solver_types.hpp:70`

### `is_derived`

```cpp
bool d2::lsx::VariableIndex::is_derived() const
```

Source: `core/lsx/d2_solver_types.hpp:73`

### `is_internal`

```cpp
bool d2::lsx::VariableIndex::is_internal() const
```

Source: `core/lsx/d2_solver_types.hpp:77`

### `is_pivotable`

```cpp
bool d2::lsx::VariableIndex::is_pivotable() const
```

Source: `core/lsx/d2_solver_types.hpp:86`

### `is_external`

```cpp
bool d2::lsx::VariableIndex::is_external() const
```

Source: `core/lsx/d2_solver_types.hpp:93`

### `type`

```cpp
Type d2::lsx::VariableIndex::type() const
```

Source: `core/lsx/d2_solver_types.hpp:98`

### `index`

```cpp
std::uint32_t d2::lsx::VariableIndex::index() const
```

Source: `core/lsx/d2_solver_types.hpp:102`

### `code`

```cpp
std::uint32_t d2::lsx::VariableIndex::code() const
```

Source: `core/lsx/d2_solver_types.hpp:106`

### `print`

```cpp
std::string d2::lsx::VariableIndex::print() const
```

Source: `core/lsx/d2_solver_types.hpp:111`

### `operator=`

```cpp
VariableIndex & d2::lsx::VariableIndex::operator=(const VariableIndex &)=default
```

Source: `core/lsx/d2_solver_types.hpp:113`

### `operator=`

```cpp
VariableIndex & d2::lsx::VariableIndex::operator=(VariableIndex &&)=default
```

Source: `core/lsx/d2_solver_types.hpp:114`

### `operator==`

```cpp
bool d2::lsx::VariableIndex::operator==(const VariableIndex &copy) const
```

Source: `core/lsx/d2_solver_types.hpp:116`
