# 08. Rendering, inline images, matrix buffers and optional audio

**Source:** `core/types/{d2_pixel,d2_image}.hpp`, `core/mods/{d2_images,d2_image_loader,d2_audio}.hpp`, `elements/{d2_image,d2_matrix}.hpp`, `os/` concrete backends, `examples/images.cpp`, `tests/images.cpp`, `tests/image_backend.cpp`.

## Terminal rendering has two paths

1. **Cell rendering:** core `pixel`, `PixelBuffer` and `PixelBuffer::View`, with text, colors, container borders, matrices, graph widgets. `SystemOutput::write_frame(...)` serializes/diffs terminal cell content. No advanced graphics protocol required.
2. **Inline image rendering:** `image::Raster` in an `image::Scene`, composed by `SystemImages` backends. Kitty and Sixel have separate terminal protocol implementations, discovered/selected conditionally. Image rendering support depends on the output backend, terminal capabilities and image loading modules.

`SystemOutput` is a `TUnsafe` module. Do not call it from a generic network callback. The UI normally paints through elements, not direct `output()->write(...)` calls.

## Raster and scene types

- `d2::image::Raster` is an owning pixel image object; available creation paths include RGB/RGBA factories.
- `d2::image::Rect` selects source crop for `Image::SourceRect`.
- `d2::image::Scene` manages image placements (`place`, `erase`, `compose` APIs).
- `d2::image::Composite` is used when preparing terminal-graphics payloads.
- `d2::image::LoadOptions`: `max_bytes` default 64 MiB, `max_pixels` default `Raster::max_pixels`, `validate()`.
- `d2::sys::SystemImageLoader`: synchronous `decode(span, opts)` and `load(path, opts)`; async `load_async(path, opts) -> mt::future<image::Raster::ptr>`.

Use `Raster::ptr` for a pre-decoded source; avoid treating it as a filename or terminal escape sequence.

## `dx::Image`

UAI: `ILayout`, `IContainer`, `IColors`, `IImage`; extra properties `Source`, `SourceRect`.

```cpp
ctx.elem([](d2::TreeCtx<d2::dx::Image> ctx)
{
    dstyle(Width, 20.0_px);
    dstyle(Height, 10.0_px);
    // dstyle(Source, raster_ptr); // Already-decoded image::Raster::ptr
    // auto task = ctx.ptr()->load("photo.png");
    // Retain/use asynchronous result appropriately; do not block UI thread.
});
```

`Image::load(std::filesystem::path, LoadOptions={}, loader_name={})` returns `mt::co_future<bool>`; `cancel_load()` cancels/supersedes a pending request. Implementation uses a ticket and weak element/context references so the operation can avoid updating a destroyed target. Still, avoid awaiting its future synchronously from the UI tick.

PNG decode is optional (libpng-backed `SystemPngLoader`). A file existing on disk does not imply the loader can decode its format. Check `sys_if<sys::image_loader>()` and image backend compatibility where the app needs graceful degradation.

## Kitty and Sixel image output

`SystemImages` is a system module depending on the output path, with `configure(BoundingBox cell_pixels)`, `prepare(cells, viewport, scene, metrics)`, `reset()`, and `invalidate()`. Concrete implementations implement `_encode`/`_erase` and decide repaint policy. This normally stays below application level: use `dx::Image` and the registered runtime modules.

Testing concerns: terminal can reject one or both protocols; pixel-cell measurement can be unknown; resizing or a moved image may require protocol-specific erase/repaint. Never promise support based solely on `TERM` or whether a PNG decoder is installed.

## Matrix and pixel buffer

`dx::Matrix` is the lower-level widget to display a `d2::MatrixModel::ptr` through a pixel-like matrix. Properties `Model`, `BackgroundMask`, `ForegroundMask`, `MaskOptions`; mask bit flags `ApplyFg`, `ApplyBg`, `InterpFg`, `InterpBg`.

`MatrixModel` supplies model creation, dimensions and save/load operations (`Raw` and `Visual` formats in source). This is separate from arbitrary RGBA bitmaps and from terminal-native image transport. Consult `core/utils/d2_model.hpp` and `elements/d2_matrix.hpp` for the correct model constructor and representation.

Custom drawing widgets override `_frame_impl(PixelBuffer::View)` and sometimes `_reserve_buffer_impl`, `_fetch_pixel_buffer_impl`, `_layout_impl`; they should set cache/invalidation flags appropriate to their output. See graph and matrix implementations as source patterns. Do not manually emit ANSI escapes from `_frame_impl`.

## Optional audio module

`d2::sys::audio` (`SystemAudio`) is an optional thread-safe audio facility; the inspected concrete backend uses PipeWire. It is **not** required by ordinary UI widgets and has distinct setup/runtime requirements.

| Type / API | Purpose |
| --- | --- |
| `SystemAudio::Format` | `PCM16`, `PCM32`, `Float32`, `Float64` |
| `FormatInfo`, `DeviceConfig` | Sample format, channels (default 2), sample rate (default 48 kHz), latency configuration |
| `DeviceName` | Device ID and display name |
| `SystemAudio::Stream` | Per-chunk audio data view, `apply`, `push`, `push_expand`, `stop`, `info`, `size`, `chunk`, `at` |
| `FilterPipeline` | Compose named or callable transforms with overloaded pipeline operators |
| `SystemAudio::preset` | Reusable transforms including gain/volume and fade, plus other signal effects in header |

The `Stream` is a data view within audio processing; respect its lifetime and channel count. Pipeline work can be high-frequency; don't perform blocking I/O or UI updates in the audio data path. See `core/mods/d2_audio.hpp` for device enumeration, stream activation, recording/transmission methods and exact overloads; these are more specialized than the TUI's core API.

## Choosing the right representation

| Need | Appropriate path |
| --- | --- |
| Label, border, button, ASCII graph | Standard widgets and cell renderer |
| Custom per-cell drawing | `PixelBuffer::View` / `Matrix` |
| Real photo in supported terminal | `dx::Image` + `image_loader` + compatible Kitty/Sixel backend |
| Volume meter or signal visualization | `graph::CyclicVerticalBar` or custom matrix |
| Audio output/input | `sys::audio` optional backend |
