# DeltaAscii documentation

DeltaAscii (delt2a) is a C++20 retained-mode terminal UI framework. The core owns trees, layout, cells, signals and scheduling. Modules provide services and platform backends; applications choose which ones to load.

## Start here

1. [Build and link the library](guide/building.md).
2. [Create a runnable application](guide/getting-started.md).
3. [Understand runtime ownership and modules](guide/runtime.md).
4. [Build trees and layouts](guide/trees-and-layout.md), then [add styles, state and events](guide/styles-and-events.md).

## Application guides

| Area | Guide |
| --- | --- |
| Standard elements | [Widget catalogue](guide/widgets.md) |
| Text, Unicode, editing and selection | [Text and selection](guide/text-and-selection.md) |
| Cells, painting, clipping and caching | [Rendering](guide/rendering.md) |
| Image loading, asynchronous widgets and terminal protocols | [Images](guide/images.md) |
| Workers, queues, futures and coroutines | [Concurrency](guide/concurrency.md) |
| HTTP requests and streaming | [HTTP](guide/http.md) |
| Clipboard, notifications, WebSockets, audio and logs | [Services](guide/services.md) |
| Conversations, tools, context and metrics | [Inference](guide/inference.md) |
| Native Chat Completions adapter | [OpenAI-compatible provider](guide/openai.md) |
| Custom elements, styles and modules | [Extending DeltaAscii](guide/extending.md) |

## Reference

- [API and source map](reference/api.md): entry points by subsystem and public header.
- [Style properties](reference/styles.md): property names, backing fields and invalidation flags.
- [Build configuration](reference/configuration.md): options, targets and backend selection.
- [Core contracts](reference/core-contracts.md): ownership, synchronization and low-level guarantees.
- [Limitations and troubleshooting](reference/limitations.md): explicit boundaries and common failures.

Markdown is the canonical guide and can be read directly by humans or tools. `D2_BUILD_DOCS=ON` generates searchable Doxygen HTML and XML with this page as the entry point, including source-linked declarations from core, widgets, extras, platform implementations, scheduler and logs. See [building documentation](guide/building.md#documentation).

For automated code navigation: start at the API map, read the relevant guide, then inspect the linked header and implementation. Property IDs are local to an element's inherited interface chain; use symbolic names rather than copying numeric IDs. Generated listings describe available declarations, not a promise that every experimental subsystem is complete.
