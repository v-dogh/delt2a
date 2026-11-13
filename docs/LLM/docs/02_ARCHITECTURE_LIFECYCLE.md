# 02. Architecture, ownership and lifecycle

## Mental model

```text
App main()
  └─ IOContext::make<runtime module interfaces...>()
       ├─ runtime module registry / backend selection
       ├─ scheduler, main worker and synchronization
       └─ run<NamedTree<"name", Tree>...>()
            └─ SystemScreen
                 ├─ active TreeState + element root
                 │    └─ retained children built by TreeCtx::elem/embed
                 ├─ dependencies (named reactive values)
                 ├─ theme, animation, screen cache, focus
                 ├─ input frame and signals
                 └─ terminal output / optional image scene
```

The builder lambda creates a retained element hierarchy. Each tick handles input, updates state and rendering, and emits a diff/cache-aware terminal frame. You **do not** rebuild the full interface each frame as with an immediate-mode UI.

## Root application and multiple screens

```cpp
using Settings = d2::Tree<d2::TreeState, d2::dx::Box,
    [](d2::TreeCtx<d2::dx::Box> ctx) { /* build once */ }>;

// Registration happens at run time:
// context->run<d2::NamedTree<"Home", Home>,
//              d2::NamedTree<"Settings", Settings>>();
// Switch later via screen()->set("Settings").
```

`SystemScreen::set(std::string_view)` switches a named tree. This is a lifecycle transition, not ordinary property mutation. Root/tree state swap hooks may run; ephemeral element handles and event subscriptions should not be assumed to survive without deliberate design.

### `TreeState` hooks and tags

Custom state objects derive from `d2::TreeState`. The source defines hooks named `construct`, `swap_in`, `swap_out`, `update`. Check `core/tree/d2_tree_state.hpp` for the precise override signatures before writing overrides.

`TreeCtx::tag<"SwapClean">(bool)`, `tag<"SwapOut">(bool)`, and `tag<"OverrideRefresh">(std::chrono::milliseconds)` set known screen/tree configuration tags. Defaults found in source: `SwapClean=false`, `SwapOut=true`, `OverrideRefresh=0ms`. `tag` is statically restricted to these names and types. Do not use it as an arbitrary untyped configuration store.

`TreeCtx::persist(...)` is an explicit mechanism for preserving eligible top-level children across tree switches; do not assume arbitrary nested subtree persistence.

## Element lifetime and ownership

Elements have shared backing ownership inside the retained tree. `d2::TreeIter<T>` is a handle that holds a **weak** reference; `TreeIter::shared()` obtains a strong reference where needed. Holding strong element references in long-lived callbacks can create retention cycles or delay destruction. Check weak handles before use in deferred callbacks and when switching screens.

`ctx.elem(...)` creates an element and inserts it via the parent's `override` route. This matters for **named elements**: an element with an existing name is replaceable. `ParentElement::create`, `override`, `remove`, `extract`, `foreach`, etc. are the lower-level container APIs; do not confuse them with the higher-level tree DSL.

`ctx.embed(...)` attaches a separate constructed tree/subtree. `ctx.anchor<Type>(...)` constructs an element without automatically using the usual `elem` insertion path. `ctx.branch` handles conditional variants of constructed children. Consult the tree API before changing ownership.

## Layout and rendering

`Element` exposes `layout`, `internal_layout`, `box`, `position`, `position_screen_space`, `resolve_units`, `frame`, and cache policy (`Static`, `Dynamic`, `Volatile`). Standard widgets implement `_layout_impl`, `_frame_impl`, `_event_impl`, `_state_change_impl`, `_signal_write_impl` and related hooks where necessary. Ordinary users should set UAI properties rather than manually invoke internal lifecycle methods.

Coordinates are terminal-cell centric, with units that may depend on viewport, parent or computed natural dimensions. `FlowBox` and scroll/virtual variants can affect how child positions resolve. Raw frame-buffer edits are an extension API, not the preferred method for simple styled widgets.

## Screen and context APIs

- `IOContext`: `make`, `run`, `stop`, `ping`, `deadline`, `sync`, `sync_async`, `is_synced`, `scheduler`, `launch_thread`, `load`, `unload`, `sys`, `sys_if`, `input`, `output`, `screen`.
- `SystemScreen`: `root`, `name`, `set`, `state`, `erase_tree`, `clear_tree`, `tags`, `deps`, `theme`, `make_theme`, `focus`, `focused`, `clicked`, `hovered`, `focus_next_minor`, `focus_next_major`, `apply`, `apply_all`, `fps`, `delta`, `tick`, `animate`, `clear_animations`, `set_refresh_rate`.
- Context lookup `IOContext::get/set` uses **thread-local storage**. In workers, pass/capture the context explicitly or use the library's context-aware thread launch path.

### Read the source for

`core/io/d2_context.{hpp,cpp}`, `core/screen/d2_screen.{hpp,cpp}`, `core/tree/d2_tree.hpp`, `core/tree/d2_tree_construct.hpp`, `core/tree/d2_tree_state.hpp`, `core/tree/d2_tree_element.hpp`, `core/tree/d2_tree_parent.hpp`.
