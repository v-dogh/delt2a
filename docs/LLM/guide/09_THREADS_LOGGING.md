# 09. Threading, scheduling, futures and logging

**Source:** `core/io/d2_context.{hpp,cpp}`, `core/io/d2_main_worker.hpp`, `mt/{pool,future,control,task_queue,task_ring,defer}.hpp`, `logs/{logd.hpp,logd.cpp}`, `core/tree/{d2_styles,d2_theme}.hpp`.

## Where code executes

The main context owns the TUI event loop, output and many layout/render operations. Worker scheduler tasks, HTTP and WS callbacks, image decode and audio can run on other threads. `IOContext::get()/set()` is **thread-local**. The existence of a context on the main thread does not imply `IOContext::get()` finds it on a random worker.

`SystemModule::Access::TSafe` and `TUnsafe` are part of module contracts; a `ModulePtr` checks access to `TUnsafe` modules. UAI property setters/readers have synchronization paths, but **this does not make every element method or raw pointer magically thread-safe**.

## Safely dispatch UI work

```cpp
// Capture a context shared_ptr before leaving the UI thread:
auto runtime = ctx.context();

// In a background callback/task:
runtime->sync_async([/* safe captures */]
{
    // Work against still-live UI handles on the main worker.
});
```

`IOContext::sync(func)` dispatches and waits for completion when required; `sync_async(func)` returns an `mt::future<...>`; if already on the synced/main thread it **runs the callback inline**, otherwise it queues it to the main worker. This is a potential reentrancy detail, not a guarantee of delayed execution. Use `is_synced()` to assess whether calling directly is appropriate. Property `set<Property>` already carries its own UI synchronization path; don't add unnecessary `sync` recursively without checking call flow.

**Deadlock rule:** do not block the UI thread waiting for a future that requires a queued UI callback to complete. Do not schedule main-thread work before the main worker has entered a state in which it can service it, unless the API explicitly supports that phase.

## Scheduler helpers from `TreeCtx`

- `ctx.async(fn,args...)`: submit scheduler work.
- `ctx.defer(duration,fn,args...)`: schedule delayed work.
- `ctx.repeat(duration,fn,args...)`: launch cyclic work.
- `ctx.sync(fn,args...)` / `ctx.sync_async(fn,args...)`: bounce work to context/main worker.

`ctx.scheduler()` returns the underlying `mt::ConcurrentPool::ptr`, which exposes `make<Topology>()`, `start`, `stop`, `grow`, `shrink`, `reconfigure`, `active`, `node`, `launch`, `launch_void`, `launch_deferred`, `launch_cyclic`, `co_launch`. Check the `mt` topology API before adjusting thread count or affinity.

### Futures

`mt::future<T>`, `mt::man_future<T>`, and `mt::co_future<T>` expose assorted synchronization/control APIs, including `value`, `value_or`, `sync`, `sync_value`, `discard`, `pause`, `unpause`, `has_exception`, `has_value`, `is_discarded`, `is_paused`, `block`; coroutine futures can be `co_await`ed in the intended coroutine context. Not every future wrapper has identical ownership or cancellation semantics. Do not assume a `discard()` magically interrupts arbitrary OS I/O: inspect the producer for cooperative cancellation.

`IOContext::launch_thread(...)` initializes contextual behavior for the launched thread; prefer it over a manually launched `std::thread` when the callback relies on context lookup. Explicitly retaining a shared context is still clearer and safer for long-running work.

## Callback lifetime and captures

Potentially long-lived work includes `ctx.repeat`, deferred jobs, `on_response`, `on_message`, screen events and dependency listeners. For each callback answer:

1. Does it run on the main worker or some other thread?
2. Who owns the callback and when does it end?
3. Are captured `TreeIter` handles weak, and can their tree be swapped out?
4. Are captured references to local variables still alive?
5. Can it synchronously re-enter the UI or dependency subscription during initialization?
6. Does cancellation occur at object teardown, or must the caller call `close/cancel/discard`?

A safe default for remote callbacks is **copy incoming `string_view` data immediately**, then dispatch a bounded update to the main thread using a still-live context and weak UI handle.

## Reactive properties and threading

`style::Dependency<T>` can be modified and subscribed to; UAI layers implement synchronization and write notification. A subscription's initial callback runs immediately in the subscribing call path. Other updates can reach callbacks through the dependency's synchronization rules. Treat callbacks as possibly reentrant during construction and avoid holding application locks while calling dependencies unless designed for it.

`getref<Property>()` returns a reference/pointer-like view into a backing style value, not a protected mutation transaction. Prefer `set`, `apply_set`, `get`, `apply_get` for thread-aware, rendering-aware access.

## Logging without destroying terminal output

The `logs/` subsystem is under namespace `rs`, with a `rs::Config` accepted by `IOContext::make` and support for logging severity, memory limits and signal handling. Project code also uses `D2_TLOG(...)` and `D2_TAG_MODULE(...)` macros from its logging integration. Read `logs/logd.hpp` for actual logger entrypoints.

During UI execution, printing free-form diagnostics to `std::cout` can corrupt the terminal frame or disrupt cursor positioning. Prefer runtime logs, configured log storage, a debug widget, or a dedicated diagnostic pane. Use stderr only when UI context is no longer active or when platform behavior is explicitly known.

## Build/tests for concurrent changes

The CMake `D2_SANITIZER` option supports address, undefined, thread, and memory modes with constraints (MSan requires Clang and a compatible toolchain/dependency stack). `D2_BUILD_TESTS` enables regression tests, including signal, module, transport and input cases. For main-worker changes, add tests for startup/shutdown order, sync reentrancy, cancellation and tree destruction mid-flight.
