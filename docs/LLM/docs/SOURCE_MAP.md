# Source map: where to verify delt2a APIs

**Scope:** supplied library snapshot. Paths below are relative to the root of the original `delt2a` archive. This is a curated index for an LLM to inspect the smallest relevant header first, then its `.cpp` implementation and tests. It is not a list of every private helper.

## Start files and tooling

| Source path | What to look for |
| --- | --- |
| `README.md` | Purpose, baseline CMake and hello example |
| `CMakeLists.txt` | C++ version, options, dependency detection, targets, installation and docs/test toggles |
| `cmake/target.cmake` | CMake target composition/link propagation |
| `cmake/system.cmake` | Module registration/discovery and exclusion controls |
| `cmake/DeltaAsciiConfig.cmake.in` | Installed consumer CMake config |
| `cmake/d2_platform.hpp.in` | Generated platform/module registry glue |
| `d2_std.hpp` | Core umbrella include |
| `elements/d2_std.hpp` | Standard widgets umbrella include |

## Runtime, event loop, modules

| Source path | Primary symbols / role |
| --- | --- |
| `core/io/d2_context.hpp`, `.cpp` | `IOContext`, `make`, `run`, `sync`, `sync_async`, `load`, `sys`, `sys_if`, `launch_thread`, lifecycle |
| `core/io/d2_module.hpp` | `SystemModule`, `ModuleDecl`, `ModuleImpl`, `Module`, `ModulePtr`, status/access/load metadata |
| `core/io/d2_signal_handler.hpp` | `Signals`, `Handle`, signal manifests and delivery flags |
| `core/io/d2_input_base.hpp` | `d2::in::InputFrame`, `Mode`, `Special`, `Mouse`, `key`, frame state |
| `core/io/d2_main_worker.hpp` | Main-thread worker scheduling and polling |
| `core/screen/d2_screen.hpp`, `.cpp` | `SystemScreen`, named tree switching, focus, theme, update, signal events |
| `core/screen/d2_screen_comps.hpp` | Screen supporting components and tree/tag/dependency management |
| `core/screen/d2_animate.hpp` | Animations, interpolators, animation handles |
| `os/d2_registry.hpp`, `.cpp` | Runtime concrete-backend registry, compatible resolution, platform selection |
| `core/mods/d2_core.hpp` | `SystemInput`, `SystemOutput`, core module aliases |
| `core/mods/d2_ext.hpp` | `SystemClipboard`, `SystemNotify`, `SystemSelector` |
| `core/mods/d2_binder.hpp` | `SystemBinder`, key sequence bindings/configuration |

## Tree, styling, layout

| Source path | Primary symbols / role |
| --- | --- |
| `core/tree/d2_tree_construct.hpp` | `dstyle`, `dstylev`, `dstyledv`, `dapply`, `dmstyle`; `Tree`, `TreeForward`, `TreeCtx`, `Stylesheet`, `branch`, `embed`, `persist` |
| `core/tree/d2_tree.hpp` | Tree template/init infrastructure |
| `core/tree/d2_tree_state.hpp` | `TreeState`, hooks, state ownership |
| `core/tree/d2_tree_element.hpp` | `Element`, rendering/layout/event hooks, state, property write paths |
| `core/tree/d2_tree_element_frwd.hpp` | `TreeIter`, weak handle conversion/navigation |
| `core/tree/d2_tree_parent.hpp` | `ParentElement`, `VecParentElement`, `MetaParentElement`; child insert/replace/query/remove |
| `core/tree/d2_styles.hpp` | UAI templates plus shared interfaces (`ILayout`, `IColors`, `IText`, etc.) |
| `core/tree/d2_styles_base.hpp` | Property storage/binding infrastructure; UAI declaration macros themselves are in `d2_styles.hpp` |
| `core/tree/d2_theme.hpp` | `Dependency<T>`, `DependencyHandle`, `Theme`, `ThemeRegistry` |
| `core/tree/d2_text_select.hpp` | Selection model and selectable interface |
| `core/types/d2_element_units.hpp` | `Unit`, `HDUnit`, `VDUnit`, unit modifiers, `_px`, `_pv`, `_pc`, `_center`, `_auto`, `_relative` literals |
| `core/types/d2_pixel.hpp` | `pixel`, foreground/background, `PixelBuffer`/view and rendering data |
| `core/types/d2_vtypes.hpp` | Geometric helper types: `Position`, `BoundingBox` etc. |
| `core/platform/d2_colors.hpp` | Color constants and conversions |
| `core/platform/d2_text_layout.hpp` | Text layout metrics and wrapping |
| `core/platform/d2_fragmented_buffer.hpp` | Fragmented text buffer used by `MultiInput` |
| `core/platform/d2_locale.hpp` | Locale/character encoding behavior |

## Element classes

