```
    __________     __    _______  ______  ________ ________ 
   / ______  /    /  \  / _____/ / _____\/___  __//___  __/
  / /     / /    / *  \ \_____  / /         / /      / /
 / /_____/ /    / ___  \_____ \|  \____ ___/ /__ ___/ /__
/_________/    /_/   \________/\______//_______//_______/ 

```

A modular C++20 TUI framework for building retained-mode terminal interfaces.

## What is it?

delt2a is a terminal UI framework built around modularity, extensibility and performance.

It is designed for building structured, multiplatform terminal applications with 
reusable elements, state-driven updates, styling, and clear architecture

## Features

- Retained-mode element tree with caching and efficient change handling
- C++20 tree construction DSL
- Typed and extensible style/property system
- Standard elements including: buttons, text, boxes (automatic layout included), inputs, sliders, graphs, and more
- Reactive dependencies and subscriptions
- Keyboard/mouse input abstraction
- Modular scheduler and multithreading support
- Modular runtime system
- Core modules include input, output, keybind management, clipboard, notifications, network (HTTP/WebSocket), audio
- Debug box for profiling and debugging

## Usage

Fetch and link the framework:

```cmake
FetchContent_Declare(
    delt2a
    GIT_REPOSITORY https://git.pvdogh.dev/MagnariusCosmo/delt2a
    GIT_TAG impl
)
FetchContent_MakeAvailable(delt2a)
...
target_link_libraries(project PRIVATE DeltaAscii::Core)
```

And then define a tree and add some elements:

```cpp
#include <d2_std.hpp>
#include <elements/d2_std.hpp>

using app = d2::Tree<
    d2::TreeState,
    d2::dx::Box,
    [](d2::TreeCtx<d2::dx::Box> ctx)
    {
        using namespace d2::dx;
        ctx.elem([](d2::TreeCtx<Text> ctx)
        {
            dstyle(Value, "Hello World!");
            dstyle(X, 0.0_center);
            dstyle(Y, 0.0_center);
        });
    }
>;

int main()
{
    d2::IOContext::make<
        d2::sys::input,
        d2::sys::output,
        d2::sys::screen
    >()->run<d2::NamedTree<"App", app>>();
}
```

More examples are in the project's 'examples' directory


## Core cleanup checkpoint

See [building and tests](docs/building.md) for dependency setup, source and installed consumers, sanitizers and API documentation. See [core contracts](docs/core-contracts.md) for queue ordering, future/coroutine behavior, Unicode palette limits and lifetime rules.

The [checkpoint notes](docs/checkpoint-1.md) describe the changes, migration points and validation limits. These changes are supplied in the accompanying source bundle; the remote `impl` branch has not been updated.

The framework now uses a growing moodycamel queue; `TaskRing` is legacy. The second checkpoint adds [shared text layout and generic selection](docs/text-and-selection.md), including a runnable `d2_selection` example. The third checkpoint migrates editing widgets to grapheme positions and cell geometry. The fourth checkpoint adds [image composition and optional Kitty/Sixel output](docs/images.md). The fifth adds [provider-neutral inference and conversation APIs](docs/inference.md), including model switching, custom tools and context management. The concrete OpenAI-compatible provider remains the next addition.

See [checkpoint 2](docs/checkpoint-2.md) for the delivered scope, test matrix and remaining limits.

See [checkpoint 3](docs/checkpoint-3.md) for editing/history changes and validation.

See [checkpoint 4](docs/checkpoint-4.md) for image rendering, module lifecycle changes and validation.

See [checkpoint 5](docs/checkpoint-5.md) for inference contracts and validation.

See [checkpoint 6](docs/checkpoint-6.md) for the consistent `core/mods/inference/` layout and [HTTP request controls](docs/http.md) needed by the upcoming native provider.
