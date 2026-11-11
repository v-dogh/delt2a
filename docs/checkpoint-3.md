# Checkpoint 3: grapheme-aware editing

This checkpoint builds on the core cleanup and shared text/selection checkpoints. It keeps the existing widget/style interfaces, byte-backed FragmentedBuffer and compact terminal cells. The remote `impl` branch has not been changed.

## Delivered

- Input and MultiInput now use shared grapheme layout for movement, selection, deletion, painting and hit testing. Public cursor/edit positions are grapheme indices; viewport coordinates are terminal columns.
- Combining sequences, emoji/ZWJ sequences, wide characters, tabs and CRLF are handled as complete editable units. Inserting text that merges adjacent clusters snaps the cursor to a resulting boundary.
- Prefixes and hints use the same renderer. Masking produces one star per grapheme. Inserted and overlapping cursors preserve wide-cell integrity, and an end-of-line cursor can scroll into view.
- MultiInput vertical navigation retains its preferred column across short lines. Shift+Up/Down retain the selection anchor. Input gains Home/End handling and a public cursor accessor; large jump values clamp safely.
- Selection replacement is one undoable operation. Byte-level history restores cursors, including append and positional erase behavior. Unrecorded edits cannot leave stale history: disabling history clears past and redo entries.
- FragmentedBuffer is usable immediately after construction. MultiInput no longer wipes preloaded content during element creation. Content changes invalidate automatic dimensions.
- Ctrl+A and MultiInput undo/redo no longer require a clipboard module. Terminal text is inserted verbatim rather than uppercasing individual bytes.
- The screen's global text-sequence notification no longer consumes the sequence before the focused widget receives it. Explicit consumption by listeners still works.
- Added `d2_editing`, a terminal example with both widgets, plus focused editor regressions and a real pseudo-terminal smoke test. `vertical_scrollbar()` fixes the accessor spelling while retaining the old alias.

## Compatibility

Code passing UTF-8 byte offsets into widget `jump`, `erase`, `erase_at` or interpreting `cursor()` as bytes must migrate. Use `text::Layout::byte_offset` / `grapheme_at` at byte-oriented integration boundaries. FragmentedBuffer remains byte-oriented. `MultiInput::is_synced()` is no longer noexcept because it may rebuild layout.

See [text and editing contracts](text-and-selection.md) for source ownership, selection, masking, coordinate units and history behavior. Input remains a lightweight single-line widget without undo history; MultiInput owns the editing history.

## Validation

All ten CTest groups passed with:

- Clang 19, Release, Unicode, examples enabled;
- GCC 14.2, Release, Unicode, examples enabled;
- Clang 19, Release, ASCII, examples enabled;
- Clang 19, ASan + UBSan, Unicode.

The editor tests cover keyboard events with consumption enabled, selection replacement, independent selection/undo shortcuts, read-only input, cursor/prefix/clipping rendering, tabs, CRLF, combining insertion, ZWJ merging, preferred vertical columns, history cursor restoration, history branching/disable, and 100 edits across buffer shard boundaries followed by full undo/redo replay.

The installed-package consumer constructs both editors and exercises MultiInput edits/history against the installed headers and libraries. The editing pseudo-terminal smoke test passes in Unicode and ASCII: it clicks both widgets, types text, selects and replaces a range, undoes/redoes without a clipboard module, and exits cleanly. The existing selection pseudo-terminal smoke test also passes.

Validation logs and incremental/cumulative patches are included in the bundle. ASan ran with leak detection disabled because LeakSanitizer is incompatible with this environment's tracing. No clean ThreadSanitizer result is claimed; checkpoint 1's queue diagnostic remains unresolved. PipeWire and Doxygen were not exercised. Pseudo-terminal tests inspect bytes and lifecycle, not font rendering or a desktop clipboard.

## Limits and next work

Layout is cached but rebuilt after edits; this is not an incremental large-document editor. The existing fragmented buffer still flattens on deletion. Width follows the shared Unicode policy; bidi, shaping, IME preedit and soft-wrapped editing are outside this checkpoint. Editors own their focused selections and do not participate in the cross-element document selector.

The next stage is image representation, composition/clipping and optional terminal backends. OpenAI-compatible inference remains the final addition after the core work. Palette ownership/reclamation and broader terminal input protocol coverage remain documented limits.
