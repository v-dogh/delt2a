#include "d2_signal_handler.hpp"

#include <algorithm>
#include <chrono>
#include <utility>

namespace d2
{
    namespace impl
    {
        bool RuntimeArg::operator==(const RuntimeArg& other) const noexcept
        {
            return type != nullptr && other.type != nullptr && *type == *other.type &&
                   lvalue_ref == other.lvalue_ref && is_const == other.is_const &&
                   is_volatile == other.is_volatile;
        }
        bool RuntimeArg::operator!=(const RuntimeArg& other) const noexcept
        {
            return !(*this == other);
        }

        bool RuntimeSignature::operator==(const RuntimeSignature& other) const noexcept
        {
            if (size != other.size)
                return false;
            for (std::size_t i = 0; i < size; ++i)
            {
                if (args[i] != other.args[i])
                    return false;
            }
            return true;
        }
    } // namespace impl

    void* Signals::SpecifierKey::_ptr() noexcept
    {
        return _ops->local ? static_cast<void*>(_storage.data()) : _buffer;
    }
    const void* Signals::SpecifierKey::_ptr() const noexcept
    {
        return _ops->local ? static_cast<const void*>(_storage.data()) : _buffer;
    }

    void Signals::SpecifierKey::_destroy() noexcept
    {
        if (_ops == nullptr)
            return;

        const auto* ops = _ops;
        auto* ptr = _ptr();

        ops->destroy(ptr);
        if (!ops->local)
            ::operator delete(ptr, std::align_val_t{ops->alignment});

        _ops = nullptr;
        _hash = 0;
    }

    void Signals::SpecifierKey::_copy_from(const SpecifierKey& copy)
    {
        if (copy._ops == nullptr)
            return;

        _ops = copy._ops;
        _hash = copy._hash;

        if (_ops->local)
        {
            _ops->copy(_storage.data(), copy._ptr());
        }
        else
        {
            _buffer = ::operator new(_ops->size, std::align_val_t{_ops->alignment});
            try
            {
                _ops->copy(_buffer, copy._ptr());
            }
            catch (...)
            {
                ::operator delete(_buffer, std::align_val_t{_ops->alignment});
                _ops = nullptr;
                _hash = 0;
                throw;
            }
        }
    }
    void Signals::SpecifierKey::_move_from(SpecifierKey&& copy) noexcept
    {
        if (copy._ops == nullptr)
            return;

        _ops = copy._ops;
        _hash = copy._hash;

        if (_ops->local)
        {
            _ops->move(_storage.data(), const_cast<void*>(copy._ptr()));
        }
        else
        {
            _buffer = copy._buffer;
            copy._buffer = nullptr;
        }

        copy._ops = nullptr;
        copy._hash = 0;
    }

    Signals::SpecifierKey::SpecifierKey(const SpecifierKey& copy)
    {
        _copy_from(copy);
    }
    Signals::SpecifierKey::SpecifierKey(SpecifierKey&& copy) noexcept
    {
        _move_from(std::move(copy));
    }
    Signals::SpecifierKey::~SpecifierKey()
    {
        _destroy();
    }

    Signals::SpecifierKey& Signals::SpecifierKey::operator=(const SpecifierKey& copy)
    {
        if (this != &copy)
        {
            _destroy();
            _copy_from(copy);
        }

        return *this;
    }
    Signals::SpecifierKey& Signals::SpecifierKey::operator=(SpecifierKey&& copy) noexcept
    {
        if (this != &copy)
        {
            _destroy();
            _move_from(std::move(copy));
        }

        return *this;
    }

    bool Signals::SpecifierKey::empty() const noexcept
    {
        return _ops == nullptr;
    }
    std::size_t Signals::SpecifierKey::hash() const noexcept
    {
        return _hash;
    }

    bool Signals::SpecifierKey::operator==(const SpecifierKey& other) const
    {
        if (_ops == nullptr || other._ops == nullptr)
            return _ops == other._ops;
        return _ops->type == other._ops->type && _ops->equal(_ptr(), other._ptr());
    }
    bool Signals::SpecifierKey::operator!=(const SpecifierKey& other) const
    {
        return !(*this == other);
    }

