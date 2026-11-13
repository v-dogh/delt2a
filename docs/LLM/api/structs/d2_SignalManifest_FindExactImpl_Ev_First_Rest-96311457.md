# `d2::SignalManifest::FindExactImpl< Ev, First, Rest... >`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:233`

## Typedefs

### `type`

```cpp
using d2::SignalManifest< Spec, Specs >::FindExactImpl< Ev, First, Rest... >::type = std::conditional_t<CaseMatches<Ev, First>::value, First, rest_t>
```

Source: `core/io/d2_signal_handler.hpp:238`
