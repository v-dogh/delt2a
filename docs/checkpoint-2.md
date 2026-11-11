# Shared text and selection checkpoint

This builds on checkpoint 1. The full source bundle includes both checkpoints, a cumulative patch against the original `impl` upload, and a smaller patch against the checkpoint-1 source. No remote branch was changed.

## Delivered

- `text::Layout`: owned source, grapheme/byte mappings, cell geometry, source revisions, hard lines, soft wrapping, alignment, tabs, hit testing, extraction and painting.
- `dx::Text` and common text-rendering helpers share the layout implementation. Text auto-sizing no longer counts UTF-8 bytes, and the newline/wrapping modes have distinct behavior.
- The existing `IVSelectable` / `ISelectable` path exposes layout and highlight ranges. Text participates; custom elements can implement the same provider interface.
- The existing `SystemSelector` now supports cross-element mouse selection, copy, clearing, release notifications and lifetime/revision validation. Clipboard support is optional. `TextSelection` also works independently of the module.
- Wide-character continuation cells use existing fields without growing the ten-byte pixel. Unix output handles clipped/covered wide glyphs, same-area resizes, SGR state and failed-output retries. The old image encoding remains a legacy path; this is not the image compositor stage.
- A runnable `d2_selection` example and documentation in [text-and-selection.md](text-and-selection.md).

## Validation

| Check | Result |
| --- | --- |
| Clang 19, Release, Unicode, examples | Build and 9/9 CTest groups passed |
| GCC 14.2, Release, Unicode, examples | Build and 9/9 CTest groups passed |
| Clang 19, Release, ASCII, examples | Build and 9/9 CTest groups passed |
| Clang 19, RelWithDebInfo, ASan + UBSan | Build and 9/9 CTest groups passed |
| Fresh installed package with separate consumer | Configure, build and execution passed; consumer uses the new layout and selection APIs |
| Unicode and ASCII example in a pseudo-terminal | Both paragraphs rendered; cross-paragraph drag highlighted; Ctrl+Q exited with status 0 |
| Unicode table regeneration | Reproduced the checked-in table exactly |

New regression cases include combining marks, CJK, ZWJ emoji and flags; malformed input; CRLF/trailing empty lines; whitespace-preserving wrapping; tabs and alignment; soft-wrap affinity; clipping; RLE continuation round trips; selection across elements in either direction; reflow, edits, removal and hiding; nonselectable overlays; selector unload/reload; terminal output diffs; truncated image markers; and output-error recovery using `/dev/full`.

LeakSanitizer remains disabled in this environment. The unresolved queue diagnostic from checkpoint 1's TSan run remains unresolved; the scheduler was not changed in this checkpoint. These results are not a claim of a clean TSan run. The pseudo-terminal check verifies bytes and lifecycle, not font shaping, physical terminal compatibility or the system clipboard. Doxygen generation and PipeWire were not exercised.

## Boundaries and migration

`Input` and `MultiInput` editing, history and cursor operations are still byte-based. They return no generic text provider and must migrate before they can participate safely. That is the next text stage, followed by image composition and inference.

Layout is left-to-right and uses a documented Unicode 16 width policy; it does not implement bidi/shaping or full UAX #14 line breaking. The selector does not yet auto-scroll, extend ranges from the keyboard, select words by double-click, or transform anchors through edits. It clears stale selections instead.

`SystemSelector::Event::Selected` now carries an owned string rather than a borrowed string view. Providers use half-open grapheme ranges. The reserved pixel style bit is internal metadata: a space-valued reserved marker is RLE, while a non-space reserved cell is a wide-glyph continuation. Code that writes raw pixels should replace the complete cell or blend a normal foreground when covering a continuation.

The compact Unicode palette still has 159 extended slots and the thread/ownership restrictions documented in [core-contracts.md](core-contracts.md).
