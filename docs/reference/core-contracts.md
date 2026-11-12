# Core contracts

These contracts apply to the current implementation. Read them when crossing thread, buffer or module boundaries.

## Scheduling and futures

`mt::TaskQueue<T>` wraps moodycamel's growing `BlockingConcurrentQueue`. It accepts move-only, nothrow-movable payloads. Enqueue can allocate; there is no configured maximum capacity. Allocation failure is reported by `try_enqueue` or thrown by `enqueue`. `TaskRing` remains available as legacy source; the framework no longer uses it.

Ordinary queues preserve each producer's order, not a total order between producer threads. Consumers may run tasks concurrently. `TaskQueue<T, true>` uses a shared explicit producer token with a short producer-side lock when cross-thread handoff must preserve ordering. Curl command and upload queues use that mode. Worker queues use independent producers and no submission mutex. Combined signals use a separate, protected latest-value mailbox; normal signals use the growing queue. The old `DropIfFull` signal flag is retained but has no capacity limit to act on.

Workers stop accepting submissions, wait for in-progress submissions to publish, and drain accepted immediate work before joining. Periodic tasks are returned by `Worker::stop`; stopping does not run them to completion. Pool shutdown discards the returned delayed/periodic tasks and wakes their futures. Custom task callbacks can handle `Task::Query::Discard` for cancellation cleanup. Start/stop and reconfiguration are lifecycle operations: serialize them, and stop UI/main workers from their owning thread. Do not restart a worker from inside its own callback. The pool's default pressure limit is infinite; applications can configure an explicit pressure policy.

A future has one consumable value, or a stored exception, or a discarded state. Reading a value keeps the terminal/completed state intact. Reading a discarded or already-consumed terminal result throws `mt::TaskDiscarded`. Exceptions retain their original type. Periodic futures retain the latest unread tick; they are not a result stream.

Both `future<T>` and `co_future<T>` can be awaited. Registration and completion share synchronization, and only one coroutine may await a particular pending result. A second waiter is rejected. Continuations resume on the thread publishing the result; awaiting does not select a UI thread or scheduler. Use `IOContext::sync_async` to transfer UI work. Dropping a future does not cancel a running task or a suspended coroutine. Explicit `discard()` wakes waiters but is a cancellation request, **not a join**. Callbacks already executing may finish. External code must not destroy a suspended coroutine while it is registered as a waiter.

Managed hooks receive `ManagedControlBlock&`, not an owning shared pointer. A destruction hook cannot extend the lifetime of the object being destroyed. Configure hooks before publishing the block to other threads; hook state itself is not a concurrent container.

## Compact Unicode cells

A cell still contains a one-byte character token. Printable ASCII is stored directly. A thread-local `ExtendedCodePage` interns other strings into 159 slots. Tab and the existing image marker are reserved. Entries own their full UTF-8 string, with no eight-byte grapheme limit.

`try_write(text)` returns `std::nullopt` on palette exhaustion. `write(text)` throws `std::overflow_error`. Existing entries remain usable when full. `size()` and `capacity()` expose usage. Invalid/unallocated tokens read as `?`.

Tokens belong to the thread's palette. Intern and render on the same UI thread. `clear()` invalidates every token previously issued by that palette: only clear it after invalidating all buffers and styles holding those tokens. The implementation does not reclaim slots each frame or solve cached-buffer palette ownership. It deliberately avoids making cached cells silently change meaning. A future overflow/ownership design can preserve the compact cell format.

`StringIterator` segments Unicode graphemes through uni-algo. Iterator distance requires the same source range and accepts either endpoint order. It does not infer terminal cell width. `Input` and `MultiInput` editing APIs use grapheme positions; their viewport geometry uses terminal cells. The underlying `FragmentedBuffer` remains byte-oriented. See [text and editing contracts](../guide/text-and-selection.md).

## Buffers and lifetimes

`PixelBuffer(w, h)` allocates its cells. Negative dimensions and invalid coordinates fail explicitly. Views use local coordinates, clip inscription to their rectangle, and clear only that rectangle. Inscription accepts compressed source subviews and overlapping source/destination buffers. Compressed buffers cannot be mutated or randomly indexed until represented as uncompressed data.

A strided view has no contiguous `data()` span. Such access throws; use `at(x, y)` or row-wise processing. A full compressed view exposes its encoded storage. RLE readers reject truncated runs.

`FragmentedBuffer` is temporarily nonmovable: its facades, iterators and borrowed shard views cannot be safely memberwise-moved. Position-based edits restore the complete cursor through a fresh lookup. Owned text remains owned; `setup_view` still requires its source to outlive the buffer.

Element listeners retain their existing persistent semantics: dropping `EventListener` does not disconnect it. Call `destroy()` when the subscription should end. Muting, unmuting and destroying are idempotent, and dispatch keeps callbacks alive while they remove themselves. Binder disconnects its owned subscriptions. The `animate_twoway` and `animate_toggle` DSL helpers return their event listener handles, since their work starts on later input events. Input blink jobs use weak element references and request cancellation at destruction.

Module load/unload transitions are serialized and idempotent. Unload executes both cleanup phases once, records failures, and does not run callbacks under the context registry lock. Context teardown unloads dependents before dependencies. Curl workers stop before their transport resources are destroyed. Unload curl modules from outside transport callbacks.

HTTP requests return control handles for every body mode and expose typed errors, early headers and deadlines. See [HTTP contracts](../guide/http.md) for callback delivery, borrowed views, cancellation and shutdown rules.

## Image frames

PixelBuffer carries an image scene independently of its compact cells. Use `SystemOutput::write_frame` to preserve it through output. [Image contracts](../guide/images.md) cover coordinates, clipping, paint ordering, optional backends and output integration. Cache policy selection replaces the previous policy; uncached children redraw when a parent reconstructs its frame.