| Widget | Header |
| --- | --- |
| `dx::Box` | `elements/d2_box.hpp` |
| `dx::FlowBox`, `dx::ScrollFlowBox`, `dx::VirtualFlowBox` | `elements/d2_flow_box.hpp` |
| `dx::VirtualBox` | `elements/d2_draggable_box.hpp` |
| `dx::ScrollBox` | `elements/d2_scrollbox.hpp` |
| `dx::DemandScrollBox`, `dx::EntrySeed` | `elements/d2_demand_scrollbox.hpp` |
| `dx::Text` | `elements/d2_text.hpp` |
| `dx::Button` | `elements/d2_button.hpp` |
| `dx::Checkbox` | `elements/d2_checkbox.hpp` |
| `dx::Input` | `elements/d2_input.hpp` |
| `dx::MultiInput` | `elements/d2_multiline_input.hpp` |
| `dx::Slider`, `dx::VerticalSlider` | `elements/d2_slider.hpp` |
| `dx::Switch`, `dx::VerticalSwitch` | `elements/d2_switch.hpp` |
| `dx::Dropdown` | `elements/d2_dropdown.hpp` |
| `dx::Image` | `elements/d2_image.hpp` |
| `dx::Matrix` | `elements/d2_matrix.hpp` |
| `dx::graph::CyclicVerticalBar`, `dx::graph::NodeWaveGraph` | `elements/d2_graph.hpp` |
| `dx::Separator`, `dx::VerticalSeparator` | `elements/d2_separator.hpp` |
| Shared widget helpers | `elements/d2_element_utils.hpp` |

## Network, image and audio

| Source path | Symbols / purpose |
| --- | --- |
| `core/mods/d2_http.hpp` | `sys::net::SystemHTTP`, `Request`, `RequestBuilder`, `Response`, `Stream`, `Chunk`, `Error` |
| `core/mods/d2_ws.hpp` | `sys::net::SystemWS`, `Connection`, `Client`, `RequestBuilder`, callbacks, reconnection |
| `core/mods/d2_session.hpp` | `sys::net::SystemSession`, scoped auth headers |
| `core/mods/d2_images.hpp` | `SystemImages`, terminal image scene preparation and backend hooks |
| `core/mods/d2_image_loader.hpp` | `SystemImageLoader`, `LoadOptions`, async decode |
| `core/types/d2_image.hpp` | Raster, RGBA/RGB, scene, compositing and image rectangles |
| `core/mods/d2_audio.hpp` | `SystemAudio`, devices/stream processing, presets, pipelines |
| `core/utils/d2_model.hpp` | `MatrixModel`, model load/save and generation |
| `os/image/kitty/` | Kitty terminal image backend |
| `os/image/sixel/` | Sixel terminal image backend |
| `os/image/png/` | Optional libpng decoder |
| `os/` network implementations | Curl HTTP and WS backends; search for the interface's `ModuleImpl` |
| `os/` audio implementations | Optional PipeWire backend |

## Scheduler and logging

| Source path | Main focus |
| --- | --- |
| `mt/pool.hpp`, `.cpp` | `mt::ConcurrentPool`, worker topology, scheduling |
| `mt/future.hpp` | `mt::future`, `man_future`, `co_future`, result/cancellation mechanics |
| `mt/control.hpp` | Task lifecycle/control |
| `mt/task_queue.hpp`, `mt/task_ring.hpp` | Task queue/ring internals |
| `mt/defer.hpp` | Deferral helpers |
| `logs/logd.hpp`, `.cpp` | `rs` runtime logs, severity and config |
| `extras/d2_default_theme.hpp` | Default theme variable registry |
| `extras/d2_default_styles.hpp` | `d2::ex::df` stylesheets |
| `extras/d2_loading_indicators.hpp` | Loading animation styles |
| `extras/d2_debug_box.hpp`, `.cpp` | Debug/profiling widget |

## Best source examples/tests for particular tasks

| Task | Example | Tests |
| --- | --- | --- |
| Hello and quitting | `examples/manual.cpp`, `examples/e0-hello/ui.hpp` | `tests/core.cpp`, `tests/consumer.cpp` |
| Reactive counter, stylesheets | `examples/e1-counter/ui.hpp` | `tests/styles.cpp` |
| Single and multi-input editors | `examples/editing.cpp` | `tests/editing_pty.py` |
| Selection/clipboard | `examples/selection.cpp` | `tests/selection_pty.py` |
| Images, image protocols and PNG | `examples/images.cpp` | `tests/images.cpp`, `tests/image_backend.cpp`, `tests/images_pty.py` |
| Input/screen keyboard events | `examples/editing.cpp` | `tests/input.cpp`, `tests/events.cpp`, `tests/terminal.cpp` |
| Module selection & lifecycle | source runtime modules | `tests/module.cpp` |
| HTTP, WS and session | `core/mods/d2_http.hpp`, `d2_ws.hpp` | `tests/http.cpp`, `tests/transport.cpp` |

## Rules for source exploration

- Read **declarations** in a corresponding header, then **implementation** to learn side effects and callbacks.
- For an unfamiliar property, find `D2_UAI_PROPS(...)`, then `D2_UAI_LINK(...)` to identify storage type and invalidation behavior.
- For templated context helpers, read `core/tree/d2_tree_construct.hpp`; don't rely on only a single example.
- For terminal/backend behavior, verify implementation is actually selected by CMake and compatible with the runtime platform.
- Treat `tests/` as evidence of an expected behavior for the tested version, not proof of universal platform support.
- Do not infer removed functionality from a stale or disconnected example.
