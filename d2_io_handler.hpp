#ifndef D2_IO_HANDLER_HPP
#define D2_IO_HANDLER_HPP

#include <absl/container/flat_hash_set.h>
#include <chrono>
#include <memory>
#include <shared_mutex>

#include "mt/pool.hpp"
#include <d2_exceptions.hpp>
#include <d2_io_handler_frwd.hpp>
#include <d2_main_worker.hpp>
#include <d2_module.hpp>
#include <d2_signal_handler.hpp>
#include <mods/d2_core.hpp>

namespace d2
{
    class IOContext : public Signals
    {
        D2_TAG_MODULE(ctx)
    public:
        template<typename Type> using module = sys::module<Type>;
        template<typename Type> using future = mt::future<Type>;
        using ptr = std::shared_ptr<IOContext>;
        using wptr = std::weak_ptr<IOContext>;
    private:
        mutable std::shared_mutex _module_mtx{};
        std::vector<std::unique_ptr<sys::SystemModule>> _components{};
        rs::RuntimeLogs::ptr _logs{nullptr};
        MainWorker::ptr _worker{nullptr};
        mt::ConcurrentPool::ptr _scheduler{nullptr};
        std::thread::id _main_thread{};
        std::chrono::steady_clock::time_point _deadline{
            std::chrono::steady_clock::time_point::max()
        };
        std::atomic<bool> _stop{true};
        std::atomic<bool> _suspended{false};

        std::weak_ptr<const IOContext> _weak() const;
        std::shared_ptr<const IOContext> _shared() const;
        std::weak_ptr<IOContext> _weak();
        std::shared_ptr<IOContext> _shared();

        void _initialize();
        void _deinitialize();
        void _run();

        template<typename Type> module<Type> _get_component()
        {
            auto* mod = _get_component_if<Type>();
            if (mod == nullptr)
                D2_THRW("Attempt to query inactive component");
            return mod;
        }
        template<typename Type> Type* _get_component_if()
        {
            return static_cast<Type*>(_get_component_if_uc(Type::uidc));
        }
        void _insert_component(std::unique_ptr<sys::SystemModule> ptr);

        sys::SystemModule* _get_component_if_uc(std::size_t id);
        void _load_modules(std::vector<std::unique_ptr<sys::SystemModule>> comps);
        bool _load_module_recursive(std::size_t uid, std::vector<bool>& visit_map);
    public:
        template<typename... Components> static ptr make(rs::Config logs_cfg = {})
        {
            auto logs = rs::RuntimeLogs::make(logs_cfg);
            auto scheduler = mt::ConcurrentPool::make<mt::SimpleTopology>();
            auto ptr = std::make_shared<IOContext>(scheduler, logs);
            ptr->load<Components...>();
            return ptr;
        }

        IOContext(mt::ConcurrentPool::ptr scheduler, rs::RuntimeLogs::ptr logs);
        virtual ~IOContext();

        // State

        template<typename Tree, typename... Trees> void run(auto&&... themes);
        void stop();
        void deadline(std::chrono::milliseconds ms);
        void ping();

        bool is_suspended() const;
        void suspend(bool state);

        // Synchronization

        mt::ConcurrentPool::ptr scheduler();

        bool is_synced() const;

