#include "d2_io_handler.hpp"
#include "d2_screen.hpp"

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

    IOContext::IOContext(mt::ThreadPool::ptr scheduler, rs::RuntimeLogs::ptr logs)
        : _scheduler(scheduler), _logs(logs), Signals(scheduler)
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
        using wflags = mt::ThreadPool::Worker::Flags;
        _main_thread = std::this_thread::get_id();
        _worker = _scheduler->worker(
            wflags::MainWorker |
            wflags::HandleCyclicTask |
            wflags::HandleDeferredTask
            );
        _worker.start();
        _scheduler->start();
        D2_TLOG(Module, "Success in initialization")
    }
    void IOContext::_deinitialize()
    {
        D2_TLOG(Module, "Deinitializing IO context")
        _worker.stop();
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
            if (_deadline ==  std::chrono::steady_clock::time_point::max()) _worker.wait();
            else
            {
                const auto sleep = std::chrono::duration_cast<std::chrono::milliseconds>(
                    _deadline - std::chrono::steady_clock::now()
                );
                _worker.wait(std::clamp(sleep,
                    std::chrono::milliseconds(0),
                    std::chrono::milliseconds::max()
                ));
            }
            _deadline = std::chrono::steady_clock::time_point::max();
        }
        D2_TLOG(Info, "Ending main loop")

        D2_TLOG(Info, "Deactivating extended code page")
        ExtendedCodePage::deactivate_thread();
        D2_TLOG(Module, "Stopped IOContext")
    }

    void IOContext::_insert_component(std::unique_ptr<sys::SystemComponent> ptr)
    {
        D2_TLOG(Info, "Inserting new component: ", ptr->name())
        const auto idx = ptr->uid();
        _components.resize(std::max(_components.size(), idx + 1));
        _components[idx] = std::move(ptr);
    }

    void IOContext::ping()
    {
        _worker.ping();
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

    mt::ThreadPool::ptr IOContext::scheduler()
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
    void IOContext::sysenum(std::function<void(sys::SystemComponent*)> callback)
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
}
