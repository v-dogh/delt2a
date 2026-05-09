#include "d2_io_handler.hpp"
#include "logs/runtime_logs.hpp"
#include "mods/d2_core.hpp"
#include "mt/pool.hpp"

#include <algorithm>
#include <d2_module.hpp>
#include <d2_screen.hpp>

namespace d2
{
    std::weak_ptr<const IOContext> IOContext::_weak() const
    {
        return std::static_pointer_cast<const IOContext>(weak_from_this().lock());
    }
    std::shared_ptr<const IOContext> IOContext::_shared() const
    {
        return std::static_pointer_cast<const IOContext>(shared_from_this());
    }
    std::weak_ptr<IOContext> IOContext::_weak()
    {
        return std::static_pointer_cast<IOContext>(weak_from_this().lock());
    }
    std::shared_ptr<IOContext> IOContext::_shared()
    {
        return std::static_pointer_cast<IOContext>(shared_from_this());
    }

    sys::SystemModule* IOContext::_get_component_if_uc(std::size_t uid)
    {
        std::shared_lock lock(_module_mtx);
        if (_components.size() < uid)
            return nullptr;
        auto* ptr = _components[uid].get();
        lock.release();
        if (ptr)
        {
            if (ptr->status() == sys::SystemModule::Status::Offline)
            {
                D2_TLOG(Info, "Dynamically loading module: ", ptr->name())
                ptr->load();
            }
            else if (ptr->status() != sys::SystemModule::Status::Ok)
            {
                D2_TLOG(Warning, "Attempt to use a failed module: ", ptr->name());
                return nullptr;
            }
        }
        return ptr;
    }
    void IOContext::_load_modules(std::vector<std::unique_ptr<sys::SystemModule>> comps)
    {
        std::vector<std::size_t> load_list;
        std::vector<bool> visit_map;
        const auto max = std::max_element(
            comps.begin(),
            comps.end(),
            [](const auto& v, const auto& m) { return v->uid() < m->uid(); }
        );
        {
            std::lock_guard lock(_module_mtx);
            _components.resize(std::max(_components.size(), (*max)->uid() + 1));
            load_list.reserve(comps.size());
            visit_map.resize(_components.size());
            std::fill(visit_map.begin(), visit_map.end(), false);
            for (decltype(auto) it : comps)
            {
                load_list.push_back(it->uid());
                _components[it->uid()] = std::move(it);
            }
        }
        for (decltype(auto) it : load_list)
        {
            const auto result = _load_module_recursive(it, visit_map);
#if D2_COMPATIBILITY_MODE == STRICT
            if (!result)
                D2_THRW(
                    std::format(
                        "Cycle detected, failed to load module: {}", _components[it]->name()
                    )
                );
#endif
        }
    }
    bool IOContext::_load_module_recursive(std::size_t uid, std::vector<bool>& visit_map)
    {
        if (visit_map[uid])
            return false;
        if (_components[uid]->status() != sys::SystemModule::Status::Offline)
            return true;
        visit_map[uid] = true;
        std::unique_lock lock(_module_mtx);
        const auto dv = _components[uid]->dependencies();
        const auto deps = std::vector(dv.begin(), dv.end());
        const auto addr = _components[uid].get();
        lock.unlock();
        for (decltype(auto) it : deps)
            if (!_load_module_recursive(it, visit_map))
            {
                D2_TLOG(
                    Warning,
                    "Detected cycle in module: ",
                    _components[it]->name(),
                    "; Required by: ",
                    _components[uid]->name()
                )
                return false;
            }
        lock.lock();
        if (addr == _components[uid].get())
            _components[uid]->load();
        visit_map[uid] = false;
        return true;
    }

    IOContext::IOContext(mt::ConcurrentPool::ptr scheduler, rs::RuntimeLogs::ptr logs) :
        _scheduler(scheduler), _logs(logs), Signals(scheduler)
    {
        rs::context::set(_logs);
    }
    IOContext::~IOContext()
    {
        std::lock_guard lock(_module_mtx);
        for (auto it = _components.rbegin(); it != _components.rend(); ++it)
            it->reset();
    }

