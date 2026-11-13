# 07. Runtime modules, platform backends and networking

**Primary source:** `core/io/d2_module.hpp`, `core/io/d2_context.hpp`, `core/mods/*.hpp`, `os/d2_registry.cpp`, `cmake/system.cmake`, `tests/module.cpp`, `tests/http.cpp`, `tests/transport.cpp`.

## Modules: interface vs implementation

Modules isolate services and platform-dependent behavior:

- `d2::sys::SystemModule`: lifecycle and common metadata. `Status`: `Ok`, `Degraded`, `Failure`, `Offline`, `Loading`; `Compat`: `Native`, `Generic`, `Unknown`, `Failed`.
- `d2::sys::ModuleDecl<Interface, "Name", Access, Dependencies...>`: abstract API and runtime requirements.
- `d2::sys::ModuleImpl<Interface, LoadType, Dependencies...>`: implementation part for concrete backend.
- `d2::sys::Module<...>`: combined interface/implementation for platform-independent services.
- `Access::TSafe` or `Access::TUnsafe`: whether module methods may be accessed from ordinary threads. An unsafe module's `ModulePtr::operator->()` checks accessibility and can throw.
- `Load::Immediate` or `Load::Lazy`: load at initialization/explicit request vs first query. Dependencies may also be loaded automatically.

`IOContext::make<Interfaces...>()` selects/constructs the requested runtime. Module implementations are registered with build/OS-discovery code; not every interface has a working implementation on every machine.

### Queries and lifecycle

```cpp
auto context = ctx.context();

// Requires a usable module, or throws if unavailable:
auto http = context->sys<d2::sys::net::http>();

// More defensive query, null when missing/unhealthy:
auto maybe_http = context->sys_if<d2::sys::net::http>();
if (maybe_http)
{
    // Safe to use this module, subject to its thread-access rules.
}

// Other APIs:
// context->load<Interface>();
// context->unload<Interface>();
```

For optional features prefer `sys_if`. `sys_if` checks health/status (`Status::Ok` in inspected code). Simply including a module header does not enable an implementation. A concrete implementation may have a lazy load type; query-time side effects should be expected.

### Important module families

| Interface alias / header | Role / availability |
| --- | --- |
| `sys::input`, `sys::output`, `sys::screen` / `d2_core.hpp`, `d2_screen.hpp` | Essential terminal UI modules; main runtime |
| `sys::binder` / `d2_binder.hpp` | Keyboard shortcut/sequences service |
| `sys::clipboard`, `sys::notify`, `sys::selector` / `d2_ext.hpp` | Platform clipboard, notification and screen text-selection services |
| `sys::net::http` / `d2_http.hpp` | HTTP client, optional libcurl backend |
| `sys::net::ws` / `d2_ws.hpp` | WebSocket client, optional libcurl backend |
| `sys::net::session` / `d2_session.hpp` | Scoped request headers / session-level matching |
| `sys::image_loader` / `d2_image_loader.hpp` | Abstract image decoder; PNG backend optional |
| `sys::images` / `d2_images.hpp` | Terminal inline image transport and compositing |
| `sys::audio` / `d2_audio.hpp` | Audio stream/filter API; optional PipeWire backend |

Check the alias in its actual header before writing `IOContext::make<...>`. Terminal image protocols are backends with terminal-specific compatibility checks; there is no universal assumption that all terminals support Kitty or Sixel.

## HTTP: `d2::sys::net::SystemHTTP`

`SystemHTTP::request(Request)` returns `std::unique_ptr<Stream>`; operations are callback-driven and can run off the UI thread. `using http = SystemHTTP;`.

### Request fields

| Field | Type / behavior |
| --- | --- |
| `method` | `std::string_view`, defaults to `Method::Get` (`Get`, `Post`, `Put`, `Patch`, `Delete`, `Head`, `Options`) |
| `url` | owning `std::string` |
| `on_setup` | `void(RequestBuilder&)` with `set(header,value)`, `body(value)`, `stream()` |
| `on_headers` | `void(Response&)` as soon as headers arrive |
| `on_chunk` | `void(Chunk&)` to stream response payload; `read()`, `cancel()`, `down_current()`, `down_total()` |
| `on_response` | `void(Response&)` final callback; `status()`, `get(header)`, `body()` |
| `on_error` | `void(const Error&)` |
| `deadline` | optional steady-clock deadline |
| `timeout`, `connect_timeout` | optional millisecond durations |
| `max_response_bytes`, `max_header_bytes` | size limits; `0` for default/unlimited contract as implemented |
| `follow_redirects`, `max_redirects` | redirect behavior |

