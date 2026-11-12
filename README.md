# DeltaAscii

A modular C++20 retained-mode terminal UI framework: typed styles, reusable elements, grapheme-aware text editing/selection, composited images, asynchronous services and optional inference conversations.

- [Documentation](docs/index.md)
- [Getting started](docs/guide/getting-started.md)
- [Build and installation](docs/guide/building.md)
- [API map](docs/reference/api.md)
- [Examples](examples/)

Link `DeltaAscii::Core` from a source checkout or installed CMake package. Choose native backends through the module registry; abstract service interfaces live under `core/mods/` and implementations under `os/`.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DD2_TEST=ON -DD2_BUILD_TESTS=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Requires a C++20 compiler with `<format>`, CMake 3.21+, Abseil, ada and (in Unicode mode) uni-algo. Optional services discover libcurl, libpng and PipeWire independently. See the build guide for offline dependencies, sanitizer builds and generated Doxygen HTML/XML.
