# Extending DeltaAscii

[Documentation index](../index.md) · [API map](../reference/api.md)

## Custom widgets

Derive from an existing widget for a small behavioral change. Use its inherited `data`/UAI chain so inherited property IDs and field routing remain correct. For a new leaf, compose `style::UAI<YourType, Interfaces...>`; for a different parent base use `style::UAIE<ParentBase, YourType, Interfaces...>`. `UAIC` chains new interfaces onto an existing styled widget.

Implement `_frame_impl(PixelBuffer::View)` with local coordinates. Report geometry through the layout interfaces or `_layout_impl` when specialized measurement is needed. Use `_event_impl` and `_state_change_impl` for input/state behavior. Forward base hooks when overriding a widget that already handles them. Internal data changes must signal the appropriate style/layout write; ordinary public setters do this automatically.

Custom selectable text implements the `ISelectable` style interface's geometry/data hooks. Return grapheme ranges and cell geometry using the shared text layout rather than treating bytes as positions. See [selection integration](text-and-selection.md).

## New style interfaces

`D2_UAI_INTERFACE` declares options, backing fields, properties and links. Each `D2_UAI_PROP` links a public property to storage and an `Element::WriteType` invalidation flag. Use the existing element headers as templates. Keep flags accurate: a value that changes dimensions needs layout invalidation, while colors typically need a style repaint only. Avoid numeric property IDs and direct storage access from consumers.

After adding or changing an interface, regenerate `docs/reference/styles.md` with `python3 tools/style_reference.py`. `tests/styles.cpp` demonstrates inherited-chain checks. Doxygen can navigate declarations; the generated Markdown table also exposes macro-defined properties directly to readers and text-based tools.

## New modules

A platform-independent concrete service can derive from `Module<Concrete, "Name", Access, Load, Dependencies...>`. A replaceable service uses `ModuleDecl<Abstract, ...>` and a backend derives from both that interface and `ModuleImpl<Concrete, Load, Dependencies...>`. Preserve thread-safety and lifecycle promises in the abstract contract.

Implement `_load_impl`/`_unload_impl`, returning a module `Status`. Do not invoke user callbacks under registry locks, and settle asynchronous operations before destroying resources they use. Declare dependencies by their abstract keys and load them in the application manifest. Configuration should have explicit defaults and reject invalid combinations.

Place native/backend code under `os/<service>/<implementation>/` with its own CMake file:

```cmake
add_library(MyLoader STATIC my_loader.hpp my_loader.cpp)
delta_register_module(
    TYPE "my::Loader"
    BASE "d2::sys::SystemImageLoader"
    HEADER "my_loader.hpp"
    TARGET MyLoader
)
```

The directory scanner finds leaf CMake files and generates the registry header. `PLATFORMS` can constrain registration. `D2_MODULE_ROOTS` supports external roots with dependency targets; those extensions own their header installation. Query instances through the abstract key even when loaded using a concrete type.

## Image decoders

Implement `SystemImageLoader::decode(bytes, options) const`. Validate `LoadOptions`, encoded size and dimensions before allocating. Return a non-null immutable straight-alpha RGBA raster. The default `load(path, options)` performs bounded binary file reading and calls `decode`; override it only for a different source policy. `load_async` already uses the context pool and retains the loader.

The loader is declared `Access::TSafe`: independent decodes must be safe concurrently. Protect shared codec state or use request-local state. A decoder with unload-sensitive resources must keep accepted work valid until completion; the supplied PNG decoder is stateless. A dispatcher can compose multiple formats behind this interface without touching widgets or output encoders.

## Image output encoders

Derive from `SystemImages` and add `ModuleImpl`. Shared `prepare` manages composition, occupied-cell masking, scene/metrics cache and invalidation. Implement:

- `_encode(composite, cells, columns)`: output commands for a valid composed raster.
- `_erase()`: commands to remove the previous placement; for full-repaint protocols this also prepares the initial viewport.
- `_repaint_on_change()`: whether graphics changes require clearing/repainting cells and whether cell changes require re-encoding.
- `_fallback_metrics()`: optional cell sampling size when real terminal metrics are missing.

The base returns output plans; encoders must not write directly to stdout. Keep terminal resources scoped to the renderer instance. The output backend executes cleanup/cells/placement in order and invalidates caches after failures. `SystemImages` depends on output, and all renderer calls belong on its UI thread.

## Inference adapters

Implement `SystemProvider::model_info` and `_generate_impl` using framework futures and cancellation. Preserve per-request callback ordering, final tool argument assembly, typed errors, deadlines, usage and provider-specific continuation state. See [provider contracts](inference.md#implementing-a-provider). The native adapter under `os/inference/openai` is a concrete example using HTTP rather than a second networking runtime.
