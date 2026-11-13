# `d2::SignalRegistry< void >::SatisfiedTupleImpl< Ev, std::tuple< Argv... > >`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:340`

**Bases:** `std::bool_constant< std::is_same_v< std::tuple< Argv... >, SignatureLookupImpl< Ev, SignalRegistry< decltype(Ev)>::manifest_t >::signature_t > >`

## Typedefs

### `manifest_t`

```cpp
using d2::SignalRegistry< typename >::manifest_t =
```

Source: `core/io/d2_signal_handler.hpp:208`
