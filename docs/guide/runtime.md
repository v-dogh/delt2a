# Runtime and modules

[Documentation index](../index.md)

## Ownership

`IOContext` owns the module registry, scheduler, signal hub and main worker. `SystemScreen` owns registered trees, themes and rendering state. Parents hold their children; elements share a `TreeState` that refers back to the context weakly. Use `TreeIter<T>` for tree navigation and typed access; use `weak()` or `std::weak_ptr` in long-lived callbacks to avoid ownership cycles.

`IOContext::make<Modules...>()` stores a module manifest. `run<Trees...>()` performs initialization; `make()` alone does not start a UI. Input is initialized early to supply the main worker. The scheduler starts with the main worker and a background worker, remaining modules load, screen trees are registered, and the input/render/wait loop runs until `stop()`.

UI objects, renderer state and terminal writes belong to the main thread. `IOContext::get()` is thread-local, not a process-wide service locator. Framework worker startup sets its context; independently created threads should use `launch_thread()` or retain an explicit context. A coroutine can resume on a different thread, so prefer explicit handles across suspension.

## Abstract interfaces and concrete implementations

An abstract service derives from `ModuleDecl<Base, Name, Access, Dependencies...>`. A concrete backend adds `ModuleImpl<Concrete, LoadPolicy, Dependencies...>`. A platform-independent concrete service can use `Module<...>` directly.

```cpp
ctx->load<d2::sys::net::SystemHTTP, d2::sys::SystemOpenAI>();
auto provider = ctx->sys<d2::sys::SystemProvider>();
```

Loading an abstract type uses the generated platform registry to choose an implementation. Loading a concrete type selects it explicitly. Modules are indexed by their **abstract base**, so query `sys<SystemProvider>()`, not `sys<SystemOpenAI>()`. For images select `SystemImagesKitty` or `SystemImagesSixel` explicitly; loading `sys::images` is not terminal capability detection.

Dependencies define ordering and removal constraints; they do not automatically insert missing modules into the manifest. Include required modules in `make<...>()` or `load<...>()`. For example a renderer needs output, and the OpenAI provider needs HTTP. Image decoders are independent of output and the renderer.

`sys<T>()` demands an instance and can initialize a lazy module. `sys_if<T>()` returns an empty handle when unavailable/not ready. `sys::module<T>` is an owning pointer wrapper with `operator->` and `ptr()`; retaining it keeps the object alive after registry removal, but does not prevent unload or make an inactive service usable. `Access::TUnsafe` enforces context-thread access through module handles; `Access::TSafe` permits concurrent public operations but does not make application callbacks thread-safe.

## Named instances

```cpp
ctx->load<d2::sys::SystemPngLoader>("assets");
auto decoder = ctx->sys<d2::sys::SystemImageLoader>("assets");
```

Named and unnamed instances are separate. Load and query with the same name; pass the abstract key to `unload<T>(name)`. Names are useful for provider accounts, independent HTTP services or alternate loaders. Declared dependencies resolve against the unnamed service. Named providers can still be passed explicitly to the conversation module's binding API.

## Lifecycle

`load()`/`unload()` transitions are serialized and idempotent on a module instance. Context teardown unloads dependents before dependencies. Explicitly removing an unnamed dependency while dependents remain is rejected. Registry locks are released before cleanup callbacks run.

Stop accepting application work before teardown and settle/cancel operations whose callbacks refer to application state. A retained module pointer is not a shutdown barrier. HTTP/provider and image-load lifetime rules are specified in their guides. Do not stop transport workers from their own callbacks; marshal lifecycle changes to an owning/control thread.

`context->sync_async(callback)` executes immediately on the UI thread or enqueues the callback on the main worker. Its future contains the result or exception. `sync(callback)` blocks an outside caller until completion; avoid it if the UI thread is waiting for that caller. Do not call `.value()` on a pending UI-dependent future from the UI thread.

## Screen and tree selection

`screen()->set<NamedTree<...>, ...>()` registers trees; `set(name)` selects one. `name()`, `state<T>()`, `root()`, `deps()` and `tags()` expose active-tree state. `TreeState::construct`, `swap_in`, `swap_out` and `update` are application hooks. Keep state updates small; offload blocking work to the scheduler.

The screen handles focus, hovered/clicked elements, input dispatch, animations, layout and frame submission. `focus(element)`, `focused()`, `focus_next_minor()` and `focus_next_major()` control navigation. `set_refresh_rate()` can request regular ticks; otherwise dirty work and input drive updates.
