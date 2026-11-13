# `d2::SignalRegistry< void >`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:319`

## Functions

### `ensure_satisfied`

```cpp
template<auto Ev, typename... Argv>
static consteval void d2::SignalRegistry< void >::ensure_satisfied()
```

Source: `core/io/d2_signal_handler.hpp:362`

### `ensure_tuple_satisfied`

```cpp
template<auto Ev, typename Tuple>
static consteval void d2::SignalRegistry< void >::ensure_tuple_satisfied()
```

Source: `core/io/d2_signal_handler.hpp:366`


## Typedefs

### `signature_for_t`

```cpp
template<auto Ev>
using d2::SignalRegistry< void >::signature_for_t = typename SignatureLookupImpl<Ev, typename SignalRegistry<decltype(Ev)>::manifest_t>:: signature_t
```

Source: `core/io/d2_signal_handler.hpp:352`

### `manifest_t`

```cpp
using d2::SignalRegistry< typename >::manifest_t =
```

Source: `core/io/d2_signal_handler.hpp:208`


## Variables

### `satisfied`

```cpp
template<auto Ev, typename... Argv>
bool d2::SignalRegistry< void >::satisfied
```

Source: `core/io/d2_signal_handler.hpp:357`

### `satisfied_tuple`

```cpp
template<auto Ev, typename Tuple>
bool d2::SignalRegistry< void >::satisfied_tuple
```

Source: `core/io/d2_signal_handler.hpp:360`
