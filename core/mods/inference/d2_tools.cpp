#include "d2_tools.hpp"

namespace d2::ai
{
    ToolSet::ToolSet(std::vector<Tool> tools)
    {
        for (auto& tool : tools)
        {
            const auto name = tool.definition.name;
            if (name.empty() || !tool.definition.input_schema.is_object() ||
                (tool.definition.output_schema && !tool.definition.output_schema->is_object()) ||
                !_tools.emplace(name, std::move(tool)).second)
                throw Error(ErrorCode::InvalidRequest, "Invalid or duplicate tool definition");
        }
    }
    std::vector<ToolDefinition> ToolSet::definitions() const
    {
        std::vector<ToolDefinition> result;
        for (const auto& [name, tool] : _tools) result.push_back(tool.definition);
        return result;
    }
    mt::future<ToolResult> ToolSet::_execute(Tool tool, ToolCall call, std::stop_token stop)
    {
        if (stop.stop_requested()) throw Error(ErrorCode::Cancelled, "Tool cancelled");
        ToolResult result{call.id};
        try
        {
            if (!call.arguments.is_object() || call.id.empty())
                throw Error(ErrorCode::InvalidRequest, "Tool call requires an ID and object arguments");
            if (tool.validate) tool.validate(call.arguments);
            if (!tool.invoke) throw Error(ErrorCode::Unsupported, "Tool requires external execution");
            result.value = co_await tool.invoke(std::move(call.arguments), stop);
        }
        catch (const std::exception& error)
        {
            if (const auto* ai_error = dynamic_cast<const Error*>(&error); ai_error && ai_error->code == ErrorCode::Cancelled)
                throw;
            result.error = true;
            result.value = Json{{"error", error.what()}};
        }
        if (stop.stop_requested()) throw Error(ErrorCode::Cancelled, "Tool cancelled");
        co_return result;
    }
    mt::future<ToolResult> ToolSet::execute(ToolCall call, std::stop_token stop) const
    {
        const auto it = _tools.find(call.name);
        if (it == _tools.end()) throw Error(ErrorCode::InvalidRequest, "Unknown tool: " + call.name);
        return _execute(it->second, std::move(call), stop);
    }
}
