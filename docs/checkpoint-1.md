# Core cleanup checkpoint

This is the first implementation checkpoint after the `impl` audit: a reproducible build/test baseline and fixes to core correctness and lifetime handling. Shared text layout, generic selection, the image compositor and inference are subsequent stages.

The input was `delt2a-impl.zip`, whose archive comment identifies commit `c206b59e2c237a61d47ac35003f6cb9a718e0ba0`. Its SHA-256 is `5b174781886328f359a7b37234cb3ced0c96aee5fc08161691ff328361d59fe5`. This bundle contains the modified source and a patch against that input. No remote branch was updated.

## Changes

- Replaced framework uses of `TaskRing` with a growing moodycamel queue pinned to v1.0.4. Worker producers do not share a submission mutex. Ordered transport queues preserve cross-thread command handoffs; the distinction is explicit in the contracts.
- Repaired future completion, cancellation, exception propagation and coroutine registration races. Shutdown drains accepted immediate work and cancels pending delayed work. Removed unsafe coroutine-handle destruction and destructor-time `shared_from_this`.
- Repaired module unload transitions, dependency teardown order, curl resource ownership and exception propagation through synchronous UI calls.
- Kept one-byte cells while making palette exhaustion explicit. Extended entries own complete UTF-8 strings; tab/image markers are reserved, clearing resets allocation, and invalid tokens fail safely. Fixed grapheme iterator distance and Unicode whitespace handling.
- Fixed pixel-buffer allocation, view coordinates, clipping, compressed source strides, overlapping inscription and truncated RLE reads. Disabled unsafe memberwise moves of `FragmentedBuffer`.
- Fixed listener accounting and self-removal, Binder subscription cleanup and input blink callback lifetimes. Corrected several public template/iterator errors exposed by compiler and consumer checks.
- Added regression tests, a runnable example, package exports and installation support, ASCII build fixes, Markdown contracts and an optional Doxygen reference target.

## Migration

`ManagedControlBlock` hooks take a block reference instead of an owning pointer. `FragmentedBuffer` is nonmovable. `animate_twoway` and `animate_toggle` return their listener handles. Compact palette overflow now throws from `write`; use `try_write` for a local fallback. Consuming a terminal future result a second time throws instead of hanging. `Task::Query::Discard` lets custom tasks release their own pending state when the pool shuts down.

The legacy `TaskRing` remains in the source, but is no longer the framework's queue. `DropIfFull` remains a compatibility flag without a fixed capacity to act on. Growing queues are bounded by available memory; no throughput or latency improvement is claimed without a benchmark on the application's workload.

See [core-contracts.md](core-contracts.md) for ownership and threading rules, and [building.md](building.md) for build and consumer commands.

## Validation

Checked on Linux with CMake 4.4.4 and libstdc++. The six CTest groups cover queues/futures/workers, Unicode/buffers, modules, public headers, events and curl lifecycle.

| Configuration | Result |
| --- | --- |
| Clang 19, Release, Unicode, examples enabled | Build and 6/6 CTest groups passed |
| GCC 14.2, Release, Unicode, examples enabled | Build and 6/6 CTest groups passed |
| Clang 19, Release, ASCII, examples enabled | Build and 6/6 CTest groups passed |
| Clang 19, RelWithDebInfo, ASan + UBSan | Build and 6/6 CTest groups passed |
| Installed package, independent Clang consumer | Configure, build and execution passed |
| Standalone scheduler, pinned dependency fetch and install | Configure, build and installation passed |
| GCC ThreadSanitizer, standalone scheduler tests | Not clean: diagnostic in moodycamel's implicit-producer block reclamation |

LeakSanitizer was disabled with `ASAN_OPTIONS=detect_leaks=0` because it cannot run under this environment's tracing restrictions. The TSan build warns that `atomic_thread_fence` is not supported by its instrumentation; a fence-related false positive is plausible, but has not been established. The diagnostic was not suppressed and must not be counted as a passing race check. Its log is included in the bundle.

GCC still emits warnings in older enum-switch paths; this is not a warning-free build. Doxygen generation and PipeWire were not exercised because those dependencies were unavailable. The runnable example was built but not tested interactively. Curl tests cover load/unload/reload and resource lifetime, not real HTTP or WebSocket traffic. No performance benchmark was run.

## Deliberate limits

This does not make the old input widgets grapheme-aware. Their source positions need the shared layout model before generic selection can be implemented consistently. The palette still has 159 extended entries and thread-local ownership; it now detects exhaustion rather than silently corrupting entries. Cached buffers still constrain palette clearing.

The constraint solver's removal operation remains unfinished. Terminal protocol coverage, image composition and rendering, streaming HTTP behavior and inference are not completed by these changes. Tests are headless; interactive terminal behavior requires a real terminal check.
