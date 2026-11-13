# `d2::Signals::SignalStorage`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:750`

## Typedefs

### `HandleList`

```cpp
using d2::Signals::SignalStorage::HandleList = std::vector<std::shared_ptr<HandleState>>
```

Source: `core/io/d2_signal_handler.hpp:752`


## Variables

### `mtx`

```cpp
std::shared_mutex d2::Signals::SignalStorage::mtx
```

Source: `core/io/d2_signal_handler.hpp:761`

### `instances`

```cpp
mt::TaskQueue<SignalEnvelope> d2::Signals::SignalStorage::instances
```

Source: `core/io/d2_signal_handler.hpp:762`

### `combined_mtx`

```cpp
std::mutex d2::Signals::SignalStorage::combined_mtx
```

Source: `core/io/d2_signal_handler.hpp:763`

### `combined`

```cpp
std::optional<SignalEnvelope> d2::Signals::SignalStorage::combined
```

Source: `core/io/d2_signal_handler.hpp:764`

### `handles`

```cpp
absl::flat_hash_map<event_idx, EventBucket> d2::Signals::SignalStorage::handles
```

Source: `core/io/d2_signal_handler.hpp:765`

### `verify`

```cpp
signature_verifier d2::Signals::SignalStorage::verify
```

Source: `core/io/d2_signal_handler.hpp:766`

### `is_queued`

```cpp
std::atomic<bool> d2::Signals::SignalStorage::is_queued
```

Source: `core/io/d2_signal_handler.hpp:767`

### `flags`

```cpp
unsigned char d2::Signals::SignalStorage::flags
```

Source: `core/io/d2_signal_handler.hpp:768`
