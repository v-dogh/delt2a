# `d2::Signals::SignalEnvelope`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:733`

## Functions

### `SignalEnvelope`

```cpp
d2::Signals::SignalEnvelope::SignalEnvelope()=default
```

Source: `core/io/d2_signal_handler.hpp:739`

### `SignalEnvelope`

```cpp
d2::Signals::SignalEnvelope::SignalEnvelope(event_idx event, SpecifierKey dispatch, SignalInstance instance)
```

Source: `core/io/d2_signal_handler.hpp:740`

### `SignalEnvelope`

```cpp
d2::Signals::SignalEnvelope::SignalEnvelope(SignalEnvelope &&) noexcept=default
```

Source: `core/io/d2_signal_handler.hpp:744`

### `SignalEnvelope`

```cpp
d2::Signals::SignalEnvelope::SignalEnvelope(const SignalEnvelope &)=delete
```

Source: `core/io/d2_signal_handler.hpp:745`

### `operator=`

```cpp
SignalEnvelope & d2::Signals::SignalEnvelope::operator=(SignalEnvelope &&) noexcept=default
```

Source: `core/io/d2_signal_handler.hpp:747`

### `operator=`

```cpp
SignalEnvelope & d2::Signals::SignalEnvelope::operator=(const SignalEnvelope &)=delete
```

Source: `core/io/d2_signal_handler.hpp:748`


## Variables

### `event`

```cpp
event_idx d2::Signals::SignalEnvelope::event
```

Source: `core/io/d2_signal_handler.hpp:735`

### `dispatch`

```cpp
SpecifierKey d2::Signals::SignalEnvelope::dispatch
```

Source: `core/io/d2_signal_handler.hpp:736`

### `instance`

```cpp
SignalInstance d2::Signals::SignalEnvelope::instance
```

Source: `core/io/d2_signal_handler.hpp:737`
