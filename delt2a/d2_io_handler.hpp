#ifndef D2_IO_HANDLER_HPP
#define D2_IO_HANDLER_HPP

#include <absl/container/flat_hash_set.h>
#include <shared_mutex>
#include <mutex>
#include <memory>

#include "d2_signal_handler.hpp"
#include "d2_exceptions.hpp"
#include "d2_io_handler_frwd.hpp"
#include "d2_module.hpp"
#include "mt/thread_pool.hpp"

namespace d2
{
    class IOContext : public Signals
	{
        D2_TAG_MODULE(ctx)
	public:
        template<typename Type>
        using module = sys::module<Type>;
		template<typename Type>
        using future = mt::future<Type>;
		using ptr = std::shared_ptr<IOContext>;
		using wptr = std::weak_ptr<IOContext>;
	private:
        mutable std::shared_mutex _module_mtx{};
        std::vector<std::unique_ptr<sys::SystemComponent>> _components{};
        rs::RuntimeLogs::ptr _logs{ nullptr };
        mt::ThreadPool::ptr _scheduler{ nullptr };
        mt::ThreadPool::Worker _worker{};
        std::thread::id _main_thread{};
        std::chrono::steady_clock::time_point _deadline{ std::chrono::steady_clock::time_point::max() };
        std::atomic<bool> _stop{ true };
        std::atomic<bool> _suspended{ false };

        std::weak_ptr<const IOContext> _weak() const;
        std::shared_ptr<const IOContext> _shared() const;
        std::weak_ptr<IOContext> _weak();
        std::shared_ptr<IOContext> _shared();

        void _initialize();
        void _deinitialize();
        void _run();

        template<typename Type> module<Type> _get_component()
		{
            const auto uid = Type::uidc;
            [[ unlikely ]] if (_components.size() < uid)
                D2_THRW("Attempt to query inactive component");
            auto* ptr = _components[uid].get();
            if (ptr->status() == sys::SystemComponent::Status::Offline)
            {
                D2_TLOG(Info, "Dynamically loading module: ", ptr->name())
                ptr->load();
            }
            return static_cast<Type*>(ptr);
		}
		template<typename Type> Type* _get_component_if()
		{
			const auto uid = Type::uidc;
            [[ unlikely ]] if (_components.size() < uid)
				return nullptr;
            auto* ptr = _components[uid].get();
            if (ptr->status() == sys::SystemComponent::Status::Offline)
            {
                D2_TLOG(Info, "Dynamically loading module: ", ptr->name())
                ptr->load();
            }
            return static_cast<Type*>(ptr);
		}
        void _insert_component(std::unique_ptr<sys::SystemComponent> ptr);
	public:
        template<typename... Components>
        static ptr make(
            mt::ThreadPool::Config scfg = {
                .flags =
                    mt::ThreadPool::Config::Flags::AutoGrow |
                    mt::ThreadPool::Config::Flags::AutoShrink,
                .min_threads = 1,
                .max_idle_time = std::chrono::seconds(5)
            },
            rs::Config lcfg = {}
        )
		{
            using flags = mt::ThreadPool::Config::Flags;
            auto logs = rs::RuntimeLogs::make(lcfg);
            scfg.flags |= flags::MainCyclicWorker;
            scfg.event_launch = [p = std::weak_ptr(logs)](std::size_t) {
                rs::context::set(p.lock());
            };
            auto scheduler = mt::ThreadPool::make(scfg);
            auto ptr = std::make_shared<IOContext>(scheduler, logs);
			ptr->load<Components...>();
            return ptr;
		}

        IOContext(mt::ThreadPool::ptr scheduler, rs::RuntimeLogs::ptr logs);
        virtual ~IOContext();

        // State

        template<typename Tree, typename... Trees>
        void run(auto&&... themes);
        void stop();
        void deadline(std::chrono::milliseconds ms);
        void ping();

        bool is_suspended() const;
        void suspend(bool state);

        // Synchronization

        mt::ThreadPool::ptr scheduler();

        bool is_synced() const
        {
            return std::this_thread::get_id() == _main_thread;
        }
        template<typename Func>
        auto sync(Func&& callback) -> mt::future<decltype(callback())>
        {
            return scheduler()->launch_deferred(std::chrono::milliseconds(0), [callback = std::forward<Func>(callback)]() -> decltype(auto)
            {
                return callback();
            });
        }
        template<typename Func>
        auto sync_async_if(Func&& callback) -> mt::future<decltype(callback())>
        {
            if (is_synced())
            {
                using ret = decltype(callback());
                mt::future<ret> task = mt::future<ret>::make();
                if constexpr (std::is_same_v<ret, void>)
                {
                    callback();
                    task.block()->set_value();
                }
                else
                {
                    task.block()->set_value(callback());
                }
                return task;
            }
            else
            {
                return scheduler()->launch_deferred(std::chrono::milliseconds(0), [callback = std::forward<Func>(callback)]() -> decltype(auto)
                {
                    return callback();
                });
            }
        }
        template<typename Func>
        auto sync_if(Func&& callback) -> decltype(callback())
        {
            if (is_synced())
            {
                return callback();
            }
            else
            {
                return scheduler()->launch_deferred(std::chrono::milliseconds(0), [callback = std::forward<Func>(callback)]() -> decltype(auto)
                {
                    return callback();
                }).value();
            }
        }

        // Modules

        template<typename... Components>
        void load()
        {
            std::lock_guard lock(_module_mtx);
            auto insert = [this]<typename Component>()
            {
#               if D2_COMPATIBILITY_MODE == STRICT
                static_assert(!std::is_same_v<Component, void>, "Attempt to load invalid component");
                _insert_component(std::make_unique<Component>(
                    _weak(),
                    std::string(Component::name),
                    Component::tsafe
                    ));
#               else
                if constexpr (!std::is_same_v<Component, void>)
                    _insert_component(std::make_unique<Component>(
                        _weak(),
                        std::string(Component::name),
                        Component::tsafe
                        ));
#               endif
                if (!Component::lazy)
                    _get_component<Component>();
            };
            (insert.template operator()<Components>(), ...);
        }

        std::size_t syscnt() const;
        void sysenum(std::function<void(sys::SystemComponent*)> callback);

        template<typename Component> void sys_run(
            auto&& callback,
            auto&& fallback = [](sys::SystemComponent::Status) {}
        )
        {
            if (auto* ptr = sys_if<Component>();
                ptr && ptr->status() == sys::SystemComponent::Status::Ok)
                sync_if(callback, ptr);
            else
                fallback(
                    ptr ?
                        ptr->status() :
                        sys::SystemComponent::Status::Offline
                );
        }
        template<typename Component> auto sys()
		{
            std::shared_lock lock(_module_mtx);
			return _get_component<Component>();
		}
        template<typename Component> auto sys_if()
		{
            std::shared_lock lock(_module_mtx);
            auto* ptr = _get_component_if<Component>();
            return ptr ?
                (ptr->status() == sys::SystemComponent::Status::Ok ?
                    ptr : nullptr) :
                nullptr;
		}

        module<sys::input> input();
        module<sys::output> output();
        module<sys::screen> screen();
    };
} // d2

#endif // D2_IO_HANDLER_HPP
