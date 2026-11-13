# `d2::EvDefault`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:198`

## Typedefs

### `raw_signature_t`

```cpp
using d2::EvDefault< Ev, Argv >::raw_signature_t = std::tuple<Argv...>
```

Source: `core/io/d2_signal_handler.hpp:201`

### `signature_t`

```cpp
using d2::EvDefault< Ev, Argv >::signature_t = typename impl::args_normalize<raw_signature_t>::type
```

Source: `core/io/d2_signal_handler.hpp:202`

### `event_t`

```cpp
using d2::EvDefault< Ev, Argv >::event_t = Ev
```

Source: `core/io/d2_signal_handler.hpp:203`


## Variables

### `def`

```cpp
bool d2::EvDefault< Ev, Argv >::def
```

Source: `core/io/d2_signal_handler.hpp:200`
