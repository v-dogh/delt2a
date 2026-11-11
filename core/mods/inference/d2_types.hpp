#pragma once

#include <nlohmann/json.hpp>
#include <mt/future.hpp>
#include <chrono>
#include <compare>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <stop_token>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace d2::ai
{
    using Json = nlohmann::json;
    struct Model
    {
        std::string provider, name;
        auto operator<=>(const Model&) const = default;
    };
    enum class Role { System, Developer, User, Assistant, Tool };
    struct Text { std::string value; bool operator==(const Text&) const = default; };
    struct Image
    {
        std::string uri, media_type;
        bool operator==(const Image&) const = default;
    };
    struct ToolCall
    {
        std::string id, name;
        Json arguments = Json::object();
        bool operator==(const ToolCall&) const = default;
    };
    struct ToolResult
    {
        std::string call_id;
        Json value;
        bool error{false};
        bool operator==(const ToolResult&) const = default;
    };
    struct Opaque
    {
        Model origin;
        std::string kind;
        Json value;
        bool operator==(const Opaque&) const = default;
    };
    using Content = std::variant<Text, Image, ToolCall, ToolResult, Opaque>;
    struct Message
    {
        Role role{Role::User};
        std::vector<Content> content;
        std::optional<Model> origin;
        std::uint64_t id{0};
        bool pinned{false};
        bool operator==(const Message&) const = default;
        static Message text(Role role, std::string value, bool pinned = false);
    };
    struct Sampling
    {
        std::optional<double> temperature, top_p, min_p, frequency_penalty, presence_penalty, repeat_penalty;
        std::optional<std::uint64_t> seed, top_k, max_output_tokens;
        std::vector<std::string> stop;
        bool operator==(const Sampling&) const = default;
    };
    struct OutputFormat
    {
        enum class Kind { Text, JsonObject, JsonSchema };
        Kind kind{Kind::Text};
        std::string name;
        Json schema = Json::object();
        bool strict{false};
        bool operator==(const OutputFormat&) const = default;
    };
    struct ToolChoice
    {
        enum class Kind { Auto, None, Required, Named };
        Kind kind{Kind::Auto};
        std::string name;
        bool operator==(const ToolChoice&) const = default;
    };
    enum class UnknownCapabilities { Allow, Reject };
    struct Settings
    {
        Sampling sampling;
        OutputFormat output;
        ToolChoice tool_choice;
        std::optional<bool> parallel_tools;
        std::optional<std::string> reasoning_effort;
        // Keys are provider binding names, so switching providers cannot leak options.
        std::map<std::string, Json> extensions;
        UnknownCapabilities unknown_capabilities{UnknownCapabilities::Allow};
        bool operator==(const Settings&) const = default;
    };
    struct ToolDefinition
    {
        std::string name, description;
        Json input_schema = Json::object();
        std::optional<Json> output_schema;
        bool strict{false};
        bool operator==(const ToolDefinition&) const = default;
    };
    enum class Feature
    {
        Tools, Images, JsonObject, JsonSchema, StrictTools, Temperature, TopP, TopK,
        MinP, Penalties, Seed, Stop, Reasoning, ParallelTools, ForcedTools
    };
    enum class Support { Unknown, Supported, Unsupported };
    struct ModelInfo
    {
        std::optional<std::uint64_t> context_window, max_output_tokens;
        std::map<Feature, Support> features;
        Support supports(Feature feature) const;
    };
    struct Request
    {
        Model model;
        std::vector<Message> messages;
        std::vector<ToolDefinition> tools;
        Settings settings;
        std::optional<std::chrono::steady_clock::time_point> deadline;
        Json metadata = Json::object();
    };
    struct Usage
    {
        std::optional<std::uint64_t> input_tokens, output_tokens, cached_input_tokens, reasoning_tokens;
        Json native = Json::object();
    };
    struct Timing
    {
        std::optional<std::chrono::microseconds> first_output, total;
    };
    enum class Finish { Stop, ToolCalls, Length, Refusal, Other };
    struct Response
    {
        Message message{Role::Assistant, {}};
        Finish finish{Finish::Stop};
        Usage usage;
        Timing timing;
        std::string request_id, reported_model;
        Json native = Json::object();
    };
    struct TextDelta { std::size_t block{0}; std::string text; };
    struct ToolDelta { std::size_t block{0}; std::string id, name, arguments; };
    struct OpaqueDelta { std::size_t block{0}; std::string kind; Json value; };
    using Event = std::variant<TextDelta, ToolDelta, OpaqueDelta, Usage>;
    using EventHandler = std::function<void(const Event&)>;
    enum class ErrorCode
    {
        InvalidRequest, Unsupported, IncompatibleState, Busy, Closed, Cancelled,
        ContextLimit, Protocol, Transport, Authentication, RateLimit, Provider
    };
    class Error : public std::runtime_error
    {
    public:
        ErrorCode code;
        std::optional<int> http_status;
        std::optional<std::chrono::milliseconds> retry_after;
        Error(ErrorCode code, std::string message) : std::runtime_error(std::move(message)), code(code) {}
    };
    struct Generation
    {
        mt::future<Response> result;
        std::stop_source cancellation;
        void cancel() const { cancellation.request_stop(); }
    };
    struct Record
    {
        Model model;
        std::optional<Usage> usage;
        Timing timing;
        std::optional<Finish> finish;
        std::optional<ErrorCode> error;
        std::string request_id, reported_model;
    };
    using PendingTools = std::map<std::string, ToolCall>;
    PendingTools validate_history(const std::vector<Message>& messages, bool allow_pending = false);
    void validate_request(const Request& request, const ModelInfo& info);
    void validate_response(const Request& request, const Response& response);
} // namespace d2::ai
