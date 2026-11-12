#include "d2_openai_protocol.hpp"
#include <ada.h>
#include <algorithm>
#include <charconv>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

namespace d2::ai::openai::detail
{
    namespace
    {
        [[noreturn]] void invalid(std::string message)
        { throw Error(ErrorCode::InvalidRequest, std::move(message)); }
        [[noreturn]] void protocol(std::string message)
        { throw Error(ErrorCode::Protocol, std::move(message)); }
        std::string lower(std::string value)
        {
            for (char& c : value) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
            return value;
        }
        bool control(std::string_view value)
        { return std::any_of(value.begin(), value.end(), [](unsigned char c) { return c < 32 || c == 127; }); }
        void compatibility(const Compatibility& wire)
        {
            if (wire.token_limit_field != "max_tokens" && wire.token_limit_field != "max_completion_tokens")
                invalid("Token limit field must be max_tokens or max_completion_tokens");
            if (!wire.reasoning_field.empty() && wire.reasoning_field != "reasoning_content" && wire.reasoning_field != "reasoning")
                invalid("Unsupported reasoning field mapping");
        }
        Json parse(std::string_view text)
        {
            try { return Json::parse(text); }
            catch (const Json::exception&) { protocol("Invalid JSON in provider response"); }
        }
        std::string string(const Json& object, std::string_view key, bool required = false)
        {
            auto it = object.find(key);
            if (it == object.end() || it->is_null())
            {
                if (required) protocol("Missing response field: " + std::string(key));
                return {};
            }
            if (!it->is_string()) protocol("Expected response string: " + std::string(key));
            return it->get<std::string>();
        }
        std::optional<std::uint64_t> number(const Json& object, std::string_view key)
        {
            auto it = object.find(key);
            if (it == object.end() || it->is_null()) return {};
            if (!it->is_number_integer() || (!it->is_number_unsigned() && it->get<std::int64_t>() < 0))
                protocol("Expected nonnegative response integer: " + std::string(key));
            return it->get<std::uint64_t>();
        }
        std::size_t index(const Json& object)
        {
            auto value = number(object, "index");
            if (!value || *value > std::numeric_limits<std::size_t>::max()) protocol("Invalid response index");
            return static_cast<std::size_t>(*value);
        }
        const char* role(Role role, const Compatibility& wire)
        {
            switch (role)
            {
            case Role::System: return "system";
            case Role::Developer: return wire.developer_as_system ? "system" : "developer";
            case Role::User: return "user";
            case Role::Assistant: return "assistant";
            case Role::Tool: return "tool";
            }
            invalid("Invalid message role");
        }
    }

