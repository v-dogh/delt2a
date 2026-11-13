# `d2::sys::SystemNotify`

**Kind:** class
**Declared in:** `core/mods/d2_ext.hpp:24`

**Bases:** `d2::sys::ModuleDecl< SystemNotify, "Notifications", Access::TSafe >`

## Enums

### `Mode`

```cpp
enum Mode
```

Source: `core/mods/d2_ext.hpp:27`

Values: `Local`, `System`, `Passive`, `Prompt`, `SoftPrompt`

### `Level`

```cpp
enum Level
```

Source: `core/mods/d2_ext.hpp:42`

Values: `Info`, `Warning`, `Error`


## Functions

### `supports`

```cpp
virtual bool d2::sys::SystemNotify::supports(unsigned char mode)=0
```

Source: `core/mods/d2_ext.hpp:51`

### `notify`

```cpp
virtual void d2::sys::SystemNotify::notify(Level level, std::string_view title, std::string_view content, unsigned char targets=Local|Passive)=0
```

Source: `core/mods/d2_ext.hpp:53`

### `notify`

```cpp
virtual void d2::sys::SystemNotify::notify(Level level, std::string_view title, std::string_view content, Element::ptr view, unsigned char targets=Local|Passive)=0
```

Source: `core/mods/d2_ext.hpp:59`

### `clear`

```cpp
virtual void d2::sys::SystemNotify::clear()=0
```

Source: `core/mods/d2_ext.hpp:67`
