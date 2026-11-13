# Instructions for an LLM coding with delt2a

Use this as a high-priority, repo-specific context file when editing applications that depend on delt2a. The companion detailed references are in [`docs/`](docs/).

## Identity and entry points

- C++20 **retained-mode TUI**, **not** an immediate-mode API, web DOM, or Qt.
- Standard headers: `#include <d2_std.hpp>` and `#include <elements/d2_std.hpp>`; include individual widget headers when useful.
- Main namespace `d2`; standard widgets `d2::dx`; graphs `d2::dx::graph`; optional extras `d2::ex`; module interfaces `d2::sys` / `d2::sys::net`.
- CMake link target: `DeltaAscii::Core` (`DeltaAscii` is its backing interface target).
- App entry: `d2::IOContext::make<d2::sys::input,d2::sys::output,d2::sys::screen>()->run<d2::NamedTree<"App", App>>();`.
- Default target platforms observed in this source are Unix-like terminals. Do not invent a Windows backend.

## Always follow these API rules

1. Define a tree via `d2::Tree<State, RootElement, BuildLambda>`; its lambda takes `d2::TreeCtx<RootElement>`.
2. Declare children with `ctx.elem([](d2::TreeCtx<d2::dx::Text> ctx) { ... });`. The callback argument type determines the element type; it is **not** `ctx.add<Text>()`.
3. `dstyle(Property, expression)` is a **macro using the exact local name `ctx`**. It expands to `ctx.ptr()->set<decltype(ctx)::element_type::Property>(...)`. Do not rename the callback parameter while using it. `dapply(StyleSheet, args...)` is similar.
4. Reactive state: `auto& count = ctx.dep<int>("count"); count = 0;`, then `ctx.subscribe(count, callback)` or `dstyle(Value, dependency)` for compatible properties. A subscription invokes its callback immediately at registration; account for this.
5. Subscription handles are RAII. A short-lived handle normally unsubscribes when destroyed. Use `.release()` when intentionally detaching a dependency subscription tied to the element, or retain an appropriate handle. Do not routinely call `.release()` on unrelated handles without understanding ownership.
6. `d2::TreeIter<T>` is a **weak/non-owning element handle**. Check validity before dereferencing an asynchronously retained iterator; do not assume lifetime across tree swaps.
7. Set a typed property with `element->set<T::Property>(value)` and read with `get<T::Property>()`. The style/UAI type of the element controls property availability. Don't invent string-based `.style("color", ...)` APIs.
8. Use unit literals such as `0.0_center`, `1.0_relative`, `1.0_auto`, `20.0_px`, `100.0_pc`. The floating literal forms matter. Width and height resolve relative to layout context; inspect units before asserting geometry.
9. Event callback signatures are **typed**, e.g. `Button::OnSubmit` accepts `d2::TreeIter<d2::dx::Button>`, whereas a checkbox passes an additional `bool`. Consult the widget reference before emitting callbacks.
10. UI context/module access is thread-aware. Prefer `context->sync(...)` or `context->sync_async(...)` for UI-thread operations originating in workers. Do not wait on queued main-thread work from the main thread.
11. Module availability is conditional. `ctx.context()->sys_if<Module>()` can be null; `sys<Module>()` requires it to be available. Optional libcurl/libpng/PipeWire and terminal-image modules must not be assumed present.
12. Input comes from `d2::in::InputFrame` and screen signals. `ctx.on(...)` returns a RAII signal handle; save or explicitly release it if the callback must persist.
13. An element's `getref` and raw layout/frame buffer references are lower-level and can bypass safe invalidation. Prefer `set/get/apply_set/apply_get`.
14. Do not reconstruct styles from intuitive widget names. Check `D2_UAI_*` declarations in `core/tree/d2_styles_base.hpp`, `core/tree/d2_styles.hpp` and `elements/d2_*.hpp`.
15. If a method seems missing, look it up in [`docs/SOURCE_MAP.md`](docs/SOURCE_MAP.md) then verify the relevant header and implementation. Never fabricate API surface.

For screen-writing conventions (not just library C++ formatting), follow [`docs/13_APPLICATION_UI_STYLE.md`](docs/13_APPLICATION_UI_STYLE.md).

## Good implementation sequence

1. Identify the widget(s) and required modules.
2. Check their parent class and UAI interface for supported properties/callback types.
3. Define the tree structure and named elements; put UI configuration inside `ctx.elem` lambdas.
4. Define state dependencies and decide whether each view should use a direct dependency binding or explicit subscription.
5. Introduce asynchronous work only after understanding UI lifetime and synchronization.
6. Compile under the actual CMake options; tests and optional backends are not necessarily enabled.
7. For framework changes, use the project's native style and keep abstract `core/mods` interfaces separate from `os` implementations.

## Guardrails and common misreadings

- `ctx.elem` uses the parent's `override` insertion path; **named children can be replaced**, so do not assume names are only descriptive.
- Tree changes (`screen()->set("Name")`) involve swap hooks and can invalidate element handles.
- `Dependency::subscribe` can invoke user code synchronously during setup.
- The active context is thread-local, not magically ambient in arbitrary threads.
- Do not draw terminal output with `std::cout` while the UI owns the console. Use the `rs` logging system where appropriate.
- Do not use removed inference-code examples as evidence of the public UI library.
- See [`docs/12_GOTCHAS.md`](docs/12_GOTCHAS.md) for lifecycle, event, and threading traps before relying on edge-case behavior.

## Conventions when contributing

Use C++20, 4-space indentation, opening braces on a new line, `snake_case` for member functions/local variables, `_leading_underscore` for private fields, namespaced symbols, `#pragma once` for headers, `d2_` filename prefix for library headers, and explicit `const`/`noexcept` when the current adjacent API uses them. Mirror existing `D2_UAI_INTERFACE`/`D2_UAI_PROP` patterns; do not hand-roll an incompatible style storage layer. Detailed exceptions and formatting observations: [`docs/11_STYLE_GUIDE.md`](docs/11_STYLE_GUIDE.md).
