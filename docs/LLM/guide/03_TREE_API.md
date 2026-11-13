# 03. Tree construction, elements and context API

**Main source:** `core/tree/d2_tree_construct.hpp`, `d2_tree.hpp`, `d2_tree_element.hpp`, `d2_tree_element_frwd.hpp`, `d2_tree_parent.hpp`, `d2_tree_state.hpp`.

## Static builders

| Declaration | Meaning |
| --- | --- |
| `d2::Tree<State, Root, Build, Argv...>` | Tree with typed `TreeCtx<Root,State>` callback and optional build arguments |
| `d2::TreeForward<Base, State, Root, Argv...>` | Delegates creation to a type's `Base::construct(...)` |
| `d2::NamedTree<"Name", Tree>` | Named tree registered with `IOContext::run` |
| `d2::TreeCtx<Element, State = TreeState>` | Typed builder/runtime helper tied to a concrete element |
| `d2::Stylesheet<Callback, ArgTypes...>` | Compile-time reusable style/builder operation |

Builder `Build` signature must be invocable as `Build(TreeCtx<Root,State>, Argv...)`. Most projects pass a captureless non-type-template-argument lambda.

## `TreeCtx` lookup and runtime access

```cpp
ctx.ptr();              // TreeIter<Element>
ctx.state();            // tree_state_ptr<State>
ctx.context();          // shared IOContext
ctx.scheduler();        // scheduler shared_ptr
ctx.screen();           // SystemScreen module handle
ctx.input();            // SystemInput module handle
ctx.output();           // SystemOutput module handle
ctx->set<Text::Value>("new text");  // when ctx is TreeCtx<Text>
```

`TreeCtx::operator->()` exposes a `TreeIter<Element>`; the conventional `dstyle` macro calls through `ctx.ptr()`. The compiler checks properties against the concrete element type.

## Creation and composition

```cpp
ctx.elem([](d2::TreeCtx<d2::dx::Text> ctx)
{
    dstyle(Value, "Unnamed child");
});

ctx.elem("status", [](d2::TreeCtx<d2::dx::Text> ctx)
{
    dstyle(Value, "Named child");
});
```

- `ctx.elem(callback, ctor_args...)`: deduces child widget from callback's `TreeCtx<Widget>` argument, creates and inserts it, invokes builder immediately, returns `TreeIter<Widget>`.
- `ctx.elem(name, callback, ctor_args...)`: same with a name used in parent lookup and replacement.
- `ctx.elem(existing_shared_element, callback)`: attaches an existing shared element and calls builder.
- `ctx.anchor<Type>(name?, args...)`: constructs a type with current tree state but does not perform normal `elem` attachment.
- `ctx.embed(...)`: embeds a separately constructed tree/subtree. It has overloads accepting a name, callback, argument tuples and tree type.
- `ctx.clone(iter, callback)`: calls callback with a typed `TreeCtx` wrapping the target iterator; it is **not** a deep copy.
- `ctx.branch(dep, std::pair{value, callback}...)`: rebuilds owned branch children in response to a dependency's value. The implementation tracks and removes prior branch-created children; useful for conditional UI.
- `ctx.persist(bool preserve_position=true, std::vector<std::string> whitelist={})`: transfers an eligible root child across swaps; throws below the root layer.
- `ctx.apply<Stylesheet>(args...)` / `dapply(Stylesheet, args...)`: apply reuseable builders.

Use `dstyle` inside the named `ctx` lambda; in a generic free function, use explicit `iter->set<T::Property>(...)` or a typed `TreeCtx`.

## `TreeIter<T>`

The element iterator is a **weak reference abstraction**. Source methods include:

- `shared()`, `weak()` for ownership access; `as<U>()`, `asp()` to adapt type/parent access; `up()` to navigate to parent.
- `foreach(...)` to walk children when applicable, and `operator/` / parent lookup for named descendants.
- `operator->` for element APIs, with validity governed by element lifetime.

