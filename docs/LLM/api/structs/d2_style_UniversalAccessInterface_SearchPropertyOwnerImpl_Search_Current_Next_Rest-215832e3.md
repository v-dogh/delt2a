# `d2::style::UniversalAccessInterface::SearchPropertyOwnerImpl< Search, Current, Next, Rest... >`

**Kind:** struct
**Declared in:** `core/tree/d2_styles.hpp:172`

## Typedefs

### `type`

```cpp
using d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::SearchPropertyOwnerImpl< Search, Current, Next, Rest... >::type = std::conditional < Search >= Current::base&& Search< Current::offset, Current, typename SearchPropertyOwnerImpl<Search, Next<Current::offset>, Rest...>::type >::type
```

Source: `core/tree/d2_styles.hpp:174`
