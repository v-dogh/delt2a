# `d2::style::impl::LastImpl`

**Kind:** struct
**Declared in:** `core/tree/d2_styles.hpp:33`

## Typedefs

### `type`

```cpp
template<std::size_t Prop>
using d2::style::impl::LastImpl< First, Rest >::type = LastImpl<Rest...>::template type<Prop>
```

Source: `core/tree/d2_styles.hpp:35`
