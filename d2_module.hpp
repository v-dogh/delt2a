#ifndef D2_MODULE_HPP
#define D2_MODULE_HPP

#include <absl/container/flat_hash_set.h>
#include <atomic>
#include <memory>
#include <span>
#include <thread>
#include <typeindex>

#include <d2_exceptions.hpp>
#include <d2_io_handler_frwd.hpp>
#include <d2_meta.hpp>

namespace d2
{
    namespace sys
    {
        struct Load
        {
            struct Spec
            {
                enum class Type
                {
                    Immediate,
                    Lazy,
                    Deferred,
                };
                Type type{Type::Lazy};
                // Used for Deferred type
                std::size_t ms{0};
            };
            static constexpr auto Immediate = Spec{.type = Spec::Type::Immediate, .ms = 0};
            static constexpr auto Lazy = Spec{.type = Spec::Type::Lazy, .ms = 0};
            static constexpr Spec Deferred(std::size_t ms)
            {
                return Spec{.type = Spec::Type::Deferred, .ms = ms};
            }
        };
        enum class Access
        {
            TSafe,
            TUnsafe,
        };

        class SystemModule : public std::enable_shared_from_this<SystemModule>
        {
        public:
            enum class Status
            {
                Ok,
                Degraded,
                Failure,
                Offline,
                Loading,
            };
            struct ModPreset
            {
                Access access{Access::TUnsafe};
                Load::Spec spec{};
                std::vector<std::type_index> deps{};
                std::vector<std::string> dep_names{};
            };
            struct ModInfo
            {
                std::string_view name;
                std::type_index index;
            };
        private:
            const std::weak_ptr<IOContext> _ctx{};
            const ModPreset _preset{};
            const std::size_t _static_usage{0};
            absl::flat_hash_set<std::thread::id> _safe_threads{};
            std::atomic<Status> _status{Status::Offline};
        protected:
            void _mark_safe(std::thread::id id = std::this_thread::get_id());
            void _ensure_loaded(std::function<Status()> callback);
            void _stat(Status status);

            virtual Status _load_impl();
            virtual Status _unload_impl();
        public:
            template<typename Type> static std::unique_ptr<Type> make(std::weak_ptr<IOContext> ctx)
            {
                return std::make_unique<Type>(ctx, Type::module_preset, sizeof(Type));
            }

            SystemModule(std::weak_ptr<IOContext> ptr, ModPreset preset, std::size_t static_usage);
            virtual ~SystemModule() = default;

            virtual ModInfo info() const = 0;
            Load::Spec load_spec() const noexcept;
            Access access() const noexcept;
            std::size_t static_usage() const noexcept;

            std::span<const std::type_index> dependencies() const noexcept;
            std::span<const std::string> dependency_names() const noexcept;
            std::shared_ptr<IOContext> context() const noexcept;

            Status status();
            void load();
            void unload();
            bool accessible() const;

            template<typename Type> Type* as()
            {
                auto* p = dynamic_cast<Type*>(this);
                if (p == nullptr)
                    throw std::logic_error{
                        "Attempt to access SystemComponent through an invalid object"
                    };
                return p;
            }
        };

        template<typename Module> class ModulePtr
        {
        private:
            std::shared_ptr<Module> _ptr{nullptr};
        public:
            ModulePtr(std::nullptr_t) {}
            ModulePtr() = default;
            ModulePtr(const ModulePtr&) = default;
            ModulePtr(ModulePtr&&) = default;
            ModulePtr(std::shared_ptr<Module> ptr) : _ptr(ptr) {}

            std::shared_ptr<Module> ptr() const
            {
                auto bptr = std::static_pointer_cast<SystemModule>(_ptr);
                if (!bptr->accessible())
                    throw std::logic_error{std::format(
                        "Module '{}' must be accessed from the main thread", bptr->info().name
                    )};
                return _ptr;
            }

            std::shared_ptr<Module> operator->() const
            {
                return ptr();
            }

            operator bool() const noexcept
            {
                return _ptr != nullptr;
            }

            bool operator==(const ModulePtr& ptr) const noexcept
            {
                return _ptr == ptr._ptr;
            };
            bool operator!=(const ModulePtr& ptr) const noexcept
            {
                return _ptr != ptr._ptr;
            };

            ModulePtr& operator=(const ModulePtr&) = default;
            ModulePtr& operator=(ModulePtr&&) = default;
        };
        template<typename Type> using module = ModulePtr<Type>;

        template<typename Base, meta::ConstString Name> struct AbstractModule : public SystemModule
        {
            D2_TAG_MODULE_VALUE(Name.view());
        public:
            using SystemModule::SystemModule;

            static ModInfo module_info()
            {
                return {.name = Name.view(), .index = typeid(Base)};
            }
            virtual ModInfo info() const override final
            {
                return module_info();
            }
        };
        template<typename Base, Access Ac, Load::Spec LoadSpec, typename... Deps>
        struct ConcreteModule
        {
            static inline const SystemModule::ModPreset module_preset{
                .access = Ac,
                .spec = LoadSpec,
                .deps = {typeid(Deps)...},
                .dep_names = {std::string(Deps::module_info().name)...}
            };

            module<Base> ptr() const noexcept
            {
                return module<Base>(
                    std::static_pointer_cast<Base>(static_cast<Base*>(this)->shared_from_this())
                );
            }
        };
    } // namespace sys
} // namespace d2
#endif // D2_MODULE_HPP
