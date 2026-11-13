# `d2::Signals::HandleState`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:585`

## Enums

### `State`

```cpp
enum State
```

Source: `core/io/d2_signal_handler.hpp:587`

Values: `Active`, `Inactive`, `Muted`


## Functions

### `HandleState`

```cpp
d2::Signals::HandleState::HandleState(std::function< void(SignalInstance &)> callback)
```

Source: `core/io/d2_signal_handler.hpp:598`


## Variables

### `state`

```cpp
std::atomic<State> d2::Signals::HandleState::state
```

Source: `core/io/d2_signal_handler.hpp:593`

### `callback`

```cpp
std::function<void(SignalInstance&)> d2::Signals::HandleState::callback
```

Source: `core/io/d2_signal_handler.hpp:594`

### `event`

```cpp
event_idx d2::Signals::HandleState::event
```

Source: `core/io/d2_signal_handler.hpp:595`

### `dispatch`

```cpp
SpecifierKey d2::Signals::HandleState::dispatch
```

Source: `core/io/d2_signal_handler.hpp:596`
