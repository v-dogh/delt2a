#pragma once

#include <core/utils/d2_exceptions.hpp>
#include <core/io/d2_module.hpp>

namespace d2::sys
{
    template<typename... Argv> class Registry
    {
    public:
        static constexpr std::size_t size()
        {
            return sizeof...(Argv);
        }
        static auto apply(auto&& callback)
        {
            return callback.template operator()<Argv...>();
        }
    };

    class Platform
    {
        D2_TAG_MODULE(ctx)
    private:
        static sys::ModuleStub _resolve_impl(std::type_index index, const std::string& name);
    public:
        template<typename Module> static sys::ModuleStub resolve()
        {
            if constexpr (sys::is_abstract<Module>)
            {
                return _resolve_impl(
                    Module::module_info().index, std::string(Module::module_info().name)
                );
            }
            else
                return sys::ModuleStub::make<Module>();
        }
    };
} // namespace d2::sys