    void validate_config(const Config& config)
    {
        auto url = ada::parse<ada::url_aggregator>(config.endpoint);
        if (!url || control(config.endpoint) || config.endpoint.find(' ') != std::string::npos ||
            (url->get_protocol() != "http:" && url->get_protocol() != "https:") ||
            url->get_hostname().empty() || !url->get_username().empty() || !url->get_password().empty() ||
            !url->get_hash().empty()) invalid("Expected an HTTP(S) endpoint without userinfo or fragment");
        if (control(config.api_key)) invalid("Invalid API key characters");
        std::set<std::string> names;
        for (const auto& [name, value] : config.headers)
        {
            const auto key = lower(name);
            if (name.empty() || !std::all_of(name.begin(), name.end(), [](unsigned char c)
                { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                    std::string_view("!#$%&'*+-.^_`|~").find(c) != std::string_view::npos; }) ||
                control(value) || !names.insert(key).second) invalid("Invalid or duplicate HTTP header");
            if (key == "host" || key == "content-length" || key == "transfer-encoding" ||
                key == "content-type" || key == "accept" || (key == "authorization" && !config.api_key.empty()))
                invalid("HTTP header is managed by the provider: " + name);
        }
        const auto max_timeout = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::duration::max() / 2);
        if (config.timeout.count() <= 0 || config.timeout > max_timeout || config.connect_timeout.count() <= 0 ||
            !config.max_response_bytes || !config.max_event_bytes || !config.max_header_bytes)
            invalid("Timeouts and response limits must be positive");
        compatibility(config.compatibility);
        for (const auto& [name, model] : config.models)
        {
            if (name.empty()) invalid("Empty model configuration name");
            if (model.compatibility) compatibility(*model.compatibility);
        }
    }

    Json encode(const Request& request, const Compatibility& wire)
    {
        compatibility(wire);
        if (request.messages.empty()) invalid("Chat Completions requires at least one message");
        Json messages = Json::array();
        for (const auto& message : request.messages)
        {
            if (message.role == Role::Tool)
            {
                for (const auto& block : message.content)
                {
                    const auto& result = std::get<ToolResult>(block);
                    messages.push_back({{"role", "tool"}, {"tool_call_id", result.call_id},
                        {"content", result.error ? Json{{"error", result.value}}.dump() : result.value.dump()}});
                }
                continue;
            }
            Json out{{"role", role(message.role, wire)}}, parts = Json::array(), calls = Json::array();
            std::string text;
            bool images = false;
            for (const auto& block : message.content)
            {
                if (const auto* value = std::get_if<Text>(&block))
                {
                    text += value->value;
                    parts.push_back({{"type", "text"}, {"text", value->value}});
                }
                else if (const auto* image = std::get_if<Image>(&block))
                {
                    if (message.role != Role::User || image->uri.empty())
                        invalid("Chat Completions images require a user message and a URI");
                    if (!image->media_type.empty() && !image->media_type.starts_with("image/"))
                        invalid("Image media type must be image/*");
                    images = true;
                    parts.push_back({{"type", "image_url"}, {"image_url", {{"url", image->uri}}}});
                }
                else if (const auto* call = std::get_if<ToolCall>(&block))
                    calls.push_back({{"id", call->id}, {"type", "function"},
                        {"function", {{"name", call->name}, {"arguments", call->arguments.dump()}}}});
                else if (const auto* opaque = std::get_if<Opaque>(&block))
                {
                    if (message.role != Role::Assistant || wire.reasoning_field.empty() ||
                        opaque->kind != "openai." + wire.reasoning_field || !opaque->value.is_string() ||
                        out.contains(wire.reasoning_field))
                        throw Error(ErrorCode::IncompatibleState, "Opaque state has no configured OpenAI mapping");
                    out[wire.reasoning_field] = opaque->value;
                }
            }
            out["content"] = images ? parts : Json(text);
            if (!calls.empty()) { out["tool_calls"] = std::move(calls); if (text.empty()) out["content"] = nullptr; }
            messages.push_back(std::move(out));
        }
        Json out{{"model", request.model.name}, {"messages", std::move(messages)}, {"stream", wire.stream}, {"n", 1}};
        if (wire.stream && wire.include_usage) out["stream_options"] = {{"include_usage", true}};
        if (!request.metadata.empty()) out["metadata"] = request.metadata;
        const auto& settings = request.settings;
        const auto& sample = settings.sampling;
        auto put = [&](const char* key, const auto& value) { if (value) out[key] = *value; };
        if (sample.temperature && *sample.temperature > 2 && !wire.extended_sampling)
            invalid("OpenAI temperature must be in [0, 2]");
        for (auto penalty : {sample.frequency_penalty, sample.presence_penalty})
            if (penalty && (*penalty < -2 || *penalty > 2) && !wire.extended_sampling)
                invalid("OpenAI penalties must be in [-2, 2]");
        if (!wire.extended_sampling && (sample.min_p || sample.top_k || sample.repeat_penalty))
            throw Error(ErrorCode::Unsupported, "Extended sampling requires explicit endpoint compatibility");
        put("temperature", sample.temperature); put("top_p", sample.top_p);
        put("frequency_penalty", sample.frequency_penalty); put("presence_penalty", sample.presence_penalty);
        put("seed", sample.seed); put("top_k", sample.top_k); put("min_p", sample.min_p);
        put("repeat_penalty", sample.repeat_penalty); put(wire.token_limit_field.c_str(), sample.max_output_tokens);
        if (!sample.stop.empty()) out["stop"] = sample.stop;
        put("reasoning_effort", settings.reasoning_effort);
        if (!request.tools.empty())
        {
            out["tools"] = Json::array();
            for (const auto& tool : request.tools)
            {
                Json function{{"name", tool.name}, {"description", tool.description}, {"parameters", tool.input_schema}};
                if (tool.strict) function["strict"] = true;
                out["tools"].push_back({{"type", "function"}, {"function", std::move(function)}});
            }
        }
        if (settings.parallel_tools && request.tools.empty()) invalid("Parallel tool setting requires tools");
        put("parallel_tool_calls", settings.parallel_tools);
        switch (settings.tool_choice.kind)
        {
        case ToolChoice::Kind::Auto: if (!request.tools.empty()) out["tool_choice"] = "auto"; break;
        case ToolChoice::Kind::None: out["tool_choice"] = "none"; break;
        case ToolChoice::Kind::Required: out["tool_choice"] = "required"; break;
        case ToolChoice::Kind::Named:
            out["tool_choice"] = {{"type", "function"}, {"function", {{"name", settings.tool_choice.name}}}}; break;
        }
        if (settings.output.kind == OutputFormat::Kind::JsonObject)
            out["response_format"] = {{"type", "json_object"}};
        if (settings.output.kind == OutputFormat::Kind::JsonSchema)
            out["response_format"] = {{"type", "json_schema"}, {"json_schema", {
                {"name", settings.output.name}, {"schema", settings.output.schema}, {"strict", settings.output.strict}}}};
        static const std::set<std::string> reserved{
            "model", "messages", "stream", "stream_options", "n", "tools", "tool_choice", "functions", "function_call",
            "parallel_tool_calls", "response_format", "max_tokens", "max_completion_tokens", "temperature", "top_p",
            "top_k", "min_p", "frequency_penalty", "presence_penalty", "repeat_penalty", "seed", "stop",
            "reasoning_effort", "metadata", "api_key", "headers", "base_url", "endpoint"};
        if (const auto it = settings.extensions.find(request.model.provider); it != settings.extensions.end())
            for (const auto& [key, value] : it->second.items())
            {
                if (reserved.contains(key)) invalid("Extension overrides a managed request field: " + key);
                if (key == "audio" || (key == "modalities" && value != Json::array({"text"})))
                    throw Error(ErrorCode::Unsupported, "This provider supports text output only");
                out[key] = value;
            }
        return out;
    }

    Error provider_error(int status, std::string_view body)
    {
        auto code = status == 401 || status == 403 ? ErrorCode::Authentication :
            status == 429 ? ErrorCode::RateLimit : status == 408 || status == 504 ? ErrorCode::Timeout :
            status == 400 || status == 422 ? ErrorCode::InvalidRequest : ErrorCode::Provider;
        auto native = Json::parse(body, nullptr, false);
        std::string message = "Provider HTTP status " + std::to_string(status);
        if (native.is_object() && native.contains("error"))
        {
            const auto& error = native["error"];
            if (error.is_string()) message = error.get<std::string>();
            else if (error.is_object())
            {
                if (auto it = error.find("message"); it != error.end() && it->is_string()) message = it->get<std::string>();
                if (auto it = error.find("code"); it != error.end() && it->is_string())
                {
                    const auto name = it->get<std::string>();
                    if (name == "context_length_exceeded" || name == "context_window_exceeded") code = ErrorCode::ContextLimit;
                    else if (name == "rate_limit_exceeded") code = ErrorCode::RateLimit;
                    else if (name == "invalid_api_key") code = ErrorCode::Authentication;
                }
            }
        }
        Error error(code, message.substr(0, 4096));
        error.http_status = status;
        error.native = native.is_discarded() ? Json{{"body", std::string(body.substr(0, 4096))}} : std::move(native);
        return error;
    }

    std::optional<std::chrono::milliseconds> retry_after(std::string_view value)
    {
        std::uint64_t seconds{};
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), seconds);
        if (error == std::errc{} && end == value.data() + value.size())
        {
            if (seconds > std::uint64_t(INT64_MAX / 1000)) return {};
            return std::chrono::milliseconds(seconds * 1000);
        }
        std::tm date{};
        std::istringstream input{std::string(value)};
        input.imbue(std::locale::classic());
        input >> std::get_time(&date, "%a, %d %b %Y %H:%M:%S GMT");
        if (input.fail() || input.peek() != std::char_traits<char>::eof()) return {};
        const std::chrono::year_month_day day{std::chrono::year{date.tm_year + 1900},
            std::chrono::month{unsigned(date.tm_mon + 1)}, std::chrono::day{unsigned(date.tm_mday)}};
        if (!day.ok() || date.tm_hour > 23 || date.tm_min > 59 || date.tm_sec > 60) return {};
        const auto when = std::chrono::sys_days(day) + std::chrono::hours(date.tm_hour) +
            std::chrono::minutes(date.tm_min) + std::chrono::seconds(date.tm_sec);
        return std::max(std::chrono::milliseconds::zero(),
            std::chrono::duration_cast<std::chrono::milliseconds>(when - std::chrono::system_clock::now()));
    }

    void EventStream::line(const std::function<void(std::string_view, std::string_view)>& emit)
    {
        if (_first) { _first = false; if (_line.starts_with("\xef\xbb\xbf")) _line.erase(0, 3); }
        if (_line.empty())
        {
            auto data = std::move(_data), event = std::move(_event);
            _data.clear(); _event.clear();
            if (!data.empty()) { data.pop_back(); emit(event, data); }
        }
        else if (_line.front() != ':')
        {
            const auto colon = _line.find(':');
            const auto field = std::string_view(_line).substr(0, colon);
            auto value = colon == _line.npos ? std::string_view{} : std::string_view(_line).substr(colon + 1);
            if (value.starts_with(' ')) value.remove_prefix(1);
            if (field == "data")
            {
                if (value.size() >= _limit || _data.size() > _limit - value.size() - 1) protocol("SSE event exceeds size limit");
                _data.append(value); _data += '\n';
            }
            else if (field == "event") _event = value;
        }
        _line.clear();
    }
    void EventStream::feed(std::string_view bytes,
                          const std::function<void(std::string_view, std::string_view)>& emit)
    {
        for (const char c : bytes)
        {
            if (std::exchange(_skip_lf, false) && c == '\n') continue;
            if (c == '\r' || c == '\n') { _skip_lf = c == '\r'; line(emit); }
            else
            {
                if (_line.size() >= _limit) protocol("SSE line exceeds size limit");
                _line += c;
            }
        }
    }
    void EventStream::finish() const
    { if (!_line.empty() || !_data.empty()) protocol("Truncated SSE event"); }

    Decoder::Decoder(Request request, Compatibility wire, EventHandler events) :
        _request(std::move(request)), _wire(std::move(wire)), _events(std::move(events))
    { _response.message.origin = _request.model; }
    void Decoder::text(std::string_view value, std::optional<std::size_t>& block)
    {
        if (value.empty()) return;
        if (!block) { block = _response.message.content.size(); _response.message.content.push_back(Text{}); }
        std::get<Text>(_response.message.content[*block]).value += value;
        if (_events) _events(TextDelta{*block, std::string(value)});
    }
    void Decoder::message(const Json& message, bool streaming)
    {
        if (!message.is_object()) protocol("Expected a response message object");
        const auto author = string(message, "role");
        if (!author.empty() && author != "assistant") protocol("Expected assistant response role");
        if (message.contains("function_call") && !message["function_call"].is_null())
            protocol("Legacy function_call responses are unsupported; use tools");
        text(string(message, "content"), _text);
        text(string(message, "refusal"), _refusal);
        for (const auto* field : {"reasoning_content", "reasoning"})
        {
            auto it = message.find(field);
            if (it == message.end() || it->is_null()) continue;
            if (_wire.reasoning_field != field)
            {
                if (!it->empty()) protocol("Response reasoning requires an explicit field mapping");
                continue;
            }
            const auto value = string(message, field);
            if (value.empty()) continue;
            if (!_reasoning)
            {
                _reasoning = _response.message.content.size();
                _response.message.content.push_back(Opaque{_request.model, "openai." + _wire.reasoning_field, ""});
            }
            std::get<Opaque>(_response.message.content[*_reasoning]).value.get_ref<std::string&>() += value;
            if (_events) _events(OpaqueDelta{*_reasoning, "openai." + _wire.reasoning_field, value});
        }
        if (const auto calls = message.find("tool_calls"); calls != message.end() && !calls->is_null())
        {
            if (!calls->is_array()) protocol("Expected tool_calls array");
            std::size_t ordinal = 0;
            for (const auto& call : *calls)
            {
                if (!call.is_object()) protocol("Expected tool call object");
                const auto id = streaming ? index(call) : ordinal++;
                const auto type = string(call, "type");
                if (!type.empty() && type != "function") protocol("Unsupported tool call type");
                auto [it, inserted] = _calls.try_emplace(id, Call{_response.message.content.size(), {}});
                if (inserted) _response.message.content.push_back(ToolCall{});
                auto& target = std::get<ToolCall>(_response.message.content[it->second.block]);
                auto call_id = string(call, "id");
                if (!call_id.empty())
                {
                    if (!target.id.empty() && target.id != call_id) protocol("Tool call ID changed during streaming");
                    target.id = call_id;
                }
                std::string name, arguments;
                if (auto function = call.find("function"); function != call.end())
                {
                    if (!function->is_object()) protocol("Expected tool function object");
                    name = string(*function, "name"); arguments = string(*function, "arguments");
                    target.name += name; it->second.arguments += arguments;
                }
                if (_events) _events(ToolDelta{it->second.block, std::move(call_id), std::move(name), std::move(arguments)});
            }
        }
    }
    void Decoder::accept(const Json& object, bool streaming)
    {
        if (!object.is_object()) protocol("Expected provider response object");
        if (object.contains("error")) throw provider_error(200, object.dump());
        if (_done) protocol("Response data after stream terminator");
        const auto id = string(object, "id"), model = string(object, "model");
        if (!id.empty())
        {
            if (!_response.request_id.empty() && _response.request_id != id) protocol("Response ID changed during streaming");
            _response.request_id = id;
        }
        if (!model.empty()) _response.reported_model = model;
        for (const auto& [key, value] : object.items())
            if (key != "choices" && key != "usage") _response.native[key] = value;
        if (const auto usage = object.find("usage"); usage != object.end() && !usage->is_null())
        {
            if (!usage->is_object()) protocol("Expected usage object");
            auto& result = _response.usage;
            if (auto value = number(*usage, "prompt_tokens")) result.input_tokens = value;
            if (auto value = number(*usage, "completion_tokens")) result.output_tokens = value;
            for (const auto* field : {"prompt_tokens_details", "completion_tokens_details"})
                if (auto details = usage->find(field); details != usage->end() && !details->is_null())
                {
                    if (!details->is_object()) protocol("Expected token details object");
                    if (std::string_view(field) == "prompt_tokens_details")
                    { if (auto value = number(*details, "cached_tokens")) result.cached_input_tokens = value; }
                    else if (auto value = number(*details, "reasoning_tokens")) result.reasoning_tokens = value;
                }
            result.native.update(*usage);
            if (_events) _events(result);
        }
        const auto choices = object.find("choices");
        if (choices == object.end() || !choices->is_array() || choices->size() > 1)
            protocol("Expected choices array with at most one choice");
        if (choices->empty())
        {
            if (!streaming || !object.contains("usage") || object["usage"].is_null()) protocol("Missing response choice");
            return;
        }
        const auto& choice = choices->front();
        if (!choice.is_object() || index(choice) != 0 || _finished) protocol("Invalid or repeated response choice");
        if (streaming)
        {
            Json extra = choice;
            extra.erase("delta"); extra.erase("finish_reason"); extra.erase("index");
            if (!extra.empty()) _response.native["choice_metadata"].push_back(std::move(extra));
        }
        else _response.native["choices"] = *choices;
        const auto value = choice.find(streaming ? "delta" : "message");
        if (value == choice.end()) protocol("Missing response message or delta");
        message(*value, streaming);
        const auto finish = string(choice, "finish_reason", !streaming);
        if (!finish.empty())
        {
            _finished = true;
            _response.native["finish_reason"] = finish;
            _response.finish = finish == "stop" ? Finish::Stop : finish == "tool_calls" ? Finish::ToolCalls :
                finish == "length" ? Finish::Length : finish == "content_filter" ? Finish::Refusal : Finish::Other;
        }
    }
    void Decoder::event(std::string_view type, std::string_view data)
    {
        if (!type.empty() && type != "message" && type != "error") protocol("Unsupported SSE event type");
        if (data == "[DONE]")
        {
            if (_done || !_finished) protocol("Unexpected stream terminator");
            _done = true; return;
        }
        auto object = parse(data);
        if (type == "error") throw provider_error(200, object.dump());
        accept(object, true);
    }
    void Decoder::json(std::string_view body) { accept(parse(body), false); }
    Response Decoder::finish()
    {
        if (!_finished || (_wire.stream && _wire.require_done && !_done)) protocol("Incomplete provider response");
        for (const auto& [index, call] : _calls)
        {
            auto& target = std::get<ToolCall>(_response.message.content[call.block]);
            target.arguments = parse(call.arguments);
            if (!target.arguments.is_object()) protocol("Tool arguments must be a JSON object");
        }
        if (_refusal) _response.finish = Finish::Refusal;
        if (_response.message.content.empty()) _response.message.content.push_back(Text{});
        validate_response(_request, _response);
        return std::move(_response);
    }
}
