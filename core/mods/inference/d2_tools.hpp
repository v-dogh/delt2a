#pragma once

#include <core/mods/inference/d2_types.hpp>

namespace d2::ai
{
    struct Tool
    {
        ToolDefinition definition;
        std::function<mt::future<Json>(Json, std::stop_token)> invoke;
        std::function<void(const Json&)> validate;
    };
    class ToolSet
    {
        std::map<std::string, Tool> _tools;
        static mt::future<ToolResult> _execute(Tool tool, ToolCall call, std::stop_token stop);
    public:
        explicit ToolSet(std::vector<Tool> tools = {});
        std::vector<ToolDefinition> definitions() const;
        mt::future<ToolResult> execute(ToolCall call, std::stop_token stop = {}) const;
    };
}
