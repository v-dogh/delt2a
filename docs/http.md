# HTTP requests and streaming

`sys::net::SystemHTTP` is the transport interface in `core/mods/d2_http.hpp`. The curl backend uses its existing multi-handle worker for concurrent requests. Inference does not introduce another HTTP runtime.

Load the backend through `IOContext` and obtain `ctx->sys<sys::net::SystemHTTP>()`. Request-capable module instances need shared ownership and a live context. Named session rules, when loaded, still add headers before the request's own setup callback.

```cpp
using HTTP = d2::sys::net::SystemHTTP;
HTTP::Request request;
request.method = HTTP::Method::Post;
request.url = "http://localhost:8080/example";
request.connect_timeout = std::chrono::seconds(5);
request.timeout = std::chrono::seconds(60);
request.max_response_bytes = 8 * 1024 * 1024;
request.max_header_bytes = 64 * 1024;
request.follow_redirects = false;
request.on_setup = [](HTTP::RequestBuilder& builder)
{
    builder.set("Content-Type", "application/json");
    builder.body(R"({"input":"hello"})");
};
request.on_headers = [](HTTP::Response& response)
{
    // Status and this response block's headers are available before its body.
    auto type = response.get("Content-Type");
};
request.on_chunk = [](HTTP::Chunk& chunk)
{
    auto bytes = chunk.read();
    // Feed an incremental decoder; HTTP chunks are not application messages.
};
request.on_response = [](HTTP::Response& response) { /* transfer completed */ };
request.on_error = [](const HTTP::Error& error) { /* settle the caller's operation */ };
auto handle = http->request(std::move(request));
// Later, from another thread if necessary:
handle->cancel();
```

## Control and ownership

Every submitted request returns a non-null `Stream::ptr`, including GET and fixed-body POST requests. The existing name is retained because the same handle also supports streaming uploads. `cancel()` works before response bytes arrive and while an upload is paused. It wakes the worker, which removes cancelled transfers instead of waiting for another receive callback.

`is_open()` becomes false on cancellation or settlement. Upload progress remains readable after completion. Handles do not keep the module alive. Dropping a handle does not cancel a request; for a streaming upload its destructor retains the existing finish behavior. Call `finish()` after the last upload write. Serialize upload `write()` and `finish()` calls so no producer writes after finishing. Fixed-body requests ignore those upload methods.

`on_setup` runs synchronously on the submitting thread. Ordinary headers/chunks/completion/error callbacks run on the HTTP worker. Invalid configuration, setup exceptions and submissions to a stopped module can invoke `on_error` before `request()` returns; initialize callback state before submitting. Resource allocation errors, missing context and shared-ownership precondition failures may throw directly before a request is accepted.

Callbacks must keep their captured state alive through terminal delivery. Response/header/body views and chunks are borrowed for the callback only. Copy data that must survive it. Keep handlers short; they share the transport worker with other requests. UI work belongs on `IOContext::sync_async`. Unload the module from outside its callback thread.

## Headers and bodies

`on_headers` runs after each complete HTTP header block, including informational and redirect responses. Check the status code instead of assuming its first invocation is the final response. A new status line resets the header map and buffered body; intermediate headers cannot leak into the final block. Header lookup is ASCII case-insensitive. Duplicate header names currently retain the last value. Trailers update the final header map but do not generate another `on_headers` call.

With `on_chunk`, body bytes go directly to that callback and are not duplicated into `Response::body()`. Without it, the backend buffers the body for `on_response`. An HTTP error status such as 429 is still an HTTP response: inspect its status, headers and body to map it into application errors. It is not a transport failure.

Redirect following defaults to enabled, with `max_redirects = 10`. Disable it for an endpoint that should not redirect. Protocol-specific parsers and authentication/redirect policy remain the caller's responsibility.

## Deadlines and limits

`timeout` measures total elapsed request time from submission to completion, including setup and queue delay. An absolute steady-clock `deadline` can also be supplied; the earlier limit wins. `connect_timeout` separately limits libcurl connection establishment. Relative timeout values must be positive; omit them for the default behavior. The curl backend applies its native timeouts and also checks absolute deadlines in the worker, including for paused uploads.

Timeout/cancellation cannot preempt a blocking application callback, and resolver/platform behavior can affect timing. Shutdown waits for executing callbacks to return. These are not hard real-time guarantees.

`max_response_bytes` bounds received body bytes in both buffered and streaming modes. `max_header_bytes` bounds cumulative header bytes, including intermediate blocks and trailers. Zero means unlimited. Limits report `Error::Code::Limit` and abort transfer; earlier chunks already delivered to the application cannot be recalled.

## Errors and shutdown

`on_error` now accepts `const HTTP::Error&`, replacing the string callback. Codes distinguish invalid request configuration, an unavailable module, cancellation, timeout, transport errors, callback failures and size limits. `native_code` carries a curl error when available. `cause` preserves an original C++ exception where applicable; callback failures can be inspected with `std::rethrow_exception`.

Setup/header/chunk exceptions terminate the request and call `on_error` once. They never unwind through a C callback. A successful transfer invokes `on_response` once; if that completion callback itself throws, the exception is forwarded to `on_error` once, without replaying completion. Effects already performed by that callback cannot be rolled back. Exceptions from the final error handler are contained and do not recursively call it again.

Unload closes admission before stopping the worker. Accepted queued and active requests are drained/settled; later submissions fail with `Unavailable`. A completion already in progress can finish while unload begins. Reload starts with no leftover requests from the previous worker. Terminal dispatch marks the control handle closed before invoking its callback.

## Validation scope

The POSIX loopback suite exercises fixed-body POST and streaming reception, early headers, informational responses, HTTP error bodies, cancellation and timeout with silent peers and paused uploads, upload completion, callback exceptions, byte limits, expired deadlines and queued shutdown/reload. No external service or credentials are used. It does not establish SSE framing correctness, TLS/proxy interoperability or OpenAI API compatibility; those belong to later protocol/integration tests.
