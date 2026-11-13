# `d2::EvCase`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:189`

## Typedefs

### `raw_signature_t`

```cpp
using d2::EvCase< Ev, Argv >::raw_signature_t = std::tuple<Argv...>
```

Source: `core/io/d2_signal_handler.hpp:193`

### `signature_t`

```cpp
using d2::EvCase< Ev, Argv >::signature_t = typename impl::args_normalize<raw_signature_t>::type
```

Source: `core/io/d2_signal_handler.hpp:194`

### `event_t`

```cpp
using d2::EvCase< Ev, Argv >::event_t = decltype(Ev)
```

Source: `core/io/d2_signal_handler.hpp:195`


## Variables

### `event`

```cpp
auto d2::EvCase< Ev, Argv >::event
```

Source: `core/io/d2_signal_handler.hpp:191`

### `def`

```cpp
bool d2::EvCase< Ev, Argv >::def
```

Source: `core/io/d2_signal_handler.hpp:192`
