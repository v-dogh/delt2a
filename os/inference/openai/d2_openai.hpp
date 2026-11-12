#pragma once

#include <core/mods/inference/d2_provider.hpp>
#include <core/mods/d2_http.hpp>
#include <mutex>

namespace d2
{
    namespace ai::openai 
    {
        struct Compatibility
        {
            std::string token_limit_field{"max_completion_tokens"};
            std::string reasoning_field;
            bool stream{true}, include_usage{true}, require_done{true};
            bool developer_as_system{false}, extended_sampling{false};
        };
        struct ModelConfig
        {
            ModelInfo info;
            std::optional<Compatibility> compatibility;
        };
        struct Config
        {
            std::string endpoint{""};
            std::string api_key;
            std::map<std::string, std::string> headers;
            Compatibility compatibility;
            ModelInfo default_model;
            std::map<std::string, ModelConfig> models;
            std::chrono::milliseconds timeout{300000}, connect_timeout{10000};
            std::size_t max_response_bytes{32 * 1024 * 1024};
            std::size_t max_event_bytes{1024 * 1024}, max_header_bytes{64 * 1024};
        };
    }
    namespace sys 
    {
        class SystemOpenAI : public SystemProvider,
                            public ModuleImpl<SystemOpenAI, Load::Lazy, net::SystemHTTP>
        {
            struct State;
            struct Operation;
            std::shared_ptr<State> _state;
        protected:
            Status _load_impl() override;
            Status _unload_impl() override;
            mt::future<ai::Response> _generate_impl(ai::Request request,
                ai::EventHandler events, std::stop_token stop) override;
        public:
            SystemOpenAI(std::weak_ptr<IOContext> context, ModPreset preset, std::size_t usage);
            void configure(ai::openai::Config config);
            ai::openai::Config config() const;
            ai::ModelInfo model_info(std::string_view model) const override;
        };

        namespace ai
        {
            using openai = SystemOpenAI;
        }
    }
}
