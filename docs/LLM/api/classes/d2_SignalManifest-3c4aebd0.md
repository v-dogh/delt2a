# `d2::SignalManifest`

**Kind:** class
**Declared in:** `core/io/d2_signal_handler.hpp:211`

## Functions

### `verify`

```cpp
static bool d2::SignalManifest< Spec, Specs >::verify(event_idx ev, impl::RuntimeSignature signature) noexcept
```

Source: `core/io/d2_signal_handler.hpp:298`


## Typedefs

### `event_t`

```cpp
using d2::SignalManifest< Spec, Specs >::event_t = typename Spec::event_t
```

Source: `core/io/d2_signal_handler.hpp:283`

### `manifest_t`

```cpp
using d2::SignalManifest< Spec, Specs >::manifest_t = SignalManifest<Spec, Specs...>
```

Source: `core/io/d2_signal_handler.hpp:284`

### `case_for_t`

```cpp
template<auto Ev>
using d2::SignalManifest< Spec, Specs >::case_for_t = typename FindCase<Ev>::case_t
```

Source: `core/io/d2_signal_handler.hpp:286`
