#include "d2_signal_handler.hpp"

namespace d2
{
    void Signals::SignalInstance::_move_from(SignalInstance&& copy)
    {
        if (_manager != nullptr)
        {
            _manager(this, nullptr, nullptr, nullptr, Action::Destroy);
            _manager = nullptr;
        }
        if (copy._manager)
            copy._manager(&copy, this, nullptr, nullptr, Action::Move);
    }

    Signals::SignalInstance::SignalInstance(SignalInstance&& copy)
    {
        _move_from(std::move(copy));
    }
    Signals::SignalInstance::~SignalInstance()
    {
        if (_manager != nullptr)
        {
            _manager(this, nullptr, nullptr, nullptr, Action::Destroy);
            _manager = nullptr;
        }
    }

    Signals::SignalInstance& Signals::SignalInstance::operator=(SignalInstance&& copy)
    {
        _move_from(std::move(copy));
        return *this;
    }

    bool Signals::SignalInstance::operator==(std::nullptr_t) const
    {
        return _manager == nullptr;
    }
    bool Signals::SignalInstance::operator!=(std::nullptr_t) const
    {
        return _manager != nullptr;
    }

    Signals::Handle::Handle(
        std::shared_ptr<HandleState> ptr, std::shared_ptr<SignalStorage> storage
    ) : _state(ptr), _storage(storage)
    {
    }
    Signals::Handle::~Handle()
    {
        close();
    }

    void Signals::Handle::mute()
    {
        _state->state = State::Muted;
    }
    void Signals::Handle::unmute()
    {
        _state->state = State::Active;
    }
    void Signals::Handle::release()
    {
        _state = nullptr;
        _storage = nullptr;
    }
    void Signals::Handle::close()
    {
        if (_state)
        {
            _state->state = State::Inactive;
            _state = nullptr;
            _storage = nullptr;
        }
    }
    Signals::Handle::State Signals::Handle::state() const
    {
        return _state->state;
    }

    bool Signals::Handle::operator==(std::nullptr_t) const
    {
        return _state == nullptr;
    }
    bool Signals::Handle::operator!=(std::nullptr_t) const
    {
        return _state != nullptr;
    }

    Signals::Signal::Signal(
        std::weak_ptr<Signals> ptr, std::shared_ptr<SignalStorage> storage, signal_id id
    ) : _storage(storage), _state(ptr), _id(id)
    {
    }
    Signals::Signal::~Signal()
    {
        disconnect();
    }

    void Signals::Signal::disconnect()
    {
        if (_storage != nullptr && !_state.expired())
        {
            auto ptr = _state.lock();
            ptr->_sig_deregister(_id);
            _state.reset();
            _storage.reset();
        }
    }

    bool Signals::Signal::operator==(std::nullptr_t) const
    {
        return _state.expired();
    }
    bool Signals::Signal::operator!=(std::nullptr_t) const
    {
        return !_state.expired();
    }

    Signals::HandleState::HandleState(std::function<void(SignalInstance&)> callback) :
        callback(std::move(callback))
    {
    }

    Signals::ptr Signals::make(mt::ConcurrentPool::ptr pool)
    {
        return std::make_shared<Signals>(pool);
    }

    Signals::Signals(mt::ConcurrentPool::ptr pool) : _pool(pool) {}

    Signals::Handle Signals::_sig_listen(
        signal_id id, event_idx ev, args_code code, std::function<void(SignalInstance&)> callback
    )
    {
        std::shared_lock l1(_mtx);
        auto f = _sigs.find(id);
        if (f == _sigs.end())
            D2_THRW("Signal slot not found");
        if (f->second->argument_code != code)
            D2_THRW("Invalid signal arguments");
        auto state = std::make_shared<HandleState>(std::move(callback));
        std::lock_guard l2(f->second->mtx);
        f->second->handles[ev].push_back(state);
        return Handle(state, f->second);
    }
    void Signals::_sig_apply(SignalStorage& storage, event_idx ev, SignalInstance& args)
    {
        std::vector<std::shared_ptr<HandleState>> callbacks;

        {
            std::lock_guard lock(storage.mtx);
            auto f = storage.handles.find(ev);
            if (f == storage.handles.end())
                return;

            auto& list = f->second;
            list.erase(
                std::remove_if(
                    list.begin(),
                    list.end(),
                    [](const std::weak_ptr<HandleState>& weak)
                    {
                        auto ptr = weak.lock();
                        return ptr == nullptr || ptr->state.load(std::memory_order::acquire) ==
                                                     Handle::State::Inactive;
                    }
                ),
                list.end()
            );
            callbacks.reserve(list.size());
            for (decltype(auto) ptr : list)
            {
                if (ptr == nullptr)
                    continue;
                const auto state = ptr->state.load(std::memory_order::acquire);
                if (state == Handle::State::Active)
                    callbacks.push_back(std::move(ptr));
            }
        }

        for (decltype(auto) ptr : callbacks)
        {
            if (ptr->state.load(std::memory_order::acquire) == Handle::State::Active)
                ptr->callback(args);
        }
    }
    void Signals::_sig_apply_all(SignalStorage& storage, event_idx ev)
    {
        if (storage.flags & SignalFlags::Combine)
        {
            SignalInstance sig;
            if (storage.combined.try_dequeue(sig))
                _sig_apply(storage, ev, sig);
            return;
        }
        SignalInstance sig;
        while (storage.instances.try_dequeue(sig))
            _sig_apply(storage, ev, sig);
    }

