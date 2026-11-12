# OpenAI-compatible provider

[Documentation index](../index.md)

`sys::SystemOpenAI` implements `sys::SystemProvider` under `os/inference/openai/`, with its own CMake module registration and `DeltaAscii::OpenAI` target. Provider-neutral conversations, tools and types remain under `core/mods/inference/`.

Networking uses `sys::net::SystemHTTP`. The existing curl multi worker owns connections, streaming, deadlines and cancellation; the provider adds no network threads or HTTP runtime. It reuses nlohmann/json and ada already used by the framework. The bounded SSE decoder is local to this backend. CPR and EventSource clients were considered, but their additional transport/client lifecycle did not simplify integration with SystemHTTP.

## Setup

Include `<os/inference/openai/d2_openai.hpp>` and `<core/mods/inference/d2_std.hpp>`. Link `DeltaAscii::OpenAI` and `DeltaAscii::Core`. The aggregate also includes registered backends. The module is omitted with `D2_EXCLUDE_LIBRARY_INFERENCE=ON`; exclude just this backend with `D2_MODULE_EXCLUDE_DIRS=inference`. A custom HTTP implementation can replace curl.

```cpp
ctx->load<d2::sys::net::SystemHTTP, d2::sys::SystemOpenAI,
          d2::sys::SystemConversations>();
auto provider = ctx->sys<d2::sys::SystemProvider>()->ptr<d2::sys::SystemOpenAI>();

d2::ai::openai::Config endpoint;
endpoint.endpoint = "http://localhost:8080/v1/chat/completions";
endpoint.compatibility.token_limit_field = "max_tokens";
// Set endpoint.api_key or custom authentication headers when required.
provider->configure(std::move(endpoint));

auto conversations = ctx->sys<d2::sys::SystemConversations>();
conversations->bind("local", provider.ptr());
d2::ai::ConversationConfig config;
config.model = {"local", "your-model"};
auto chat = conversations->create("work", config);
chat->append(d2::ai::Message::text(d2::ai::Role::User, "Hello"));
auto generation = chat->step({}, [](const d2::ai::Event& event) { /* consume events */ });
auto response = co_await generation.result;
```

Load/register the HTTP module before the provider. IOContext queries use the abstract module key (`SystemProvider`), followed by the existing typed `ptr<SystemOpenAI>()` accessor after loading that implementation. No screen, elements, terminal or UI loop is needed. A headless main thread can wait on a future; a UI callback should await or schedule work instead.

For a second endpoint, use `ctx->load<SystemOpenAI>("remote")` and `ctx->sys<SystemProvider>("remote")->ptr<SystemOpenAI>()`, then bind that instance under a distinct conversation provider name. Each instance has its own configuration; all can share the HTTP backend.

## Configuration

`endpoint` is the complete Chat Completions URL, not a base URL. Defaults target `https://api.openai.com/v1/chat/completions`. `api_key` is optional and becomes a bearer header when set. `headers` supports organization/project/routing or alternative authentication headers. Duplicate header names, control characters, and attempts to override managed framing/content headers are rejected. Redirects are disabled; configure the final endpoint URL.

Defaults are a 5-minute request timeout, 10-second connection timeout, 32 MiB response limit, 1 MiB SSE line/event limit and 64 KiB header limit. The earlier conversation/request deadline wins. Limits and timeouts must be positive. Configuration is copied per request: reconfiguration affects subsequent requests, not an operation already submitted. Capability checks are repeated against that snapshot. `config()` includes credentials and should not be logged.

| Compatibility option | Default | Purpose |
| --- | --- | --- |
| `stream` | `true` | Use SSE; set false for a buffered JSON response |
| `include_usage` | `true` | Request streamed usage statistics |
| `token_limit_field` | `max_completion_tokens` | Select this or `max_tokens` for output limits |
| `developer_as_system` | `false` | Explicitly map developer messages to the system role |
| `extended_sampling` | `false` | Enable top_k/min_p/repeat_penalty and relaxed temperature/penalty ranges |
| `reasoning_field` | empty | Opt into `reasoning_content` or `reasoning` string state |
| `require_done` | `true` | Require `[DONE]`; false still requires a finish reason and complete SSE frames |

`Config::models[model]` supplies `ModelInfo` and an optional replacement `Compatibility`. Unspecified models use `default_model` and endpoint compatibility. Limits and features are unknown unless configured; no hard-coded model catalogue or automatic capability discovery is used. A per-model compatibility object replaces the entire endpoint compatibility object, so copy the defaults before changing individual fields.

Sampling and output settings come from the conversation or its `StepOptions`. Binding-scoped `Settings::extensions` adds body fields such as `chat_template_kwargs`, `logprobs`, or routing options. It cannot override model, messages, tools, streaming, typed sampling/output fields, metadata or credentials. Audio output is unsupported and rejected. Provider-specific options are sent as supplied; server rejection is surfaced. There is no arbitrary request mutation callback.

