# Getting started

[Documentation index](../index.md) · [Build prerequisites](building.md)

## A complete application

This is the `examples/manual.cpp` application, built as `d2_manual` when `D2_TEST=ON` and extras are enabled. It displays a label and a button; click **Quit** to stop the context. A terminal with mouse reporting is needed for the button.

```cpp
#include <d2_std.hpp>
#include <elements/d2_box.hpp>
#include <elements/d2_text.hpp>
#include <elements/d2_button.hpp>

using namespace d2;
using App = Tree<TreeState, dx::Box, [](TreeCtx<dx::Box> ctx)
{
    ctx.elem([](TreeCtx<dx::Text> ctx)
    {
        dstyle(Value, "Hello from DeltaAscii");
        dstyle(X, Unit(2));
        dstyle(Y, Unit(1));
        dstyle(Width, Unit(30));
        dstyle(ForegroundColor, px::foreground(230, 230, 230));
    });
    ctx.elem([](TreeCtx<dx::Button> ctx)
    {
        dstyle(Value, "Quit");
        dstyle(X, Unit(2));
        dstyle(Y, Unit(3));
        dstyle(Width, Unit(10));
        dstyle(Height, Unit(1));
        dstyle(ForegroundColor, px::foreground(255, 255, 255));
        dstyle(BackgroundColor, px::background(40, 60, 80));
        dstyle(OnSubmit, [](TreeIter<dx::Button> button) { button->context()->stop(); });
    });
}>;

int main()
{
    IOContext::make<sys::input, sys::output, sys::screen>()
        ->run<NamedTree<"Main", App>>();
}
```

The `Tree` describes construction; it is not rebuilt for each frame. `TreeCtx` creates retained elements, and `dstyle` calls their typed setters. `NamedTree` registers a screen tree under a stable name. `IOContext::run()` initializes modules and workers, constructs the screen, runs input/rendering, and tears down after `stop()`.

Include individual widget headers for smaller translation units, or `<elements/d2_std.hpp>` for the catalogue. `<d2_std.hpp>` exposes the core DSL and common modules. Inference is included separately to avoid pulling JSON into every UI translation unit.

## Link it

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_app LANGUAGES CXX)
add_subdirectory(path/to/delt2a delt2a-build)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE DeltaAscii::Core)
```

For an installed build, replace `add_subdirectory` with `find_package(DeltaAscii CONFIG REQUIRED)`. Use the patched checkout you intend to build; fetching an older remote branch does not incorporate local changes.

## Choose your next example

| Executable / source | Shows |
| --- | --- |
| `d2_hello`, `examples/e0-hello/ui.hpp` | Minimal tree construction |
| `d2_manual`, `examples/manual.cpp` | Explicit geometry, styling, submit callback and shutdown |
| `d2_selection`, `examples/selection.cpp` | Cross-element text selection |
| `d2_editing`, `examples/editing.cpp` | Single-line and multiline editing/history |
| `d2_images`, `examples/images.cpp` | Layering, clipping, renderer unload and optional asynchronous PNG loading |
| `d2_inference`, `examples/inference.cpp` | Headless provider setup, streaming, tools and model switching |
| `examples/all.cpp` | Compilation coverage of example widgets |

The editing, selection and images examples use Ctrl+Q to quit. See the individual guides for their other controls.

## Headless applications

Trees are optional for services such as HTTP and inference. A headless application can create a `ConcurrentPool`, construct an `IOContext`, register the required modules, and use their futures without starting terminal input/output. Start the scheduler when submitting pool work. UI handoff via `sync_async` requires an active main worker; use ordinary pool or transport completion handling in a headless process. The inference example is a complete implementation of this pattern.
