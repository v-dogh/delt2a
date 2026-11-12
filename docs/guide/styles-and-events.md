# Styles, state and events

[Documentation index](../index.md) · [Property reference](../reference/styles.md)

## Typed properties

Each element composes style interfaces through `style::UAI` or `style::UAIE`. The interface chain assigns property IDs and maps each property to storage, type and invalidation behavior. Properties inherited from base widgets remain accessible through the derived widget's symbolic names.

```cpp
label->set<d2::dx::Text::Value>("Updated");
label->set<d2::dx::Text::ForegroundColor>(d2::px::foreground(220, 230, 240));
auto value = label->get<d2::dx::Text::Value>();
```

`dstyle(Property, value)` is shorthand inside a construction lambda whose context variable is named `ctx`. It performs the same typed property assignment. The property table lists backing fields for reading implementations; applications should use setters. The internal bypass template parameter on setters is for code already satisfying thread/invalidation requirements.

Colors have separate foreground/background types and alpha. Character cells compose differently from image pixels; see [rendering](rendering.md). Text style flags include terminal attributes such as bold/italic; availability ultimately depends on the terminal.

## Dependencies and themes

`style::Dependency<T>` holds an observable value. Set a property to a dependency to bind updates to it; setting an ordinary value replaces the binding. A dependency subscription is associated with a weak target. Hold its `DependencyHandle` for the subscription lifetime; `close()` disconnects, while `release()` relinquishes automatic cleanup.

Themes collect typed dependencies shared by a screen. `ctx.var<&Theme::member>()` obtains one, `dstylev` binds it directly, and `dstyledv` applies a compile-time transformation. `screen()->make_theme(...)` and the arguments to `IOContext::run()` install theme instances. `DynamicDependencyManager` supplies named dependencies for runtime-defined state. Configure and mutate UI-facing dependencies on the UI thread; framework synchronization does not make arbitrary referenced objects concurrently mutable.

`Stylesheet<Callback, Args...>` is reusable application code that applies properties or builds related children. The bundled defaults live in `extras/d2_default_styles.hpp` and `extras/d2_default_theme.hpp`; applications can use their own themes without those extras.

## Three subscription lifetimes

| Subscription | Handle | How it ends |
| --- | --- | --- |
| Element state (`Clicked`, `Hovered`, etc.) | `Element::EventListener` | Persistent until `destroy()` or element destruction; dropping the handle does not disconnect |
| Context/module signal | `Signals::Handle` | RAII: destruction/`close()` disconnects; `release()` leaves the subscription active |
| Reactive dependency | `style::DependencyHandle` | RAII: destruction/`close()` disconnects; `release()` leaves it active |

Store handles in `TreeState` or a controller. Explicit disconnect, mute and unmute operations are safe to repeat. Removing a listener during dispatch is supported. A callback should not retain its owning element/controller strongly unless the resulting cycle is deliberately broken.

```cpp
auto listener = label->listen(d2::Element::Hovered, true,
    [](d2::Element::EventListener, d2::TreeIter<> element)
    { /* update application state */ });
// Later, while the owner still exists:
listener.destroy();
```

Widget callback properties are simpler for widget-specific actions: for example `Button::OnSubmit`, slider changes and input submit hooks. Their signatures appear in the property reference and widget headers. These callbacks replace a stored function; they are not subscriptions with independent listener IDs.

## Input signals and shortcuts

The screen publishes `KeyInput`, `MouseInput`, `KeySequenceInput`, `Update` and tree-swap events. `InputFrame` distinguishes key press/release/hold, mouse movement and text sequences. Treat frame references as borrowed for the callback duration. Insert text from sequence input rather than reconstructing UTF-8 from individual key codes.

`SystemBinder`/`sys::binder` manages named key sequences, timing, modifier filters and element dependencies. `Bind::FocusPolicy` determines whether focused editors suppress a shortcut; `ConsumePolicy` controls consumption on a match. `bind()`/`rebind()` return an optional conflict description; inspect it. Use `unbind`, `freeze` and `unfreeze` for explicit lifecycle changes.

## Animation

The screen owns animations targeted at an element property. `anim::Linear<T, Property>` interpolates supported values; sequential and keyframe-style helpers are declared in `core/screen/d2_animate.hpp`. Screen helpers register the animation and keep it alive. `stop`, `mute`, `unmute` and `clear_animations` manage its lifecycle.

Tree DSL `animate_twoway` and `animate_toggle` helpers return element listener handles because they start animations on later events. Keep a handle when you need to disconnect the trigger. Their listeners follow element-listener persistence rules, not signal RAII rules.
