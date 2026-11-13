# `d2::sys::SystemSelector`

**Kind:** class
**Declared in:** `core/mods/d2_ext.hpp:72`

**Bases:** `d2::sys::Module< SystemSelector, "Selector", Access::TUnsafe, Load::Immediate, SystemScreen >`

## Enums

### `Event`

```cpp
enum Event
```

Source: `core/mods/d2_ext.hpp:81`

Values: `Selected`


## Functions

### `selection`

```cpp
TextSelection & d2::sys::SystemSelector::selection() noexcept
```

Source: `core/mods/d2_ext.hpp:103`

### `value`

```cpp
std::string d2::sys::SystemSelector::value()
```

Source: `core/mods/d2_ext.hpp:104`

### `clear`

```cpp
void d2::sys::SystemSelector::clear()
```

Source: `core/mods/d2_ext.hpp:106`

### `copy`

```cpp
bool d2::sys::SystemSelector::copy()
```

Source: `core/mods/d2_ext.hpp:107`


## Typedefs

### `EventManifest`

```cpp
using d2::sys::SystemSelector::EventManifest = SignalManifest< EvDefault<Event, d2::string, Position, Position> >
```

Source: `core/mods/d2_ext.hpp:85`
