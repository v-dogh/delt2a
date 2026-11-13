# `d2::SignalRegistry< sys::screen::Event >`

**Kind:** struct
**Declared in:** `core/screen/d2_screen.hpp:390`

**Bases:** `d2::SignalManifest< EvDefault< Event >, EvCase< Event::Resize, BoundingBox >, EvCase< Event::KeyInput, in::InputFrame & >, EvCase< Event::KeySequenceInput, d2::string_view >, EvCase< Event::KeyMouseInput, in::InputFrame & >, EvCase< Event::MouseMoved, Position >, EvCase< Event::MouseInput, in::InputFrame & >, EvCase< Event::TreeSwap, std::string, std::string > >`

## Typedefs

### `manifest_t`

```cpp
using d2::SignalRegistry< typename >::manifest_t =
```

Source: `core/io/d2_signal_handler.hpp:208`
