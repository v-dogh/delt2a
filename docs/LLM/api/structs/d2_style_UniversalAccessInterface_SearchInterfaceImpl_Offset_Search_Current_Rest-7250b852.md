# `d2::style::UniversalAccessInterface::SearchInterfaceImpl< Offset, Search, Current, Rest... >`

**Kind:** struct
**Declared in:** `core/tree/d2_styles.hpp:143`

## Typedefs

### `type`

```cpp
using d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::SearchInterfaceImpl< Offset, Search, Current, Rest... >::type = std::conditional< std::is_same_v<Search<0>, Current<0>>, Current<Offset>, typename SearchInterfaceImpl<Current<Offset>::offset, Search, Rest...>::type >::type
```

Source: `core/tree/d2_styles.hpp:145`
