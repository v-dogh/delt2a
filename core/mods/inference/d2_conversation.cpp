#include "d2_conversation.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace d2::ai
{
    namespace
    {
        Request request_for(const Snapshot& state)
        {
            Request request;
            request.model = state.config.model; request.settings = state.config.settings;
            request.messages = state.context;
            if (state.config.tools) request.tools = state.config.tools->definitions();
            return request;
        }
        void check_stop(std::stop_token stop, const Request& request)
        {
            if (stop.stop_requested())
                throw Error(ErrorCode::Cancelled, "Conversation step cancelled");
            if (request.deadline && std::chrono::steady_clock::now() >= *request.deadline)
                throw Error(ErrorCode::Timeout, "Conversation step deadline expired");
        }
    }
    void Conversation::_validate_config(const ConversationConfig& config)
    {
        if (config.model.provider.empty() || config.model.name.empty())
            throw Error(ErrorCode::InvalidRequest, "Provider binding and model name are required");
        if (config.timeout && config.timeout->count() <= 0)
            throw Error(ErrorCode::InvalidRequest, "Timeout must be positive");
        if (config.context)
        {
            const auto& policy = *config.context;
            if (!policy.count || !std::isfinite(policy.compact_at) || policy.compact_at <= 0 || policy.compact_at > 1 ||
                (policy.max_input_tokens && !*policy.max_input_tokens))
                throw Error(ErrorCode::InvalidRequest, "Context policy requires an estimator and valid limits");
        }
    }
    Conversation::Conversation(Resolver resolver, ConversationConfig config) :
        Conversation(std::move(resolver), Snapshot{std::move(config), {}, {}, {}}) {}
    Conversation::Conversation(Resolver resolver, Snapshot snapshot) : _resolve(std::move(resolver)), _state(std::move(snapshot))
    {
        if (!_resolve) throw Error(ErrorCode::InvalidRequest, "Provider resolver is required");
        _validate_config(_state.config);
        validate_history(_state.context, true);
        std::set<std::uint64_t> ids;
        for (const auto& message : _state.transcript)
        {
            if (!message.id || message.id == UINT64_MAX || !ids.insert(message.id).second)
                throw Error(ErrorCode::InvalidRequest, "Invalid transcript message IDs");
            _next_id = std::max(_next_id, message.id + 1);
        }
        ids.clear();
        for (const auto& message : _state.context)
        {
            if (!message.id || message.id == UINT64_MAX || !ids.insert(message.id).second)
                throw Error(ErrorCode::InvalidRequest, "Invalid context message IDs");
            _next_id = std::max(_next_id, message.id + 1);
        }
    }
    void Conversation::_editable(bool allow_pending) const
    {
        if (_closed) throw Error(ErrorCode::Closed, "Conversation is closed");
        if (_busy) throw Error(ErrorCode::Busy, "Conversation has an active step");
        if (!allow_pending && !validate_history(_state.context, true).empty())
            throw Error(ErrorCode::Busy, "Complete pending tool calls first");
    }
    Snapshot Conversation::snapshot() const { std::lock_guard lock(_mutex); return _state; }
    Conversation::ptr Conversation::fork() const { return std::make_shared<Conversation>(_resolve, snapshot()); }
    void Conversation::append(Message message)
    {
        std::lock_guard lock(_mutex);
        _editable(true);
        auto context = _state.context;
        if (_next_id == UINT64_MAX) throw std::overflow_error("Conversation message IDs exhausted");
        message.id = _next_id;
        context.push_back(message);
        validate_history(context, true);
        _state.transcript.push_back(std::move(message));
        _state.context = std::move(context);
        ++_next_id; ++_revision;
    }
    void Conversation::add_tool_result(ToolResult result) { append({Role::Tool, {std::move(result)}}); }
    PendingTools Conversation::pending_tools() const
    { std::lock_guard lock(_mutex); return validate_history(_state.context, true); }
    void Conversation::replace_context(std::vector<Message> messages, bool preserve_pinned)
    {
        Snapshot candidate;
        std::uint64_t revision, next;
        {
            std::lock_guard lock(_mutex);
            _editable(); candidate = _state; revision = _revision; next = _next_id;
        }
        if (preserve_pinned)
            for (const auto& old : candidate.context)
                if (old.pinned && std::find(messages.begin(), messages.end(), old) == messages.end())
                    throw Error(ErrorCode::InvalidRequest, "Replacement removed pinned context");
        std::set<std::uint64_t> ids;
        for (auto& message : messages)
        {
            if (message.id &&
                std::find(candidate.context.begin(), candidate.context.end(), message) == candidate.context.end() &&
                std::find(candidate.transcript.begin(), candidate.transcript.end(), message) == candidate.transcript.end())
                throw Error(ErrorCode::InvalidRequest, "New context messages require a zero ID");
            if (!message.id)
            {
                if (next == UINT64_MAX) throw std::overflow_error("Conversation message IDs exhausted");
                message.id = next++;
            }
            if (!ids.insert(message.id).second) throw Error(ErrorCode::InvalidRequest, "Duplicate context message ID");
        }
        candidate.context = std::move(messages);
        auto provider = _resolve(candidate.config.model.provider);
        if (!provider) throw Error(ErrorCode::InvalidRequest, "Provider binding is unavailable");
        validate_request(request_for(candidate), provider->model_info(candidate.config.model.name));
        std::lock_guard lock(_mutex);
        _editable();
        if (_revision != revision) throw Error(ErrorCode::Busy, "Conversation changed during context replacement");
        _state.context = std::move(candidate.context); _next_id = next; ++_revision;
    }
    void Conversation::_configure(std::function<void(ConversationConfig&)> change, StateTransfer transfer)
    {
        Snapshot candidate;
        std::uint64_t revision;
        {
            std::lock_guard lock(_mutex);
            _editable(); candidate.config = _state.config; candidate.context = _state.context; revision = _revision;
        }
        change(candidate.config);
        _validate_config(candidate.config);
        if (transfer == StateTransfer::DropOpaque)
        {
            for (auto& message : candidate.context)
                std::erase_if(message.content, [&](const auto& block)
                {
                    const auto* state = std::get_if<Opaque>(&block);
                    if (!state || state->origin == candidate.config.model) return false;
                    if (message.pinned) throw Error(ErrorCode::IncompatibleState, "Cannot drop pinned provider state");
                    return true;
                });
            std::erase_if(candidate.context, [](const auto& message) { return message.content.empty(); });
        }
        auto provider = _resolve(candidate.config.model.provider);
        if (!provider) throw Error(ErrorCode::InvalidRequest, "Provider binding is unavailable");
        validate_request(request_for(candidate), provider->model_info(candidate.config.model.name));
        std::lock_guard lock(_mutex);
        _editable();
        if (_revision != revision) throw Error(ErrorCode::Busy, "Conversation changed during configuration");
        _state.config = std::move(candidate.config); _state.context = std::move(candidate.context); ++_revision;
    }
    void Conversation::configure(ConversationConfig config, StateTransfer transfer)
    { _configure([&](auto& target) { target = std::move(config); }, transfer); }
    void Conversation::switch_model(Model model, std::optional<Settings> settings, StateTransfer transfer)
    {
        _configure([&](auto& config)
        {
            config.model = std::move(model);
            if (settings) config.settings = std::move(*settings);
        }, transfer);
    }
    Generation Conversation::step(StepOptions options, EventHandler events)
    {
        auto self = shared_from_this();
        Snapshot state;
        std::stop_source stop;
        {
            std::lock_guard lock(_mutex);
            _editable(); state.config = _state.config; state.context = _state.context; _stop = stop; _busy = true;
        }
        try { return {_step(std::move(self), std::move(state), std::move(options), std::move(events), stop), stop}; }
        catch (...) { std::lock_guard lock(_mutex); _busy = false; throw; }
    }
    mt::future<Response> Conversation::_step(ptr self, Snapshot state, StepOptions options,
                                             EventHandler events, std::stop_source stop)
    {
        const auto started = std::chrono::steady_clock::now();
        Record record;
        try
        {
            record.model = state.config.model;
            auto request = request_for(state);
            if (options.settings) request.settings = std::move(*options.settings);
            request.deadline = options.deadline;
            if (state.config.timeout)
            {
                if (*state.config.timeout > std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::time_point::max() - started))
                    throw Error(ErrorCode::InvalidRequest, "Timeout exceeds clock range");
                const auto deadline = started + *state.config.timeout;
                request.deadline = request.deadline ? std::min(*request.deadline, deadline) : deadline;
            }
            request.metadata = std::move(options.metadata);
            check_stop(stop.get_token(), request);
            auto provider = self->_resolve(request.model.provider);
            if (!provider) throw Error(ErrorCode::InvalidRequest, "Provider binding is unavailable");
            const auto info = provider->model_info(request.model.name);
            validate_request(request, info);
            if (state.config.context)
            {
                const auto& policy = *state.config.context;
                auto budget = policy.max_input_tokens;
                if (info.context_window)
                {
                    const auto reserve = request.settings.sampling.max_output_tokens.value_or(policy.reserve_output_tokens);
                    if (reserve >= *info.context_window) throw Error(ErrorCode::ContextLimit, "No room for input context");
                    const auto available = *info.context_window - reserve;
                    budget = budget ? std::min(*budget, available) : available;
                }
                if (budget)
                {
                    auto count = policy.count(request);
                    const auto target = std::uint64_t(static_cast<long double>(*budget) * policy.compact_at);
                    if (count.value > target)
                    {
                        if (!policy.compact) throw Error(ErrorCode::ContextLimit, "Context needs compaction");
                        auto replacement = co_await policy.compact({request, count, target}, stop.get_token());
                        check_stop(stop.get_token(), request);
                        for (const auto& pinned : request.messages)
                            if (pinned.pinned && std::find(replacement.begin(), replacement.end(), pinned) == replacement.end())
                                throw Error(ErrorCode::InvalidRequest, "Compaction removed or changed pinned context");
                        std::set<std::uint64_t> ids;
                        for (const auto& message : replacement)
                            if (message.id && (!ids.insert(message.id).second ||
                                std::find(request.messages.begin(), request.messages.end(), message) == request.messages.end()))
                                throw Error(ErrorCode::InvalidRequest, "Compaction must give new messages a zero ID");
                        request.messages = std::move(replacement);
                        validate_request(request, info);
                        if (policy.count(request).value > target)
                            throw Error(ErrorCode::ContextLimit, "Compaction did not reach the target budget");
                    }
                }
            }
            check_stop(stop.get_token(), request);
            auto response = co_await provider->generate(request, std::move(events), stop.get_token());
            check_stop(stop.get_token(), request);
            validate_response(request, response);
            response.message.origin = request.model;
            response.message.pinned = false;
            record.usage = response.usage; record.finish = response.finish;
            record.timing = response.timing; record.timing.total = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started);
            record.request_id = response.request_id; record.reported_model = response.reported_model;
            {
                std::lock_guard lock(self->_mutex);
                check_stop(stop.get_token(), request);
                if (self->_closed) throw Error(ErrorCode::Closed, "Conversation closed before completion");
                auto next = self->_next_id;
                auto context = std::move(request.messages);
                std::set<std::uint64_t> ids;
                for (auto& message : context)
                {
                    if (!message.id) message.id = next++;
                    if (!message.id || message.id == UINT64_MAX || !ids.insert(message.id).second)
                        throw Error(ErrorCode::InvalidRequest, "Compaction returned invalid message IDs");
                    next = std::max(next, message.id + 1);
                }
                if (next == UINT64_MAX) throw std::overflow_error("Conversation message IDs exhausted");
                response.message.id = next++;
                context.push_back(response.message);
                auto entry = response.message;
                self->_state.transcript.reserve(self->_state.transcript.size() + 1);
                self->_state.records.reserve(self->_state.records.size() + 1);
                self->_state.transcript.push_back(std::move(entry));
                self->_state.records.push_back(std::move(record));
                self->_state.context = std::move(context); self->_next_id = next;
                self->_busy = false; ++self->_revision;
            }
            co_return std::move(response);
        }
        catch (...)
        {
            const auto error = std::current_exception();
            try { std::rethrow_exception(error); }
            catch (const Error& failure) { record.error = failure.code; }
            catch (...) { record.error = ErrorCode::Provider; }
            record.timing.total = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started);
            { std::lock_guard lock(self->_mutex); self->_busy = false; self->_state.records.push_back(std::move(record)); }
            std::rethrow_exception(error);
        }
    }
    bool Conversation::busy() const { std::lock_guard lock(_mutex); return _busy; }
    void Conversation::cancel()
    {
        std::stop_source stop;
        { std::lock_guard lock(_mutex); stop = _stop; }
        stop.request_stop();
    }
    void Conversation::close()
    {
        std::stop_source stop;
        { std::lock_guard lock(_mutex); _closed = true; stop = _stop; }
        stop.request_stop();
    }
}
namespace d2::sys
{
    SystemConversations::~SystemConversations() { _unload_impl(); }
    SystemConversations::Status SystemConversations::_load_impl()
    { std::lock_guard lock(_mutex); _closed = false; return Status::Ok; }
    SystemConversations::Status SystemConversations::_unload_impl()
    {
        decltype(_conversations) conversations;
        decltype(_providers) providers;
        {
            std::lock_guard lock(_mutex); _closed = true;
            conversations.swap(_conversations); providers.swap(_providers);
        }
        for (const auto& [name, conversation] : conversations) conversation->close();
        return Status::Ok;
    }
    std::shared_ptr<SystemProvider> SystemConversations::_resolve(std::string_view name) const
    {
        std::lock_guard lock(_mutex);
        const auto it = _providers.find(name);
        if (_closed || it == _providers.end()) throw ai::Error(ai::ErrorCode::InvalidRequest, "Provider binding is unavailable");
        return it->second;
    }
    void SystemConversations::bind(std::string name, std::shared_ptr<SystemProvider> provider)
    {
        if (name.empty() || !provider) throw ai::Error(ai::ErrorCode::InvalidRequest, "Invalid provider binding");
        std::lock_guard lock(_mutex);
        if (_closed) throw ai::Error(ai::ErrorCode::Closed, "Conversation module is closed");
        if (!_providers.emplace(std::move(name), std::move(provider)).second)
            throw ai::Error(ai::ErrorCode::InvalidRequest, "Provider binding already exists");
    }
    ai::Conversation::ptr SystemConversations::create(std::string name, ai::ConversationConfig config)
    { return restore(std::move(name), {std::move(config), {}, {}, {}}); }
    ai::Conversation::ptr SystemConversations::restore(std::string name, ai::Snapshot snapshot)
    {
        if (name.empty()) throw ai::Error(ai::ErrorCode::InvalidRequest, "Conversation name is required");
        auto weak = std::weak_ptr<SystemConversations>(std::static_pointer_cast<SystemConversations>(shared_from_this()));
        auto conversation = std::make_shared<ai::Conversation>([weak](std::string_view provider)
        {
            auto self = weak.lock();
            if (!self) throw ai::Error(ai::ErrorCode::Closed, "Conversation module no longer exists");
            return self->_resolve(provider);
        }, std::move(snapshot));
        std::lock_guard lock(_mutex);
        if (_closed) throw ai::Error(ai::ErrorCode::Closed, "Conversation module is closed");
        if (!_conversations.emplace(std::move(name), conversation).second)
            throw ai::Error(ai::ErrorCode::InvalidRequest, "Conversation already exists");
        return conversation;
    }
    ai::Conversation::ptr SystemConversations::get(std::string_view name) const
    {
        std::lock_guard lock(_mutex);
        const auto it = _conversations.find(name);
        if (it == _conversations.end()) throw ai::Error(ai::ErrorCode::InvalidRequest, "Conversation not found");
        return it->second;
    }
    void SystemConversations::erase(std::string_view name)
    {
        ai::Conversation::ptr conversation;
        {
            std::lock_guard lock(_mutex);
            const auto it = _conversations.find(name);
            if (it == _conversations.end()) return;
            conversation = std::move(it->second); _conversations.erase(it);
        }
        conversation->close();
    }
}
