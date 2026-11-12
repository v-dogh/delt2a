# Inference and conversations

[Documentation index](../index.md)

The provider-neutral layer supplies types, an abstract provider module and a concrete conversation module. The [OpenAI-compatible provider](openai.md) uses the existing HTTP module.

Include `<core/mods/inference/d2_std.hpp>` and link `DeltaAscii::Inference` (also included by the aggregate `DeltaAscii::Core`). The public namespace is `d2::ai`; runtime modules are in `d2::sys`. Inference can be disabled with `D2_EXCLUDE_LIBRARY_INFERENCE=ON`. It does not require elements, a screen or a terminal backend.

## Responsibilities

| Type | Owns |
| --- | --- |
| `sys::SystemProvider` | Model capabilities and asynchronous generation contract |
| `sys::SystemConversations` | Named provider bindings and named conversation lifetimes |
| `ai::Conversation` | Current model/settings, transcript, active context, pending calls and request records |
| `ai::ToolSet` | Immutable tool definitions, optional argument validators and execution callbacks |
| `ai::ContextPolicy` | Application token estimation and asynchronous context replacement |

A provider binding identifies one configured provider instance, for example `local` or `hosted`. It is separate from the model name and from the backend implementation type. Bindings cannot be overwritten: use another name for another endpoint/account. Endpoint URLs, credentials, HTTP headers and transport configuration belong to the provider implementation, not to conversation messages.

Load the conversation module through `IOContext`, which supplies shared ownership. When constructing it manually with `SystemModule::make`, promote the returned `unique_ptr` to `shared_ptr` before creating sessions. A standalone `Conversation` likewise requires shared ownership before `step()`.

`bind()` retains a shared provider reference; it does not load or unload that provider. Load concrete providers through their owning `IOContext` (or explicitly through `load()`) before use. Bindings are dynamic references, not registry dependency declarations. Close/settle conversations before explicitly unloading a provider; provider shutdown must still settle outstanding requests if it happens first during teardown.

## Creating and switching a conversation

After loading a provider (see [OpenAI setup](openai.md)):

```cpp
ctx->load<d2::sys::SystemConversations>();
auto conversations = ctx->sys<d2::sys::SystemConversations>();
conversations->bind("local", provider);

d2::ai::ConversationConfig config;
config.model = {"local", "model-a"};
config.settings.sampling.temperature = 0.6;
config.settings.sampling.max_output_tokens = 2048;
auto chat = conversations->create("research", config);
chat->append(d2::ai::Message::text(d2::ai::Role::System, "Instructions", true));
chat->append(d2::ai::Message::text(d2::ai::Role::User, "First question"));

auto generation = chat->step();
// In a coroutine; do not block the UI thread with result.value().
auto reply = co_await generation.result;

chat->switch_model({"local", "model-b"});
chat->append(d2::ai::Message::text(d2::ai::Role::User, "Continue"));
auto next = co_await chat->step().result;
```

Switching preserves text, images and completed tool exchanges. The new model's reported capabilities are checked before committing the change. A failed switch leaves both configuration and context unchanged. The request's selected model is recorded on new assistant messages; earlier messages keep their original provenance. A provider may additionally report the actual serving model in `Response::reported_model`.

Only an idle conversation with all tool calls answered can switch. During a generation, cancel and await settlement before switching. Pending tools can be completed with ordinary or error results. Changing configuration never cancels an in-flight operation implicitly.

`Opaque` content holds provider-specific continuation/reasoning state. It is conservatively scoped to the exact binding and model. A normal switch rejects incompatible state. `StateTransfer::DropOpaque` explicitly strips incompatible opaque blocks from active context, preserving them in the transcript; it refuses to strip pinned messages. The adapter must attach replay-required opaque state to the message, not merely to `Response::native`.

```cpp
chat->switch_model({"hosted", "other-model"}, d2::ai::Settings{},
                   d2::ai::StateTransfer::DropOpaque);
```

