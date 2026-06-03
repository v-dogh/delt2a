#include "d2_io_handler.hpp"

#include <algorithm>
#include <chrono>
#include <d2_module.hpp>
#include <d2_screen.hpp>
#include <features.h>
#include <logs/runtime_logs.hpp>
#include <memory>
#include <mods/d2_core.hpp>
#include <mt/pool.hpp>

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

    d2::sys::ModuleStub*
    IOContext::_find_module(std::type_index index, std::optional<std::string> id)
    {
        auto module_it = _modules.find(index);
        if (module_it == _modules.end())
            return nullptr;
        if (!id.has_value())
            return &module_it->second.unnamed;
        auto named_it = module_it->second.named.find(*id);
        if (named_it == module_it->second.named.end())
            return nullptr;
        return &named_it->second;
    }

    void IOContext::_remove_module(std::type_index idx, std::optional<std::string> id)
    {
        if (id.has_value())
        {
            if (id.value().empty())
                D2_THRW("Cannot remove dynamic module with empty ID");
            D2_TLOG(Info, "Removing module at ", id.value());
            const auto f = _modules.find(idx);
            if (f == _modules.end())
                D2_THRW("Failed to remove module ", id.value(), "; Not present");
            auto ff = f->second.named.find(id.value());
            if (ff == f->second.named.end())
                D2_THRW("Failed to remove module ", id.value(), "; Not present");
            f->second.named.erase(ff);
        }
        else
        {
            D2_TLOG(Info, "Removing module");
            const auto f = _modules.find(idx);
            if (f == _modules.end() || f->second.unnamed == nullptr)
                D2_THRW("Failed to remove module; Not present");

            // Check for dependencies
            for (const auto& [idx, ptr] : _modules)
            {
                if (idx != idx)
                {
                    const auto deps = ptr.unnamed.preset().deps;
                    if (std::find(deps.begin(), deps.end(), idx) != deps.end())
                        D2_THRW(
                            "Failed to remove module ",
                            f->second.unnamed.info().name,
                            "; Dependency ",
                            ptr.unnamed.info().name
                        );
                }
            }
            _modules.erase(f);
        }
    }
    void IOContext::_insert_module(sys::ModuleStub ptr, std::optional<std::string> id)
    {
        if (id.has_value())
        {
            if (id.value().empty())
                D2_THRW("Cannot insert dynamic module with empty ID");
            D2_TLOG(Info, "Inserting new module ", ptr.info().name, " at ", id.value());
            auto f = _modules.emplace(ptr.info().index, ModuleEntry{}).first;
            auto [ff, created] = f->second.named.emplace(id.value(), ptr);
            if (!created)
                D2_THRW(
                    "Failed to insert module ",
                    ptr.info().name,
                    " at ",
                    id.value(),
                    "; Already present"
                );
        }
        else
        {
            D2_TLOG(Info, "Inserting new module ", ptr.info().name)
            auto [f, created] = _modules.emplace(ptr.info().index, ModuleEntry{});
            if (!created && f->second.unnamed != nullptr)
                D2_THRW("Failed to insert module ", ptr.info().name, "; Already present");
            f->second.unnamed = ptr;
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
        sys::ModuleStub* stub = _find_module(idx, id);
        lock.release();
        if (stub)
        {
            if (stub->status() == sys::SystemModule::Status::Offline)
            {
                D2_TLOG(
                    Info,
                    "Dynamically loading module: ",
                    stub->info().name,
                    (id.has_value() ? " at " : ""),
                    (id.has_value() ? id.value() : "")
                )
                stub->commit(_shared());
                stub->ptr()->load();
            }
            else if (stub->status() != sys::SystemModule::Status::Ok)
            {
                D2_TLOG(
                    Warning,
                    "Attempt to use a failed module: ",
                    stub->info().name,
                    (id.has_value() ? " at " : ""),
                    (id.has_value() ? id.value() : "")
                );
                return nullptr;
            }
        }
        return stub->ptr();
    }
    void IOContext::_load_modules(
        std::vector<std::pair<sys::ModuleStub, std::optional<std::string>>> comps
    )
    {
        struct ModuleLoadRef
        {
            std::type_index index;
            std::optional<std::string> id;
        };
        std::vector<ModuleLoadRef> load_list;
        absl::flat_hash_set<std::type_index> visit_map;
        {
            std::lock_guard lock(_module_mtx);
            load_list.reserve(comps.size());
            for (const auto& [stub, id] : comps)
            {
                const auto info = stub.info();
                const auto preset = stub.preset();
                _insert_module(stub, id);
                if (preset.spec.type == sys::Load::Spec::Type::Immediate)
                {
                    load_list.push_back({info.index, id});
                }
            }
        }
        for (decltype(auto) load_ref : load_list)
        {
            bool result = false;
            if (load_ref.id.has_value())
            {
                std::unique_lock lock(_module_mtx);
                auto* stub = _find_module(load_ref.index, load_ref.id);
                if (!stub)
                {
                    D2_THRW("Module disappeared before load: ", load_ref.id.value());
                }

                const auto info = stub->info();
                const auto preset = stub->preset();
                const auto deps = preset.deps;

                visit_map.insert(info.index);

                lock.unlock();
                result = true;
                for (decltype(auto) dep : deps)
                {
                    if (!_load_module_recursive(dep, visit_map))
                    {
                        std::string required_by = std::string(info.name);
                        std::string dependency_name = "<unknown>";
                        {
                            std::lock_guard relock(_module_mtx);
                            auto dep_it = _modules.find(dep);
                            if (dep_it != _modules.end())
                                dependency_name = dep_it->second.unnamed.info().name;
                        }
                        D2_TLOG(
                            Warning,
                            "Detected cycle in module: ",
                            dependency_name,
                            "; Required by: ",
                            required_by
                        )
                        result = false;
                        break;
                    }
                }
                if (result)
                {
                    std::unique_lock commit_lock(_module_mtx);
                    stub = _find_module(load_ref.index, load_ref.id);
                    if (!stub)
                    {
                        D2_THRW("Module disappeared before commit: ", load_ref.id.value());
                    }
                    stub->commit(_shared());
                    stub->ptr()->load();
                }
                visit_map.erase(info.index);
            }
            else
            {
                result = _load_module_recursive(load_ref.index, visit_map);
            }

            if (!result)
            {
                std::string module_name = "<Unknown>";
                {
                    std::lock_guard lock(_module_mtx);
                    auto* stub = _find_module(load_ref.index, load_ref.id);
                    if (stub)
                        module_name = stub->info().name;
                }
#if D2_COMPATIBILITY_MODE == STRICT
                if (load_ref.id.has_value())
                    D2_THRW(
                        "Cycle detected or bad condition, failed to load module: ",
                        module_name,
                        " at ",
                        load_ref.id.value()
                    );
                else
                    D2_THRW(
                        "Cycle detected or bad condition, failed to load module: ", module_name
                    );
#else
                if (load_ref.id.has_value())
                    D2_TLOG(
                        Warning,
                        "Cycle detected or bad condition, failed to load module: ",
                        module_name,
                        " at ",
                        load_ref.id.value()
                    );
                else
                    D2_TLOG(
                        Warning,
                        "Cycle detected or bad condition, failed to load module: ",
                        module_name
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
        if (f == _modules.end() || f->second.unnamed == nullptr)
            return false;
        auto ptr = f->second;
        if (ptr.unnamed.status() != sys::SystemModule::Status::Offline)
            return true;
        visit_map.insert(idx);
        const auto deps = ptr.unnamed.preset().deps;
        lock.unlock();
        for (decltype(auto) it : deps)
            if (!_load_module_recursive(it, visit_map))
            {
                auto f = _modules.find(it);
                D2_TLOG(
                    Warning,
                    "Detected cycle in module: ",
                    ptr.unnamed.info().name,
                    "; Requires: ",
                    ((f == _modules.end() || f->second.unnamed == nullptr)
                         ? "Unknown"
                         : f->second.unnamed.info().name)
                )
                return false;
            }
        ptr.unnamed.commit(_shared());
        ptr.unnamed.ptr()->load();
        visit_map.erase(idx);
        return true;
    }

    void IOContext::_sysenum(
        std::optional<std::type_index> index,
        std::function<void(
            sys::SystemModule::ModInfo,
            sys::SystemModule::ModPreset,
            std::optional<sys::module<sys::SystemModule>>,
            std::optional<std::string>
        )> callback
    )
    {
        std::shared_lock lock(_module_mtx);
        std::vector<std::tuple<
            sys::SystemModule::ModInfo,
            sys::SystemModule::ModPreset,
            std::optional<sys::module<sys::SystemModule>>,
            std::optional<std::string>
        >>
            mods;
        auto sub = [&](const auto& it)
        {
            if (it.second.unnamed != nullptr)
            {
                const auto ptr = it.second.unnamed.ptr();
                mods.push_back(
                    {it.second.unnamed.info(),
                     it.second.unnamed.preset(),
                     ptr ? std::optional(ptr) : std::nullopt,
                     std::nullopt}
                );
            }
            for (decltype(auto) it : it.second.named)
            {
                const auto ptr = it.second.ptr();
                mods.push_back(
                    {it.second.info(),
                     it.second.preset(),
                     ptr ? std::optional(ptr) : std::nullopt,
                     it.first}
                );
            }
        };
        if (index.has_value())
        {
            auto f = _modules.find(index.value());
            if (f != _modules.end())
                sub(*f);
        }
        else
        {
            for (decltype(auto) it : _modules)
                sub(it);
        }
        lock.unlock();
        for (const auto& [info, preset, ptr, name] : mods)
            callback(info, preset, ptr, name);
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
        {
            if (it.second.unnamed != nullptr)
            {
                const auto ptr = it.second.unnamed.ptr();
                if (ptr)
                    ptr->unload();
            }
            for (decltype(auto) it : it.second.named)
            {
                const auto ptr = it.second.ptr();
                if (ptr)
                    ptr->unload();
            }
        }
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

    void IOContext::sysenum(
        std::function<void(
            sys::SystemModule::ModInfo,
            sys::SystemModule::ModPreset,
            std::optional<sys::module<sys::SystemModule>>,
            std::optional<std::string>
        )> callback
    )
    {
        _sysenum(std::nullopt, std::move(callback));
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
