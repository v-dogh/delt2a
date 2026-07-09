#include "core/tree/d2_theme.hpp"

namespace d2::style
{
    DependencyBase::Handle::~Handle()
    {
        close();
    }

    void DependencyBase::Handle::close()
    {
        const auto state = _state.lock();
        const auto index = _index;
        const auto generation = _generation;

        release();
        if (state == nullptr)
            return;

        _sync(
            [state, index, generation]
            {
                if (state->owner == nullptr || state->destroying)
                    return;
                state->owner->_close_slot_local(index, generation);
            }
        );
    }
    void DependencyBase::Handle::release() noexcept
    {
        _state = std::weak_ptr<StateData>();
        _index = 0;
        _generation = 0;
    }
    DependencyBase::Handle::State DependencyBase::Handle::state() const noexcept
    {
        auto state = _state.lock();
        const auto index = _index;
        const auto generation = _generation;

        if (state == nullptr)
            return State::Inactive;

        return _sync(
            [state, index, generation]() noexcept
            {
                if (state->owner == nullptr || state->destroying)
                    return State::Inactive;
                if (index >= state->dependencies.size())
                    return State::Inactive;

                const auto& dependency = state->dependencies[index];
                if (dependency.func == nullptr)
                    return State::Inactive;
                if (dependency.generation != generation)
                    return State::Inactive;
                return State::Active;
            }
        );
    }

    DependencyBase::Handle& DependencyBase::Handle::operator=(Handle&& other) noexcept
    {
        if (this == &other)
            return *this;

        close();

        _state = std::move(other._state);
        _index = std::exchange(other._index, 0);
        _generation = std::exchange(other._generation, 0);

        return *this;
    }

    DependencyBase::Handle::operator bool() const noexcept
    {
        return state() == State::Active;
    }

    DependencyBase::InvokeGuard::InvokeGuard(const DependencyBase& self) :
        self(self), state(self._state)
    {
        if (state != nullptr)
            ++state->recursion_ctr;
    }
    DependencyBase::InvokeGuard::InvokeGuard(
        const DependencyBase& self, std::shared_ptr<StateData> state
    ) : self(self), state(std::move(state))
    {
        if (this->state != nullptr)
            ++this->state->recursion_ctr;
    }
    DependencyBase::InvokeGuard::~InvokeGuard()
    {
        if (state == nullptr)
            return;
        --state->recursion_ctr;
        if (state->recursion_ctr == 0 && state->owner == &self && !state->destroying)
            self._flush_pending_local();
    }

    DependencyBase::~DependencyBase()
    {
        _sync(
            [this]
            {
                if (_state != nullptr)
                {
                    _state->destroying = true;
                    _state->owner = nullptr;
                }
            }
        );
    }

    std::shared_ptr<DependencyBase::StateData> DependencyBase::_ensure_state() const
    {
        if (_state == nullptr)
        {
            _state = std::make_shared<StateData>();
            _state->owner = this;
        }
        return _state;
    }
    std::uint32_t DependencyBase::_next_generation() const
    {
        auto generation = _state->next_generation++;
        if (_state->next_generation == 0)
            _state->next_generation = 1;
        return generation;
    }

    DependencyBase::Handle DependencyBase::_make_handle(std::uint32_t index) const
    {
        return Handle{_state, index, _state->dependencies[index].generation};
    }
    DependencyBase::Handle DependencyBase::_insert_slot(Deps dependency) const
    {
        auto state = _ensure_state();
        std::uint32_t index = 0;
        bool found = false;
        if (state->recursion_ctr == 0)
        {
            while (!state->free_slots.empty())
            {
                index = state->free_slots.back();
                state->free_slots.pop_back();

                if (index < state->dependencies.size() &&
                    state->dependencies[index].func == nullptr)
                {
                    found = true;
                    break;
                }
            }
        }
        dependency.generation = _next_generation();
        if (!found)
        {
            if (state->dependencies.size() >= std::numeric_limits<std::uint32_t>::max())
                D2_THRW("Dependency subscription storage overflow");

            index = static_cast<std::uint32_t>(state->dependencies.size());
            state->dependencies.push_back(std::move(dependency));
        }
        else
            state->dependencies[index] = std::move(dependency);
        return _make_handle(index);
    }

    void DependencyBase::_close_slot_local(std::uint32_t index, std::uint32_t generation) const
    {
        if (_state == nullptr)
            return;

        auto state = _state;
        if (state->destroying)
            return;
        if (index >= state->dependencies.size())
            return;
        auto& slot = state->dependencies[index];
        if (slot.func == nullptr)
            return;
        if (slot.generation != generation)
            return;

        auto dependency = slot;
        slot = Deps{};
        slot.generation = _next_generation();
        if (state->recursion_ctr != 0)
        {
            state->pending_destroy.push_back(dependency);
            state->pending_free.push_back(index);
        }
        else
        {
            _destroy(dependency);
            state->free_slots.push_back(index);
            _cleanup_local();
        }
    }
    void DependencyBase::_cleanup_local() const
    {
        if (_state == nullptr)
            return;
        auto state = _state;
        if (state->recursion_ctr != 0 || state->destroying)
            return;
        while (!state->dependencies.empty() && state->dependencies.back().func == nullptr)
            state->dependencies.pop_back();
    }
    void DependencyBase::_flush_pending_local() const
    {
        if (_state == nullptr)
            return;
        auto state = _state;
        if (state->recursion_ctr != 0 || state->destroying)
            return;

        if (!state->pending_destroy.empty())
        {
            auto pending = std::move(state->pending_destroy);
            state->pending_destroy.clear();

            for (auto& it : pending)
                _destroy(it);
        }
        if (!state->pending_free.empty())
        {
            auto pending = std::move(state->pending_free);
            state->pending_free.clear();
            for (decltype(auto) index : pending)
            {
                if (index < state->dependencies.size() &&
                    state->dependencies[index].func == nullptr)
                    state->free_slots.push_back(index);
            }
        }
        _cleanup_local();
    }
    void DependencyBase::_destroy_state_local() const
    {
        auto state = std::move(_state);
        if (state == nullptr)
            return;

        state->destroying = true;
        state->owner = nullptr;

        auto dependencies = std::move(state->dependencies);
        auto pending_destroy = std::move(state->pending_destroy);

        state->dependencies.clear();
        state->free_slots.clear();
        state->pending_free.clear();
        state->pending_destroy.clear();

        for (auto& it : dependencies)
            _destroy(it);
        for (auto& it : pending_destroy)
            _destroy(it);
    }

} // namespace d2::style
