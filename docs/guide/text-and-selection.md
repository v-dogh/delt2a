# Text layout and selection

[Documentation index](../index.md)

`d2::text::Layout` is the shared model for text measurement, painting, hit testing and selection. `dx::Text`, `Input`, `MultiInput` and the common text-rendering helpers use it. The model owns its source; returned source/extraction views remain valid until the next modifying `reset` or destruction.

## Coordinates

| API/data | Unit |
| --- | --- |
| `Glyph::bytes` | Half-open UTF-8 byte range in the original source |
| `Range`, `size()`, `caret()`, `hit_test()` | Grapheme boundaries, including the final boundary after the last grapheme |
| `Glyph::position`, `Glyph::columns`, `bounds()` | Terminal cells |
| `Line::graphemes` | Grapheme range excluding its hard newline |
| `revision()` | Source revision; reflow alone does not change it |

```cpp
d2::text::Layout layout("e\xcc\x81界!", {.width = 12});
auto end = layout.hit_test({3, 0});
auto selected = layout.extract({0, end});
auto bytes = layout.byte_offset(end);
```

`grapheme_at` snaps a byte offset backward or forward to a grapheme boundary. `caret` accepts upstream/downstream affinity to distinguish the two visual positions at a soft wrap. Hit testing uses integer cell positions, choosing the nearest boundary inside a multi-cell glyph or tab. Points before/after the document clamp to its start/end. Offsets beyond the source throw.

Newlines include CR, LF, CRLF and Unicode line/paragraph separators. Empty and trailing lines are retained. Tabs expand to tab stops (four columns by default). `Wrap::None` preserves hard lines; `Wrap::Grapheme` wraps at grapheme boundaries; `Wrap::Word` prefers whitespace boundaries, then breaks an overlong word at a grapheme boundary. Whitespace is retained. NBSP, narrow NBSP and figure space are not word-wrap opportunities. Disabling newline handling paints line breaks as a single space. A zero-width layout remains finite and paints nothing.

`extract` returns original bytes: hard newlines, tabs and repeated spaces survive; soft wrapping contributes no text. Word wrapping is a whitespace-based policy, not a complete Unicode line-breaking implementation.

## Width and output

Grapheme segmentation uses the existing uni-algo dependency. Width uses generated Unicode 16.0.0 East Asian W/F and emoji-property tables, with ambiguous characters narrow. Grapheme width is the maximum printable code-point width, with emoji-presentation sequences treated as two cells. This is a defined terminal policy, not a guarantee that every terminal/font agrees. There is no bidi reordering or shaping engine. ASCII builds use byte characters, replacing unsupported bytes when painting.

Invalid UTF-8, control graphemes and standalone zero-width graphemes paint as `?`; extraction still returns their original bytes. This prevents text content from being emitted as terminal control sequences. A wide grapheme that cannot fit completely in the clip rectangle paints blank cells at its visible portion.

Cells remain ten bytes with a one-byte character token. The reserved style bit marks a wide-glyph continuation carrying the same token/colors as its lead. RLE markers retain the reserved style with a space token. Use normal pixel assignment/blending to cover text; changing only the character byte of a continuation does not clear its metadata. The Unix renderer resolves orphaned or mismatched pairs to spaces before diffing, repaints neighbors when a wide pair changes, and explicitly positions each new row. It also detects geometry changes with equal cell counts and invalidates its cache after output errors.

The width table is checked in; no new runtime dependency or configure-time download is needed. To regenerate it, download these pinned inputs and run:

```sh
python3 tools/generate_text_width.py EastAsianWidth.txt emoji-data.txt \
  > core/platform/generated/d2_unicode_width.hpp
```