`Error::Code`: `InvalidRequest`, `Unavailable`, `Cancelled`, `Timeout`, `Transport`, `Callback`, `Limit`. Error also contains `message`, `cause` exception pointer, and optional native code.

### Safe request pattern

```cpp
using HTTP = d2::sys::net::SystemHTTP;

if (auto client = context->sys_if<d2::sys::net::http>())
{
    HTTP::Request request;
    request.method = HTTP::Method::Get;
    request.url = "https://example.com/";
    request.timeout = std::chrono::seconds(10);

    request.on_response = [](HTTP::Response& response)
    {
        const int code = response.status();
        std::string content(response.body()); // Own a copy beyond callback.
        // Deliver data to UI on the appropriate thread.
    };
    request.on_error = [](const HTTP::Error& error)
    {
        // Log or expose error.code / error.message.
    };

    auto stream = client->request(std::move(request));
    // Retain stream if cancellation/progress control will be required.
}
```

**Response lifetime:** `std::string_view` returned by `body()`, `get()` and `Chunk::read()` must not be kept beyond the callback unless copied. **Streaming behavior:** `tests/http.cpp` demonstrates that when `on_chunk` is installed, the final body is not guaranteed to contain the collected chunks. Accumulate chunks explicitly if the application needs a complete response. `Stream::write`, `finish`, `cancel`, `is_open`, `up_current`, `up_total` are for request body streaming and upload progress.

## WebSockets: `d2::sys::net::SystemWS`

`SystemWS::connect(Connection)` returns a `Client::ptr` (`std::shared_ptr<Client>`); `using ws = SystemWS`.

```cpp
using WS = d2::sys::net::SystemWS;
WS::Connection connection;
connection.url = "wss://example.com/events";
connection.reconnect = true;
connection.on_open = [](WS::Client::ptr client)
{
    client->send("hello", WS::Type::Text);
};
connection.on_message = [](std::string_view msg, WS::Type type, WS::Client::ptr client)
{
    std::string owned(msg);
    // Dispatch owned payload safely; msg is a callback-scoped view.
};
connection.on_error = [](std::string error) { /* record error */ };
connection.on_close = [](std::uint16_t code, std::string_view reason)
{
    // Copy reason if it will outlive this callback.
};

// auto client = context->sys<d2::sys::net::ws>()->connect(std::move(connection));
```

Connection config includes `ping_interval`, `timeout`, `reconnect`, min/max reconnect delay, max attempts, `on_setup(RequestBuilder&)` for handshake headers. `Client::send(message, Type::Text|Binary)`, `close(code=1000, reason="")`, `state()` with `Created`, `Connecting`, `Open`, `Closing`, `Closed`, `Failed`, `Reconnecting`. Keep client lifetime and shutdown behavior explicit.

## Session header scoping

`SystemSession::Scope` matches protocol, HTTP method, domain, path, and port using `std::variant<std::string,std::regex,std::monostate>`, with optional `short_circuit`. Configure named scopes with:

- `set_scope(name, Scope, absl::flat_hash_map<std::string,std::string> headers={})`
- `set_header(name, header, value)`, `get_header(name, header)`
- `reset(name)`, `reset()`
- `process(url, method, callback(header_name, header_value))`

This is a **header/rule mechanism**, not a full browser cookie jar. Validate security implications when sending authentication headers to URL patterns; choose the narrowest possible domain/path scope. `SystemSession` is a lazy, thread-safe module.

## Platform discovery and CMake integration

The core build uses `cmake/system.cmake` to discover/register implementations and produces a generated `d2_platform.hpp`. Backends in `os/` provide the actual terminal, image, network, clipboard, notification and audio integrations. Module filtering knobs: `D2_MODULE_EXCLUDE_BASES`, `D2_MODULE_EXCLUDE_TYPES`, `D2_MODULE_EXCLUDE_DIRS`, `D2_MODULE_ROOTS`.

**For module authors:** define the interface in `core/mods/` if platform-agnostic, concrete platform implementation in `os/`, list explicit dependencies, choose correct `Access` and `Load`, register using the repository's `delta_register_module` CMake mechanism, then test `Platform::resolve` and failure paths. Test compatibility ordering explicitly when multiple implementations match.