    std::size_t Signals::SpecifierKeyHash::operator()(const SpecifierKey& key) const noexcept
    {
        return key.hash();
    }
    bool
    Signals::SpecifierKeyEqual::operator()(const SpecifierKey& lhs, const SpecifierKey& rhs) const
    {
        return lhs == rhs;
    }

    std::size_t Signals::SignalKeyHash::operator()(const SignalKey& key) const noexcept
    {
        return absl::HashOf(key.id, key.specifier.hash());
    }
    bool Signals::SignalKeyEqual::operator()(const SignalKey& lhs, const SignalKey& rhs) const
    {
        return lhs.id == rhs.id && lhs.specifier == rhs.specifier;
    }

    void Signals::SignalInstance::_move_from(SignalInstance&& copy)
    {
        if (_manager != nullptr)
        {
            _manager(this, nullptr, nullptr, nullptr, Action::Destroy);
            _manager = nullptr;
        }

        if (copy._manager != nullptr)
            copy._manager(&copy, this, nullptr, nullptr, Action::Move);
    }

    Signals::SignalInstance::SignalInstance(SignalInstance&& copy) noexcept : _manager(nullptr)
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

    Signals::SignalInstance& Signals::SignalInstance::operator=(SignalInstance&& copy) noexcept
    {
        if (this != &copy)
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
    ) : _storage(std::move(storage)), _state(std::move(ptr))
    {
    }

    Signals::Handle::Handle(Handle&& other) noexcept :
        _storage(std::move(other._storage)), _state(std::move(other._state))
    {
    }

    Signals::Handle::~Handle()
    {
        close();
    }

    Signals::Handle& Signals::Handle::operator=(Handle&& other) noexcept
    {
        if (this != &other)
        {
            close();

            _storage = std::move(other._storage);
            _state = std::move(other._state);
        }

        return *this;
    }

    void Signals::Handle::mute()
    {
        if (_state != nullptr)
            _state->state = State::Muted;
    }

    void Signals::Handle::unmute()
    {
        if (_state != nullptr)
            _state->state = State::Active;
    }

    void Signals::Handle::release()
    {
        _state = nullptr;
        _storage = nullptr;
    }

    void Signals::Handle::close()
    {
        if (_state != nullptr)
        {
            _state->state = State::Inactive;
            _state = nullptr;
            _storage = nullptr;
        }
    }

    Signals::Handle::State Signals::Handle::state() const
    {
        if (_state == nullptr)
            return State::Inactive;

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
        std::weak_ptr<Signals> ptr, std::shared_ptr<SignalStorage> storage, SignalKey id
    ) : _state(std::move(ptr)), _storage(std::move(storage)), _id(std::move(id))
    {
    }

    Signals::Signal::Signal(Signal&& other) noexcept :
        _state(std::move(other._state)), _storage(std::move(other._storage)),
        _id(std::exchange(other._id, SignalKey{}))
    {
    }

    Signals::Signal::~Signal()
    {
        disconnect();
    }

    Signals::Signal& Signals::Signal::operator=(Signal&& other) noexcept
    {
        if (this != &other)
        {
            disconnect();

            _state = std::move(other._state);
            _storage = std::move(other._storage);
            _id = std::exchange(other._id, SignalKey{});
        }

        return *this;
    }

