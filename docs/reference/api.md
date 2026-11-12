# API and source map

[Documentation index](../index.md)

This map identifies public entry points and ownership boundaries. The linked headers contain full declarations; generated Doxygen HTML/XML adds member listings, inheritance and source navigation. For semantic behavior start with the linked guide. `internal`/`impl` namespaces and underscored hooks are implementation/extension surfaces, not ordinary application APIs.

## UI and runtime

| API | Public header | Read next |
| --- | --- | --- |
| `IOContext`, module loading, sync handoff | [d2_context.hpp](../../core/io/d2_context.hpp) | [Runtime](../guide/runtime.md) |
| `SystemModule`, `ModuleDecl`, `ModuleImpl`, `Module`, `ModulePtr` | [d2_module.hpp](../../core/io/d2_module.hpp) | [Extending](../guide/extending.md) |
| `SystemInput`, `SystemOutput` | [d2_core.hpp](../../core/mods/d2_core.hpp) | [Rendering](../guide/rendering.md) |
| `SystemScreen`, focus, registered trees, themes | [d2_screen.hpp](../../core/screen/d2_screen.hpp) | [Trees](../guide/trees-and-layout.md) |
| `Tree`, `TreeCtx`, `Stylesheet`, DSL helpers | [d2_tree_construct.hpp](../../core/tree/d2_tree_construct.hpp) | [Getting started](../guide/getting-started.md) |
| `NamedTree`, tree templates and iterators | [d2_tree.hpp](../../core/tree/d2_tree.hpp) | [Trees](../guide/trees-and-layout.md) |
| `Element`, states, frames, listeners, cache policies | [d2_tree_element.hpp](../../core/tree/d2_tree_element.hpp) | [Rendering](../guide/rendering.md) |
| `ParentElement`, child ownership | [d2_tree_parent.hpp](../../core/tree/d2_tree_parent.hpp) | [Trees](../guide/trees-and-layout.md) |
| `TreeState`, application lifecycle | [d2_tree_state.hpp](../../core/tree/d2_tree_state.hpp) | [Runtime](../guide/runtime.md) |
| `Unit`, horizontal/vertical units and literals | [d2_element_units.hpp](../../core/types/d2_element_units.hpp) | [Geometry](../guide/trees-and-layout.md#geometry) |
| UAI, interface chains, typed properties | [d2_styles.hpp](../../core/tree/d2_styles.hpp) | [Styles](styles.md) |
| `Dependency`, themes and subscription handles | [d2_theme.hpp](../../core/tree/d2_theme.hpp) | [Styles/events](../guide/styles-and-events.md) |
| `Signals`, manifests and RAII handles | [d2_signal_handler.hpp](../../core/io/d2_signal_handler.hpp) | [Styles/events](../guide/styles-and-events.md) |
| `InputFrame`, key/mouse/mode types | [d2_input_base.hpp](../../core/io/d2_input_base.hpp) | [Text](../guide/text-and-selection.md) |
| `Animation`, `anim` implementations | [d2_animate.hpp](../../core/screen/d2_animate.hpp) | [Animation](../guide/styles-and-events.md#animation) |
| All standard widgets | [d2_std.hpp](../../elements/d2_std.hpp) | [Widget catalogue](../guide/widgets.md) |

## Text, pixels and images

| API | Public header | Read next |
| --- | --- | --- |
| `pixel`, `PixelBuffer`, views, color types | [d2_pixel.hpp](../../core/types/d2_pixel.hpp) | [Rendering](../guide/rendering.md) |
| `Position`, `BoundingBox`, geometry | [d2_vtypes.hpp](../../core/types/d2_vtypes.hpp) | [Rendering](../guide/rendering.md) |
| `text::Layout`, options, positions and ranges | [d2_text_layout.hpp](../../core/platform/d2_text_layout.hpp) | [Text](../guide/text-and-selection.md) |
| `TextSelection`, selectable geometry | [d2_text_select.hpp](../../core/tree/d2_text_select.hpp) | [Text](../guide/text-and-selection.md) |
| Platform string iteration | [d2_platform_string.hpp](../../core/platform/d2_platform_string.hpp) | [Core contracts](core-contracts.md) |
| Compact extended code page | [d2_extended_page.hpp](../../core/platform/d2_extended_page.hpp) | [Core contracts](core-contracts.md) |
| Byte-oriented `FragmentedBuffer` | [d2_fragmented_buffer.hpp](../../core/platform/d2_fragmented_buffer.hpp) | [Core contracts](core-contracts.md) |
| `image::Raster`, `Scene`, `Placement`, `Composite` | [d2_image.hpp](../../core/types/d2_image.hpp) | [Images](../guide/images.md) |
| `SystemImageLoader`, `LoadOptions` | [d2_image_loader.hpp](../../core/mods/d2_image_loader.hpp) | [Loading](../guide/images.md#loading-data) |
| `SystemImages`, config and output plans | [d2_images.hpp](../../core/mods/d2_images.hpp) | [Images](../guide/images.md) |
| PNG decoder | [d2_png_loader.hpp](../../os/image/png/d2_png_loader.hpp) | [Loading](../guide/images.md#loading-data) |
| Kitty renderer | [d2_kitty_images.hpp](../../os/image/kitty/d2_kitty_images.hpp) | [Output](../guide/images.md#output-behavior) |
| Sixel renderer | [d2_sixel_images.hpp](../../os/image/sixel/d2_sixel_images.hpp) | [Output](../guide/images.md#output-behavior) |
| Asynchronous `dx::Image` | [d2_image.hpp](../../elements/d2_image.hpp) | [Widget loading](../guide/images.md#asynchronous-image-widget) |

## Services and inference

| API | Public header | Read next |
| --- | --- | --- |
| HTTP requests, controls, errors and callbacks | [d2_http.hpp](../../core/mods/d2_http.hpp) | [HTTP](../guide/http.md) |
| WebSocket connections and clients | [d2_ws.hpp](../../core/mods/d2_ws.hpp) | [Services](../guide/services.md) |
| Scoped request headers | [d2_session.hpp](../../core/mods/d2_session.hpp) | [Services](../guide/services.md) |
| Clipboard, selector, notifications | [d2_ext.hpp](../../core/mods/d2_ext.hpp) | [Services](../guide/services.md) |
| Keybinds | [d2_binder.hpp](../../core/mods/d2_binder.hpp) | [Input](../guide/styles-and-events.md#input-signals-and-shortcuts) |
| Audio devices/streams/filters | [d2_audio.hpp](../../core/mods/d2_audio.hpp) | [Services](../guide/services.md) |
| Inference umbrella | [d2_std.hpp](../../core/mods/inference/d2_std.hpp) | [Inference](../guide/inference.md) |
| Messages, requests, settings, errors, metrics | [d2_types.hpp](../../core/mods/inference/d2_types.hpp) | [Inference](../guide/inference.md) |
| Abstract `SystemProvider` | [d2_provider.hpp](../../core/mods/inference/d2_provider.hpp) | [Provider extension](../guide/inference.md#implementing-a-provider) |
| `Tool`, `ToolSet` | [d2_tools.hpp](../../core/mods/inference/d2_tools.hpp) | [Tools](../guide/inference.md#custom-tools) |
| `Conversation`, `SystemConversations`, context policy | [d2_conversation.hpp](../../core/mods/inference/d2_conversation.hpp) | [Conversations](../guide/inference.md) |
| `SystemOpenAI`, endpoint/model configuration | [d2_openai.hpp](../../os/inference/openai/d2_openai.hpp) | [Native provider](../guide/openai.md) |

## Supporting libraries

| API | Header | Notes |
| --- | --- | --- |
| `mt::ConcurrentPool`, nodes, workers and tasks | [pool.hpp](../../mt/pool.hpp) | [Concurrency guide](../guide/concurrency.md) |
| `mt::future`, `co_future` | [future.hpp](../../mt/future.hpp) | Single-consumer result and coroutine return types |
| Result/control blocks | [control.hpp](../../mt/control.hpp) | Advanced lifetime/promise integration |
| `mt::TaskQueue` | [task_queue.hpp](../../mt/task_queue.hpp) | Growing queue and optional ordered producer mode |
| `rs::RuntimeLogs` | [logd.hpp](../../logs/logd.hpp) | Thread-context logging and configuration |
| `MatrixModel` | [d2_model.hpp](../../core/utils/d2_model.hpp) | Character-cell models used by the screen/Matrix widget |
| `lsx::ConstraintArray` | [d2_solver.hpp](../../core/lsx/d2_solver.hpp) | Constraint expressions/solving; removal is unfinished |
| Default theme and styles | [d2_default_theme.hpp](../../extras/d2_default_theme.hpp), [d2_default_styles.hpp](../../extras/d2_default_styles.hpp) | Optional UI conveniences |
| Debug view | [d2_debug_box.hpp](../../extras/d2_debug_box.hpp) | In-app diagnostics |

Native implementations are grouped in `os/io/unix`, `os/net/curl`, `os/clip/local`, `os/notify/local`, `os/audio/pipewire`, `os/image` and `os/inference/openai`, each with a dedicated CMake leaf. See [configuration](configuration.md) for discovery/exclusion and [limitations](limitations.md) before relying on experimental functionality.
