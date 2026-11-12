# Images and composition

[Documentation index](../index.md)

Images are immutable shared RGBA rasters. PixelBuffer carries a separate scene of image placements, clips and optional cell masks; `pixel` remains ten bytes. Copying a buffer shares source rasters. Compression affects character cells, not the scene.

## Drawing

```cpp
auto source = d2::image::Raster::rgba(2, 1,
    {{255, 0, 0, 255}, {0, 80, 255, 128}});
d2::PixelBuffer buffer(40, 12);
d2::PixelBuffer::View view(&buffer, 0, 0, 40, 12);
view.image(source, {2, 1, 20, 8});
// Optional last argument: source crop in raster pixels.
```

Placement destinations and clips use terminal cells; source rectangles use raster pixels. Scaling is nearest-neighbor, stretching the selected source into the destination. Negative destinations and partial views clip without rescaling the remaining portion. `image::Raster::rgb` accepts packed RGB bytes; `rgba` takes RGBA values. A source's alpha is straight, not premultiplied. Zero-initialized RGBA is transparent.

The Image widget uses `Image::Source` (Raster::ptr) and `Image::SourceRect` (image::Rect). Set Width and Height explicitly. Its border geometry determines the content rectangle. Tiny widgets and partially disabled borders are supported. Use `load()` to populate `Source` asynchronously, or set a reusable raster directly.

## Ordering and text

Images participate in the normal element paint order, including ZIndex, parent clipping, cached children and compressed buffers. Later images alpha-composite over earlier images. A later painted character or nontransparent background occludes earlier images for that entire cell. This includes partially transparent cell backgrounds. Fully transparent empty cells preserve earlier images when inscribed. An image placed later hides underlying glyphs in every cell with a nonzero image sample.

This is cell-level text/image composition. It does not rasterize the terminal font or blend images through individual glyph outlines. The terminal still renders all text. Transparent portions of the image reveal cell backgrounds; occupied cells do not retain underlying glyphs.

`fill` is replacement: it removes earlier image coverage in its area, even for transparent fill. `fill_blend` and `inscribe` use the opacity/foreground rules above. Acquiring mutable `at()` is an overwrite barrier for that cell; use a const view/buffer for reads. Raw mutable `data()` bypasses image coverage tracking, so prefer the drawing methods when mixing text and images. `clear`, `reset` and resizing clear the scene. A subview exposes only its clipped, locally translated scene.

## Modules and responsibilities

| Interface | Implementation | Responsibility |
| --- | --- | --- |
| `sys::SystemImageLoader` | `sys::SystemPngLoader` (sys::img::png) | Paths/encoded bytes to immutable RGBA rasters |
| `sys::SystemImages` | `sys::SystemImagesKitty` (sys::img::kitty) | Shared composition/cache plus Kitty output encoding/cleanup |
| `sys::SystemImages` | `sys::SystemImagesSixel` (sys::img::sixel) | Shared composition/cache plus Sixel output encoding/cleanup |
| `sys::SystemOutput` | Unix terminal output | Execute graphics plans around character-cell output |

The loader and renderer are independent services. A generated raster needs no decoder, and headless decoding needs no terminal. The renderer depends on `SystemOutput`; it does not require libpng. Both renderers reuse the same core compositor rather than duplicating clipping/paint-order logic.

Include the selected implementation headers and register explicit modules:

```cpp
#include <os/image/png/d2_png_loader.hpp>
#include <os/image/kitty/d2_kitty_images.hpp>

auto context = d2::IOContext::make<d2::sys::input, d2::sys::output,
    d2::sys::screen, d2::sys::img::png, d2::sys::img::kitty>();
// context->run<YourNamedTree>();
```

Use `SystemImagesSixel` and its `os/image/sixel/d2_sixel_images.hpp` header for Sixel. Do not load both renderers under the unnamed `SystemImages` key. To switch at runtime, unload `SystemImages` and load the other concrete type on the UI thread. Cleanup restores the cell frame. Query with the abstract key:

```cpp
context->sys<d2::sys::SystemImages>()->configure({
    .enabled = true, .cell_pixels = {8, 16}
});
```

An explicitly loaded renderer starts enabled. `.enabled = false` suppresses graphics and clears previous placements on the next frame. Backend choice is explicit: no TERM inference, terminal probing or automatic fallback is performed. Without a renderer, output ignores the scene and renders ordinary cells. `sys::images` is an interface alias, not capability detection; select the concrete type in application manifests.

## Loading data

`SystemImageLoader::decode(bytes, options)` and `load(path, options)` are synchronous, thread-safe operations. They return `image::Raster::ptr` and throw on invalid data or I/O failure. The PNG implementation recognizes file contents, not filename suffixes; PNG-only means JPEG and other formats are rejected even if renamed `.png`.

```cpp
auto loader = context->sys<d2::sys::SystemImageLoader>();
auto pending = loader->load_async("assets/logo.png");
// In a coroutine; this continuation can be on a pool worker:
auto raster = co_await pending;
```

`load_async` schedules file reading and decoding on the context's existing pool. Keep the context/pool active until pending work settles. It retains the selected decoder while the job executes. Unloading a stateless PNG loader does not interrupt already accepted decoding. Direct synchronous calls work without a context; asynchronous calls require one and a running pool.

