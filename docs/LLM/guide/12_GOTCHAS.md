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

## Other snapshot-specific observations (recheck against current source)

These are separate from the maintainer-reported fixes above and have **not** been verified against newer commits.

### Named theme lookup

In the supplied archive, `SystemScreen::themev<Var>(id)` in `core/screen/d2_screen.hpp` calls `theme<...>()` without forwarding `id`. When depending on multiple named theme instances, verify the current overload behavior and add a focused test.

### Built-in Doxygen route

In the archive, the `D2_BUILD_DOCS` branch in `CMakeLists.txt` refers to `tools/doxygen_filter.py` and `docs/index.md`, absent at those paths. Check the current repository before assuming this build option works. This handbook does not depend on that route.

### Multiline scrollbar spelling

In the archive, `MultiInput` declares both `vertical_scrollbar()` and `vertical_scollbar()`. Prefer the correctly spelled method; keep the misspelled alias only if backwards compatibility matters.

### Image example against current image API

The archived `examples/images.cpp` uses `SystemImages::Config`, while the archived `core/mods/d2_images.hpp` exposes `configure(BoundingBox cell_pixels)`. Prefer the actual module header when maintaining example code; the current repo may already differ.

## Behavioral uncertainty to test instead of guessing

- Timing of slider `OnSubmit` and its input-origin details; its float argument order is source-verified as `(relative, absolute)` in `elements/d2_slider.cpp`.
- `SignalRegistry` event payloads and flags beyond the screen events; APIs are typed by event manifest, so mismatched lambda signatures may not compile.
- Terminal image protocol support and terminal-cell pixel metrics on different terminal emulators.
- Cross-thread behavior of a specific widget's imperative method (as opposed to UAI `set`); not all non-property calls synchronize.
- How a `TreeState` object and dependency store are retained/reinitialized under specific swap tags; verify with `tests` and implementation before promising persistence semantics.
- Unsupported image formats even if a generic `image_loader` interface is declared.

## Regression tests worth preserving

- An eager dependency subscription fires once initially, then on change, and unsubscribes properly after element destruction.
- `DemandScrollBox::EntrySeed` forbids multiple `create` calls per seed and keeps instantiated rows bounded while scrolling.
- Network callbacks exit safely after the destination UI element is destroyed.

## Documentation confidence

Behavioral notes are based on the supplied source snapshot. For newly fixed code, consult the current repository and its tests: this archive does not include those post-audit changes.
