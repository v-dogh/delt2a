# `d2::SignalManifest::FindCase`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:253`

## Typedefs

### `case_t`

```cpp
using d2::SignalManifest< Spec, Specs >::FindCase< Ev >::case_t = std::conditional_t<std::is_void_v<exact_t>, default_t, exact_t>
```

Source: `core/io/d2_signal_handler.hpp:259`
