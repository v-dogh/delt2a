#ifndef D2_MODULE_HPP
#define D2_MODULE_HPP

#include <absl/container/flat_hash_set.h>
#include <atomic>
#include <memory>
#include <span>
#include <thread>

#include <d2_exceptions.hpp>
#include <d2_io_handler_frwd.hpp>
#include <d2_meta.hpp>

namespace d2
{
    namespace sys
    {
        namespace impl
        {
            inline std::atomic<std::size_t> component_uid_track = 0;
            inline std::size_t component_uidgen()
            {
                return component_uid_track++;
            }

            struct ComponentUIDGeneratorBase
            {
                virtual std::size_t uid() const = 0;
                std::size_t uid()
                {
                    return const_cast<const ComponentUIDGeneratorBase*>(this)->uid();
                }
            };
        } // namespace impl

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

        class SystemModule : public impl::ComponentUIDGeneratorBase
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
                std::vector<std::size_t> deps{};
                std::vector<std::string> dep_names{};
            };
        private:
            const std::weak_ptr<IOContext> _ctx{};
            const std::string _name{"<Unknown>"};
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
                return std::make_unique<Type>(
                    ctx, std::string(Type::name), Type::module_preset, sizeof(Type)
                );
            }

            SystemModule(
                std::weak_ptr<IOContext> ptr,
                const std::string& name,
                ModPreset preset,
                std::size_t static_usage
            );
            virtual ~SystemModule() = default;

            Load::Spec load_spec() const noexcept;
            Access access() const noexcept;
            std::size_t static_usage() const noexcept;

            std::span<const std::size_t> dependencies() const noexcept;
            std::span<const std::string> dependency_names() const noexcept;
            std::shared_ptr<IOContext> context() const noexcept;

            Status status();
            void load();
            void unload();
            bool accessible() const;
            std::string name() const;

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

        template<meta::ConstString Name> struct AbstractModule : public SystemModule
        {
            D2_TAG_MODULE_VALUE(Name.view());
        public:
            static inline constexpr auto name = Name.view();
            static inline const auto uidc = impl::component_uidgen();

            using SystemModule::SystemModule;

            virtual std::size_t uid() const override
            {
                return uidc;
            }
        };
        template<typename Type, Access Ac, Load::Spec LoadSpec, typename... Deps>
        struct ConcreteModule
        {
            static inline const SystemModule::ModPreset module_preset{
                .access = Ac,
                .spec = LoadSpec,
                .deps = {Deps::uidc...},
                .dep_names = {std::string(Deps::name)...}
            };
        };

        template<typename Module> class ModulePtr
        {
        private:
            Module* _ptr{nullptr};
        public:
            ModulePtr(std::nullptr_t) {}
            ModulePtr() = default;
            ModulePtr(const ModulePtr&) = default;
            ModulePtr(ModulePtr&&) = default;
            ModulePtr(Module* ptr) : _ptr(ptr) {}

            Module* ptr() const
            {
                auto bptr = static_cast<SystemModule*>(_ptr);
                if (!bptr->accessible())
                    throw std::logic_error{std::format(
                        "Module '{}' must be accessed from the main thread", bptr->name()
                    )};
                return _ptr;
            }

            Module* operator->() const
            {
                return ptr();
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
    } // namespace sys
} // namespace d2
#endif // D2_MODULE_HPP
