#include "d2_openai.hpp"
#include "d2_openai_protocol.hpp"

#include <algorithm>

namespace d2::sys
{
    using HTTP = net::SystemHTTP;
    using Clock = std::chrono::steady_clock;
    namespace protocol = d2::ai::openai::detail;
    namespace oai = d2::ai::openai;

    struct SystemOpenAI::State
    {
        mutable std::mutex mutex;
        oai::Config config;
        bool closed{true};
        std::uint64_t next{0};
        std::map<std::uint64_t, std::shared_ptr<Operation>> active;
    };
    struct SystemOpenAI::Operation : std::enable_shared_from_this<Operation>
    {
        std::weak_ptr<State> owner;
        std::uint64_t id{};
        oai::Config config;
        oai::Compatibility wire;
        std::shared_ptr<HTTP> transport;
        mt::future<d2::ai::Response> result = mt::future<d2::ai::Response>::make();
        Clock::time_point started{Clock::now()}, deadline;
        std::optional<std::chrono::microseconds> first_output;
        std::mutex mutex;
        std::shared_ptr<HTTP::Stream> handle;
        std::atomic<bool> cancelled{false}, settled{false};
        std::unique_ptr<std::stop_callback<std::function<void()>>> stop;
        protocol::EventStream stream;
        protocol::Decoder decoder;
        int status{0};
        std::string body, request_id;
        std::optional<std::chrono::milliseconds> retry;

