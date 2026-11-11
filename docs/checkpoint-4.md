# Checkpoint 4: image composition and terminal backends

This checkpoint includes the preceding core, text/selection and editing work. It replaces the old output-owned Kitty image handles and five-cell image marker encoding. The remote impl branch has not been changed.

## Delivered

- Immutable shared RGB/RGBA sources and a separate image scene in PixelBuffer, preserving ten-byte cells. Placements survive copying, compressed caches, clipped views and element inscription.
- A CPU compositor handles source crops, destination scaling, parent clipping, image alpha overlap and cell-level text occlusion in paint order. Work is cached when scene, viewport and cell metrics are unchanged.
- An optional SystemImages module selects Kitty, Sixel or Off explicitly. Without the module, ordinary text output remains usable. Kitty uses direct, chunked RGBA transmission with owned IDs; Sixel uses a 216-color palette and run-length encoding.
- Optional libpng decoding produces reusable rasters, with explicit input and allocation limits. Builds without PNG retain both renderers and raw RGB/RGBA support.
- Image widgets use Source and SourceRect styles. The shared border helper bounds drawing for tiny elements and oversized borders.
- SystemOutput::write_frame carries image metadata and correctly handles compressed or strided views. Movement/removal restores underlying text; renderer unloading and output shutdown clear graphics.
- Runtime module removal now invokes unload outside the registry lock. Dependency checks use the removed provider's index and include named dependents.
- Terminal output waits for writable readiness on EAGAIN instead of abandoning large uploads. Ordinary write failures invalidate rendering state for retry.
- setcache now replaces the previous cache policy, including the default bypass flag. Children without cached content redraw when their parent repaints. Rendering over existing images or outside parent bounds uses an intermediate buffer so cache policy does not change composition/clipping semantics.
- Added the d2_images example, compositor regressions, protocol round-trip checks and pseudo-terminal lifecycle checks.

## Migration and limits

See [images](images.md) for examples, ownership, coordinate units and exact paint rules. Image::Path/ImageOptions and SystemOutput's ImageInstance/load_image/release_image APIs are removed. Decode a PNG through SystemImages, or provide an immutable Raster directly, then set Image::Source. Use write_frame instead of raw-cell write when submitting image-bearing buffers yourself.

Text occludes images at whole-cell boundaries. Terminal fonts are not rasterized. Scaling currently stretches with nearest-neighbor sampling. Sixel has fixed-palette quantization and requires accurate cell pixel dimensions. Graphics backend selection is explicit; there is no capability probing. The compositor caches one bounding raster rather than tracking independent image damage; large or widely separated images can be expensive and hit the documented pixel limit.

Mutable cell access invalidates underlying image coverage; raw data writes bypass that tracking. Active scene and renderer mutation belongs to the UI thread. The previous Unicode palette lifetime/reclamation limits remain in force.

## Validation

The bundle contains build/test logs and incremental/cumulative patches. Protocol tests verify Kitty chunk payloads, Sixel color/band/transparent-hole decoding, cache behavior, movement/removal, backend changes, module unloading, context teardown and write-error recovery. Composition tests cover clipping, crops, alpha overlap, text masks, compressed views, self-inscription, all cache policies and tiny borders.

All twelve CTest groups pass with Clang 19 Release/Unicode, GCC 14.2 Release/Unicode, Clang Release/ASCII with PNG disabled, and Clang ASan+UBSan/Unicode. The PNG-enabled configurations decode a fixture and reject truncated input. The installed-package consumer builds and runs with image, editor and text APIs. Kitty/Sixel pseudo-terminal tests pass in Unicode and ASCII; plain mode, existing selection and editing smoke tests also pass. Pseudo-terminal checks validate emitted bytes and process lifecycle, not the visual behavior of a real terminal emulator. Test the example in your chosen Kitty/Sixel terminal before relying on its graphics behavior. PipeWire and Doxygen were not exercised. No clean TSan result is claimed; checkpoint 1's queue diagnostic remains unresolved. ASan/UBSan uses leak detection disabled because of this environment's tracing.

## Next

The next addition is the OpenAI-compatible inference abstraction over the existing network/task facilities. Image capability detection, higher-quality resampling, incremental image damage and additional terminal protocols are follow-up work, not prerequisites for that layer.