        template<typename Func, typename... Argv>
        auto sync(Func&& callback, Argv&&... args) -> std::invoke_result_t<Func&, Argv...>
        {
            if (is_synced())
            {
                return callback();
            }
            else
            {
                using ret = std::invoke_result_t<Func, Argv...>;
                mt::Barrier lock;
                if constexpr (std::is_same_v<void, ret>)
                {
                    _worker->accept({[&](mt::Task::Query query, std::any& out)
                                     {
                                         if (query == mt::Task::Query::Type)
                                         {
                                             out = static_cast<unsigned char>(mt::Task::Immediate);
                                         }
                                         else if (query == mt::Task::Query::Run)
                                         {
                                             callback(std::forward<Argv>(args)...);
                                             lock.release();
                                         }
                                     }});
                    lock.wait();
                }
                else
                {
                    ret out;
                    _worker->accept({[&](mt::Task::Query query, std::any& out)
                                     {
                                         if (query == mt::Task::Query::Type)
                                         {
                                             out = static_cast<unsigned char>(mt::Task::Immediate);
                                         }
                                         else if (query == mt::Task::Query::Run)
                                         {
                                             out = callback(std::forward<Argv>(args)...);
                                             lock.release();
                                         }
                                     }});
                    lock.wait();
                    return out;
                }
            }
        }
        template<typename Func, typename... Argv>
        auto sync_async(Func&& callback, Argv&&... args)
            -> mt::future<std::invoke_result_t<Func&, Argv...>>
        {
            using ret = std::invoke_result_t<Func, Argv...>;
            mt::future<ret> task = mt::future<ret>::make();
            if (is_synced())
            {
                if constexpr (std::is_same_v<ret, void>)
                {
                    callback(std::forward<Argv>(args)...);
                    task.block()->set_value();
                }
                else
                {
                    task.block()->set_value(callback(std::forward<Argv>(args)...));
                }
            }
            else
            {
                _worker->accept(
                    {[callback = std::forward<Func>(callback),
                      ... args =
                          std::forward<Argv>(args)](mt::Task::Query query, std::any& out) mutable
                     {
                         if (query == mt::Task::Query::Type)
                         {
                             out = static_cast<unsigned char>(mt::Task::Immediate);
                         }
                         else if (query == mt::Task::Query::Run)
                         {
                             if constexpr (std::is_same_v<ret, void>)
                             {
                                 callback(std::forward<Argv>(args)...);
                                 task.block()->set_value();
                             }
                             else
                             {
                                 task.block()->set_value(callback(std::forward<Argv>(args)...));
                             }
                         }
                     }}
                );
            }
            return task;
        }

        // Modules

        template<typename... Components> void load()
        {
            std::vector<std::unique_ptr<sys::SystemModule>> modules{};
            modules.reserve(sizeof...(Components));
            auto insert = [&]<typename Component>()
            {
#if D2_COMPATIBILITY_MODE == STRICT
                static_assert(
                    !std::is_same_v<Component, void>, "Attempt to load invalid component"
                );
#endif
                if constexpr (!std::is_same_v<Component, void>)
                {
                    if (Component::module_preset.spec.type == sys::Load::Spec::Type::Deferred)
                    {
                        scheduler()->launch_deferred(
                            std::chrono::milliseconds{Component::module_preset.spec.ms},
                            [this](auto&&...)
                            {
                                std::vector<std::unique_ptr<sys::SystemModule>> v;
                                v.push_back(sys::SystemModule::make<Component>(_weak()));
                                _load_modules(std::move(v));
                            }
                        );
                    }
                    else
                    {
                        auto ptr = sys::SystemModule::make<Component>(_weak());
                        if (Component::module_preset.spec.type == sys::Load::Spec::Type::Lazy)
                            _insert_component(std::move(ptr));
                        else
                            modules.push_back(std::move(ptr));
                    }
                }
            };
            (insert.template operator()<Components>(), ...);
            _load_modules(std::move(modules));
        }

        std::size_t syscnt() const;
        void sysenum(std::function<void(sys::SystemModule*)> callback);

        template<typename Component>
        void sys_run(auto&& callback, auto&& fallback = [](sys::SystemModule::Status) {})
        {
            if (auto* ptr = sys_if<Component>();
                ptr && ptr->status() == sys::SystemModule::Status::Ok)
                sync_if(callback, ptr);
            else
                fallback(ptr ? ptr->status() : sys::SystemModule::Status::Offline);
        }
        template<typename Component> auto sys()
        {
            return _get_component<Component>();
        }
        template<typename Component> auto sys_if()
        {
            auto* ptr = _get_component_if<Component>();
            return ptr ? (ptr->status() == sys::SystemModule::Status::Ok ? ptr : nullptr) : nullptr;
        }

        module<sys::input> input();
        module<sys::output> output();
        module<sys::screen> screen();
    };
} // namespace d2

#endif // D2_IO_HANDLER_HPP
