# Other runtime services

[Documentation index](../index.md) · [Module lifecycle](runtime.md)

## Clipboard

`sys::SystemClipboard` (`sys::clipboard`) exposes `copy`, `paste`, `clear` and `empty`. Load the module when selection/editor clipboard commands are wanted. The supplied `LocalSystemClipboard` stores text **inside this process**; it does not integrate with Wayland, X11 or the terminal's OSC 52 clipboard. Implement another backend for those transports. Clipboard calls belong on the UI thread.

## Notifications

`sys::SystemNotify` supports level (`Info`, `Warning`, `Error`) and delivery mode flags. `LocalSystemNotify` renders passive messages using framework elements and requires screen/elements/extras. It can accept text or an element view. Its prompt handler and `clear()` are unfinished; `supports()` currently checks system-vs-local mode and is not proof that prompt behavior exists. Use passive local messages only with this backend. Native desktop notifications require another implementation.

## WebSockets

`sys::net::SystemWS` (`sys::net::ws`) takes a `Connection` with URL, setup/open/message/error/close callbacks, timeout, ping interval and reconnect configuration. `connect()` returns a shared `Client`; it exposes `send`, `close` and `state`.

```cpp
d2::sys::net::SystemWS::Connection connection;
connection.url = "ws://localhost:8080/events";
connection.on_message = [](std::string_view bytes, auto type, auto client)
{
    // Copy bytes before retaining them; hand UI work to the context.
};
auto client = context->sys<d2::sys::net::SystemWS>()->connect(std::move(connection));
```

Include `<core/mods/d2_ws.hpp>` and register the module before use. The curl backend owns transport work. Message views are borrowed during callbacks; callbacks can run off the UI thread. Reconnection can repeat open/close sequences, so application state needs its own recovery policy. Retain a client to send/close it; unload from outside transport callbacks. HTTP regressions do not imply full WebSocket protocol/interoperability coverage.

## Scoped HTTP headers

`sys::net::SystemSession` stores named header scopes matched against parsed URL/method components. A scope rule is a string, regex or `monostate` wildcard; optional short-circuiting stops processing after a match. `set_scope`, `set_header`, `get_header`, `reset` and `process` provide the interface. This is header selection, not a cookie jar or durable authenticated session. Scope order is not an application priority contract; avoid conflicting overlapping scopes. Keep authentication scopes narrow.

## Audio

`sys::SystemAudio` abstracts input/output devices, activation, format descriptions, record/transmit streams, filters and device watches. The supplied backend is PipeWire and is built only when its development package is available. Use `enumerate`, `default_device`, `activate`, `format`, `record`/`transmit`, then `deactivate` as the application lifecycle requires. Configure filters through the module's filter API and retain device-watch signal handles.

Audio data and formats are separate from image rasters and inference messages. Callbacks receive borrowed sample spans; copy before retaining them and keep processing short. Device availability and format negotiation are runtime concerns. Audio implementation changes and live device interoperability are outside the image/inference regression coverage; consult `core/mods/d2_audio.hpp` and `os/audio/pipewire/` for exact formats and filter callback contracts.

## Logging and diagnostics

`rs::RuntimeLogs` is the logging service (`logs/logd.hpp`), configured through `rs::Config` passed to `IOContext::make`. `rs::context::get()` provides the thread's logger, which may be absent. Prefer logging to writing arbitrary stdout text while a terminal UI owns the display.

```cpp
if (auto logs = rs::context::get())
    logs->log(rs::Severity::Info, "app", "Starting work");
```

`D2_TLOG` is the framework shorthand for classes with a module tag; it is not a standalone free-function logger without that tag. Build options control runtime logging, assertions and debug watching. Do not log credentials, request authorization headers or sensitive prompts by default.

`extras/d2_debug_box.hpp` provides an in-app diagnostic panel. The scheduler, screen and output also expose counters/timings for application diagnostics. These values do not constitute a metrics exporter or long-term storage system.
