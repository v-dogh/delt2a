#pragma once

#include <core/mods/inference/d2_provider.hpp>
#include <core/mods/inference/d2_tools.hpp>
#include <mutex>

namespace d2::ai
{
    struct TokenCount
    {
        std::uint64_t value{0};
        bool exact{false};
    };
    struct CompactionInput
    {
        Request request;
        TokenCount input_tokens;
        std::uint64_t target_tokens{0};
    };
    struct ContextPolicy
    {
        std::optional<std::uint64_t> max_input_tokens;
        std::uint64_t reserve_output_tokens{1024};
        double compact_at{0.9};
        std::function<TokenCount(const Request&)> count;
        std::function<mt::future<std::vector<Message>>(CompactionInput, std::stop_token)> compact;
    };
    struct ConversationConfig
    {
        Model model;
        Settings settings;
        std::shared_ptr<const ToolSet> tools;
        std::optional<ContextPolicy> context;
        std::optional<std::chrono::milliseconds> timeout;
    };
    struct StepOptions
    {
        std::optional<Settings> settings;
        std::optional<std::chrono::steady_clock::time_point> deadline;
        Json metadata = Json::object();
    };
    struct Snapshot
    {
        ConversationConfig config;
        std::vector<Message> transcript, context;
        std::vector<Record> records;
    };
    enum class StateTransfer { Preserve, DropOpaque };
    class Conversation : public std::enable_shared_from_this<Conversation>
    {
    public:
        using ptr = std::shared_ptr<Conversation>;
        using Resolver = std::function<std::shared_ptr<sys::SystemProvider>(std::string_view)>;
    private:
        mutable std::mutex _mutex;
        Resolver _resolve;
        Snapshot _state;
        std::uint64_t _revision{0}, _next_id{1};
        bool _busy{false}, _closed{false};
        std::stop_source _stop;
        void _editable(bool allow_pending = false) const;
        static void _validate_config(const ConversationConfig& config);
        void _configure(std::function<void(ConversationConfig&)> change, StateTransfer transfer);
        static mt::future<Response> _step(ptr self, Snapshot state, StepOptions options,
                                          EventHandler events, std::stop_source stop);
    public:
        Conversation(Resolver resolver, ConversationConfig config);
        Conversation(Resolver resolver, Snapshot snapshot);
        ~Conversation();
        Snapshot snapshot() const;
        ptr fork() const;
        void append(Message message);
        void add_tool_result(ToolResult result);
        PendingTools pending_tools() const;
        void replace_context(std::vector<Message> messages, bool preserve_pinned = true);
        void configure(ConversationConfig config, StateTransfer transfer = StateTransfer::Preserve);
        void switch_model(Model model, std::optional<Settings> settings = {},
                          StateTransfer transfer = StateTransfer::Preserve);
        Generation step(StepOptions options = {}, EventHandler events = {});
        bool busy() const;
        void cancel();
        void close();
    };
}
namespace d2::sys
{
    class SystemConversations : public Module<SystemConversations, "Conversations", Access::TSafe, Load::Lazy>
    {
        mutable std::mutex _mutex;
        std::map<std::string, std::shared_ptr<SystemProvider>, std::less<>> _providers;
        std::map<std::string, ai::Conversation::ptr, std::less<>> _conversations;
        bool _closed{false};
        std::shared_ptr<SystemProvider> _resolve(std::string_view name) const;
    protected:
        Status _load_impl() override;
        Status _unload_impl() override;
    public:
        using Module::Module;

        void bind(std::string name, std::shared_ptr<SystemProvider> provider);
        ai::Conversation::ptr create(std::string name, ai::ConversationConfig config);
        ai::Conversation::ptr restore(std::string name, ai::Snapshot snapshot);
        ai::Conversation::ptr get(std::string_view name) const;
        void erase(std::string_view name);
    };
    using conversations = SystemConversations;
}
