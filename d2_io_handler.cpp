#include "d2_io_handler.hpp"

#include <chrono>
#include <logs/runtime_logs.hpp>
#include <memory>
#include <mods/d2_core.hpp>
#include <mt/pool.hpp>

#include <algorithm>
#include <d2_module.hpp>
#include <d2_screen.hpp>
#include <features.h>

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

    void IOContext::_remove_module(std::type_index idx, std::optional<std::string> id)
    {
        if (id.has_value())
        {
            if (id.value().empty())
                D2_THRW("Cannot remove dynamic module with empty ID");
            D2_TLOG(Info, "Removing module at ", id.value());
            const auto f = _named_modules.find(std::make_pair(idx, id.value()));
            if (f == _named_modules.end())
                D2_THRW("Failed to remove module ", id.value(), "; Not present");
            _named_modules.erase(f);
        }
        else
        {
            D2_TLOG(Info, "Removing module");
            const auto f = _modules.find(idx);
            if (f == _modules.end())
                D2_THRW("Failed to remove module; Not present");

            // Check for dependencies
            for (const auto& [idx, ptr] : _modules)
            {
                if (idx != idx)
                {
                    const auto deps = ptr->dependencies();
                    if (std::find(deps.begin(), deps.end(), idx) != deps.end())
                        D2_THRW(
                            "Failed to remove module ",
                            f->second->info().name,
                            "; Dependency ",
                            ptr->info().name
                        );
                }
            }
            _modules.erase(f);
        }
    }
    void
    IOContext::_insert_module(std::shared_ptr<sys::SystemModule> ptr, std::optional<std::string> id)
    {
        if (id.has_value())
        {
            if (id.value().empty())
                D2_THRW("Cannot insert dynamic module with empty ID");
            D2_TLOG(Info, "Inserting new module ", ptr->info().name, " at ", id.value())
            auto [_, created] =
                _named_modules.emplace(std::make_pair(ptr->info().index, id.value()), ptr);
            if (!created)
                D2_THRW(
                    "Failed to insert module ",
                    ptr->info().name,
                    " at ",
                    id.value(),
                    "; Already present"
                );
        }
        else
        {
            D2_TLOG(Info, "Inserting new module ", ptr->info().name)
            auto [_, created] = _modules.emplace(ptr->info().index, ptr);
            if (!created)
                D2_THRW("Failed to insert module ", ptr->info().name, "; Already present");
        }
    }

    std::shared_ptr<sys::SystemModule>
    IOContext::_get_module_if_uc(std::type_index idx, std::optional<std::string> id)
    {
        std::shared_lock lock(_module_mtx);
        if (id.has_value() && id.value().empty())
        {
            D2_TLOG(Warning, "Empty module ID")
            return nullptr;
        }
        std::shared_ptr<sys::SystemModule> ptr = nullptr;
        if (id.has_value())
        {
            auto f = _named_modules.find(std::make_pair(idx, id.value()));
            if (f == _named_modules.end())
                return nullptr;
            ptr = f->second;
        }
        else
        {
            auto f = _modules.find(idx);
            if (f == _modules.end())
                return nullptr;
            ptr = f->second;
        }
        lock.release();
        if (ptr)
        {
            if (ptr->status() == sys::SystemModule::Status::Offline)
            {
                D2_TLOG(
                    Info,
                    "Dynamically loading module: ",
                    ptr->info().name,
                    (id.has_value() ? " at " : ""),
                    (id.has_value() ? id.value() : "")
                )
                ptr->load();
            }
            else if (ptr->status() != sys::SystemModule::Status::Ok)
            {
                D2_TLOG(
                    Warning,
                    "Attempt to use a failed module: ",
                    ptr->info().name,
                    (id.has_value() ? " at " : ""),
                    (id.has_value() ? id.value() : "")
                );
                return nullptr;
            }
        }
        return ptr;
    }
    void IOContext::_load_modules(
        std::vector<std::pair<std::shared_ptr<sys::SystemModule>, std::optional<std::string>>> comps
    )
    {
        std::vector<std::pair<std::shared_ptr<sys::SystemModule>, std::optional<std::string>>>
            load_list;
        absl::flat_hash_set<std::type_index> visit_map;
        {
            std::lock_guard lock(_module_mtx);
            load_list.reserve(comps.size());
            for (auto& [ptr, id] : comps)
            {
                _insert_module(ptr, id);
                if (ptr->load_spec().type == sys::Load::Spec::Type::Immediate)
                    load_list.push_back({ptr, std::move(id)});
            }
        }
        for (const auto& [ptr, id] : load_list)
        {
            bool result = false;
            if (id.has_value())
            {
                std::unique_lock lock(_module_mtx);
                visit_map.insert(ptr->info().index);
                const auto deps = ptr->dependencies();
                lock.unlock();
                result = true;
                for (decltype(auto) it : deps)
                    if (!_load_module_recursive(it, visit_map))
                    {
                        D2_TLOG(
                            Warning,
                            "Detected cycle in module: ",
                            _modules[it]->info().name,
                            "; Required by: ",
                            ptr->info().name
                        )
                        result = false;
                        break;
                    }
                if (result)
                    ptr->load();
                visit_map.erase(ptr->info().index);
            }
            else
                result = _load_module_recursive(ptr->info().index, visit_map);
            if (!result)
            {
#if D2_COMPATIBILITY_MODE == STRICT
                if (id.has_value())
                    D2_THRW(
                        "Cycle detected or bad condition, failed to load module: ",
                        ptr->info().name,
                        " at ",
                        id.value()
                    );
                else
                    D2_THRW(
                        "Cycle detected or bad condition, failed to load module: ", ptr->info().name
                    );
#else
                if (id.has_value())
                    D2_TLOG(
                        Warning,
                        "Cycle detected or bad condition, failed to load module: ",
                        ptr->name(),
                        " at ",
                        id.value()
                    );
                else
                    D2_TLOG(
                        Warning,
                        "Cycle detected or bad condition, failed to load module: ",
                        ptr->name()
                    );
#endif
            }
        }
    }
    bool IOContext::_load_module_recursive(
        std::type_index idx, absl::flat_hash_set<std::type_index>& visit_map
    )
    {
        if (visit_map.contains(idx))
            return false;
        std::unique_lock lock(_module_mtx);
        const auto f = _modules.find(idx);
        if (f == _modules.end())
            return false;
        const auto ptr = f->second;
        if (ptr->status() != sys::SystemModule::Status::Offline)
            return true;
        visit_map.insert(idx);
        const auto deps = ptr->dependencies();
        lock.unlock();
        for (decltype(auto) it : deps)
            if (!_load_module_recursive(it, visit_map))
            {
                auto f = _modules.find(it);
                D2_TLOG(
                    Warning,
                    "Detected cycle in module: ",
                    ptr->info().name,
                    "; Requires: ",
                    (f == _modules.end() ? "Unknown" : f->second->info().name)
                )
                return false;
            }
        ptr->load();
        visit_map.erase(idx);
        return true;
    }

    void IOContext::set(ptr ptr) noexcept
    {
        _ptr = ptr;
    }
    IOContext::ptr IOContext::get() noexcept
    {
        return _ptr;
    }

    IOContext::IOContext(mt::ConcurrentPool::ptr scheduler, rs::RuntimeLogs::ptr logs) :
        _scheduler(scheduler), _logs(logs), Signals(scheduler)
    {
        rs::context::set(_logs);
    }
    IOContext::~IOContext()
    {
        std::lock_guard lock(_module_mtx);
        for (decltype(auto) it : _modules)
            it.second->unload();
        for (decltype(auto) it : _named_modules)
            it.second->unload();
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
                cfg.on_worker_start.emplace_back(
                    [ctx = std::weak_ptr(std::static_pointer_cast<IOContext>(shared_from_this()))](
                        mt::Worker&, const mt::Node::Snapshot&
                    )
                    {
                        const auto lock = ctx.lock();
                        if (lock)
                        {
                            rs::context::set(lock->_logs);
                            set(lock);
                        }
                    }
                );
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

    std::chrono::steady_clock::time_point
    IOContext::deadline(std::chrono::steady_clock::time_point tp)
    {
        _deadline = std::min(tp, _deadline);
        return _deadline;
    }
    std::chrono::steady_clock::time_point IOContext::deadline(std::chrono::milliseconds ms)
    {
        if (ms == std::chrono::milliseconds::max())
            return _deadline;
        const auto t = std::chrono::steady_clock::now();
        const auto n = ms + t;
        _deadline = std::min(n, _deadline);
        return _deadline;
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
        cnt += _modules.size();
        cnt += _named_modules.size();
        return cnt;
    }
    void IOContext::sysenum(
        std::function<void(sys::module<sys::SystemModule>, std::optional<std::string>)> callback
    )
    {
        std::shared_lock lock(_module_mtx);
        std::vector<std::shared_ptr<sys::SystemModule>> unnamed;
        std::vector<std::pair<std::shared_ptr<sys::SystemModule>, std::string>> named;
        unnamed.reserve(_modules.size());
        named.reserve(_named_modules.size());
        for (decltype(auto) it : _modules)
            unnamed.push_back(it.second);
        for (decltype(auto) it : _named_modules)
            named.push_back({it.second, it.first.second});
        lock.unlock();
        for (decltype(auto) it : _modules)
            callback(it.second, std::nullopt);
        for (decltype(auto) it : _named_modules)
            callback(it.second, it.first.second);
    }

    IOContext::module<sys::input> IOContext::input()
    {
        std::shared_lock lock(_module_mtx);
        return _get_module<sys::SystemInput>();
    }
    IOContext::module<sys::output> IOContext::output()
    {
        std::shared_lock lock(_module_mtx);
        return _get_module<sys::SystemOutput>();
    }
    IOContext::module<sys::screen> IOContext::screen()
    {
        std::shared_lock lock(_module_mtx);
        return _get_module<sys::SystemScreen>();
    }
} // namespace d2
