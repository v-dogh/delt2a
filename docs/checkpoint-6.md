# Checkpoint 6: module layout and HTTP preparation

This checkpoint includes checkpoints 1–5. It prepares the existing framework for a native OpenAI-compatible provider; the provider itself is still the next stage. No remote branch was pushed.

## Delivered

- Moved the inference abstraction into `core/mods/inference/`. Namespaces and the optional `DeltaAscii::Inference` target remain unchanged. Includes, installation rules, consumers and documentation use the new location.
- Every HTTP request now returns a control handle, including fixed-body POSTs receiving streamed responses. Cancellation wakes the existing curl multi worker and works without waiting for further server bytes, including paused uploads.
- Added total/connection timeouts, absolute deadlines, early response-header callbacks, configurable response/header byte limits and redirect behavior.
- Replaced HTTP string errors with typed errors retaining original callback exceptions and native curl codes where available. HTTP error statuses remain responses for the application/provider to interpret.
- Setup/header/chunk exceptions terminate requests predictably. Completion callback failures reach the error handler, and C++ exceptions do not escape through curl's C callbacks.
- Shutdown stops admission before draining accepted requests. Worker startup uses a separate framework future so initialization cannot be confused with a later stop transition. Transport-wide failures stop admission and settle active operations.
- Handles retain progress/completion state without owning the HTTP module. Header blocks reset their fields between informational/redirect/final responses.
- Corrected the existing `CURLINFO_RESPONSE_CODE` call to use a `long` destination rather than `int`, avoiding a write past the status field on platforms where `long` is wider.

## Migration

Replace `<inference/...>` includes with `<core/mods/inference/...>`. The old include directory is removed; there are no forwarding headers. Existing CMake target names remain valid, and excluding inference still omits its JSON dependency.

HTTP `on_error` callbacks now accept `const SystemHTTP::Error&`; use `error.message` for the old textual description and `error.code` for branching. A non-null request handle no longer means that the upload body is streamed. See [HTTP contracts](http.md) for callback threads, early delivery, borrowed views, control-handle ownership and timeout limitations.

## Validation

All fourteen CTest groups pass with Clang 19 Release/Unicode, GCC 14.2 Release/Unicode, Clang Release/ASCII with PNG disabled, and Clang ASan+UBSan/Unicode. The new POSIX loopback HTTP group uses actual local sockets and curl transfers. It exercises fixed-body streaming responses, cancellation and deadlines with silent peers and paused uploads, streamed upload completion, callback exception propagation, header/body limits, HTTP 429 responses, informational headers, redirects, transport errors, expired/invalid deadlines, queued shutdown and reload.

A fresh installed-package consumer builds and runs using the relocated inference headers. A separate configuration with inference and elements disabled passes the public consumer and HTTP tests. Both incremental and cumulative patches are applied to fresh source copies and checked byte-for-byte before bundling.

No external model endpoint, SSE parser, TLS/proxy integration, OpenAI request mapping or tool-call stream assembly is implemented or tested here. Timeout/cancellation cannot preempt an application callback that blocks the transport worker. ASan/UBSan runs with leak detection disabled because of environment tracing. No clean TSan result is claimed; checkpoint 1's queue diagnostic remains unresolved. Existing unrelated compiler warnings remain. Doxygen and PipeWire were not exercised, and the earlier terminal PTY checks were not repeated for these changes.

## Next

Implement the native Chat Completions adapter over `SystemHTTP`: request serialization, incremental SSE framing and tool-call assembly, endpoint/model compatibility settings, usage/error mapping and integration with the existing provider lifecycle. Conversation/history/tool policy remains in `core/mods/inference/`.
