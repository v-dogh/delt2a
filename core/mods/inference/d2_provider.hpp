#pragma once

#include <core/io/d2_module.hpp>
#include <core/mods/inference/d2_types.hpp>

namespace d2::sys
{
    class SystemProvider : public ModuleDecl<SystemProvider, "InferenceProvider", Access::TSafe>
    {
    protected:
        virtual mt::future<ai::Response> _generate_impl(ai::Request request,
            ai::EventHandler events, std::stop_token stop) = 0;
    public:
        using ModuleDecl::ModuleDecl;
        virtual ai::ModelInfo model_info(std::string_view model) const = 0;
        mt::future<ai::Response> generate(ai::Request request, ai::EventHandler events = {},
                                         std::stop_token stop = {});
    };
    using inference_provider = SystemProvider;
}
