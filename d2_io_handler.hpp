#ifndef D2_IO_HANDLER_HPP
#define D2_IO_HANDLER_HPP

#include "logs/runtime_logs.hpp"
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <chrono>
#include <memory>
#include <shared_mutex>

#include <d2_exceptions.hpp>
#include <d2_io_handler_frwd.hpp>
#include <d2_main_worker.hpp>
#include <d2_module.hpp>
#include <d2_signal_handler.hpp>
#include <mods/d2_core.hpp>
#include <mt/pool.hpp>
#include <stop_token>

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
        static inline thread_local ptr _ptr{nullptr};

        mutable std::shared_mutex _module_mtx{};
        absl::flat_hash_map<std::type_index, std::shared_ptr<sys::SystemModule>> _modules{};
        absl::flat_hash_map<
            std::pair<std::type_index, std::string>,
            std::shared_ptr<sys::SystemModule>
        >
            _named_modules{};

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

        template<typename Type> module<Type> _get_module()
        {
            auto mod = _get_module_if<Type>();
            if (mod == nullptr)
                D2_THRW("Attempt to query inactive component");
            return mod;
        }
        template<typename Type> module<Type> _get_module_if()
        {
            return std::static_pointer_cast<Type>(_get_module_if_uc(typeid(Type)));
        }

        void _remove_module(std::type_index idx, std::optional<std::string> id = std::nullopt);
        void _insert_module(
            std::shared_ptr<sys::SystemModule> ptr, std::optional<std::string> id = std::nullopt
        );

        std::shared_ptr<sys::SystemModule>
        _get_module_if_uc(std::type_index idx, std::optional<std::string> id = std::nullopt);
        void _load_modules(
            std::vector<std::pair<std::shared_ptr<sys::SystemModule>, std::optional<std::string>>>
                comps
        );
        bool _load_module_recursive(
            std::type_index idx, absl::flat_hash_set<std::type_index>& visit_map
        );
    public:
        static void set(ptr) noexcept;
        static ptr get() noexcept;

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
                                             out = mt::Task::Token::Discard;
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
                                             out = mt::Task::Token::Discard;
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
                      ... args = std::forward<Argv>(args),
                      task](mt::Task::Query query, std::any& out) mutable
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
                             out = mt::Task::Token::Discard;
                         }
                     }}
                );
            }
            return task;
        }

        // Allocate and automatically initialize a thread
        // (stuff like logs, pool etc.)
        template<typename Func, typename... Argv>
        std::jthread launch_thread(Func&& callback, Argv&&... args)
        {
            static constexpr auto uses_stop_token =
                std::is_invocable_v<Func, std::stop_token, Argv...>;
            if constexpr (uses_stop_token)
            {
                return std::jthread(
                    [callback = std::forward<Func>(callback),
                     ... args = std::forward<Argv>(args),
                     ctx = std::weak_ptr(std::static_pointer_cast<IOContext>(shared_from_this()))](
                        std::stop_token stop
                    )
                    {
                        if (const auto lock = ctx.lock(); lock != nullptr)
                        {
                            rs::context::set(lock->_logs);
                            mt::context::set(lock->_scheduler);
                            set(lock);
                        }
                        callback(stop, std::forward<Argv>(args)...);
                    }
                );
            }
            else
            {
                return std::jthread(
                    [callback = std::forward<Func>(callback),
                     ... args = std::forward<Argv>(args),
                     ctx = std::weak_ptr(std::static_pointer_cast<IOContext>(shared_from_this()))]()
                    {
                        if (const auto lock = ctx.lock(); lock != nullptr)
                        {
                            rs::context::set(lock->_logs);
                            mt::context::set(lock->_scheduler);
                            set(lock);
                        }
                        callback(std::forward<Argv>(args)...);
                    }
                );
            }
        }

        // Modules

        template<typename Module> void unload()
        {
            std::lock_guard lock(_module_mtx);
            _remove_module(Module::module_info().index);
        }
        template<typename Module> void unload(const std::string& id)
        {
            std::lock_guard lock(_module_mtx);
            _remove_module(Module::module_info().index, id);
        }
        template<typename... Modules> void load()
        {
            std::vector<std::pair<std::shared_ptr<sys::SystemModule>, std::optional<std::string>>>
                modules{};
            auto insert = [&]<typename Module>()
            {
#if D2_COMPATIBILITY_MODE == STRICT
                static_assert(!std::is_same_v<Module, void>, "Attempt to load invalid component");
#endif
                if constexpr (!std::is_same_v<Module, void>)
                {
                    if (Module::module_preset.spec.type == sys::Load::Spec::Type::Deferred)
                    {
                        scheduler()->launch_deferred(
                            std::chrono::milliseconds{Module::module_preset.spec.ms},
                            [this](auto&&...)
                            {
                                // Force load it
                                _get_module_if<Module>();
                            }
                        );
                    }
                    auto ptr = sys::SystemModule::make<Module>(_weak());
                    modules.push_back({std::move(ptr), std::nullopt});
                }
            };
            (insert.template operator()<Modules>(), ...);
            if (!modules.empty())
            {
                // If the module is Lazy or Deferred this is only a "preload"
                // That is, we only allocate the space for the module and not initialize it
                _load_modules(std::move(modules));
            }
        }
        template<typename Module> void load(const std::string& id)
        {
            if (Module::module_preset.spec.type == sys::Load::Spec::Type::Deferred)
            {
                scheduler()->launch_deferred(
                    std::chrono::milliseconds{Module::module_preset.spec.ms},
                    [this](auto&&...) { _get_module_if<Module>(); }
                );
            }
            auto ptr = sys::SystemModule::make<Module>(_weak());
            _load_modules({std::make_pair(ptr, id)});
        }

        std::size_t syscnt() const;
        void sysenum(
            std::function<void(sys::module<sys::SystemModule>, std::optional<std::string>)> callback
        );

        template<typename Module> auto sys()
        {
            return _get_module<Module>();
        }
        template<typename Module> auto sys_if()
        {
            auto ptr = _get_module_if<Module>();
            return (ptr != nullptr)
                       ? (ptr->status() == sys::SystemModule::Status::Ok ? ptr : nullptr)
                       : nullptr;
        }

        module<sys::input> input();
        module<sys::output> output();
        module<sys::screen> screen();
    };
} // namespace d2

#endif // D2_IO_HANDLER_HPP
