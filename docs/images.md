# Images and composition

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

The Image widget uses `Image::Source` (Raster::ptr) and `Image::SourceRect` (image::Rect). Set Width and Height explicitly. Its border geometry determines the content rectangle. Tiny widgets and partially disabled borders are supported. The former Path/ImageOptions styles and output-owned ImageInstance/load_image/release_image API are removed.

## Ordering and text

Images participate in the normal element paint order, including ZIndex, parent clipping, cached children and compressed buffers. Later images alpha-composite over earlier images. A later painted character or nontransparent background occludes earlier images for that entire cell. This includes partially transparent cell backgrounds. Fully transparent empty cells preserve earlier images when inscribed. An image placed later hides underlying glyphs in every cell with a nonzero image sample.

This is cell-level text/image composition. It does not rasterize the terminal font or blend images through individual glyph outlines. The terminal still renders all text. Transparent portions of the image reveal cell backgrounds; occupied cells do not retain underlying glyphs.

`fill` is replacement: it removes earlier image coverage in its area, even for transparent fill. `fill_blend` and `inscribe` use the opacity/foreground rules above. Acquiring mutable `at()` is an overwrite barrier for that cell; use a const view/buffer for reads. Raw mutable `data()` bypasses image coverage tracking, so prefer the drawing methods when mixing text and images. `clear`, `reset` and resizing clear the scene. A subview exposes only its clipped, locally translated scene.

## Terminal module

Load `sys::images` alongside input/output/screen, then configure it on the UI thread:

```cpp
context->sys<d2::sys::SystemImages>()->configure({
    d2::sys::SystemImages::Protocol::Kitty, {0, 0}
});
```

The module defaults to Off. Backend choice is explicit; the framework does not infer support from TERM or issue terminal probes. Without a loaded, enabled image module, output renders ordinary cells and ignores image metadata. Use `SystemOutput::write_frame(view)` for custom frame submission; the legacy three-argument write sends cells only.

- Kitty transmits direct RGBA data in bounded base64 chunks and places one composited image. It owns its image ID, deletes only that image, and preserves the cursor. No shared-memory files or encoded character markers are needed.
- Sixel uses a fixed 216-color cube and run-length encoding. Partial alpha is flattened against cell backgrounds; transparent holes remain unpainted. Sixel updates clear the display and force text repaint and raster retransmission so moving/removing graphics restores the underlying frame. Color banding is possible; no dithering is performed.

Cell pixel dimensions are obtained from TIOCGWINSZ unless configured explicitly. Kitty can fall back to an 8x16 sampling grid and size the placement in terminal cells. Sixel requires real cell dimensions: if the terminal does not report them, configure `{width, height}`. Inaccurate dimensions produce misplaced/scaled Sixel graphics. Font or DPI changes require updating explicit metrics.

Scene, viewport, metrics and backend changes rebuild one bounding raster. Unchanged Kitty frames reuse the upload; Sixel also redraws when underlying cells change. This is a cached whole-composite implementation, not a per-image damage tracker. Distant images can produce a large bounding rectangle. Raster and composite allocations are capped at 16,777,216 pixels; oversized requests fail explicitly.

The module's prepare/reset methods produce output plans and are intended for output backend integration. A backend must execute the plan and call invalidate after failed writes. Protocol encoders consume valid composites returned by `image::compose`.

Unloading the module clears its graphics and restores the last cell frame. Output shutdown also clears attached graphics. Module registry removal now runs unload outside the registry lock, including when callers retain a module handle. Dependency checks prevent removing a provider while a dependent module is registered.

## Optional PNG decoding

When libpng is available, `D2_IMAGE_PNG=ON` (default) enables `images->load_png(path)` and `images->decode_png(bytes)`. Both return reusable immutable rasters. Decode off the UI thread only with an independent decoder/module instance; active renderer state belongs to the UI thread. The encoded input limit is 64 MiB, and decoded dimensions share the raster limit.

Without libpng these methods throw a descriptive logic_error. RGB/RGBA sources and both terminal protocols remain available. Use `-DD2_IMAGE_PNG=OFF` to disable decoding explicitly, or `-DD2_MODULE_EXCLUDE_DIRS=image` to omit the terminal image module entirely. The core scene representation remains available.

## Example and validation

Build with `D2_TEST=ON` and run:

```sh
./build/d2_images plain
./build/d2_images kitty
./build/d2_images sixel 8 16 # replace with your terminal's actual cell size
python3 tests/images_pty.py ./build/d2_images kitty
```

Ctrl+R moves the front image through the clipping boundary, Ctrl+D removes both images, Ctrl+L unloads the renderer, and Ctrl+Q exits. Plain mode loads no image module. Protocol tests check bytes and lifecycle; a pseudo-terminal cannot validate a terminal emulator's graphics display or font metrics.

This backend uses modern xterm DECSDM semantics: reset mode 80 positions Sixel at the cursor. Some historical DEC documentation describes the opposite polarity. The prior mode and cursor are restored after each image. Full-display erasure on Sixel redraw assumes the framework owns the terminal viewport.

Protocol references: [xterm controls](https://www.invisible-island.net/xterm/ctlseqs/ctlseqs.html), [xterm Sixel modes](https://www.invisible-island.net/xterm/manpage/xterm.html), [Kitty graphics](https://sw.kovidgoyal.net/kitty/graphics-protocol/) and [DEC Sixel](https://vt100.net/docs/vt3xx-gp/chapter14.html).
