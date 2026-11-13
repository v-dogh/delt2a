# delt2a: source-audited documentation for coding agents

**Library:** delt2a / DeltaAscii, a C++20 retained-mode terminal UI framework (`project(DeltaAscii VERSION 0.1.0)`).  
**Source baseline:** supplied repository archive, inspected 2026-11-12; embedded Git HEAD `5e5d457aad2bae48f8b6060b1055a3d3dd922425`.  
**Coverage:** public application APIs, runtime contracts, element catalogue, backends, extension patterns, and observed coding conventions.  
**Excluded:** the independently extracted inference library.
**Revision note (2026-11-13):** maintainer-reported fixes have been removed from the documentation; the updated code was not included in the uploaded source archive, so the fixes were not independently re-audited.

This is an **agent-facing handbook**, not generated Doxygen output. It aims to answer _how to use the actual API safely_ before a coding model starts inventing plausible-but-nonexistent methods. Public names were traced from headers into implementations and examples where relevant. The source, not this snapshot, remains definitive.

## Start here

1. **[AGENTS.md](AGENTS.md)**: condensed operational rules; copy beside a repository's `AGENTS.md` or add to an LLM's instructions.
2. **[Quick start](docs/01_QUICKSTART.md)**: CMake, basic executable, counter example.
3. **[Architecture and lifecycle](docs/02_ARCHITECTURE_LIFECYCLE.md)**: runtime, trees, swapping, retained-mode operation.
4. **[Tree and context API](docs/03_TREE_API.md)**: `Tree`, `TreeCtx`, `TreeIter`, parent/element APIs.
5. **[Properties, units, reactive styles](docs/04_PROPERTIES_STYLES_THEMES.md)**: `dstyle`, UAI property types, units, dependencies, themes.
6. **[Elements API](docs/05_ELEMENTS_API.md)**: widgets, notable properties and methods, callbacks.
7. **[Input, signals, bindings](docs/06_INPUT_SIGNALS_BINDINGS.md)**: events, focus, selection, keyboard/mouse.
8. **[Runtime modules, network, platform](docs/07_MODULES_PLATFORM_NETWORK.md)**: module selection, HTTP, WS, session, capabilities.
9. **[Rendering, images, audio](docs/08_IMAGES_RENDERING_AUDIO.md)**: image paths, backend dependencies, drawing, optional audio.
10. **[Concurrency, scheduling, logging](docs/09_THREADS_LOGGING.md)**: UI thread, futures, safe background work.
11. **[Working recipes](docs/10_RECIPES.md)**: common patterns with source-grounded examples.
12. **[Coding and formatting conventions](docs/11_STYLE_GUIDE.md)**: C++ and library-contributor patterns.
13. **[Behavioral gotchas and caveats](docs/12_GOTCHAS.md)**: traps, surprising behavior, limitations.
14. **[Application UI composition style](docs/13_APPLICATION_UI_STYLE.md)**: recommended conventions for writing actual screens.
15. **[Source map](docs/SOURCE_MAP.md)**: file lookup for further verification.

## Which file should an LLM read?

| Task | Read first | Also read |
| --- | --- | --- |
| New TUI application | 13, 01, 03 | 04, 05, 10 |
| Reactive UI state | 13, 04 | 03, 09 |
| Build a custom widget | 05, 03 | 04, 08, 11 |
| Focus, keybinds, scrolling | 06 | 05 |
| Async HTTP/WebSocket | 07 | 09, 12 |
| PNG/Kitty/Sixel | 08 | 07, 05 |
| Threading or freezes | 09 | 02, 12 |
| Debug strange runtime behavior | 12 | SOURCE_MAP |
| Add a platform/system module | 07 | 02, SOURCE_MAP |

## Verification legend

- **Public API**: declaration exists in an exported/includeable source header.
- **Source behavior**: implementation or existing test demonstrates behavior. This is not necessarily a formal stability promise.
- **Pattern**: established by the repository's examples.
- **Recommendation**: inferred engineering guidance, explicitly distinguished from an API requirement.

**Testing caveat:** this documentation comes from static examination of the supplied snapshot. Code blocks are intended to match the source APIs but have not all been independently compiled against every optional dependency/backend. Platform/module availability is environment-dependent.
