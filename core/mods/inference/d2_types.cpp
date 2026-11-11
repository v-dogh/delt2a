#include "d2_provider.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace d2::ai
{
    Message Message::text(Role role, std::string value, bool pinned)
    { return {role, {Text{std::move(value)}}, {}, 0, pinned}; }
    Support ModelInfo::supports(Feature feature) const
    {
        const auto it = features.find(feature);
        return it == features.end() ? Support::Unknown : it->second;
    }
    PendingTools validate_history(const std::vector<Message>& messages, bool allow_pending)
    {
        PendingTools pending;
        std::set<std::string> used;
        for (const auto& message : messages)
        {
            if (message.content.empty()) throw Error(ErrorCode::InvalidRequest, "Empty message");
            if (!pending.empty() && message.role != Role::Tool)
                throw Error(ErrorCode::InvalidRequest, "Tool results must precede the next message");
            for (const auto& block : message.content)
            {
                if (const auto* call = std::get_if<ToolCall>(&block))
                {
                    if (message.role != Role::Assistant || call->id.empty() || call->name.empty() ||
                        !call->arguments.is_object() || !used.insert(call->id).second)
                        throw Error(ErrorCode::InvalidRequest, "Invalid or duplicate tool call");
                    pending.emplace(call->id, *call);
                }
                else if (const auto* result = std::get_if<ToolResult>(&block))
                {
                    if (message.role != Role::Tool || !pending.erase(result->call_id))
                        throw Error(ErrorCode::InvalidRequest, "Unmatched or duplicate tool result");
                }
                else if (message.role == Role::Tool)
                    throw Error(ErrorCode::InvalidRequest, "Tool messages contain only tool results");
            }
        }
        if (!allow_pending && !pending.empty()) throw Error(ErrorCode::InvalidRequest, "Missing tool results");
        return pending;
    }
    void validate_request(const Request& request, const ModelInfo& info)
    {
        if (request.model.provider.empty() || request.model.name.empty())
            throw Error(ErrorCode::InvalidRequest, "Provider binding and model name are required");
        validate_history(request.messages);
        if (!request.metadata.is_object()) throw Error(ErrorCode::InvalidRequest, "Metadata must be an object");
        const auto& settings = request.settings;
        auto require = [&](Feature feature)
        {
            const auto support = info.supports(feature);
            if (support == Support::Unsupported ||
                (support == Support::Unknown && settings.unknown_capabilities == UnknownCapabilities::Reject))
                throw Error(ErrorCode::Unsupported, "Model capability is unsupported or unknown: " +
                            std::to_string(int(feature)));
        };
        for (const auto& message : request.messages)
            for (const auto& block : message.content)
            {
                if (const auto* opaque = std::get_if<Opaque>(&block); opaque && opaque->origin != request.model)
                    throw Error(ErrorCode::IncompatibleState, "Opaque context belongs to a different provider/model");
                if (std::holds_alternative<Image>(block)) require(Feature::Images);
                if (std::holds_alternative<ToolCall>(block) || std::holds_alternative<ToolResult>(block))
                    require(Feature::Tools);
            }
        std::set<std::string> names;
        for (const auto& tool : request.tools)
        {
            if (tool.name.empty() || !names.insert(tool.name).second || !tool.input_schema.is_object() ||
                (tool.output_schema && !tool.output_schema->is_object()))
                throw Error(ErrorCode::InvalidRequest, "Invalid tool definition");
            require(Feature::Tools);
            if (tool.strict) require(Feature::StrictTools);
        }
        const auto& sampling = settings.sampling;
        auto number = [&](std::optional<double> value, Feature feature, double low, double high)
        {
            if (!value) return;
            if (!std::isfinite(*value) || *value < low || *value > high)
                throw Error(ErrorCode::InvalidRequest, "Invalid sampling value");
            require(feature);
        };
        const auto infinity = std::numeric_limits<double>::infinity();
        number(sampling.temperature, Feature::Temperature, 0, infinity);
        number(sampling.top_p, Feature::TopP, 0, 1);
        number(sampling.min_p, Feature::MinP, 0, 1);
        number(sampling.frequency_penalty, Feature::Penalties, -infinity, infinity);
        number(sampling.presence_penalty, Feature::Penalties, -infinity, infinity);
        number(sampling.repeat_penalty, Feature::Penalties, 0, infinity);
        if (sampling.top_k) require(Feature::TopK);
        if (sampling.seed) require(Feature::Seed);
        if (!sampling.stop.empty()) require(Feature::Stop);
        if (sampling.max_output_tokens && (!*sampling.max_output_tokens ||
            (info.max_output_tokens && *sampling.max_output_tokens > *info.max_output_tokens)))
            throw Error(ErrorCode::InvalidRequest, "Output token limit is invalid for this model");
        if (settings.reasoning_effort) require(Feature::Reasoning);
        if (settings.parallel_tools) require(Feature::ParallelTools);
        if (settings.output.kind == OutputFormat::Kind::JsonObject) require(Feature::JsonObject);
        if (settings.output.kind == OutputFormat::Kind::JsonSchema)
        {
            require(Feature::JsonSchema);
            if (!settings.output.schema.is_object() || settings.output.name.empty())
                throw Error(ErrorCode::InvalidRequest, "Named JSON output schema is required");
        }
        if (settings.tool_choice.kind == ToolChoice::Kind::Required || settings.tool_choice.kind == ToolChoice::Kind::Named)
        {
            require(Feature::ForcedTools);
            if (request.tools.empty() || (settings.tool_choice.kind == ToolChoice::Kind::Named &&
                !names.contains(settings.tool_choice.name)))
                throw Error(ErrorCode::InvalidRequest, "Forced tool is not available");
        }
        for (const auto& [scope, options] : settings.extensions)
            if (scope.empty() || !options.is_object())
                throw Error(ErrorCode::InvalidRequest, "Provider extensions must be named objects");
    }
    void validate_response(const Request& request, const Response& response)
    {
        if (response.message.role != Role::Assistant)
            throw Error(ErrorCode::Protocol, "Provider returned a non-assistant message");
        auto messages = request.messages;
        messages.push_back(response.message);
        PendingTools pending;
        try { pending = validate_history(messages, true); }
        catch (const Error& error) { throw Error(ErrorCode::Protocol, error.what()); }
        if ((response.finish == Finish::ToolCalls) != !pending.empty())
            throw Error(ErrorCode::Protocol, "Tool calls require a complete tool-call finish");
        if ((request.settings.tool_choice.kind == ToolChoice::Kind::Required || request.settings.tool_choice.kind == ToolChoice::Kind::Named) &&
            pending.empty() && response.finish == Finish::Stop)
            throw Error(ErrorCode::Protocol, "Provider ignored required tool choice");
        if (request.settings.parallel_tools == false && pending.size() > 1)
            throw Error(ErrorCode::Protocol, "Provider returned parallel tool calls when disabled");
        for (const auto& [id, call] : pending)
            if ((request.settings.tool_choice.kind == ToolChoice::Kind::Named && call.name != request.settings.tool_choice.name) || request.settings.tool_choice.kind == ToolChoice::Kind::None ||
                std::none_of(request.tools.begin(), request.tools.end(), [&](const auto& tool) { return tool.name == call.name; }))
                throw Error(ErrorCode::Protocol, "Provider called an unavailable tool");
        for (const auto& block : response.message.content)
            if (const auto* opaque = std::get_if<Opaque>(&block); opaque && opaque->origin != request.model)
                throw Error(ErrorCode::Protocol, "Provider returned incorrectly scoped state");
    }
}
namespace d2::sys
{
    mt::future<ai::Response> SystemProvider::generate(ai::Request request, ai::EventHandler events, std::stop_token stop)
    {
        if (stop.stop_requested()) throw ai::Error(ai::ErrorCode::Cancelled, "Generation cancelled");
        if (request.deadline && std::chrono::steady_clock::now() >= *request.deadline)
            throw ai::Error(ai::ErrorCode::Cancelled, "Generation deadline expired");
        ai::validate_request(request, model_info(request.model.name));
        // Only the selected binding's extensions reach the implementation.
        std::erase_if(request.settings.extensions, [&](const auto& item) { return item.first != request.model.provider; });
        return _generate_impl(std::move(request), std::move(events), stop);
    }
}
