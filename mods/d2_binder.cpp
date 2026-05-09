#include "d2_binder.hpp"

#include <chrono>
#include <format>

namespace d2::sys
{
    void SystemBinder::_check_event()
    {
        if (!_active_set.empty() && _key_event == nullptr)
        {
            const auto ctx = context();
            _key_event = ctx->listen(
                sys::SystemScreen::Event::KeyInput,
                [this](IOContext::ptr ctx)
                {
                    const auto in = ctx->screen()->input();
                    bool any = false;
                    std::vector<callback_type> cbs;
                    std::vector<std::string> active_names;

                    active_names.reserve(_active_set.size());
                    for (decltype(auto) name : _active_set)
                        active_names.push_back(name);
                    for (decltype(auto) name : active_names)
                    {
                        auto it = _binds.find(name);
                        if (it == _binds.end())
                            continue;

                        auto& bind = it->second;
                        if (!bind.active || !bind.bind.enabled || bind.bind.sequences.empty())
                            continue;
                        if (bind.sequence_idx >= bind.bind.sequences.size())
                            bind.sequence_idx = 0;
                        if (std::chrono::steady_clock::now() - bind.sequence_ts >
                            bind.bind.sequential_delay)
                            bind.sequence_idx = 0;

                        auto& aq_seq = bind.bind.sequences[bind.sequence_idx];
                        bool good = true;
                        std::size_t idx = 0;
                        for (auto [key, mode] : aq_seq)
                        {
                            if (mode == in::Mode::Hold)
                            {
                                bind.sequence_idx = 0;
                                good = false;
                                break;
                            }
                            else if (!in->active(key, mode))
                            {
                                bind.sequence_idx = 0;
                                good = false;
                                break;
                            }
                            idx++;
                        }
                        if (good)
                        {
                            bind.sequence_idx++;
                            bind.sequence_ts = std::chrono::steady_clock::now();
                            any = true;
                        }
                        if (bind.sequence_idx >= bind.bind.sequences.size())
                        {
                            cbs.push_back(bind.callback);
                            bind.sequence_idx = 0;
                        }
                    }
                    // Consume the sequence input
                    if (any) [[maybe_unused]]
                        const auto seq = in->sequence();
                    for (decltype(auto) it : cbs)
                        it(ctx);
                }
            );
        }
        else if (_active_set.empty() && _key_event != nullptr)
        {
            _key_event.close();
            _key_event = nullptr;
        }
    }

    std::optional<SystemBinder::BindError>
    SystemBinder::_bind_dcheck(absl::flat_hash_map<std::string, BindValue>::iterator it)
    {
        auto name = it->first;
        auto& bind = it->second;

        static auto dependency_state = [](const std::shared_ptr<Element>& ptr, Bind::DepCond cnd)
        {
            switch (cnd)
            {
            case Bind::DepCond::Visible:
                return ptr->getstate(Element::State::Display) &&
                       !ptr->getstate(Element::State::Swapped);
            case Bind::DepCond::Active:
                return !ptr->getstate(Element::State::Swapped);
            case Bind::DepCond::Focused:
                return ptr->getstate(Element::State::Focused);
            case Bind::DepCond::Hovered:
                return ptr->getstate(Element::State::Hovered);
            }
            return false;
        };

        static auto check_all = [](SystemBinder& state, const std::string& name)
        {
            auto it = state._binds.find(name);
            if (it == state._binds.end())
                return;
            auto& bind = it->second;
            bool active = bind.bind.enabled;
            if (active)
            {
                for (const auto& [wptr, level, cnd] : bind.bind.dependencies)
                {
                    auto ptr = wptr.lock();
                    if (ptr == nullptr)
                    {
                        if (level == Bind::DepMode::Strong)
                        {
                            D2_TLOG(Warning, "One of the dependencies became null")
                            active = false;
                            break;
                        }
                        continue;
                    }
                    if (!dependency_state(ptr, cnd))
                    {
                        active = false;
                        break;
                    }
                }
            }
            if (bind.active != active)
            {
                bind.active = active;
                bind.sequence_idx = 0;
                bind.sequence_ts = {};
                if (active)
                    state._active_set.emplace(name);
                else
                    state._active_set.erase(name);
                state._check_event();
            }
        };

        for (decltype(auto) seq : bind.bind.sequences)
        {
            if (seq.empty())
            {
                auto err = BindError{
                    .type = ConflictType::Failure, .description = "Bind sequence cannot be empty"
                };
                D2_TLOG(Warning, err.description)
                return err;
            }

            for (const auto& [key, mode] : seq)
            {
                if (mode == in::Mode::Hold)
                {
                    auto err = BindError{
                        .type = ConflictType::Failure, .description = "Hold mode is not implemented"
                    };
                    D2_TLOG(Warning, err.description)
                    return err;
                }
            }
        }

        bind.listeners.clear();
        bind.active = false;
        bind.sequence_idx = 0;
        bind.sequence_ts = {};
        _active_set.erase(name);

        if (!bind.bind.enabled)
        {
            _check_event();
            return std::nullopt;
        }

        for (const auto& [wptr, level, cnd] : bind.bind.dependencies)
        {
            auto ptr = wptr.lock();
            if (ptr == nullptr)
            {
                if (level == Bind::DepMode::Strong)
                {
                    auto err = BindError{
                        .type = ConflictType::Failure,
                        .description = "One of the dependencies is inaccessible"
                    };
                    D2_TLOG(Warning, err.description)
                    return err;
                }

                D2_TLOG(Warning, "One of the weak dependencies is inaccessible")
                continue;
            }
            switch (cnd)
            {
            case Bind::DepCond::Visible:
                bind.listeners.push_back(ptr->listen(
                    Element::State::Display,
                    [this, name](Element::EventListener, TreeIter<> ptr) { check_all(*this, name); }
                ));
                bind.listeners.push_back(ptr->listen(
                    Element::State::Swapped,
                    [this, name](Element::EventListener, TreeIter<> ptr) { check_all(*this, name); }
                ));
                break;
            case Bind::DepCond::Active:
                bind.listeners.push_back(ptr->listen(
                    Element::State::Swapped,
                    [this, name](Element::EventListener, TreeIter<> ptr) { check_all(*this, name); }
                ));
                break;
            case Bind::DepCond::Focused:
                bind.listeners.push_back(ptr->listen(
                    Element::State::Focused,
                    [this, name](Element::EventListener, TreeIter<> ptr) { check_all(*this, name); }
                ));
                break;
            case Bind::DepCond::Hovered:
                bind.listeners.push_back(ptr->listen(
                    Element::State::Hovered,
                    [this, name](Element::EventListener, TreeIter<> ptr) { check_all(*this, name); }
                ));
                break;
            }
        }
        check_all(*this, name);
        return std::nullopt;
    }

