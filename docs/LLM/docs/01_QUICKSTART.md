# 01. Quick start and build integration

## Prerequisites and dependency shape

- CMake **3.21+**; C++ **20**. The build project is `DeltaAscii` version `0.1.0`.
- Source includes core (`core/`), elements (`elements/`), extras (`extras/`), OS backends (`os/`), concurrency (`mt/`), and logging (`logs/`).
- The code/configure process uses Abseil, ada, and, in Unicode mode, uni-algo; `mt` may fetch moodycamel concurrentqueue. Optional capability modules use libcurl, libpng, and PipeWire.
- No guarantee of Windows terminal support in this snapshot; inspect platform registry and concrete `os/` implementations for the target system.

### Consumer CMake

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_tui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    delt2a
    GIT_REPOSITORY https://git.pvdogh.dev/MagnariusCosmo/delt2a
    GIT_TAG master
)
FetchContent_MakeAvailable(delt2a)

add_executable(my_tui main.cpp)
target_link_libraries(my_tui PRIVATE DeltaAscii::Core)
```

For reproducible projects, replace `master` with a tested commit hash or tagged release. `DeltaAscii::Core` is an exported/aliased CMake target, **not** a C++ namespace.

### Minimal hello world, adapted from the repository README

```cpp
#include <d2_std.hpp>
#include <elements/d2_std.hpp>

using App = d2::Tree<
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
    >()->run<d2::NamedTree<"App", App>>();
}
```

`0.0_center` is a **unit literal**, not a standalone keyword. Its operator is defined in the units layer and made available by the general includes. The `dstyle` macro assumes that the lambda's parameter is called `ctx`.

## Runtime features you opt into

To get more than input, output and screen, request/load corresponding modules. For example, clipboard/selection functionality requires their module interfaces and available implementations. Backends may be unavailable even if their interface headers compile.

At startup, `IOContext::make<Modules...>(rs::Config logs_cfg = {})` builds the context. `run<NamedTree...>(themes...)` registers tree factories/themes, initializes the worker/screen and enters its event loop. `stop()` exits the loop. Avoid `std::cout` inside an active UI.

## CMake knobs in this snapshot

| Option | Default | Effect |
| --- | --- | --- |
| `D2_UNICODE_MODE` | `ON` | Unicode vs ASCII-oriented build (`D2_LOCALE_MODE`) |
| `D2_EXCLUDE_LIBRARY_ELEMENTS` | `OFF` | Omit standard widget library |
| `D2_EXCLUDE_LIBRARY_EXTRAS` | `OFF` | Omit extras/themes/helpers |
| `D2_SECURITY_WATCH` / `D2_SECURITY_LOG` / `D2_SECURITY_ASSERT` | `ON` | Runtime inspection/log/assert features |
| `D2_ENABLE_INSTALL` | `OFF` | Enable installation of headers and CMake package |
| `D2_TEST` | `OFF` | Enable repository examples |
| `D2_BUILD_TESTS` | `OFF` | Build regression test suite |
| `D2_BUILD_DOCS` | `OFF` | Attempt repository's built-in Doxygen route; see gotchas |
| `D2_SANITIZER` | `none` | `none`, `address`, `memory`, `thread`, `undefined` |

The module system also supports CMake filtering via `D2_MODULE_EXCLUDE_BASES`, `D2_MODULE_EXCLUDE_TYPES`, `D2_MODULE_EXCLUDE_DIRS` and additional `D2_MODULE_ROOTS`. See [modules](07_MODULES_PLATFORM_NETWORK.md).

## Source entry points

`README.md`, `CMakeLists.txt`, `d2_std.hpp`, `elements/d2_std.hpp`, `examples/manual.cpp`, `examples/e0-hello/ui.hpp`, `examples/e1-counter/ui.hpp`.
