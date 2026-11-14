# 12. Behavioral gotchas, caveats, and historical snapshot limitations

This section covers usage pitfalls and compatibility notes from the supplied archive. Previously flagged implementation defects were reported fixed by the maintainer and their obsolete warnings were removed. Updated source was not supplied for a fresh audit. Treat the remaining snapshot-specific caveats as historical rather than assertions about the latest repository.

## High-priority behavioral gotchas

| Risk | Why it matters | Safe practice |
| --- | --- | --- |
| `dstyle` is a macro, not a generic function | It refers to local variable `ctx` and `decltype(ctx)::element_type` | Name builder parameter `ctx`; use explicit `set<Property>` elsewhere |
| `ctx.subscribe` is eager | Implementation invokes callback before registering the subscription slot | Initialize all referenced state first; avoid code that assumes subscription only fires on later changes |
| Discarding RAII signal handle | A returned `Signals::Handle` may disconnect when destroyed | Store in `TreeState`/`ctx.declare<Signals::Handle>()` or knowingly `.release()` |
| `ctx.elem` and names | Builder calls parent's `override`; named duplicate can replace an existing child | Use intentional, stable names; do not assume append-only behavior |
| `TreeIter<T>` is weak | Tree swapping/removal can destroy target | Check handle validity and avoid unsafe deferred dereference |
| `getref<Property>` bypasses normal access discipline | Can mutate underlying property without proper invalidation/thread sync | Prefer `set/get/apply_set/apply_get` |
| UI worker vs async callbacks | HTTP/WS/scheduler callback can execute off main thread | Copy payload and schedule through `IOContext::sync_async` |
| `IOContext::get()` is thread-local | Worker may see no context | Capture `shared_ptr`/`weak_ptr` or use `launch_thread` |
| Lazy and optional modules | Interface exists but matching backend may not | Check `sys_if<Interface>()` and status |
| `MetaParentElement` is not `Box` | Virtualized/managed container can reject ordinary insertion | Use its public seed/fetch API |
| HTTP streaming final body | With `on_chunk`, final `Response::body()` can be empty | Collect chunks explicitly |
| `sync_async` is not necessarily deferred | When `is_synced()` it runs the callback immediately | Design callbacks for possible synchronous reentrancy |
| `Switch` change handler precedence | `_submit()` calls `OnChange` **or else** `OnChangeValues`, not both | Register the intended callback, or have one handler invoke shared logic |
| Dropdown callback timing | `choose(idx)` invokes `OnSubmit` **before** storing `_selected = idx` | Use callback arguments for the new selection, not `index()` during callback |
| `MultiInput` callback/ownership | Does not derive from `Input`, yet uses `IResponsive<Input,string>` | Verify callbacks from current implementation before choosing callback parameter type |
| `Units::Pc` treats a value of N as N * 100% rather than N% | x | x |