    std::optional<SystemBinder::BindError>
    SystemBinder::bind(const std::string& name, Bind value, callback_type callback)
    {
        auto f = _binds.find(name);
        if (f != _binds.end())
        {
            auto err = BindError{
                .type = ConflictType::Collision,
                .description = std::format("Key {} is already bound", name)
            };
            D2_TLOG(Warning, err.description)
            return err;
        }

        auto [it, _] = _binds.emplace(name, BindValue());
        auto& bind = it->second;
        bind.bind = std::move(value);
        bind.callback = std::move(callback);
        bind.epoch = _epoch_ctr++;

        auto err = _bind_dcheck(it);
        if (err.has_value())
            _binds.erase(it);

        return err;
    }

    std::optional<SystemBinder::BindError> SystemBinder::rebind(const std::string& name, Bind value)
    {
        auto f = _binds.find(name);
        if (f == _binds.end())
        {
            auto err = BindError{
                .type = ConflictType::Failure,
                .description = std::format("Invalid bind name {}", name)
            };
            D2_TLOG(Warning, err.description)
            return err;
        }

        _active_set.erase(name);
        f->second.active = false;
        f->second.listeners.clear();
        f->second.sequence_idx = 0;
        f->second.sequence_ts = {};
        f->second.bind = std::move(value);
        f->second.epoch = _epoch_ctr++;

        return _bind_dcheck(f);
    }

    const SystemBinder::Bind& SystemBinder::check(const std::string& name) const noexcept
    {
        static const Bind empty{};
        auto f = _binds.find(name);
        if (f == _binds.end())
            return empty;
        return f->second.bind;
    }

    void SystemBinder::freeze(const std::string& name)
    {
        auto f = _binds.find(name);
        if (f == _binds.end())
            D2_THRW("Invalid bind name");

        for (auto& listener : f->second.listeners)
            listener.mute();

        if (f->second.active)
        {
            _active_set.erase(name);
            f->second.active = false;
        }

        f->second.sequence_idx = 0;
        f->second.sequence_ts = {};
        f->second.bind.enabled = false;
        _check_event();
    }

    void SystemBinder::unfreeze(const std::string& name)
    {
        auto f = _binds.find(name);
        if (f == _binds.end())
            D2_THRW("Invalid bind name");
        f->second.bind.enabled = true;
        _bind_dcheck(f);
    }

    void SystemBinder::unbind(const std::string& name)
    {
        auto f = _binds.find(name);
        if (f == _binds.end())
            return;
        _active_set.erase(name);
        _binds.erase(f);
        _check_event();
    }

    std::vector<SystemBinder::Bind> SystemBinder::list() const
    {
        std::vector<Bind> out;
        out.reserve(_binds.size());
        for (const auto& [name, value] : _binds)
            out.push_back(value.bind);
        return out;
    }
} // namespace d2::sys
