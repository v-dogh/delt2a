#include "d2_module.hpp"
#include <d2_io_handler.hpp>
#include <limits>

namespace d2::sys
{
    void SystemModule::_mark_safe(std::thread::id id)
    {
        if (_status == Status::Ok)
        {
            D2_TAG_MODULE_RUNTIME(name())
            D2_THRW("Cannot add safe threads after initialization");
        }
        _safe_threads.insert(id);
    }
    void SystemModule::_ensure_loaded(std::function<Status()> callback)
    {
        D2_TAG_MODULE_RUNTIME(name())
        Status s = _status.load(std::memory_order::acquire);
        while (true)
        {
            if (s == Status::Ok)
                return;
            if (s == Status::Loading)
            {
                _status.wait(Status::Loading, std::memory_order::acquire);
                s = _status.load(std::memory_order::acquire);
                continue;
            }
            Status expected = s;
            if (_status.compare_exchange_weak(
                    expected,
                    Status::Loading,
                    std::memory_order::acq_rel,
                    std::memory_order::acquire
                ))
                break;
            s = expected;
        }
        Status out = Status::Offline;
        const auto result = D2_SAFE_BLOCK_BEGIN out = callback();
        D2_SAFE_BLOCK_END
        if (!result)
            _status.store(Status::Failure, std::memory_order::release);
        _status.store(out, std::memory_order::release);
        _status.notify_all();
    }
    void SystemModule::_stat(Status status)
    {
        _status = status;
    }

    SystemModule::Status SystemModule::_load_impl()
    {
        return Status::Ok;
    }
    SystemModule::Status SystemModule::_unload_impl()
    {
        return Status::Ok;
    }

    SystemModule::SystemModule(
        std::weak_ptr<IOContext> ptr,
        const std::string& name,
        ModPreset preset,
        std::size_t static_usage
    ) : _ctx(ptr), _name(name), _preset(std::move(preset)), _static_usage(static_usage)
    {
    }

    Load::Spec SystemModule::load_spec() const noexcept
    {
        return _preset.spec;
    }
    Access SystemModule::access() const noexcept
    {
        return _preset.access;
    }
    std::size_t SystemModule::static_usage() const noexcept
    {
        return _static_usage;
    }
    std::span<const std::size_t> SystemModule::dependencies() const noexcept
    {
        return _preset.deps;
    }
    std::span<const std::string> SystemModule::dependency_names() const noexcept
    {
        return _preset.dep_names;
    }
    std::shared_ptr<IOContext> SystemModule::context() const noexcept
    {
        return _ctx.lock();
    }

    SystemModule::Status SystemModule::status()
    {
        return _status;
    }
    void SystemModule::load()
    {
        _ensure_loaded(
            [this]()
            {
                D2_TAG_MODULE_RUNTIME(name())
                const auto stat = _load_impl();
                if (_status == Status::Offline)
                {
                    D2_TLOG(Warning, "Module degraded")
                    return Status::Degraded;
                }
                else
                    D2_TLOG(Module, "Loaded module")
                return stat;
            }
        );
    }
    void SystemModule::unload()
    {
        _ensure_loaded(
            [this]()
            {
                D2_TAG_MODULE_RUNTIME(name())
                const auto stat = _unload_impl();
                if (_status == Status::Ok)
                    return Status::Offline;
                return stat;
            }
        );
    }
    bool SystemModule::accessible() const
    {
        if (_preset.access == Access::TUnsafe && _safe_threads.empty())
            return _ctx.lock()->is_synced();
        return _preset.access == Access::TSafe ||
               _safe_threads.contains(std::this_thread::get_id());
    }
    std::string SystemModule::name() const
    {
        return _name;
    }
} // namespace d2::sys
