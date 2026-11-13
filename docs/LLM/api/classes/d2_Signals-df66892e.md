# `d2::Signals`

**Kind:** class
**Declared in:** `core/io/d2_signal_handler.hpp:415`

**Bases:** `std::enable_shared_from_this< Signals >`

## Enums

### `SignalFlags`

```cpp
enum SignalFlags
```

Source: `core/io/d2_signal_handler.hpp:777`

Values: `Async`, `Deferred`, `Combine`, `DropIfFull`


## Functions

### `make`

```cpp
Signals::ptr d2::Signals::make(mt::ConcurrentPool::ptr pool=nullptr)
```

Source: `core/io/d2_signal_handler.hpp:930`

### `Signals`

```cpp
d2::Signals::Signals(mt::ConcurrentPool::ptr pool)
```

Source: `core/io/d2_signal_handler.hpp:932`

### `Signals`

```cpp
d2::Signals::Signals(const Signals &)=delete
```

Source: `core/io/d2_signal_handler.hpp:933`

### `Signals`

```cpp
d2::Signals::Signals(Signals &&)=delete
```

Source: `core/io/d2_signal_handler.hpp:934`

### `connect`

```cpp
template<typename Manifest>
Signal d2::Signals::connect(Config cfg=Config{})
```

Source: `core/io/d2_signal_handler.hpp:936`

### `connect`

```cpp
template<typename Manifest, typename SigSpec>
Signal d2::Signals::connect(SignalSpecifier< SigSpec > specifier, Config cfg=Config{})
```

Source: `core/io/d2_signal_handler.hpp:942`

### `disconnect`

```cpp
template<typename Manifest>
void d2::Signals::disconnect()
```

Source: `core/io/d2_signal_handler.hpp:952`

### `disconnect`

```cpp
template<typename Manifest, typename SigSpec>
void d2::Signals::disconnect(SignalSpecifier< SigSpec > specifier)
```

Source: `core/io/d2_signal_handler.hpp:958`

### `listen`

```cpp
template<auto Ev, typename Func>
Handle d2::Signals::listen(Func &&callback)
```

Source: `core/io/d2_signal_handler.hpp:964`

### `listen`

```cpp
template<auto Ev, typename SigSpec, typename Func>
Handle d2::Signals::listen(SignalSpecifier< SigSpec > signal_specifier, Func &&callback)
```

Source: `core/io/d2_signal_handler.hpp:984`

### `listen`

```cpp
template<auto Ev, typename DispatchSpec, typename Func>
Handle d2::Signals::listen(DispatchSpecifier< DispatchSpec > dispatch_specifier, Func &&callback)
```

Source: `core/io/d2_signal_handler.hpp:1004`

### `listen`

```cpp
template<auto Ev, typename SigSpec, typename DispatchSpec, typename Func>
Handle d2::Signals::listen(SignalSpecifier< SigSpec > signal_specifier, DispatchSpecifier< DispatchSpec > dispatch_specifier, Func &&callback)
```

Source: `core/io/d2_signal_handler.hpp:1024`

### `listen`

```cpp
template<typename Event, typename Func>
Handle d2::Signals::listen(Event ev, Func &&callback)
```

Source: `core/io/d2_signal_handler.hpp:1047`

### `listen`

```cpp
template<typename Event, typename SigSpec, typename Func>
Handle d2::Signals::listen(Event ev, SignalSpecifier< SigSpec > signal_specifier, Func &&callback)
```

Source: `core/io/d2_signal_handler.hpp:1066`

### `listen`

```cpp
template<typename Event, typename DispatchSpec, typename Func>
Handle d2::Signals::listen(Event ev, DispatchSpecifier< DispatchSpec > dispatch_specifier, Func &&callback)
```

Source: `core/io/d2_signal_handler.hpp:1085`

### `listen`

```cpp
template<typename Event, typename SigSpec, typename DispatchSpec, typename Func>
Handle d2::Signals::listen(Event ev, SignalSpecifier< SigSpec > signal_specifier, DispatchSpecifier< DispatchSpec > dispatch_specifier, Func &&callback)
```

Source: `core/io/d2_signal_handler.hpp:1104`

### `signal`

```cpp
template<auto Ev, typename... Argv>
void d2::Signals::signal(Argv &&... args)
```

Source: `core/io/d2_signal_handler.hpp:1128`

### `signal`

```cpp
template<auto Ev, typename SigSpec, typename... Argv>
void d2::Signals::signal(SignalSpecifier< SigSpec > signal_specifier, Argv &&... args)
```

Source: `core/io/d2_signal_handler.hpp:1145`

### `signal`

```cpp
template<auto Ev, typename DispatchSpec, typename... Argv>
void d2::Signals::signal(DispatchSpecifier< DispatchSpec > dispatch_specifier, Argv &&... args)
```

Source: `core/io/d2_signal_handler.hpp:1162`

### `signal`

```cpp
template<auto Ev, typename SigSpec, typename DispatchSpec, typename... Argv>
void d2::Signals::signal(SignalSpecifier< SigSpec > signal_specifier, DispatchSpecifier< DispatchSpec > dispatch_specifier, Argv &&... args)
```

Source: `core/io/d2_signal_handler.hpp:1179`

### `signal`

```cpp
template<typename Event, typename... Argv>
void d2::Signals::signal(Event ev, Argv &&... args)
```

Source: `core/io/d2_signal_handler.hpp:1199`

### `signal`

```cpp
template<typename Event, typename SigSpec, typename... Argv>
void d2::Signals::signal(Event ev, SignalSpecifier< SigSpec > signal_specifier, Argv &&... args)
```

Source: `core/io/d2_signal_handler.hpp:1214`

### `signal`

```cpp
template<typename Event, typename DispatchSpec, typename... Argv>
void d2::Signals::signal(Event ev, DispatchSpecifier< DispatchSpec > dispatch_specifier, Argv &&... args)
```

Source: `core/io/d2_signal_handler.hpp:1229`

### `signal`

```cpp
template<typename Event, typename SigSpec, typename DispatchSpec, typename... Argv>
void d2::Signals::signal(Event ev, SignalSpecifier< SigSpec > signal_specifier, DispatchSpecifier< DispatchSpec > dispatch_specifier, Argv &&... args)
```

Source: `core/io/d2_signal_handler.hpp:1244`

### `operator=`

```cpp
Signals & d2::Signals::operator=(const Signals &)=delete
```

Source: `core/io/d2_signal_handler.hpp:1264`

### `operator=`

```cpp
Signals & d2::Signals::operator=(Signals &&)=delete
```

Source: `core/io/d2_signal_handler.hpp:1265`


## Typedefs

### `ptr`

```cpp
using d2::Signals::ptr = std::shared_ptr<Signals>
```

Source: `core/io/d2_signal_handler.hpp:842`
