# 11. Coding style and formatting guide inferred from source

This guide is **descriptive**: it reflects patterns in the supplied source snapshot rather than claiming the repository has a written, enforced style policy. It should guide new code and especially LLM-generated changes. Mirror the immediate neighborhood when it differs. For **application screen composition**, also read [13_APPLICATION_UI_STYLE.md](13_APPLICATION_UI_STYLE.md), which covers how actual UIs should be structured instead of only C++ formatting.

## Language and layout

| Observation | Recommended agent behavior |
| --- | --- |
| C++20, templates, concepts, lambdas, `std::span`, `std::chrono`, `std::optional`, `std::shared_ptr` | Stick to C++20; prefer typed interfaces and constrained templates to runtime string maps |
| Most C++ uses **4-space indentation** | Preserve four spaces; avoid tabs in new code |
| Classes, functions and `if` bodies generally put `{` on the **next line** | Follow Allman-like braces rather than K&R in new multiline blocks |
| Nested `namespace d2` / `namespace d2::dx` and trailing `// namespace ...` comments appear | Scope exported types narrowly; close long namespaces with explanatory comments |
| Headers primarily use `#pragma once` | Follow this for new headers |
| `d2_` prefix on core/element/platform files | Name new library files consistently with their directory |
| Significant template and UAI macro expressions sometimes use `// clang-format off` | Allow surgical exclusions rather than defeating macro alignment with automatic formatting |
| CMake options use `D2_` uppercase prefix | Put new options in the existing named groups with explicit defaults and comments |

## Naming conventions

- **Classes and type aliases:** `PascalCase` for public types, e.g. `IOContext`, `TreeState`, `SystemScreen`, `DependencyHandle`, `InputFrame`, `MatrixModel`; some deliberate lowercase aliases for module names (`sys::input`, `sys::net::http`).
- **Member functions, free functions, local variables:** mostly `snake_case`, e.g. `scroll_to`, `on_response`, `screen_capacity`, `max_response_bytes`, `fetch_callback`.
- **Private/protected implementation:** frequently `_leading_underscore` with `_impl` suffix for virtual hooks (`_frame_impl`, `_event_impl`, `_load_impl`); some fields use trailing underscores (`ptr_`), so **follow adjacent class conventions** rather than mechanically renaming.
- **Compile-time UAI properties and enum values:** `PascalCase`/camel forms such as `ForegroundColor`, `OnSubmit`, `TreeSwap`, `TSafe`. These are API identifiers, not ordinary instance variables.
- **Macros:** uppercase `D2_...` for infrastructure and lowercase `dstyle`, `dapply` for the intentionally ergonomic tree DSL.
- **Test and example files:** short lowercase names (`editing.cpp`, `styles.cpp`, `module.cpp`).

## Dependency and ownership style

- Public interfaces are typically lean and defined in headers; concrete OS logic lives in `os/`.
- The framework uses `std::shared_ptr`/`weak_ptr` deliberately. Prefer weak captures in global input, network, scheduler and tree-swap callbacks to avoid leaks or invalid access. 
- Classes meant to be used with shared/weak will usually expose ClassName::ptr or ClassName::wptr. Use these instead of std::shared_ptr/weak_ptr<ClassName>.
- RAII for event signal connections; a stored `Signals::Handle` is a common idiom (`examples/editing.cpp`, `examples/images.cpp`).
- Use `TreeIter<T>` for temporary element access and convert to strong pointer only with an explicit ownership reason.
- Prefer value semantics for callback payloads and copy `string_view` when asynchronous lifetime requires it.
- Avoid extra mutable global state for applications unless integrating with explicitly process-global subsystems.

## Patterns to copy

### Public widget

1. Header in `elements/d2_widget.hpp`.
2. A `style::IWidget` interface declared through `D2_UAI_INTERFACE(...)`, with backing fields, property ID list, and `D2_UAI_PROP` mapping.
3. A `dx::Widget` inheriting `style::UAI<Widget, style::ILayout, ...>` (or a `UAIC`/`UAIE` specialization for chained/meta widgets).
4. `using data = ...; using data::data;` where needed for constructor forwarding.
5. Protected `_layout_impl`, `_frame_impl`, `_event_impl`, `_state_change_impl`, `_signal_write_impl` overrides, as appropriate.
6. Minimal public imperative methods for operations that do not fit a typed property (`scroll_to`, `reset`, `value`, `close`).
7. Unit tests and a focused example exercising both initial render and updates.

### Concrete runtime module

1. Abstract service contract in `core/mods/d2_service.hpp` (if needed).
2. Concrete `os/<service>/<backend>/...` implementation inheriting `ModuleImpl`.
3. Module compatibility and explicit dependencies declared correctly.
4. CMake `delta_register_module` integration and optional dependency checks.
5. Lifecycle, unavailability and threading tests.

### Application tree

```cpp
using App = d2::Tree<d2::TreeState, d2::dx::Box,
    [](d2::TreeCtx<d2::dx::Box> ctx)
    {
        ctx.elem("title", [](d2::TreeCtx<d2::dx::Text> ctx)
        {
            dstyle(Value, "Title");
            dstyle(X, 0.0_center);
        });
    }>;
```

The exact spelling `ctx` matters for DSL macros. Use explicit fully-qualified names outside small `using namespace d2::dx;` scopes.

## Formatting principles for an LLM

1. **Make small patches.** Do not reflow enormous macro chains or adjacent public APIs without need.
2. **Keep ABI/API changes explicit.** New UAI property names/callback signatures are user-facing; avoid casually renaming them.
3. **Preserve compiler checks.** Use typed `D2_UAI_PROP` and `TreeCtx` constraints; don't bypass with reinterpretation or hidden global maps.
4. **Write defensive platform code.** Optional modules should degrade cleanly; avoid unconditional `sys<T>()` in optional UI paths.
5. **Use full enum types when clarity matters.** Especially `SystemModule::Status`, `in::mode::Press`, input modifier tokens, `style::` flags.
6. **Keep logging separate from output rendering.** Use project logging infrastructure instead of print debugging into the live terminal.
7. **Comment tricky invariants**, especially tree swap ownership, signal handle lifetimes, dependency eager-callback behavior, and why a module is `TSafe` or `TUnsafe`.
8. **Run formatter selectively.** No root `.clang-format` contract is assumed here; some files explicitly toggle `clang-format` off around macros. Default to source-local layout rather than mass reformatting.
9. **Use project tests for validation.** Add regressions under `tests/`, not ad hoc extra executable paths baked into the library.
10. **Don't emulate removed subsystems.** Treat the retained package as a TUI library and any detached inference package as a separate dependency.

## What the codebase does not consistently enforce

It is not perfectly uniform: there are both `_prefix` and `suffix_` member fields, compact and expanded macro formatting, explicit and aliased namespace names, and occasional unusual comments/typos. These are reasons to avoid a blanket style rewrite. The guide describes dominant patterns, not strict lint rules.
