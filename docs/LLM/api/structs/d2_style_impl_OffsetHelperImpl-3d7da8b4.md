# `d2::style::impl::OffsetHelperImpl`

**Kind:** struct
**Declared in:** `core/tree/d2_styles.hpp:49`

## Typedefs

### `type`

```cpp
using d2::style::impl::OffsetHelperImpl< Search, Current, CurrentType, Next, Rest >::type = std::conditional< std::is_same_v<Next<0>, Search<0>>, Current, typename OffsetHelperImpl<Search, Next<Current::offset>, Next, Rest...>::type >::type
```

Source: `core/tree/d2_styles.hpp:51`