`image::LoadOptions` supplies `max_bytes` (default 64 MiB) and `max_pixels` (default 16,777,216). Limits must be positive; decoded pixel limits cannot exceed the raster's global cap. File size and PNG dimensions are checked before their large allocations. PNG decoding produces straight-alpha RGBA through libpng. Animated playback, EXIF orientation, color-management policy and arbitrary image format discovery are not part of this interface.

To use another loader, register a concrete `SystemImageLoader` implementation or a named instance. Call `load(path, options, "assets")` on a widget to use a named loader. A future format-dispatching loader can route by content without changing `Image` or terminal output.

## Asynchronous Image widget

```cpp
auto completion = picture->load("assets/logo.png", {.max_pixels = 1024 * 1024});
// In a coroutine:
bool applied = co_await completion;
```

Call `Image::load` on the active UI thread. It returns `mt::co_future<bool>`; file reading/decoding runs on a pool worker and successful `Source` assignment is marshalled through `IOContext::sync_async`. The old source stays visible until the new raster is applied. Set `Width`/`Height` independently; loading does not resize the element. `SourceRect` remains in effect, so reset an old crop when it does not fit the replacement raster.

The boolean is `true` only when the result was applied. A newer `load`, `cancel_load()` or explicit `set<Image::Source>` supersedes pending application. Destroying the element or moving it to another context also prevents application. A detached element that is still alive in the same context can still receive its result. Jobs hold weak widget references.

I/O/decode errors propagate through the returned future and leave the current source intact. Missing context/loader, wrong-thread invocation or invalid setup can throw directly from `load`. Completion on failure can occur on a worker; marshal UI error handling explicitly. Dropping or discarding the future does not cancel decoding or pending assignment. `cancel_load()` invalidates assignment on the UI thread; it is not a decoder interrupt or join. Superseded jobs can still report decode errors through their own futures.

## Output behavior

- Kitty transmits direct RGBA in bounded base64 chunks and places one composited image. It owns/deletes only its own image ID and preserves the cursor.
- Sixel uses a fixed 216-color cube and run-length encoding. Partial alpha is flattened against cell backgrounds; transparent holes remain unpainted. Updates clear the display, repaint cells and retransmit graphics. Color banding is possible; no dithering is performed.

Cell dimensions come from TIOCGWINSZ unless overridden. Kitty can fall back to an 8x16 sampling grid and size placement in cells. Sixel needs actual metrics; if the terminal does not report them, configure both dimensions. Font/DPI changes require updating an explicit override.

Scene, viewport and metrics changes rebuild one bounding raster. Unchanged Kitty frames reuse the upload; Sixel also redraws when cells change. Distant placements can make a large composite rectangle. Raster/composite allocations share the 16,777,216-pixel cap. This is whole-composite caching, not incremental per-image damage tracking.

`prepare` and `reset` return output plans. An output backend executes the plan and calls `invalidate` after failed writes. Protocol encoders expect valid composites from `image::compose`. Unloading the renderer clears graphics and restores cells. Output shutdown also clears attached graphics.

## Build selection

`DeltaAscii::ImagesKitty`, `DeltaAscii::ImagesSixel` and `DeltaAscii::PngLoader` are distinct targets; registered implementations are also linked by the aggregate core target. `D2_IMAGE_PNG=OFF` or a missing libpng omits the PNG module entirely, without affecting either renderer. There are no decoder methods that merely throw “not compiled” on the renderer.

Use `D2_MODULE_EXCLUDE_DIRS=image` to omit all native image implementations, or `D2_MODULE_EXCLUDE_TYPES`/`BASES` to control registry selection. Core rasters, scene composition and loader/renderer interfaces remain available. See [configuration](../reference/configuration.md) for the distinction between omitting a directory and excluding a registry entry.

## Example and validation

```sh
./build/d2_images plain
./build/d2_images kitty
./build/d2_images sixel 8 16
./build/d2_images kitty 8 16 assets/logo.png
python3 tests/images_pty.py ./build/d2_images kitty
```

Supply your actual cell dimensions for Sixel. The optional path replaces the front checkerboard asynchronously; it requires the PNG loader. Ctrl+R moves, Ctrl+D hides, Ctrl+L unloads the renderer and Ctrl+Q exits. Plain mode loads no renderer. Tests cover decoding limits, content-based input, concurrent loads, UI handoff, supersession, manual replacement, errors, destruction, composition and terminal byte output.

A pseudo-terminal cannot establish visual correctness in an emulator. Sixel uses modern xterm DECSDM semantics: resetting mode 80 positions graphics at the cursor. The previous mode/cursor are restored. Full-display erasure assumes ownership of the viewport.

Protocol references: [xterm controls](https://www.invisible-island.net/xterm/ctlseqs/ctlseqs.html), [xterm Sixel modes](https://www.invisible-island.net/xterm/manpage/xterm.html), [Kitty graphics](https://sw.kovidgoyal.net/kitty/graphics-protocol/) and [DEC Sixel](https://vt100.net/docs/vt3xx-gp/chapter14.html).
