#include "d2_module.hpp"
#include "d2_exceptions.hpp"
#include <d2_io_handler.hpp>
#include <memory>
#include <typeindex>
#include <variant>

namespace d2::sys
{
    void SystemModule::_mark_safe(std::thread::id id)
    {
        if (_status == Status::Ok)
        {
            D2_TAG_MODULE_RUNTIME(info().name)
            D2_THRW("Cannot add safe threads after initialization");
        }
        _safe_threads.insert(id);
    }
    void SystemModule::_ensure_loaded(std::function<Status()> callback)
    {
        D2_TAG_MODULE_RUNTIME(info().name)
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

    SystemModule::Compat SystemModule::compatible()
    {
        return Compat::Unknown;
    }

    SystemModule::SystemModule(
        std::weak_ptr<IOContext> ptr, ModPreset preset, std::size_t static_usage
    ) : _ctx(ptr), _preset(std::move(preset)), _static_usage(static_usage)
    {
    }

    SystemModule::ModPreset SystemModule::preset() const
    {
        return _preset;
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
    std::span<const std::type_index> SystemModule::dependencies() const noexcept
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
                D2_TAG_MODULE_RUNTIME(info().name)
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
                D2_TAG_MODULE_RUNTIME(info().name)
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

    void ModuleStub::commit(std::shared_ptr<IOContext> ctx)
    {
        if (std::holds_alternative<std::monostate>(_module))
            D2_THRW("Module stub is null");
        if (!std::holds_alternative<std::shared_ptr<SystemModule>>(_module))
            _module = std::get<std::shared_ptr<SystemModule>>(
                std::get<handle_type>(_module)(Query::Commit, ctx)
            );
    }
    std::shared_ptr<SystemModule> ModuleStub::ptr() const
    {
        if (std::holds_alternative<std::monostate>(_module))
            D2_THRW("Module stub is null");
        return std::get<std::shared_ptr<SystemModule>>(_module);
    }
    SystemModule::ModInfo ModuleStub::info() const
    {
        if (std::holds_alternative<std::monostate>(_module))
            D2_THRW("Module stub is null");
        if (std::holds_alternative<std::shared_ptr<SystemModule>>(_module))
            return ptr()->info();
        else
            return std::get<SystemModule::ModInfo>(
                std::get<handle_type>(_module)(Query::Info, nullptr)
            );
    }
    SystemModule::ModPreset ModuleStub::preset() const
    {
        if (std::holds_alternative<std::monostate>(_module))
            D2_THRW("Module stub is null");
        if (std::holds_alternative<std::shared_ptr<SystemModule>>(_module))
            return ptr()->preset();
        else
            return std::get<SystemModule::ModPreset>(
                std::get<handle_type>(_module)(Query::Preset, nullptr)
            );
    }
    SystemModule::Status ModuleStub::status() const
    {
        if (std::holds_alternative<std::monostate>(_module))
            D2_THRW("Module stub is null");
        if (std::holds_alternative<std::shared_ptr<SystemModule>>(_module))
            return ptr()->status();
        else
            return SystemModule::Status::Offline;
    }

    bool ModuleStub::operator==(std::nullptr_t) const noexcept
    {
        return std::holds_alternative<std::monostate>(_module);
    }
    bool ModuleStub::operator!=(std::nullptr_t) const noexcept
    {
        return !std::holds_alternative<std::monostate>(_module);
    }
} // namespace d2::sys