Avoid persisting a `TreeIter` unguarded in timer, HTTP or WS callbacks. Capture a weak handle, test it for validity before dereference, and ensure its tree is still relevant.

## `Element` public surface

| Group | API / semantics |
| --- | --- |
| Construction/identity | `Element::make<T>(...)`, `name()`, `parent()`, `state()`, `context()`, `screen()` |
| Property access | `set<Property>(value)`, `get<Property>()`, `getref<Property>()`, `apply_get<Property>(fn)`, `apply_set<Property>(fn)` |
| State | `getstate(State)`, `setstate(State,bool)`, `listen(State, callback)`, `listen(State,bool,callback)` |
| Coordinates | `layout(Layout)`, `internal_layout`, `box()`, `position()`, `internal_box`, `internal_position`, `position_screen_space`, `resolve_units` |
| Painting/cache | `frame()`, `setcache(CachePolicy)`; policies `Static`, `Dynamic`, `Volatile` |
| Advanced geometry | `override_layout`, `override_position`, `override_dimensions` |

`get<Property>()` is the ordinary typed read. `getref` exposes data that can bypass synchronization or signal/invalidation logic, so it is inappropriate for most UI updates. `apply_set`/`apply_get` are designed for controlled custom mutations.

### Element state identifiers

`Display`, `Swapped`, `Created`, `Embedded`, `Event`, `RcEvent`, `Clicked`, `Hovered`, `Focused`, `RcClick`, `RcHover`, `RcFocus`, `Keynavi`, `Public1`, `Public2`, `Public3` are declared states. State listener signature is based on an `Element::EventListener` and `TreeIter<>`; a value filter is optional.

```cpp
ctx.onv(d2::Element::State::Clicked,
    [](d2::Element::EventListener, d2::TreeIter<> element)
    {
        if (element && element->getstate(d2::Element::State::Clicked))
        {
            // clicked state became active
        }
    });
```

The listener is stored by the element, unlike an arbitrary globally registered signal listener; check detailed source when manually destroying/muting listeners.

## Parent containers

`ParentElement` supplies typed `create`, `override`, `create_after`, `override_after`, `at`, `exists`, `index_of`, `remove`, `remove_if`, `extract`, `clear`, `foreach`. `Box` and flow widgets expose ordinary parent containers. **Not all elements are ordinary parents:** `MetaParentElement` derivatives such as virtualized widgets can reject generic child insertion at runtime. A `DemandScrollBox` instead creates its entries through `FetchCallback`.

Element and parent handles are not STL vector iterators. Read `core/tree/d2_tree_parent.hpp` before relying on index stability or modification during traversal.

## More advanced construction helpers

| `TreeCtx` member | Purpose |
| --- | --- |
| `tag<"SwapClean">`, etc. | Standard tree/screen behavior tags |
| `dep<T>(name)` / `dep<T>(tree,name)` | Named reactive dependency in tree scope |
| `var<EnumValue>()` / `dynavar<EnumValue, Callback>()` | Theme dependency or transformed dynamic value |
| `subscribe(dep, callback)` | Subscribe to dependency changes (immediate initial callback) |
| `declare<T>(ctor args...)` | Attach allocated auxiliary object's lifetime to the element |
| `on(screen_event, fn)` | Connect to context signal; returns RAII signal handle |
| `onv(element_state, fn)` | Add element state listener |
| `async(fn)`, `repeat(period,fn)`, `defer(delay,fn)` | Scheduler launch helpers |
| `sync(fn)`, `sync_async(fn)` | Main-thread scheduling via context |
| `animate<Interpolator,Property>(...)` | Animate a typed property |
| `animate_twoway`, `animate_toggle` | Click-driven animation helpers |

Animations live in `core/screen/d2_animate.hpp`; typical interpolators are in `d2::anim` (e.g. `anim::Linear`). Do not infer a particular property is interpolatable merely because it exists: inspect the interpolator requirements.
