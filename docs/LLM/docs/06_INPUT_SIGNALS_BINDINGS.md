# 06. Input, event signals, selection and key binding

**Source:** `core/io/d2_input_base.hpp`, `core/io/d2_signal_handler.hpp`, `core/screen/d2_screen.hpp`, `core/mods/d2_binder.hpp`, `core/mods/d2_ext.hpp`, `core/tree/d2_text_select.hpp`, `tests/events.cpp`, `examples/selection.cpp`.

## Input frame

`d2::in::InputFrame` is a snapshot of keyboard/mouse state and input pulses for an update cycle. Input module `d2::sys::input` is a `SystemInput`; `input()->frame()` returns a lock-holding `FramePtr` that exposes `InputFrame` via `operator->`.

Representative operations:

```cpp
// Inspect a frame supplied by a signal or obtained through input()->frame().
if (frame.active(d2::in::key('q'), d2::in::mode::Press))
{
    // Handle q press.
}

// Other classes of input:
// frame.active(d2::in::special::Shift, ...)
// frame.active(d2::in::mouse::Left, ...)
// frame.mouse_position()
// frame.scroll_delta()
// frame.sequence()
```

Frame operations include `active`, `map`, `had_event`, `had_pulse`, `scroll_delta`, `mouse_position`, `screen_size`, `screen_capacity`, `active_list`, `sequence`, and `mask`. Check each function's exact consume/mask semantics: checking an input pulse can affect downstream consumers. The input module also forwards common queries such as `active`, `mouse_position` and `sequence`.

`InputFrame` supplies different categories of keys, modifiers and mouse buttons; don't treat `key('a')`, `special::Shift`, and `mouse::Left` as interchangeable tokens.

## Screen/context event signals

`SystemScreen::Event` defines:

| Event | Event payload documented in source |
| --- | --- |
| `Resize` | `BoundingBox` |
| `PreRedraw`, `PostRedraw`, `Update` | event notification |
| `KeyInput`, `MouseInput`, `KeyMouseInput` | `d2::in::InputFrame&` |
| `KeySequenceInput` | `d2::string_view` sequence |
| `MouseMoved` | `Position` |
| `TreeSwap` | old and new `std::string` names |

`ctx.on(event, callback)` delegates to the IO context signal dispatcher, returning `Signals::Handle`. The signal system also exposes typed `listen`, `connect`, `signal`, `disconnect` and flags such as `Async`, `Deferred`, `Combine`, `DropIfFull`. Inspect the signature and scheduler behavior of a specific event before attaching a handler; network and main-loop callbacks do not all have the same thread ownership.

### RAII listener lifetime

```cpp
// Use when lifetime of a global/screen listener should follow this element.
// Assign into the element-attached object returned by declare<T>().
ctx.declare<d2::Signals::Handle>() =
    ctx.on(d2::sys::screen::Event::Update, /* exact event signature */ callback);
```

The snippet above illustrates ownership, **not** a fully typed callback: inspect the event's `sigspec` in the screen header first. Handles unsubscribe on destruction unless released, so calling `ctx.on(...);` as a discarded temporary can result in an unexpectedly short listener lifetime.

### Element-local events

`Element::State` includes `Clicked`, `Hovered`, `Focused`, `Keynavi`, etc. Use `ctx.onv(state,callback)` to attach an element listener. The `onv(state,bool,callback)` overload fires for a chosen value transition. `Element::EventListener` supports muting, unmuting and destroying listeners.

A `Button`'s `OnSubmit` is a **style callback** invoked by its own widget logic and is not equivalent to a raw mouse signal. Prefer `OnSubmit` for ordinary button action; use raw screen input only when designing cross-element interactions or shortcuts.

## Focus and navigation

The screen exposes `focus(element)`, `focused()`, `clicked()`, `hovered()`, `focus_next_minor()` and `focus_next_major()`. Interactive widgets opt into keyboard navigation through their own input implementations and often `IKeyboardNav::FocusedColor`. Do not assume a plain `Text` is focusable just because it has `X/Y`.

## Binder: named shortcuts and sequences

`d2::sys::binder` maps to a lazily loaded `SystemBinder` interface. The public binder exposes `Bind` configuration (one or more key sequences, whitelist/blacklist, dependencies, timing and focus/consume policies) plus:

`bind(name, Bind, callback)`, `rebind`, `check`, `unbind`, `freeze`, `unfreeze`, `list`.

The implementation can report bind conflicts and provides sequence delay/hold threshold configuration. To avoid incorrect assumptions about key representations, copy/adjust `Bind` construction patterns from `tests/input.cpp` and the binder header rather than inventing a textual shortcut parser.

## Selection and clipboard

- Selectable widgets expose `EnableSelection` and `HighlightMask`. `Text` and editable inputs implement selection-specific behavior.
- `sys::selector` is a selection manager associated with the screen. Methods include `selection()`, `value()`, `clear()`, `copy()`.
- `SystemClipboard`: `copy`, `paste`, `clear`, `empty`. Clipboard access depends on installed/platform backend.
- `SystemNotify`: `notify(Level, title, content, targets)` with `Local`, `System`, `Passive`, `Prompt`, `SoftPrompt` flags.

`examples/selection.cpp` demonstrates multi-widget text selection with clipboard and selector modules; both need to be available in the selected context. Behavior depends on terminal mouse reports and backend capabilities.

## Pitfalls

- If a callback is unexpectedly silent, check whether its RAII signal handle was destroyed.
- If input appears consumed, inspect `InputFrame` masking and binder consume policy.
- If focus navigation fails, inspect whether a widget provides input and how the screen's focus state is updated.
- If clipboard paste does not work, verify the module backend exists before suspecting the text widget.
