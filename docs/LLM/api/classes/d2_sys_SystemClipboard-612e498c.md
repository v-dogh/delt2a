# `d2::sys::SystemClipboard`

**Kind:** class
**Declared in:** `core/mods/d2_ext.hpp:11`

**Bases:** `d2::sys::ModuleDecl< SystemClipboard, "Clipboard", Access::TUnsafe >`

## Functions

### `clear`

```cpp
virtual void d2::sys::SystemClipboard::clear()=0
```

Source: `core/mods/d2_ext.hpp:16`

### `copy`

```cpp
virtual void d2::sys::SystemClipboard::copy(const d2::string &value)=0
```

Source: `core/mods/d2_ext.hpp:17`

### `paste`

```cpp
virtual d2::string d2::sys::SystemClipboard::paste()=0
```

Source: `core/mods/d2_ext.hpp:18`

### `empty`

```cpp
virtual bool d2::sys::SystemClipboard::empty()=0
```

Source: `core/mods/d2_ext.hpp:19`
