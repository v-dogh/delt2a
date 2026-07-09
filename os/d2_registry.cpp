#include "os/d2_registry.hpp"

// cmake generated
#include <d2_platform.hpp>

namespace d2::sys
{
    // Uses stuff from d2_platform.hpp
    sys::ModuleStub Platform::_resolve_impl(std::type_index index, const std::string& name)
    {
        // We need to find the best match from the cmake exposed module list
        // From best to worse it is:
        // 1. Native
        // 2. Generic
        // 3. Unknown
        // 4. Failed

        sys::ModuleStub candidate;
        sys::SystemModule::Compat candidate_compat;
        bool duplicate = false;
        module_list::apply(
            [&]<typename... Modules>()
            {
                (
                    [&]<typename Module>()
                    {
                        if (Module::module_info().index == index)
                        {
                            const auto compat = Module::compatible();
                            const auto nc = std::size_t(compat);
                            const auto oc = std::size_t(candidate_compat);
                            if (candidate == nullptr || nc > oc)
                            {
                                candidate = sys::ModuleStub::make<Module>();
                                candidate_compat = compat;
                                duplicate = false;
                            }
                            else if (
                                nc == oc && candidate_compat == sys::SystemModule::Compat::Native
                            )
                                duplicate = true;
                        }
                    }.template operator()<Modules>(),
                    ...);
            }
        );
        if (duplicate)
            D2_TLOG(Warning, "Multiple modules have support for: ", name);
        return candidate;
    }
} // namespace d2::sys
