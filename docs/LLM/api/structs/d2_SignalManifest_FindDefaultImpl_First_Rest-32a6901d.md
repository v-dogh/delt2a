# `d2::SignalManifest::FindDefaultImpl< First, Rest... >`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:245`

## Typedefs

### `type`

```cpp
using d2::SignalManifest< Spec, Specs >::FindDefaultImpl< First, Rest... >::type = std::conditional_t<First::def, First, rest_t>
```

Source: `core/io/d2_signal_handler.hpp:250`
