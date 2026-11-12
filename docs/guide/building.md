# Building and testing

[Documentation index](../index.md)

Requirements: a C++20 compiler with `<format>` support, CMake 3.21+, Abseil, ada and uni-algo (when Unicode is enabled). libpng (optional PNG loader), libcurl and PipeWire are detected independently; their backends are omitted when unavailable. The default terminal backend targets Linux/POSIX, but platform-specific terminal behavior still needs testing on each supported OS.

moodycamel ConcurrentQueue v1.0.4 is fetched at commit `6dd38b8a1dbaa7863aa907045f32308a56a6ff5d`. To use an existing checkout or build offline, set `TPMP_CONCURRENTQUEUE_SOURCE_DIR` to the directory containing its three public headers. The installed package includes those headers and their upstream license.

The optional inference component uses nlohmann/json (installed CMake package 3.11+ or a fetched v3.12.0). For an offline source checkout, set `FETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON`. The fallback installs JSON's headers and package config alongside DeltaAscii when installation is enabled. Set `D2_EXCLUDE_LIBRARY_INFERENCE=ON` to omit inference and its JSON dependency. Its headers are included explicitly through `<core/mods/inference/d2_std.hpp>`; the common widget umbrella does not include JSON headers.

The concrete OpenAI-compatible backend is discovered under `os/inference/openai/` and exports `DeltaAscii::OpenAI`. It depends on the inference abstractions and the runtime SystemHTTP interface; no additional HTTP/JSON dependency is introduced. `D2_MODULE_EXCLUDE_DIRS=inference` omits this backend, while `D2_EXCLUDE_LIBRARY_INFERENCE=ON` omits both abstraction and backend. `D2_TEST=ON` also builds the headless `d2_inference` example, including when elements are disabled. See [OpenAI provider](openai.md).

Build from the unpacked source:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DD2_BUILD_TESTS=ON -DD2_TEST=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Use `-DCMAKE_PREFIX_PATH=/path/to/dependencies` if the packages are not in a standard prefix. Build dependencies with compatible compiler, standard-library, debug and sanitizer settings. In particular, mixing an Abseil debug header configuration with release-only libraries can produce link failures.

`D2_TEST` builds the example translation units and runnable `d2_hello`, `d2_manual`, `d2_selection`, `d2_editing` and `d2_images` executables (images requires its backend). Run them in a terminal; Ctrl+Q exits the selection example. Automated tests use headless consumers, including captured terminal output, decoder limits and asynchronous image loading.

## Consuming

For a source checkout:

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_app LANGUAGES CXX)
add_subdirectory(path/to/delt2a delt2a-build)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE DeltaAscii::Core)
```

For an installed package:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DD2_ENABLE_INSTALL=ON
cmake --build build -j
cmake --install build --prefix /path/to/install
```

The consumer uses `find_package(DeltaAscii CONFIG REQUIRED)` and the same `DeltaAscii::Core` target. Add the installation and dependency prefixes to `CMAKE_PREFIX_PATH`. Headers, generated backend registry, backend archives and dependency discovery are included in the export. External module roots remain responsible for installing their own headers.

## Checks

`D2_BUILD_TESTS` enables CTest groups covering concurrent queues/futures/workers, Unicode and buffers, text layout, selection, image composition and Kitty/Sixel encoding, captured terminal output, module transitions, events, inference conversations/tools/context, a public consumer, and curl worker lifecycle when curl is present. Inference tests use scripted providers without network access. Tests retain checks in Release builds.

When curl and POSIX sockets are available, `d2_http` also exercises request control, deadlines, headers, callback errors and shutdown against loopback HTTP servers. These tests use no external services. See [HTTP contracts](http.md).

`d2_styles` checks inherited property IDs, types and field routing, including three levels of style inheritance. It also instantiates the setters used by DebugBox. C++23 consumers can test their configuration with `-DCMAKE_CXX_STANDARD=23 -DCMAKE_CXX_EXTENSIONS=OFF -DCMAKE_BUILD_TYPE=Debug`; the framework still requires only C++20.

```sh
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DD2_BUILD_TESTS=ON -DD2_SANITIZER=address -DD2_SANITIZER_RECOVER=OFF
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

Address mode also enables UndefinedBehaviorSanitizer. Sanitized builds fetch pinned Abseil `20250127.1` (`d9e4955c65cd4367dd6bf46f4ccb8cd3d100540b`) so it can be instrumented consistently. To reuse its source, set `FETCHCONTENT_SOURCE_DIR_ABSL`. MemorySanitizer additionally requires a consistently instrumented C++ runtime and dependencies; merely selecting the CMake option is insufficient.

Other useful configurations are `D2_UNICODE_MODE=OFF` and `D2_EXCLUDE_LIBRARY_ELEMENTS=ON` (also omits extras). Sanitized packages cannot be installed using the regular export.

## Documentation

The Markdown files are the hand-maintained source for architecture and contracts. An optional Doxygen target provides API navigation without requiring prose comments on every trivial method:

```sh
cmake -S . -B build-docs -DD2_BUILD_DOCS=ON
cmake --build build-docs --target d2_docs
```

Doxygen and Python 3.9+ must be installed. The Markdown filter keeps repository-relative header links and section anchors usable in the generated site. Doxygen may warn about older anonymous facade types/template relationships; those declarations remain available through source listings. Open `build-docs/docs/html/index.html`. XML is in `build-docs/docs/xml/`. The [documentation index](../index.md) is the HTML home page; the [API map](../reference/api.md) provides a route into source declarations. Generated listings supplement the guides and contracts.

Run `python3 tools/check_docs.py` to check local Markdown links and the generated style-property reference. Run `python3 tools/style_reference.py` after changing UAI declarations.

## Editing example

With `D2_TEST=ON`, run `d2_editing` to exercise Input and MultiInput. It intentionally loads no clipboard module; Ctrl+A and MultiInput Ctrl+Z/Ctrl+Y still work. Ctrl+Q exits.

```sh
python3 tests/editing_pty.py /path/to/build/d2_editing
# For D2_UNICODE_MODE=OFF:
python3 tests/editing_pty.py /path/to/build/d2_editing --ascii
```

See [images](images.md) for backend selection, optional PNG decoding and the image terminal smoke test.
