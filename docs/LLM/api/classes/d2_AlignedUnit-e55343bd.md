# `d2::AlignedUnit`

**Kind:** class
**Declared in:** `core/types/d2_element_units.hpp:229`

**Bases:** `d2::Unit`

## Functions

### `AlignedUnit`

```cpp
d2::AlignedUnit< Ctx >::AlignedUnit()=default
```

Source: `core/types/d2_element_units.hpp:245`

### `AlignedUnit`

```cpp
d2::AlignedUnit< Ctx >::AlignedUnit(const Unit &value)
```

Source: `core/types/d2_element_units.hpp:246`

### `AlignedUnit`

```cpp
d2::AlignedUnit< Ctx >::AlignedUnit(float value, Type units=Px, unsigned char mods=0x00)
```

Source: `core/types/d2_element_units.hpp:250`

### `AlignedUnit`

```cpp
d2::AlignedUnit< Ctx >::AlignedUnit(Type units, unsigned char mods=0x00)
```

Source: `core/types/d2_element_units.hpp:255`

### `AlignedUnit`

```cpp
d2::AlignedUnit< Ctx >::AlignedUnit(Mods mods)
```

Source: `core/types/d2_element_units.hpp:259`

### `AlignedUnit`

```cpp
d2::AlignedUnit< Ctx >::AlignedUnit(const AlignedUnit &)=default
```

Source: `core/types/d2_element_units.hpp:263`

### `AlignedUnit`

```cpp
d2::AlignedUnit< Ctx >::AlignedUnit(AlignedUnit &&)=default
```

Source: `core/types/d2_element_units.hpp:264`

### `getcontext`

```cpp
bool d2::AlignedUnit< Ctx >::getcontext(UnitContext value) const
```

Source: `core/types/d2_element_units.hpp:266`

### `getcontext`

```cpp
bool d2::AlignedUnit< Ctx >::getcontext(UnitContext value)
```

Source: `core/types/d2_element_units.hpp:270`

### `operator=`

```cpp
AlignedUnit & d2::AlignedUnit< Ctx >::operator=(const AlignedUnit &)=default
```

Source: `core/types/d2_element_units.hpp:275`

### `operator=`

```cpp
AlignedUnit & d2::AlignedUnit< Ctx >::operator=(AlignedUnit &&)=default
```

Source: `core/types/d2_element_units.hpp:276`

### `operator=`

```cpp
Unit & d2::AlignedUnit< Ctx >::operator=(Mods mods)
```

Source: `core/types/d2_element_units.hpp:277`

### `operator=`

```cpp
Unit & d2::AlignedUnit< Ctx >::operator=(Type units)
```

Source: `core/types/d2_element_units.hpp:281`
