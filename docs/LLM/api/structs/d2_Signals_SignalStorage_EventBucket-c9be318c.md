# `d2::Signals::SignalStorage::EventBucket`

**Kind:** struct
**Declared in:** `core/io/d2_signal_handler.hpp:754`

## Variables

### `generic`

```cpp
HandleList d2::Signals::SignalStorage::EventBucket::generic
```

Source: `core/io/d2_signal_handler.hpp:756`

### `keyed`

```cpp
absl::flat_hash_map<SpecifierKey, HandleList, SpecifierKeyHash, SpecifierKeyEqual> d2::Signals::SignalStorage::EventBucket::keyed
```

Source: `core/io/d2_signal_handler.hpp:758`