    bool Signals::_sig_pending(const SignalStorage& storage) const
    {
        if (storage.flags & SignalFlags::Combine)
            return !storage.combined.empty();
        return !storage.instances.empty();
    }
    void Signals::_sig_drain_queued(std::shared_ptr<SignalStorage> storage, event_idx ev)
    {
        while (true)
        {
            while (_sig_pending(*storage))
                _sig_apply_all(*storage, ev);

            storage->is_queued.store(false, std::memory_order::release);
            if (!_sig_pending(*storage))
                break;

            bool expected = false;
            if (storage->is_queued.compare_exchange_strong(
                    expected, true, std::memory_order::acq_rel, std::memory_order::acquire
                ))
            {
                continue;
            }
            break;
        }
    }
    void Signals::_sig_schedule_drain(std::shared_ptr<SignalStorage> storage, event_idx ev)
    {
        bool expected = false;
        if (!storage->is_queued.compare_exchange_strong(
                expected, true, std::memory_order::acq_rel, std::memory_order::acquire
            ))
        {
            return;
        }

        auto self = shared_from_this();
        if (storage->flags & SignalFlags::Async)
        {
            _pool->launch_void([self = std::move(self), storage = std::move(storage), ev]()
                               { self->_sig_drain_queued(std::move(storage), ev); });

            return;
        }
        if (storage->flags & SignalFlags::Deferred)
        {
            _pool->launch_deferred(
                std::chrono::milliseconds(0),
                [self = std::move(self), storage = std::move(storage), ev]()
                { self->_sig_drain_queued(std::move(storage), ev); }
            );

            return;
        }
        storage->is_queued.store(false, std::memory_order_release);
    }

    void Signals::_sig_trigger(signal_id id, event_idx ev, args_code code, SignalInstance sig)
    {
        std::shared_ptr<SignalStorage> storage;
        {
            std::shared_lock lock(_mtx);
            auto f = _sigs.find(id);
            if (f == _sigs.end())
                D2_THRW("Signal slot not found");
            if (f->second->argument_code != code)
                D2_THRW("Invalid signal arguments");
            storage = f->second;
        }

        const bool queued = storage->flags & (SignalFlags::Async | SignalFlags::Deferred);
        if (!queued)
        {
            _sig_apply(*storage, ev, sig);
            return;
        }
        if (!_pool)
            D2_THRW("Cannot launch async task without pool");

        if (storage->flags & SignalFlags::Combine)
        {
            SignalInstance tmp;
            while (!storage->combined.try_enqueue_ref(sig))
                storage->combined.try_dequeue(tmp);
        }
        else
        {
            while (sig != nullptr && !storage->instances.try_enqueue_ref(sig))
            {
                if (storage->flags & SignalFlags::DropIfFull)
                    return;
                SignalInstance old;
                storage->instances.try_dequeue(old);
            }
        }
        _sig_schedule_drain(std::move(storage), ev);
    }

    Signals::Signal
    Signals::_sig_register(signal_id id, args_code code, std::size_t size, unsigned char flags)
    {
        std::lock_guard l1(_mtx);
        const auto [sig, placed] = _sigs.emplace(id, std::make_shared<SignalStorage>());
        if (!placed)
            D2_THRW("Signal slot is occupied");
        sig->second->flags = flags;
        sig->second->argument_code = code;
        sig->second->packed_size = size;
        return Signal(weak_from_this(), sig->second, id);
    }
    void Signals::_sig_deregister(signal_id id)
    {
        std::lock_guard l1(_mtx);
        auto f = _sigs.find(id);
        if (f == _sigs.end())
            D2_THRW("Signal slot not found");
        _sigs.erase(f);
    }

} // namespace d2
