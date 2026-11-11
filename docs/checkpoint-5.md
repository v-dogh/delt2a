# Checkpoint 5: inference contracts and conversations

This checkpoint includes checkpoints 1–4 and adds the provider-neutral layer requested before choosing an OpenAI implementation. No remote branch was pushed and no concrete network provider is included.

## Delivered

- `DeltaAscii::Inference`, an optional component with explicit headers and nlohmann/json data. The framework's existing modules and futures remain the lifetime/asynchronous mechanisms.
- Abstract `SystemProvider` with model capabilities, normalized requests/responses, streaming events, usage, deadlines, cancellation and typed errors.
- Concrete `SystemConversations` with named provider bindings and conversations; standalone shared conversations are also supported.
- Model/provider switching within a conversation, including capability checks, retained message provenance and explicit handling of incompatible opaque state. Failed changes do not mutate the conversation.
- Typed sampling, output format/schema, tool choice and reasoning settings, plus JSON options scoped to provider bindings. Request overrides replace settings explicitly.
- Custom tool definitions, optional validation callbacks, asynchronous execution and matching results. Incomplete, duplicate and unknown tool calls/results are rejected; execution remains application-controlled.
- Separate transcript and active context, asynchronous compaction policy, model-aware input budgets/output reservation, pinned messages and manual context replacement.
- In-memory snapshots, forks/restoration and per-step records with optional token counters and timing. Unknown usage remains unknown.
- Cooperative cancellation and close/unload behavior, with no assistant/context commit on failed or cancelled steps.

See [inference contracts](inference.md) for API examples, ownership, scheduler behavior and extension points. Comments in the implementation stay sparse; the Markdown contract is the primary design reference and is included in Doxygen's inputs.

## Validation

All thirteen CTest groups pass with Clang 19 Release/Unicode, GCC 14.2 Release/Unicode, Clang Release/ASCII with PNG disabled, and Clang ASan+UBSan/Unicode. The inference group uses scripted providers and covers preserved history during model switches, unsupported settings, scoped opaque state, tool exchange/validation, context compaction/replacement, snapshot restoration, malformed provider responses, expired deadlines, cancellation during generation and compaction, and module unload during an active request.

The installed-package consumer builds and runs using the exported inference target and JSON dependency alongside existing image/text/editor APIs. A separate build with inference and elements disabled builds/runs the public consumer without fetching JSON. The provider-neutral inference test also builds/runs with elements disabled.

These checks use no credentials, actual model endpoint or network stream. They validate the abstraction, not OpenAI compatibility. ASan/UBSan runs with leak detection disabled because of environment tracing. No clean TSan result is claimed; the earlier queue diagnostic remains unresolved. Existing compiler warnings outside the new inference code remain. Doxygen and PipeWire were not exercised; terminal PTY checks from checkpoint 4 were not rerun for this inference-only addition.

## Deliberate limits and next step

The next implementation is an OpenAI-compatible provider. Its HTTP transport, endpoint dialect, streaming parser, tool-call assembly and configuration are still to be selected. No external inference SDK has been adopted.

There is no automatic tool loop/retry policy, MCP client, Anthropic provider, built-in tokenizer, full JSON Schema validator, durable snapshot format, pricing table or autonomous context summarizer. The generic contracts expose callbacks and owned data for those additions. Cancellation/deadlines require cooperation from providers and compaction callbacks; the conversation layer cannot forcibly stop arbitrary application code. Context budgeting needs an application estimator and known/configured limits.

Bindings retain providers but do not manage their load/unload lifecycle. Modules own registered conversations; detached forks have explicit independent cancellation lifetime. These distinctions are documented rather than hidden behind implicit background work.
