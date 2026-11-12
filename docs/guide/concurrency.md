# Concurrency and asynchronous work

[Documentation index](../index.md) · [Core contracts](../reference/core-contracts.md)

## Schedule work

An active UI context owns a `mt::ConcurrentPool`. `scheduler()->launch(callback, args...)` queues immediate work and returns `mt::future<T>`; `launch_void` reports unhandled exceptions through the pool's exception policy. Delayed/cyclic variants schedule timed work. Capture owned input values and weak UI references rather than references to a construction stack.

```cpp
auto result = ctx->scheduler()->launch([] { return 6 * 7; });
// In a coroutine:
auto answer = co_await result;
co_await ctx->sync_async([answer] { /* update UI here */ });
```

The `ctx` above must remain owned by the application while the operation runs. In long-lived library work use a weak context and handle shutdown. Awaiting does not automatically return to the UI thread. A completion that is already ready can also continue inline. Keep transport callbacks short and marshal UI mutations explicitly.

For headless pool work: `auto pool = mt::ConcurrentPool::make<>(); pool->start();`, submit jobs, consume results, then `pool->stop()` from an owning thread. Starting/stopping/reconfiguring is a serialized lifecycle operation, not a concurrent job API.

## Futures and coroutines

`future<T>` and `co_future<T>` are awaitable and can serve as coroutine return types. Coroutines start immediately. Their result is a single consumable value, stored exception or discarded state. Copies share that same result; they do not create independent consumers. `.value()` waits and consumes; `has_value`, `has_exception`, `exception`, `is_discarded` and `block()->is_ready()` allow inspection where needed.

Only one coroutine can await a particular pending result. A second pending waiter is rejected. Completion resumes the continuation on the publishing thread, outside the result lock. An exception keeps its original type. Dropping a future does not cancel its producer or destroy a suspended coroutine. Do not externally destroy a coroutine while it is registered as a waiter.

`discard()` wakes consumers with `TaskDiscarded` but is not a join or forced interruption. An already-running callback can still finish. For APIs with their own cancellation control (HTTP requests, inference generations, image application), use that control. Periodic futures expose the latest unread tick; they are not a lossless event stream.

## Queue behavior

`TaskQueue<T>` wraps moodycamel's growing `BlockingConcurrentQueue`; there is no fixed capacity. It accepts move-only, nothrow-movable payloads. Enqueue can allocate and fail. Ordinary mode preserves order per producer rather than globally between concurrent producers. Worker queues avoid a global submission mutex.

`TaskQueue<T, true>` adds a shared explicit producer token with a short producer-side lock for cases needing cross-thread ordering, such as transport commands/uploads. `TaskRing` remains legacy code and is not used by the framework. Configure a pressure policy if the application needs backpressure; growing storage does not make unbounded producers free.

## Shutdown and deadlocks

Workers close admission, let in-progress submissions publish and drain accepted immediate tasks before joining. Delayed/periodic jobs are discarded by pool shutdown and their futures are awakened. A custom task can handle `Task::Query::Discard` for cleanup.

Never wait on UI-dependent work from the UI thread. Never join a worker from its own callback. Avoid lock-held calls into user callbacks or `sync()`. Keep the context and scheduler alive until operations that use them settle. See each service guide for its unload/cancellation behavior; thread-safe module access does not make shutdown arbitrary.