    void IOContext::_initialize()
    {
        D2_TLOG(Module, "Initializing IO context")
        _main_thread = std::this_thread::get_id();
        _worker = sys<sys::input>()->worker();
        _scheduler->start(
            [&](std::size_t, mt::Node::Config& cfg)
            {
                cfg.growth_callback =
                    [main = _worker](std::size_t count, const mt::Node::Snapshot& snapshot)
                {
                    std::vector<mt::Worker::ptr> out;
                    out.resize(count == 0 ? 1 : count);
                    const auto off = std::size_t(snapshot.workers == 0);
                    if (snapshot.workers == 0)
                        out[0] = main;
                    std::transform(
                        out.begin() + off,
                        out.end(),
                        out.begin() + off,
                        [](const auto&) -> mt::Worker::ptr
                        {
                            return mt::Worker::make<mt::RingWorker>(
                                mt::Task::Immediate, std::chrono::seconds(10)
                            );
                        }
                    );
                    return out;
                };
                cfg.on_worker_start.emplace_back([logs =
                                                      _logs](mt::Worker&, const mt::Node::Snapshot&)
                                                 { rs::context::set(logs); });
            }
        );
        D2_TLOG(Module, "Success in initialization")
    }
    void IOContext::_deinitialize()
    {
        D2_TLOG(Module, "Deinitializing IO context")
        _worker->stop();
        _scheduler->stop();
        D2_TLOG(Module, "Success in deinitialization")
    }
    void IOContext::_run()
    {
        if (!_stop)
            D2_THRW("Another instance is already running");
        D2_TLOG(Module, "Starting IOContext")
        D2_TLOG(Info, "Activating extended code page")
        ExtendedCodePage::activate_thread();

        const auto in = input();
        const auto src = screen();

        D2_TLOG(Info, "Beginning main loop")
        _stop = false;
        while (!_stop)
        {
            D2_SAFE_BLOCK_BEGIN
            in->begincycle();
            src->tick();
            in->endcycle();
            D2_SAFE_BLOCK_END
            if (_deadline == std::chrono::steady_clock::time_point::max())
                _worker->wait();
            else
                _worker->wait(_deadline);
            _deadline = std::chrono::steady_clock::time_point::max();
        }
        D2_TLOG(Info, "Ending main loop")

        D2_TLOG(Info, "Deactivating extended code page")
        ExtendedCodePage::deactivate_thread();
        D2_TLOG(Module, "Stopped IOContext")
    }

    void IOContext::_insert_component(std::unique_ptr<sys::SystemModule> ptr)
    {
        D2_TLOG(Info, "Inserting new component: ", ptr->name())
        const auto idx = ptr->uid();
        std::lock_guard lock(_module_mtx);
        _components.resize(std::max(_components.size(), idx + 1));
        _components[idx] = std::move(ptr);
    }

    bool IOContext::is_synced() const
    {
        return std::this_thread::get_id() == _main_thread;
    }

    void IOContext::ping()
    {
        _worker->ping();
    }
    void IOContext::stop()
    {
        _stop = true;
        ping();
    }
    void IOContext::deadline(std::chrono::milliseconds ms)
    {
        const auto n = std::chrono::steady_clock::now() + ms;
        _deadline = std::min(n, _deadline);
    }

    bool IOContext::is_suspended() const
    {
        return _suspended;
    }
    void IOContext::suspend(bool state)
    {
        _suspended = true;
    }

    mt::ConcurrentPool::ptr IOContext::scheduler()
    {
        return _scheduler;
    }

    std::size_t IOContext::syscnt() const
    {
        std::shared_lock lock(_module_mtx);
        std::size_t cnt = 0;
        for (decltype(auto) it : _components)
            cnt += (it != nullptr);
        return cnt;
    }
    void IOContext::sysenum(std::function<void(sys::SystemModule*)> callback)
    {
        for (decltype(auto) it : _components)
            callback(it.get());
    }

    IOContext::module<sys::input> IOContext::input()
    {
        std::shared_lock lock(_module_mtx);
        return _get_component<sys::SystemInput>();
    }
    IOContext::module<sys::output> IOContext::output()
    {
        std::shared_lock lock(_module_mtx);
        return _get_component<sys::SystemOutput>();
    }
    IOContext::module<sys::screen> IOContext::screen()
    {
        std::shared_lock lock(_module_mtx);
        return _get_component<sys::SystemScreen>();
    }
} // namespace d2
