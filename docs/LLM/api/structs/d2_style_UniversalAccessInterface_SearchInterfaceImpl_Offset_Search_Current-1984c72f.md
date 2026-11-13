# `d2::style::UniversalAccessInterface::SearchInterfaceImpl< Offset, Search, Current >`

**Kind:** struct
**Declared in:** `core/tree/d2_styles.hpp:152`

## Typedefs

### `type`

```cpp
using d2::style::UniversalAccessInterface< Chain, Base, Interface, Interfaces >::SearchInterfaceImpl< Offset, Search, Current >::type = std:: conditional<std::is_same_v<Search<0>, Current<0>>, Current<Offset>, void>::type
```

Source: `core/tree/d2_styles.hpp:154`