The adapter supports text messages, user image URLs/data URIs, custom function tools and results, forced/parallel tool settings, JSON object/schema output, stop sequences and configured reasoning effort. Tool output schemas remain application metadata because Chat Completions has no corresponding tool-output schema field. Tool results are JSON-serialized strings; error results are wrapped as `{"error": value}`. Images use their URI; an optional image media type is checked as `image/*` but has no separate wire field.

## Streaming, tools and model switching

SSE framing supports LF, CRLF, CR, a leading BOM, comments, multiple data lines and arbitrary byte boundaries, including UTF-8 split across HTTP chunks. It does not reconnect automatically or replay requests. Successful streaming responses must have an event-stream content type. Malformed JSON, truncated events, missing finish/terminator, changing call IDs, invalid argument JSON, unknown tools and incompatible final tool choices fail the generation.

Tool calls are assembled by their wire index. Public event block indices remain stable even when calls are interleaved with text. Tool argument deltas are incomplete strings; only the final validated response is eligible for execution. The adapter never invokes tools. Use `ToolSet::execute()` and `Conversation::add_tool_result()` as described in [inference.md](inference.md).

Reasoning string fields require an explicit compatibility mapping. Mapped reasoning is emitted as `OpaqueDelta` and retained as model/binding-scoped `Opaque` content for replay. Unconfigured nonempty reasoning fields fail explicitly. The conversation's existing `StateTransfer::DropOpaque` option can strip incompatible state when switching models; pinned messages remain protected. Text and completed tool exchanges otherwise survive a switch. This does not implement Responses API continuation tokens or arbitrary provider-specific opaque formats.

One generation returns one choice (`n=1`). Usage-only final chunks are accepted. Missing token counts remain unknown. Response usage preserves native details, first-output timing measures the first content/tool/reasoning event, and total timing covers the provider operation. An `x-request-id` header takes precedence over the completion ID; the latter is then retained in `Response::native["completion_id"]`. Buffered responses retain native choices; streaming responses retain additional choice metadata such as logprobs. Unknown native fields are diagnostic data, not automatically replayable context.

## Errors and lifetime

`ai::Error` distinguishes cancellation, timeout, HTTP transport failures, authentication, rate limits, protocol errors, provider errors and recognized context-limit errors. HTTP errors carry status, request ID, parsed native error data and `Retry-After` when present (seconds or an HTTP date). Malformed non-JSON error bodies retain a bounded excerpt. Handler exceptions propagate through the future. Failed generations are not committed to conversation history.

Requests retain callback state and the HTTP transport until terminal delivery. Stop requests cancel the HTTP control handle, including with a silent server. Provider unload closes admission and requests cancellation for all active generations without blocking the callback thread. Await outstanding futures before assuming cancellation is settled; the HTTP worker delivers their terminal callbacks. Reload can accept new requests while older cancelled operations settle independently. No events are emitted after the operation's future is settled. Event handlers share the HTTP worker, so blocking them delays other requests and cooperative cancellation.

No automatic retries are performed. The caller decides whether a failed generation can be repeated, particularly after streaming output. MCP, an autonomous tool loop, tokenization, pricing and persistent conversation storage remain separate concerns.

## Example and validation

With `D2_TEST=ON`, `d2_inference` is a headless example with an `add` tool, a coroutine-driven bounded tool loop, streaming output, and optional model switching:

```sh
D2_OPENAI_TOKEN_FIELD=max_tokens ./build/d2_inference \
  http://localhost:8080/v1/chat/completions your-model 'Use add to compute 2 + 3'
```

`D2_OPENAI_API_KEY` and `D2_OPENAI_REASONING_FIELD` are optional environment settings. A fourth positional argument after the prompt selects a second model for a follow-up. The example only runs when invoked; building and tests do not contact an external model service.

Run `python3 tests/openai_example.py /path/to/build/d2_inference` for a local smoke test of the example's tool round trip and model switch.

`d2_openai` tests serialization, compatibility, framing, interleaved calls, usage, errors, reasoning replay and synchronous transport failure. `d2_openai_http` uses loopback sockets and the actual SystemHTTP curl backend for conversation/tool/model-switch round trips, cancellation/deadlines, callback errors, unload/reload, concurrent requests, independent bindings and configuration snapshots. These tests do not establish interoperability with every endpoint calling itself OpenAI-compatible. Live provider and TLS/proxy interoperability still need endpoint-specific testing.

Protocol references: [OpenAI Chat Completions](https://developers.openai.com/api/reference/resources/chat/subresources/completions/methods/create), [streaming events](https://developers.openai.com/api/reference/resources/chat/subresources/completions/streaming-events), and [WHATWG event-stream framing](https://html.spec.whatwg.org/multipage/server-sent-events.html#parsing-an-event-stream).