The optional settings argument replaces the full settings object. Omit it to retain settings. `configure()` replaces the full conversation configuration. `StepOptions::settings` replaces settings for one request only. Neither API performs an implicit JSON merge.

## Settings and capabilities

Typed settings cover sampling, output token limits, stop sequences, reasoning effort, tool choice, parallel tool calls and JSON/schema output. An unset optional means “use the provider default”; it is distinct from zero or false. Provider-specific settings live in `settings.extensions[binding]`, a JSON object. Only the selected binding's object reaches `_generate_impl`. It persists across model changes within that binding; replace settings when it should not carry over.

`model_info()` returns optional limits and three-state feature support. Known unsupported features fail locally. Unknown features are allowed by default for custom endpoints, or rejected with `UnknownCapabilities::Reject`. This is a preflight check, not a promise that an endpoint accepts a request. Adapters must reject unsupported options explicitly and must not silently discard settings. They also enforce endpoint-specific numeric ranges and roles. Extension options must not override protected request fields or transport credentials accidentally.

`Image` is a URI plus optional media type, independent of terminal image rendering. `Opaque` and namespaced JSON extensions leave space for provider-specific features without pretending that all providers share them. Audio, embeddings, batch jobs and provider-side session APIs are not modeled here.

## Custom tools

Tools declare input/output JSON schemas and optional strict generation behavior. `Tool::invoke` accepts owned JSON and a stop token, returning `mt::future<Json>`. It can await existing framework facilities. Blocking work should be scheduled through the task pool; calling `execute()` does not automatically move work off the calling thread.

```cpp
d2::ai::Tool tool;
tool.definition = {"add", "Add two integers", {
    {"type", "object"},
    {"properties", {{"a", {{"type", "integer"}}}, {"b", {{"type", "integer"}}}}},
    {"required", {"a", "b"}}
}};
tool.invoke = [](d2::ai::Json args, std::stop_token) -> mt::future<d2::ai::Json>
{
    co_return args.at("a").get<int>() + args.at("b").get<int>();
};
auto tools = std::make_shared<d2::ai::ToolSet>(std::vector<d2::ai::Tool>{std::move(tool)});
auto config = chat->snapshot().config;
config.tools = tools;
chat->configure(std::move(config));
```

One `step()` makes one generation request. A tool-call response is committed as an assistant message and blocks the next step until every call has a matching result:

```cpp
for (const auto& [id, call] : chat->pending_tools())
    chat->add_tool_result(co_await tools->execute(call));
auto answer = co_await chat->step().result;
```

Results may arrive in any order, but IDs must match and cannot repeat. Multiple tools can be started concurrently when the application chooses; the loop above is sequential. Tools without local callbacks may be executed externally and returned through `add_tool_result()`. A later MCP bridge can use this route without changing conversation ownership.

The framework checks schema shape and call/result structure; it does **not** implement a JSON Schema validator. Use `Tool::validate` for application validation or integration with a schema library. `strict` is a provider request, not a substitute for local validation. Ordinary callback exceptions become error tool results; cancellation remains an exception. Unknown tool names fail explicitly. No automatic retry or autonomous tool loop is provided, so side effects remain under application control.

## Context management

`snapshot().transcript` retains appended messages, tool results and successful assistant responses. `snapshot().context` is the current request window. Compaction and `replace_context()` modify only that window. New summary/retrieval messages receive fresh IDs but are not added to the transcript. Streaming fragments and failed responses are not committed.

`ContextPolicy::count` estimates the complete request, including tool schemas and other overhead relevant to the selected model. Its result distinguishes an exact count from an estimate. No universal tokenizer or model-size table is built in.

When a policy is configured, the input budget is the smaller of `max_input_tokens` and the model's reported context window minus output reservation. Explicit `max_output_tokens` takes precedence over the policy's reservation. If neither input limit nor model window is known, no budget check is possible. Without a context policy, the provider is responsible for context overflow errors.

