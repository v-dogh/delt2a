# `d2::style::IVSelectable`

**Kind:** class
**Declared in:** `core/tree/d2_text_select.hpp:10`

## Functions

### `~IVSelectable`

```cpp
virtual d2::style::IVSelectable::~IVSelectable()=default
```

Source: `core/tree/d2_text_select.hpp:16`

### `selectable`

```cpp
bool d2::style::IVSelectable::selectable() const noexcept
```

Source: `core/tree/d2_text_select.hpp:17`

### `text_layout`

```cpp
virtual const text::Layout * d2::style::IVSelectable::text_layout() const
```

Source: `core/tree/d2_text_select.hpp:18`

### `select_text`

```cpp
virtual void d2::style::IVSelectable::select_text(std::optional< text::Range > range)
```

Source: `core/tree/d2_text_select.hpp:19`

### `selection_separator`

```cpp
virtual std::string_view d2::style::IVSelectable::selection_separator() const
```

Source: `core/tree/d2_text_select.hpp:20`

### `text_selection`

```cpp
std::optional< text::Range > d2::style::IVSelectable::text_selection() const noexcept
```

Source: `core/tree/d2_text_select.hpp:21`


## Variables

### `enable_selection`

```cpp
bool d2::style::IVSelectable::enable_selection
```

Source: `core/tree/d2_text_select.hpp:13`

### `_text_selection`

```cpp
std::optional<text::Range> d2::style::IVSelectable::_text_selection
```

Source: `core/tree/d2_text_select.hpp:14`
