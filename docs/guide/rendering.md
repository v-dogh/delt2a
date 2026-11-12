# Rendering and buffers

[Documentation index](../index.md) · [Image composition](images.md)

## Frame path

The screen resolves layout, asks retained elements for frames and combines child buffers according to clipping and paint order. A frame contains compact character cells plus a separate image scene. `SystemOutput::write_frame(view)` preserves both. The three-argument `write(cells, width, height)` API submits cells alone.

`Element::Frame` retains the element while exposing its buffer view. `PixelBuffer::View` is a borrowed rectangular view: keep its buffer/frame alive for the entire use. `PixelBuffer` owns cells and image placements; rasters referenced by the scene are immutable shared objects.

## Drawing cells

```cpp
d2::PixelBuffer buffer(20, 4);
d2::PixelBuffer::View view(&buffer, 0, 0, 20, 4);
view.fill(d2::pixel::combine(d2::px::foreground(240, 240, 240),
                            d2::px::background(20, 20, 20)));
d2::text::Layout("hello").draw(view, d2::pixel{}, {1, 1});
```

Views use local coordinates. Inscription clips at source/destination boundaries and supports overlapping buffers and compressed source views. `fill` replaces cells and image coverage; `fill_blend` and `inscribe` compose according to color/foreground opacity. Mutable `at(x,y)` is an overwrite barrier for image coverage in that cell even if the caller intended only to read it. Read through a const view when mixing text and images.

`data()` is contiguous only for compatible full-width views; a strided subview throws. Raw mutable spans bypass coverage tracking, so use drawing methods for image-aware edits. Negative sizes, invalid coordinates and malformed RLE input fail explicitly.

## Compression and caching

Cell buffers can be RLE-compressed. Random mutation requires an uncompressed representation; image metadata remains separate. Compression does not copy or re-encode raster sources.

`Element::CachePolicy` controls retained pixel/layout caching. `setcache()` replaces the policy. Dynamic/static/volatile policies affect whether an element's frame can be reused; custom elements must signal writes when content changes. A parent reconstructing its frame still needs uncached children to redraw. Avoid bypassing property setters and then diagnosing the stale frame as an output problem.

## Unicode representation

Compact cells use a byte token for the character. ASCII is direct; other grapheme strings are interned in a thread-local extended code page. There are 159 extended slots. Exhaustion is explicit (`try_write` returns empty, `write` throws) and existing entries remain valid. There is no automatic palette reclamation; clearing a palette invalidates all cells/styles that hold its tokens.

Use `text::Layout` for grapheme segmentation, terminal-cell widths, wrapping and drawing. UTF-8 bytes, graphemes and terminal cells are different units. Width is based on Unicode tables, but terminal/font behavior can differ for ambiguous width and emoji. The framework does not rasterize terminal fonts. See [text contracts](text-and-selection.md).

## Output integration

A terminal backend compares cells with its previous frame and writes changes. Image output modules produce `Plan::before`, `after` and `repaint`: emit cleanup first, repaint cells when requested, then place graphics. On a failed write invalidate both relevant output/graphics caches so a retry reconstructs the frame. Unloading graphics must remove its terminal resources and restore cells.

The current Unix backend uses POSIX terminal I/O. It owns the terminal viewport; Sixel full-screen redraw is not appropriate for embedding in a region owned by another renderer. Alternate terminal/remote output implementations should preserve these contracts while providing their own transport.
