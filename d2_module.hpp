#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <atomic>
#include <cstddef>
#include <memory>
#include <span>
#include <thread>
#include <typeindex>
#include <variant>

#include <d2_exceptions.hpp>
#include <d2_io_handler_frwd.hpp>
#include <d2_meta.hpp>

namespace d2::sys
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

    template<typename Base, Load::Spec LoadSpec, typename... Deps> struct ConcreteModule;

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
        enum class Compat
        {
            Native,
            Generic,
            Unknown,
            Failed,
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

        virtual Status _pre_load_impl();
        virtual Status _post_unload_impl();

        virtual Status _load_impl();
        virtual Status _unload_impl();
    public:
        static Compat compatible();

        template<typename Module> static std::unique_ptr<Module> make(std::weak_ptr<IOContext> ctx)
        {
            return std::make_unique<Module>(ctx, Module::module_preset, sizeof(Module));
        }

        SystemModule(std::weak_ptr<IOContext> ptr, ModPreset preset, std::size_t static_usage);
        virtual ~SystemModule() = default;

        virtual ModInfo info() const = 0;
        ModPreset preset() const;
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

    namespace impl
    {
        template<typename DepsA, typename... DepsB> struct MergeDeps;

        template<typename... DepsA, typename... DepsB>
        struct MergeDeps<std::tuple<DepsA...>, DepsB...>
        {
            static auto list()
            {
                return std::vector<std::type_index>{typeid(DepsA)..., typeid(DepsB)...};
            }
            static auto names()
            {
                return std::vector<std::string>{
                    std::string(DepsA::module_info().name)...,
                    std::string(DepsB::module_info().name)...
                };
            }
        };
    } // namespace impl

    template<typename Base, meta::ConstString Name, Access Ac, typename... Deps>
    struct AbstractModule : public SystemModule
    {
        D2_TAG_MODULE_VALUE(Name.view());
    private:
        static constexpr auto module_access = Ac;
        using module_deps = std::tuple<Deps...>;
    public:
        template<typename B, Load::Spec L, typename... D> friend struct ConcreteModule;

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
    struct ConcreteModuleTag
    {
    };

    template<typename Module>
    static constexpr bool is_abstract = !std::is_base_of_v<ConcreteModuleTag, Module>;

    template<typename Base, Load::Spec LoadSpec, typename... Deps>
    struct ConcreteModule : private ConcreteModuleTag
    {
        template<typename B, Load::Spec L, typename... D> friend struct ConcreteModule;

        static inline const SystemModule::ModPreset module_preset{
            .access = Base::module_access,
            .spec = LoadSpec,
            .deps = impl::MergeDeps<typename Base::module_deps, Deps...>::list(),
            .dep_names = impl::MergeDeps<typename Base::module_deps, Deps...>::names()
        };

        module<const Base> ptr() const noexcept
        {
            return module<const Base>(std::static_pointer_cast<const Base>(
                static_cast<const Base*>(this)->shared_from_this()
            ));
        }
        module<Base> ptr() noexcept
        {
            return module<Base>(
                std::static_pointer_cast<Base>(static_cast<Base*>(this)->shared_from_this())
            );
        }
    };

    struct ModuleStub
    {
        D2_TAG_MODULE(ctx)
    public:
        enum class Query
        {
            Commit,
            Preset,
            Info,
        };

        using handle_info = std::variant<
            SystemModule::ModInfo,
            SystemModule::ModPreset,
            std::shared_ptr<SystemModule>,
            std::monostate
        >;
        using handle_type = handle_info (*)(Query, std::shared_ptr<IOContext>);

        template<typename Module>
        static constexpr handle_type handle =
            [](Query query, std::shared_ptr<IOContext> ctx) -> handle_info
        {
            if (query == Query::Commit)
            {
                return SystemModule::make<Module>(ctx);
            }
            else if (query == Query::Info)
            {
                return Module::module_info();
            }
            else if (query == Query::Preset)
            {
                return Module::module_preset;
            }
            return std::monostate{};
        };
    private:
        std::variant<handle_type, std::shared_ptr<SystemModule>, std::monostate> _module{
            std::monostate{}
        };
    public:
        template<typename Module> static auto make()
        {
            return ModuleStub(handle<Module>);
        }

        ModuleStub() = default;
        ModuleStub(handle_type handle) : _module(handle) {}
        ModuleStub(const ModuleStub&) = default;
        ModuleStub(ModuleStub&&) = default;

        void commit(std::shared_ptr<IOContext> ctx);
        std::shared_ptr<SystemModule> ptr() const;
        SystemModule::ModInfo info() const;
        SystemModule::ModPreset preset() const;
        SystemModule::Status status() const;

        ModuleStub& operator=(const ModuleStub&) = default;
        ModuleStub& operator=(ModuleStub&&) = default;

        bool operator==(std::nullptr_t) const noexcept;
        bool operator!=(std::nullptr_t) const noexcept;
    };
} // namespace d2::sys