When the estimate exceeds `floor(budget * compact_at)`, `compact` receives the full request, count and target. It returns a replacement message vector asynchronously, using the same cancellation token. It can summarize, select history, retrieve context, or call a separate provider. It cannot mutate its busy conversation. The result must fit the target, preserve pinned messages exactly, and retain complete tool exchanges. Existing messages keep their IDs; newly created messages use ID zero. Compaction is attempted once per step; failure or cancellation keeps the original active context.

`replace_context(messages)` supplies an explicit replacement while idle, for retrieval or application-controlled editing. Pinned messages are preserved by default; passing `false` explicitly permits replacing them. Existing IDs must refer to unchanged known messages. Manual replacement validates history/capabilities; budgeting happens on the next step.

`snapshot()` and `fork()` are in-memory operations. Snapshots include configuration callbacks and shared tool sets; they are not a durable serialization format. `module->restore(name, snapshot)` registers a new conversation. `fork()` creates a detached conversation: register a snapshot through the module if module-owned cancellation is required. A detached fork shares the resolver and needs the originating module to resolve providers; an already-started request retains its provider until settlement.

## Streaming, cancellation and metrics

`step(options, handler)` returns a `Generation` with a consumable result future and `cancel()`. Events carry text deltas, tool-argument fragments, opaque deltas or usage snapshots. Block indices identify content blocks. Fragments may be incomplete JSON; only the final assembled response is eligible for tool execution. The final response is authoritative and is appended once.

Callbacks are serialized per request by the provider, run outside conversation/module locks and may run on a transport/completion thread. They must copy borrowed event data before retaining it. UI updates must be marshalled through `IOContext::sync_async`. Separate conversations/providers can run concurrently; an individual conversation permits one active step. Tool callbacks shared between conversations must manage their own concurrency.

Cancellation is cooperative. The provider and compaction callback must honor the token and settle their futures. A timeout supplies a deadline and is checked at stage boundaries; this layer does not own a timer that interrupts arbitrary callbacks. Providers must enforce deadlines during I/O. Dropping a future is not cancellation or a join, and `discard()` is not a substitute for `Generation::cancel()`. Use one consumer per future, following the core future contracts.

Erasing a registered conversation or unloading `SystemConversations` closes it and requests cancellation. Completion after close/cancellation does not commit a response. The outstanding operation retains its conversation and provider until settlement. Provider unload behavior must cancel and settle its own outstanding I/O.

`Record` stores the selected model, reported model/request ID, optional usage, finish/error and elapsed step time. Missing token counters remain unknown, never fabricated as zero. Step total includes compaction; first-output timing comes from the provider. Provider-native metrics remain in `Usage::native`; response-native fields are available to the direct caller. There is no built-in pricing table, cost estimate, retry engine or metrics exporter. Compaction/tool execution costs are tracked by their implementation, not silently added to generation usage.

## Implementing a provider

Derive from `sys::SystemProvider` and the framework's `ModuleImpl<Concrete, Load::...>` pattern. Implement `model_info(model)` and `_generate_impl(Request, EventHandler, stop_token)`. `generate()` performs generic request validation and extension filtering before delegation. The conversation also validates final response structure, tool choice and opaque-state scope before commit. Direct provider callers can use `validate_response()` themselves.

An implementation must:

- Be safe for concurrent requests and capability queries; retain request/callback resources until settlement.
- Map normalized settings and content faithfully, rejecting unsupported combinations.
- Assemble complete messages and tool calls from transport frames, assign stable call IDs, and return typed errors on malformed streams or failed requests.
- Serialize events for each request, propagate handler failures, and stop emitting before settling the result.
- Honor cancellation/deadlines, provide final usage when available and retain required provider continuation data in scoped content.

The native OpenAI-compatible implementation uses SystemHTTP and lives under `os/inference/openai/` with a dedicated CMake target. No Anthropic adapter, MCP client or external inference SDK is required. JSON data uses nlohmann/json; asynchronous results and module lifetime use the existing DeltaAscii facilities. Expired deadlines report `ErrorCode::Timeout`, separately from explicit cancellation.
