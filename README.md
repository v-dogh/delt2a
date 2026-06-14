```
    __________     __    _______  ______  ______  ________
   / ______  /    /  \  / _____/ / _____\/ _____\/___  __/
  / /     / /    / *  \ \_____  / /     / /         / /
 / /_____/ /    / ___  \_____ \|  \____|  \____ ___/ /__
/_________/    /_/   \________/\______/\______//_______/ 

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
    GIT_TAG master
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