        Operation(std::weak_ptr<State> owner, oai::Config config,
                  oai::Compatibility wire, d2::ai::Request request, d2::ai::EventHandler events) :
            owner(std::move(owner)), config(std::move(config)), wire(std::move(wire)),
            deadline(std::min(request.deadline.value_or(Clock::time_point::max()), started + this->config.timeout)),
            stream(this->config.max_event_bytes),
            decoder(std::move(request), this->wire, [this, events = std::move(events)](const d2::ai::Event& event)
            {
                check();
                if (!first_output && !std::holds_alternative<d2::ai::Usage>(event))
                    first_output = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - started);
                if (events) events(event);
                check();
            }) {}
        void check() const
        {
            if (cancelled) throw d2::ai::Error(d2::ai::ErrorCode::Cancelled, "Generation cancelled");
            if (Clock::now() >= deadline) throw d2::ai::Error(d2::ai::ErrorCode::Timeout, "Generation deadline expired");
        }
        void cancel()
        {
            cancelled = true;
            std::shared_ptr<HTTP::Stream> current;
            { std::lock_guard lock(mutex); current = handle; }
            if (current) current->cancel();
        }
        void install(HTTP::Stream::ptr value)
        {
            std::shared_ptr<HTTP::Stream> current(std::move(value));
            { std::lock_guard lock(mutex); handle = current; }
            if (cancelled && current) current->cancel();
        }
        bool settle()
        {
            if (settled.exchange(true)) return false;
            if (auto state = owner.lock()) { std::lock_guard lock(state->mutex); state->active.erase(id); }
            return true;
        }
        void fail(std::exception_ptr failure)
        {
            if (!settle()) return;
            try { std::rethrow_exception(failure); }
            catch (const d2::ai::Error& error)
            {
                try
                {
                    auto enriched = error;
                    if (enriched.request_id.empty()) enriched.request_id = request_id;
                    failure = std::make_exception_ptr(std::move(enriched));
                }
                catch (...) { failure = std::current_exception(); }
            }
            catch (...) {}
            result.block()->set_exception(std::move(failure));
        }
        void headers(HTTP::Response& response)
        {
            check();
            if (response.status() < 200) return;
            status = response.status(); body.clear(); request_id.clear(); retry.reset();
            if (auto value = response.get("x-request-id")) request_id = *value;
            if (auto value = response.get("retry-after")) retry = protocol::retry_after(*value);
            if (status >= 200 && status < 300 && wire.stream)
            {
                auto type = response.get("content-type").value_or("");
                type = type.substr(0, type.find(';'));
                while (!type.empty() && type.back() == ' ') type.remove_suffix(1);
                std::string normalized(type);
                for (auto& c : normalized) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
                if (normalized != "text/event-stream")
                    throw d2::ai::Error(d2::ai::ErrorCode::Protocol, "Expected text/event-stream response");
            }
        }
        void chunk(HTTP::Chunk& chunk)
        {
            check();
            if (settled) return;
            const auto bytes = chunk.read();
            if (status >= 200 && status < 300 && wire.stream)
                stream.feed(bytes, [this](auto type, auto data) { check(); decoder.event(type, data); });
            else
            {
                if (bytes.size() > config.max_response_bytes || body.size() > config.max_response_bytes - bytes.size())
                    throw d2::ai::Error(d2::ai::ErrorCode::Protocol, "Response exceeds size limit");
                body.append(bytes);
            }
        }
        void complete(HTTP::Response& response)
        {
            try
            {
                check();
                if (settled) return;
                status = response.status();
                if (status < 200 || status >= 300)
                {
                    auto error = protocol::provider_error(status, body);
                    error.retry_after = retry;
                    throw error;
                }
                if (wire.stream) stream.finish(); else decoder.json(body);
                auto answer = decoder.finish();
                check();
                answer.timing.first_output = first_output;
                answer.timing.total = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - started);
                if (!request_id.empty())
                {
                    answer.native["completion_id"] = answer.request_id;
                    answer.request_id = request_id;
                }
                if (settle()) result.block()->set_value(std::move(answer));
            }
            catch (...) { fail(std::current_exception()); }
        }
        void error(const HTTP::Error& error)
        {
            try
            {
                if (cancelled) throw d2::ai::Error(d2::ai::ErrorCode::Cancelled, "Generation cancelled");
                if (error.code == HTTP::Error::Code::Callback && error.cause) std::rethrow_exception(error.cause);
                auto code = d2::ai::ErrorCode::Transport;
                switch (error.code)
                {
                case HTTP::Error::Code::InvalidRequest: code = d2::ai::ErrorCode::InvalidRequest; break;
                case HTTP::Error::Code::Unavailable: code = d2::ai::ErrorCode::Closed; break;
                case HTTP::Error::Code::Cancelled: code = d2::ai::ErrorCode::Cancelled; break;
                case HTTP::Error::Code::Timeout: code = d2::ai::ErrorCode::Timeout; break;
                case HTTP::Error::Code::Limit: code = d2::ai::ErrorCode::Protocol; break;
                default: break;
                }
                d2::ai::Error failure(code, error.message);
                if (error.native_code) failure.native["transport_code"] = *error.native_code;
                throw failure;
            }
            catch (...) { fail(std::current_exception()); }
        }
    };

    SystemOpenAI::SystemOpenAI(std::weak_ptr<IOContext> context, ModPreset preset, std::size_t usage) :
        SystemProvider(std::move(context), std::move(preset), usage), _state(std::make_shared<State>()) {}

    SystemOpenAI::Status SystemOpenAI::_load_impl()
    { std::lock_guard lock(_state->mutex); _state->closed = false; return Status::Ok; }
    SystemOpenAI::Status SystemOpenAI::_unload_impl()
    {
        decltype(State::active) active;
        { std::lock_guard lock(_state->mutex); _state->closed = true; active.swap(_state->active); }
        for (const auto& [id, operation] : active) operation->cancel();
        return Status::Ok;
    }
    void SystemOpenAI::configure(oai::Config config)
    {
        protocol::validate_config(config);
        std::lock_guard lock(_state->mutex);
        _state->config = std::move(config);
    }
    oai::Config SystemOpenAI::config() const
    { std::lock_guard lock(_state->mutex); return _state->config; }
    d2::ai::ModelInfo SystemOpenAI::model_info(std::string_view model) const
    {
        std::lock_guard lock(_state->mutex);
        const auto it = _state->config.models.find(std::string(model));
        return it == _state->config.models.end() ? _state->config.default_model : it->second.info;
    }
    mt::future<d2::ai::Response> SystemOpenAI::_generate_impl(d2::ai::Request request, d2::ai::EventHandler events, std::stop_token stop)
    {
        oai::Config config;
        { std::lock_guard lock(_state->mutex);
          if (_state->closed) throw d2::ai::Error(d2::ai::ErrorCode::Closed, "OpenAI provider is unloaded");
          config = _state->config; }
        auto wire = config.compatibility;
        auto info = config.default_model;
        if (const auto model = config.models.find(request.model.name); model != config.models.end())
        {
            info = model->second.info;
            if (model->second.compatibility) wire = *model->second.compatibility;
        }
        d2::ai::validate_request(request, info);
        std::string body;
        try { body = protocol::encode(request, wire).dump(); }
        catch (const d2::ai::Json::exception&) { throw d2::ai::Error(d2::ai::ErrorCode::InvalidRequest, "Request cannot be encoded as JSON"); }
        const auto ctx = context();
        if (!ctx) throw d2::ai::Error(d2::ai::ErrorCode::Closed, "OpenAI provider context is unavailable");
        auto operation = std::make_shared<Operation>(_state, std::move(config), std::move(wire), std::move(request), std::move(events));
        operation->transport = ctx->sys<HTTP>().ptr();
        {
            std::lock_guard lock(_state->mutex);
            if (_state->closed) throw d2::ai::Error(d2::ai::ErrorCode::Closed, "OpenAI provider is unloaded");
            if (_state->next == UINT64_MAX) throw std::overflow_error("Provider request IDs exhausted");
            operation->id = ++_state->next;
            _state->active.emplace(operation->id, operation);
        }
        try
        {
            operation->stop = std::make_unique<std::stop_callback<std::function<void()>>>(stop,
                [weak = std::weak_ptr<Operation>(operation)] { if (auto current = weak.lock()) current->cancel(); });
            operation->check();
            HTTP::Request http;
            http.method = HTTP::Method::Post; http.url = operation->config.endpoint;
            http.deadline = operation->deadline;
            http.connect_timeout = operation->config.connect_timeout;
            http.max_response_bytes = operation->config.max_response_bytes;
            http.max_header_bytes = operation->config.max_header_bytes;
            http.follow_redirects = false;
            http.on_setup = [operation, body = std::move(body)](HTTP::RequestBuilder& builder)
            {
                for (const auto& [name, value] : operation->config.headers) builder.set(name, value);
                builder.set("Content-Type", "application/json");
                builder.set("Accept", operation->wire.stream ? "text/event-stream" : "application/json");
                if (!operation->config.api_key.empty()) builder.set("Authorization", "Bearer " + operation->config.api_key);
                builder.body(body);
            };
            http.on_headers = [operation](auto& response) { operation->headers(response); };
            http.on_chunk = [operation](auto& chunk) { operation->chunk(chunk); };
            http.on_response = [operation](auto& response) { operation->complete(response); };
            http.on_error = [operation](const auto& error) { operation->error(error); };
            operation->install(operation->transport->request(std::move(http)));
        }
        catch (...) { operation->fail(std::current_exception()); }
        return operation->result;
    }
}
