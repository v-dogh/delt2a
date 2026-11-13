# `d2::dx::detail::InputLayout`

**Kind:** struct
**Declared in:** `elements/d2_input_layout.hpp:7`

## Functions

### `reset`

```cpp
void d2::dx::detail::InputLayout::reset(std::string_view value, bool hide, bool newlines)
```

Source: `elements/d2_input_layout.hpp:13`

### `display`

```cpp
const text::Layout & d2::dx::detail::InputLayout::display() const
```

Source: `elements/d2_input_layout.hpp:27`

### `hit`

```cpp
std::size_t d2::dx::detail::InputLayout::hit(Position position, std::optional< std::size_t > gap={}) const
```

Source: `elements/d2_input_layout.hpp:28`


## Variables

### `source`

```cpp
text::Layout d2::dx::detail::InputLayout::source
```

Source: `elements/d2_input_layout.hpp:9`

### `masked`

```cpp
text::Layout d2::dx::detail::InputLayout::masked
```

Source: `elements/d2_input_layout.hpp:10`

### `censor`

```cpp
bool d2::dx::detail::InputLayout::censor
```

Source: `elements/d2_input_layout.hpp:11`
