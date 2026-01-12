#include "d2_signal_handler.hpp"

namespace d2
{
    void Signals::SignalInstance::_move_from(SignalInstance&& copy)
    {
        if (copy._manager)
            copy._manager(
                &copy,
                this,
                nullptr,
                nullptr,
                Action::Move
            );
    }

    Signals::SignalInstance::SignalInstance(SignalInstance&& copy)
    {
        _move_from(std::move(copy));
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

    Signals::Handle::Handle(std::shared_ptr<HandleState> ptr) :
        _state(ptr) {}

    void Signals::Handle::mute()
    {
        _state->state = State::Muted;
    }
    void Signals::Handle::unmute()
    {
        _state->state = State::Active;
    }
    void Signals::Handle::close()
    {
        _state->state = State::Inactive;
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

    Signals::Signal::Signal(std::weak_ptr<Signals> ptr, signal_id id) :
        _id(id), _state(ptr) {}

    void Signals::Signal::disconnect()
    {
        auto ptr = _state.lock();
        ptr->_sig_deregister(_id);
        _state.reset();
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
        callback(std::move(callback)) {}

    Signals::ptr Signals::make(mt::ThreadPool::ptr pool)
    {
        return std::make_shared<Signals>(pool);
    }

    Signals::Signals(mt::ThreadPool::ptr pool) :
        _pool(pool) {}

    Signals::Handle Signals::_sig_listen(signal_id id, event_idx ev, args_code code, std::function<void(SignalInstance&)> callback)
    {
        std::shared_lock l1(_mtx);
        auto f = _sigs.find(id);
        if (f == _sigs.end())
            throw std::runtime_error{ "Signal slot not found" };
        if (f->second->argument_code != code)
            throw std::runtime_error{ "Invalid signal arguments" };
        auto state = std::make_shared<HandleState>(std::move(callback));
        std::lock_guard l2(f->second->mtx);
        f->second->handles[ev].push_back(state);
        return state;
    }
    void Signals::_sig_apply(SignalStorage& storage, event_idx ev, SignalInstance& args)
    {
        auto f = storage.handles.find(ev);
        if (f == storage.handles.end())
            return;

        bool cleanup = false;
        for (decltype(auto) it : f->second)
        {
            auto ptr = it.lock();
            if (ptr == nullptr || ptr->state == Handle::State::Inactive)
            {
                cleanup = true;
                continue;
            }
            if (ptr->state != Handle::State::Muted)
                ptr->callback(args);
        }

        if (cleanup)
        {
            f->second.erase(std::remove_if(f->second.begin(), f->second.end(), [](const auto& ptr) {
                auto v = ptr.lock();
                return v == nullptr || v->state == Handle::State::Inactive;
            }), f->second.end());
        }
    }
    void Signals::_sig_apply_all(SignalStorage& storage, event_idx ev)
    {
        if (storage.flags & SignalFlags::Combine)
        {
            SignalInstance sig;
            if (storage.combined.try_dequeue(sig))
                _sig_apply(storage, ev, sig);
        }
        else
        {
            SignalInstance sig;
            while (storage.instances.try_dequeue(sig))
                _sig_apply(storage, ev, sig);
        }
    }
    void Signals::_sig_trigger(signal_id id, event_idx ev, args_code code, SignalInstance sig)
    {
        std::shared_lock l1(_mtx);
        auto f = _sigs.find(id);
        if (f == _sigs.end())
            throw std::runtime_error{ "Signal slot not found" };
        if (f->second->argument_code != code)
            throw std::runtime_error{ "Invalid signal arguments" };

        if (f->second->flags & SignalFlags::Async)
        {
            if (!_pool)
                throw std::logic_error{ "Cannot launch async task without pool" };
            if (f->second->flags & SignalFlags::Combine)
            {
                SignalInstance tmp;
                while (!f->second->combined.try_enqueue_ref(sig))
                    f->second->combined.try_dequeue(tmp);
            }
            else
            {
                while (sig != nullptr && !f->second->instances.try_enqueue_ref(sig))
                {
                    if (!(f->second->flags & SignalFlags::DropIfFull))
                    {
                        SignalInstance value;
                        f->second->instances.try_dequeue(value);
                        f->second->instances.try_enqueue_ref(sig);
                    }
                    else
                        return;
                }
            }

            bool expected = false;
            if (f->second->is_queued.compare_exchange_strong(expected, true))
            {
                _pool->launch_void([ptr = shared_from_this(), this, ev, storage = &f->second]() {
                    _sig_apply_all(**storage, ev);
                });
            }
        }
        else if (f->second->flags & SignalFlags::Deferred)
        {
            if (!_pool)
                throw std::logic_error{ "Cannot launch deferred task without pool" };

            bool expected = false;
            if (f->second->is_queued.compare_exchange_strong(expected, true))
            {
                _pool->launch_deferred(std::chrono::milliseconds(0), [ptr = shared_from_this(), this, ev, storage = &f->second]() {
                    _sig_apply_all(**storage, ev);
                });
            }
        }
        else
        {
            _sig_apply(*f->second, ev, sig);
        }
    }

    Signals::Signal Signals::_sig_register(signal_id id, args_code code, std::size_t size, unsigned char flags)
    {
        std::lock_guard l1(_mtx);
        const auto [ sig, placed ] = _sigs.emplace(id, std::make_unique<SignalStorage>());
        if (!placed)
            throw std::runtime_error{ "Signal slot is occupied" };
        sig->second->flags = flags;
        sig->second->argument_code = code;
        sig->second->packed_size = size;
        return Signal(weak_from_this(), id);
    }
    void Signals::_sig_deregister(signal_id id)
    {
        std::lock_guard l1(_mtx);
        auto f = _sigs.find(id);
        if (f == _sigs.end())
            throw std::runtime_error{ "Signal slot not found" };
        _sigs.erase(f);
    }

}