    void Signals::Signal::disconnect()
    {
        if (_storage != nullptr && !_state.expired())
        {
            auto ptr = _state.lock();
            ptr->_sig_deregister(_id.id, _id.specifier);
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
        signal_id id,
        SpecifierKey signal_specifier,
        event_idx ev,
        SpecifierKey dispatch_specifier,
        impl::RuntimeSignature signature,
        std::function<void(SignalInstance&)> callback
    )
    {
        std::shared_ptr<SignalStorage> storage;

        {
            std::shared_lock l1(_mtx);
            auto f = _sigs.find(SignalKey{id, std::move(signal_specifier)});
            if (f == _sigs.end())
                D2_THRW("Signal slot not found");

            if (f->second->verify == nullptr || !f->second->verify(ev, signature))
                D2_THRW("Invalid signal arguments");

            storage = f->second;
        }

        auto state = std::make_shared<HandleState>(std::move(callback));
        state->event = ev;
        state->dispatch = dispatch_specifier;

        std::lock_guard l2(storage->mtx);
        auto& bucket = storage->handles[ev];
        if (state->dispatch.empty())
            bucket.generic.push_back(state);
        else
            bucket.keyed[state->dispatch].push_back(state);

        return Handle(state, storage);
    }

    Signals::Handle Signals::_sig_listen_unchecked(
        signal_id id,
        SpecifierKey signal_specifier,
        event_idx ev,
        SpecifierKey dispatch_specifier,
        std::function<void(SignalInstance&)> callback
    )
    {
        std::shared_ptr<SignalStorage> storage;

        {
            std::shared_lock l1(_mtx);
            auto f = _sigs.find(SignalKey{id, std::move(signal_specifier)});
            if (f == _sigs.end())
                D2_THRW("Signal slot not found");

            storage = f->second;
        }

        auto state = std::make_shared<HandleState>(std::move(callback));
        state->event = ev;
        state->dispatch = dispatch_specifier;

        std::lock_guard l2(storage->mtx);
        auto& bucket = storage->handles[ev];
        if (state->dispatch.empty())
            bucket.generic.push_back(state);
        else
            bucket.keyed[state->dispatch].push_back(state);

        return Handle(state, storage);
    }

    void Signals::_sig_apply(
        SignalStorage& storage,
        event_idx ev,
        const SpecifierKey& dispatch_specifier,
        SignalInstance& args
    )
    {
        std::vector<std::shared_ptr<HandleState>> callbacks;

        auto collect = [&](SignalStorage::HandleList& list)
        {
            list.erase(
                std::remove_if(
                    list.begin(),
                    list.end(),
                    [](const std::shared_ptr<HandleState>& ptr)
                    {
                        return ptr == nullptr || ptr->state.load(std::memory_order::acquire) ==
                                                     Handle::State::Inactive;
                    }
                ),
                list.end()
            );

            callbacks.reserve(callbacks.size() + list.size());

            for (const auto& ptr : list)
            {
                if (ptr == nullptr)
                    continue;

                const auto state = ptr->state.load(std::memory_order::acquire);
                if (state == Handle::State::Active)
                    callbacks.push_back(ptr);
            }
        };

        {
            std::lock_guard lock(storage.mtx);

            auto f = storage.handles.find(ev);
            if (f == storage.handles.end())
                return;

            collect(f->second.generic);

            if (!dispatch_specifier.empty())
            {
                auto keyed = f->second.keyed.find(dispatch_specifier);
                if (keyed != f->second.keyed.end())
                {
                    collect(keyed->second);
                    if (keyed->second.empty())
                        f->second.keyed.erase(keyed);
                }
            }

            if (f->second.generic.empty() && f->second.keyed.empty())
                storage.handles.erase(f);
        }

        for (const auto& ptr : callbacks)
        {
            if (ptr->state.load(std::memory_order::acquire) == Handle::State::Active)
                ptr->callback(args);
        }
    }

    void Signals::_sig_apply_all(SignalStorage& storage)
    {
        if (storage.flags & SignalFlags::Combine)
        {
            SignalEnvelope sig;
            if (storage.combined.try_dequeue(sig))
                _sig_apply(storage, sig.event, sig.dispatch, sig.instance);

            return;
        }

        SignalEnvelope sig;
        while (storage.instances.try_dequeue(sig))
            _sig_apply(storage, sig.event, sig.dispatch, sig.instance);
    }

    bool Signals::_sig_pending(const SignalStorage& storage) const
    {
        if (storage.flags & SignalFlags::Combine)
            return !storage.combined.empty();

        return !storage.instances.empty();
    }

    void Signals::_sig_drain_queued(std::shared_ptr<SignalStorage> storage)
    {
        while (true)
        {
            while (_sig_pending(*storage))
                _sig_apply_all(*storage);

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

    void Signals::_sig_schedule_drain(std::shared_ptr<SignalStorage> storage)
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
            _pool->launch_void([self = std::move(self), storage = std::move(storage)]()
                               { self->_sig_drain_queued(std::move(storage)); });

            return;
        }

        if (storage->flags & SignalFlags::Deferred)
        {
            _pool->launch_deferred(
                std::chrono::milliseconds(0),
                [self = std::move(self), storage = std::move(storage)]()
                { self->_sig_drain_queued(std::move(storage)); }
            );

            return;
        }

        storage->is_queued.store(false, std::memory_order::release);
    }

    void Signals::_sig_trigger(
        signal_id id,
        SpecifierKey signal_specifier,
        event_idx ev,
        SpecifierKey dispatch_specifier,
        impl::RuntimeSignature signature,
        SignalInstance sig
    )
    {
        std::shared_ptr<SignalStorage> storage;

        {
            std::shared_lock lock(_mtx);
            auto f = _sigs.find(SignalKey{id, std::move(signal_specifier)});
            if (f == _sigs.end())
                D2_THRW("Signal slot not found");

            if (f->second->verify == nullptr || !f->second->verify(ev, signature))
                D2_THRW("Invalid signal arguments");

            storage = f->second;
        }

        const bool queued = storage->flags & (SignalFlags::Async | SignalFlags::Deferred);
        if (!queued)
        {
            _sig_apply(*storage, ev, dispatch_specifier, sig);
            return;
        }

        if (!_pool)
            D2_THRW("Cannot launch async task without pool");

        SignalEnvelope envelope(ev, std::move(dispatch_specifier), std::move(sig));

        if (storage->flags & SignalFlags::Combine)
        {
            SignalEnvelope tmp;
            while (!storage->combined.try_enqueue_ref(envelope))
                storage->combined.try_dequeue(tmp);
        }
        else
        {
            while (envelope.instance != nullptr && !storage->instances.try_enqueue_ref(envelope))
            {
                if (storage->flags & SignalFlags::DropIfFull)
                    return;

                SignalEnvelope old;
                storage->instances.try_dequeue(old);
            }
        }
        _sig_schedule_drain(std::move(storage));
    }

    void Signals::_sig_trigger_unchecked(
        signal_id id,
        SpecifierKey signal_specifier,
        event_idx ev,
        SpecifierKey dispatch_specifier,
        SignalInstance sig
    )
    {
        std::shared_ptr<SignalStorage> storage;

        {
            std::shared_lock lock(_mtx);
            auto f = _sigs.find(SignalKey{id, std::move(signal_specifier)});
            if (f == _sigs.end())
                D2_THRW("Signal slot not found");
            storage = f->second;
        }

        const bool queued = storage->flags & (SignalFlags::Async | SignalFlags::Deferred);
        if (!queued)
        {
            _sig_apply(*storage, ev, dispatch_specifier, sig);
            return;
        }

        if (!_pool)
            D2_THRW("Cannot launch async task without pool");

        SignalEnvelope envelope(ev, std::move(dispatch_specifier), std::move(sig));

        if (storage->flags & SignalFlags::Combine)
        {
            SignalEnvelope tmp;
            while (!storage->combined.try_enqueue_ref(envelope))
                storage->combined.try_dequeue(tmp);
        }
        else
        {
            while (envelope.instance != nullptr && !storage->instances.try_enqueue_ref(envelope))
            {
                if (storage->flags & SignalFlags::DropIfFull)
                    return;

                SignalEnvelope old;
                storage->instances.try_dequeue(old);
            }
        }
        _sig_schedule_drain(std::move(storage));
    }

    Signals::Signal Signals::_sig_register(
        signal_id id,
        SpecifierKey signal_specifier,
        signature_verifier verifier,
        unsigned char flags
    )
    {
        std::lock_guard lock(_mtx);

        SignalKey key{id, std::move(signal_specifier)};
        const auto [sig, placed] = _sigs.emplace(key, std::make_shared<SignalStorage>());
        if (!placed)
            D2_THRW("Signal slot is occupied");

        sig->second->flags = flags;
        sig->second->verify = verifier;

        return Signal(weak_from_this(), sig->second, std::move(key));
    }

    void Signals::_sig_deregister(signal_id id, SpecifierKey signal_specifier)
    {
        std::lock_guard lock(_mtx);
        auto f = _sigs.find(SignalKey{id, std::move(signal_specifier)});
        if (f == _sigs.end())
            D2_THRW("Signal slot not found");
        _sigs.erase(f);
    }
} // namespace d2