Inputs: [EastAsianWidth.txt](https://www.unicode.org/Public/16.0.0/ucd/EastAsianWidth.txt), [emoji-data.txt](https://www.unicode.org/Public/16.0.0/ucd/emoji/emoji-data.txt). The Unicode license is included beside the generated header. See [UAX #11](https://www.unicode.org/reports/tr11/tr11-43.html) for the distinction between width properties and terminal tailoring.

## Generic selection

Load `sys::selector` with the screen module. Load `sys::clipboard` when copying through a clipboard service is wanted (the bundled backend is process-local); selection itself works without it.

```cpp
d2::IOContext::make<d2::sys::input, d2::sys::output, d2::sys::screen,
                   d2::sys::selector, d2::sys::clipboard>();
```

`dx::Text` includes `style::ISelectable`: `EnableSelection` controls participation and `HighlightMask` controls the highlight background. Left-drag selects across participating elements. Ctrl+C copies an existing selection when an editable input is not focused; Escape clears it. The module emits `SystemSelector::Event::Selected` on release with an owned string and the screen positions of the drag endpoints. It does not copy automatically on release or replace widget click/focus handling. Disable selection on labels inside controls where dragging should only operate the control.

The independent `TextSelection` controller can be used without a module: `begin(root, position)`, `extend(position)`, `finish()`, `value()`, `clear()` and `select_all(root)`. Call `refresh()` after document changes; the module does this on screen updates. All these operations run on the UI thread.

Copy order is visible-element tree traversal order, independent of where layout places those elements. Providers outside a parent's clip may contribute text between the endpoints, but cannot start a selection outside that clip. Hidden subtrees are excluded. Nonselectable overlays block starting a selection on covered text. Each provider supplies a separator after its text; the default is a newline, and inline providers can return an empty separator.

To expose another element, implement `style::IVSelectable::text_layout()` and `select_text(optional<Range>)`, usually through `ISelectable`. Return stable source revisions, local cell coordinates, and invalidate the element's paint cache when the highlight changes. Returning null from `text_layout` opts out. The editing widgets retain their own focused selection and opt out of the document-wide selector, as browser input controls do. Their selection, scrolling and cursor use the same grapheme layout model.

Selection holds weak element references and source revisions. Content edits, removal, hiding, disabling selection or changes to provider order clear the selection. Reflow and movement preserve its logical anchors. The controller does not transform anchors through edits, auto-scroll while dragging, implement double-click word selection, or provide keyboard range extension.

## Editing widgets

`Input` and `MultiInput` use grapheme boundaries for cursor positions and edit ranges. `MultiInput::buffer()` still exposes byte-oriented storage; translate deliberately when crossing that boundary.

| API/data | Unit |
| --- | --- |
| `Input::jump`, `Input::cursor` | Grapheme boundary |
| `MultiInput::jump`, `cursor`, `erase`, `erase_at` | Grapheme position/count |
| `MultiInput::insert`, `append`, `value` | Original UTF-8 bytes |
| Viewport offsets, preferred vertical cursor column | Terminal cells |
| `FragmentedBuffer` positions and history | Bytes |

Arrow keys and Backspace/Delete move or erase whole graphemes. Home/End and Shift extend selection; MultiInput supports Ctrl+Home/End. Vertical navigation remembers its preferred cell column across short lines. Ctrl+A and MultiInput's Ctrl+Z/Ctrl+Y work without a clipboard module. Ctrl+C/X/V require one. ReadOnly blocks user edits but allows selection and programmatic mutation.

Single-line input displays hard newlines as spaces and preserves their source bytes. MultiInput recognizes CRLF as one editable grapheme, as well as CR, LF and Unicode line separators. Tabs occupy tab stops. Terminal input sequences are inserted as received; the widget does not apply byte-wise casing. Mouse hits use the same cell geometry as drawing, including the prefix and inserted cursor cell.

`Censor` displays one star per non-newline grapheme and uses that geometry for cursor movement, hit testing and scrolling. It is a display option, not secure memory storage: source/layout/history and clipboard operations may retain plaintext. `PtrOverlap` replaces the grapheme under the cursor; otherwise a separate cursor cell shifts the remainder of its line. Wide glyphs are never rendered as partial bytes or orphaned continuation cells.

MultiInput replacement of a selection is one undo operation. Undo/redo restore the edit's byte cursor, then snap to a valid grapheme boundary. This matters when inserting combining marks or a ZWJ merges adjacent clusters. An insertion lands after the resulting cluster. `value()` replacement clears history and selection; `append()` preserves the current cursor and viewport; `erase_at()` adjusts the cursor around the removed range. Disabling history clears past/redo entries so unrecorded edits cannot leave stale actions addressing unrelated text. History records individual edit operations; it does not coalesce successive keystrokes.

`FragmentedBuffer::act.replace(pos, len, text, cursor_after)` is the byte-level atomic history operation used by the widget. The optional final byte position overrides the default cursor after inserted text. Default construction creates an empty, usable byte buffer. Existing positional insert/erase overloads retain their cursor-preserving behavior. The correctly spelled `vertical_scrollbar()` accessor is available; `vertical_scollbar()` remains an alias.

The layout cache rebuilds the document after edits. It is not an incremental large-file editor. No bidi/shaping, IME preedit, soft-wrapped editing or cross-widget editable selection is provided.

Build with `D2_TEST=ON` and run `d2_selection` or `d2_editing` in a terminal. Ctrl+Q exits. Headless tests cover layout, extraction, selection lifecycle, editor events, editing/history and terminal bytes.

On POSIX systems, `python3 tests/selection_pty.py /path/to/build/d2_selection` runs the selection example in a disposable pseudo-terminal. `python3 tests/editing_pty.py /path/to/build/d2_editing` exercises real terminal input into both editing widgets. These check terminal bytes and process lifecycle; they do not verify a particular terminal's font rendering or system clipboard integration.
