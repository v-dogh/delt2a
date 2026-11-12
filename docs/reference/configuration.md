# Build configuration reference

[Documentation index](../index.md) · [Build instructions](../guide/building.md)

## Options

| Variable | Default | Effect |
| --- | --- | --- |
| `D2_UNICODE_MODE` | `ON` | Unicode strings/layout and uni-algo dependency; OFF selects the ASCII build |
| `D2_EXCLUDE_LIBRARY_ELEMENTS` | `OFF` | Omit standard widgets and dependent extras |
| `D2_EXCLUDE_LIBRARY_EXTRAS` | `OFF` | Omit default styling/debug extras |
| `D2_EXCLUDE_LIBRARY_INFERENCE` | `OFF` | Omit inference abstractions and the OpenAI backend/JSON requirement |
| `D2_IMAGE_PNG` | `ON` | Build PNG loader when libpng is found |
| `D2_TEST` | `OFF` | Build examples (including headless inference when available) |
| `D2_BUILD_TESTS` | `OFF` | Build CTest regression executables |
| `D2_BUILD_DOCS` | `OFF` | Enable Doxygen HTML/XML target; requires Doxygen |
| `D2_ENABLE_INSTALL` | `OFF` | Generate headers, archives and CMake package installation |
| `D2_SECURITY_WATCH` | `ON` | Runtime debug checks |
| `D2_SECURITY_LOG` | `ON` | Runtime logging |
| `D2_SECURITY_ASSERT` | `ON` | Debug assertions |
| `D2_SANITIZER` | `none` | `address` (includes undefined), `undefined`, `memory` or `thread` |
| `D2_SANITIZER_RECOVER` | `ON` | Sanitizer recovery where supported |
| `D2_ASAN_USE_AFTER_SCOPE` | `ON` | AddressSanitizer stack-scope checking |
| `D2_MSAN_TRACK_ORIGINS` | `ON` | MemorySanitizer origin tracking |
| `TPMP_CONCURRENTQUEUE_SOURCE_DIR` | empty | Existing ConcurrentQueue headers instead of fetching |
| `FETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON` | empty | Existing JSON source for FetchContent fallback |
| `FETCHCONTENT_SOURCE_DIR_ABSL` | empty | Existing pinned Abseil source for sanitizer builds |

Set `CMAKE_PREFIX_PATH` for installed Abseil, ada, uni-algo and other packages. Compiler/standard-library/debug/sanitizer settings must match dependency binaries. C++20 is required; a C++23 application is supported. Normal package installation is disabled for sanitized builds.

## Module selection

`D2_MODULE_EXCLUDE_DIRS` is a semicolon-separated list of first-level module groups, such as `image`, `net`, `audio` or `inference`. This skips directory discovery. `D2_MODULE_EXCLUDE_TYPES` and `D2_MODULE_EXCLUDE_BASES` suppress matching concrete/abstract registry entries; the leaf CMake file may still create its library target. They are registry filters, not dependency-removal guarantees.

Use directory/component options when the purpose is avoiding a dependency. `D2_MODULE_ROOTS` accepts absolute external roots followed by dependency target names, repeated for each root. Native module headers are installed by the main build; external roots must handle their own installation.

The generated `d2_platform.hpp` records discovered backends. After changing backend selection, reconfigure before compiling. Loading an abstract key at runtime uses this registry; it does not probe terminal graphics capabilities.

## Targets

| Public target | Purpose |
| --- | --- |
| `DeltaAscii::Core` | Aggregate framework interface, including enabled components and registered backends |
| `DeltaAscii::Inference` | Optional provider/conversation abstraction |
| `DeltaAscii::OpenAI` | Native OpenAI-compatible provider |
| `DeltaAscii::ImagesKitty` | Kitty graphics renderer |
| `DeltaAscii::ImagesSixel` | Sixel graphics renderer |
| `DeltaAscii::PngLoader` | PNG decoder when built |
| `d2_docs` | Generated documentation (not a link target) |

Internal libraries include `DeltaCore`, `DeltaCoreOS`, `DeltaElements`, `DeltaExtras`, `DeltaLSX`, `TPMP` and `logd`. Prefer documented public targets in applications. Optional exported targets exist only when the corresponding component was built/registered for installation.

## Dependencies

Abseil and ada are core requirements. Unicode adds uni-algo. The scheduler uses a pinned ConcurrentQueue source; inference uses nlohmann/json 3.11+ or its pinned fallback. libcurl, libpng and PipeWire provide independent optional backends. Missing libpng removes only the PNG loader, while raster generation and Kitty/Sixel remain available. The OpenAI implementation uses the HTTP interface; it does not bring a second transport dependency.
