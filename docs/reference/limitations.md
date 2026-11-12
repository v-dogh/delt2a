# Limitations and troubleshooting

[Documentation index](../index.md)

## Known boundaries

| Area | Current limit |
| --- | --- |
| Unicode cells | 159 thread-local extended tokens; exhaustion is explicit, no automatic palette reclamation |
| Text width | Grapheme/cell geometry uses Unicode tables; terminal font/emoji behavior can differ |
| Images | Nearest-neighbor scaling, cell-level text occlusion, one cached bounding composite |
| Image formats | PNG concrete loader; no animated playback or generic format discovery |
| Sixel | Fixed 216-color palette, no dithering, full viewport redraw, explicit real cell metrics when unavailable |
| Terminal support | POSIX backend; comprehensive terminal protocol/platform coverage is not established |
| Clipboard | Included backend is process-local, not the system clipboard |
| Notifications | Passive local messages; prompt handler and clear are unfinished |
| Constraint solver | `ConstraintArray::remove_constraint` is unfinished; do not rely on incremental removal |
| Inference | Chat Completions adapter; no Responses, embeddings, audio, MCP transport or Anthropic backend |
| Tools | Shape checks and optional validator callback, not a complete JSON Schema validator |
| Context/cost | Application tokenizer/compaction policy; no built-in tokenizer, price table or durable storage |
| Lifecycle | Cooperative cancellation; discarding a future is not a join |

API listings include legacy/experimental declarations. In particular `TaskRing` remains available but is unused by the framework. Audio, WebSocket and graph controls have less coverage than the core/editor/image/inference paths. Passing local tests does not certify every terminal, device or compatible model server.

## Common failures

**A module is inactive or missing.** Check the generated registry, optional dependency detection and application load manifest. Register dependencies explicitly. Query by the abstract key (`SystemImages`, `SystemImageLoader`, `SystemProvider`), even when you loaded a concrete type.

**A style setter fails to compile.** Include the widget's public header and use its symbolic property name. Match the declared property type. A bare numeric ID copied from another element can address a different interface. Build dependencies with compatible compiler/configuration settings.

**A value changes but rendering does not.** Use the property setter rather than direct storage writes. Custom drawing state must invalidate style/layout. Confirm `Display`, dimensions, clipping and cache policy.

**Text highlights appear in MultiInput but not Text.** Editors select internally. Generic Text selection requires `sys::selector`, selectable geometry and nonempty visible bounds. A higher painted element can block hit-testing. Clipboard copying additionally needs a clipboard backend.

**Image loading finishes but nothing appears.** Check renderer registration, terminal support, widget dimensions, crop bounds and `enabled`. Decoding can succeed without any renderer. Inspect the returned load future; `false` means application was superseded/cancelled or the target changed/disappeared.

**Sixel is misplaced.** Supply the real cell pixel width and height. A guessed Kitty sampling fallback cannot be used to establish correct Sixel placement. Reconfigure explicit metrics after changing terminal font size.

**An asynchronous operation hangs.** Ensure its scheduler/transport is running and its UI completion worker is being pumped. Never block the UI thread on a future that needs that UI thread. Keep owners alive and use service-specific cancellation before shutdown.

**The model endpoint rejects an option.** Confirm the complete `/chat/completions` URL, model name, token-limit field and configured compatibility profile. Use per-binding extensions only for fields the endpoint supports. “OpenAI-compatible” servers vary; the adapter surfaces errors instead of silently dropping requested settings.

**Sanitizer or dependency links fail.** Match Abseil debug/release headers and archives and the C++ runtime. MemorySanitizer needs instrumented dependencies. Sanitizer selection alone is not evidence of a clean run.
