#pragma once

#include <d2_exceptions.hpp>
#include <d2_io_handler.hpp>
#include <d2_meta.hpp>
#include <d2_pixel.hpp>
#include <memory>
#include <variant>
#include <vector>

namespace d2::style
{
    enum class DepQuery
    {
        Invoke,
        Destroy,
    };

    class DependencyBase
    {
        D2_TAG_MODULE(react)
    protected:
        struct Deps
        {
            std::weak_ptr<void> obj{};
            void* base{nullptr};
            void (*func)(){nullptr};
            std::uint32_t generation{0};
        };
    private:
        struct StateData
        {
            DependencyBase* owner{nullptr};
            std::size_t recursion_ctr{0};
            std::uint32_t next_generation{1};
            bool destroying{false};

            std::vector<Deps> dependencies{};
            std::vector<std::uint32_t> free_slots{};
            std::vector<std::uint32_t> pending_free{};
            std::vector<Deps> pending_destroy{};
        };
    public:
        class Handle
        {
        public:
            enum class State
            {
                Inactive,
                Active
            };
        private:
            std::weak_ptr<StateData> _state{};
            std::uint32_t _index{0};
            std::uint32_t _generation{0};

            Handle(std::weak_ptr<StateData> state, std::uint32_t index, std::uint32_t generation) :
                _state(std::move(state)), _index(index), _generation(generation)
            {
            }

            friend class DependencyBase;
        public:
            Handle() = default;
            Handle(const Handle&) = delete;
            Handle(Handle&& other) noexcept :
                _state(std::move(other._state)), _index(std::exchange(other._index, 0)),
                _generation(std::exchange(other._generation, 0))
            {
            }
            ~Handle()
            {
                close();
            }

            void close()
            {
                const auto state = _state.lock();
                const auto index = _index;
                const auto generation = _generation;

                release();
                if (state == nullptr)
                    return;

                IOContext::get()->sync(
                    [state, index, generation]
                    {
                        if (state->owner == nullptr || state->destroying)
                            return;
                        state->owner->_close_slot_local(index, generation);
                    }
                );
            }
            void release() noexcept
            {
                _state = std::weak_ptr<StateData>();
                _index = 0;
                _generation = 0;
            }
            State state() const noexcept
            {
                auto state = _state.lock();
                const auto index = _index;
                const auto generation = _generation;

                if (state == nullptr)
                    return State::Inactive;

                return IOContext::get()->sync(
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

            Handle& operator=(const Handle&) = delete;
            Handle& operator=(Handle&& other) noexcept
            {
                if (this == &other)
                    return *this;

                close();

                _state = std::move(other._state);
                _index = std::exchange(other._index, 0);
                _generation = std::exchange(other._generation, 0);

                return *this;
            }

            operator bool() const noexcept
            {
                return state() == State::Active;
            }
        };
    protected:
        std::shared_ptr<StateData> _state{};

        struct InvokeGuard
        {
            DependencyBase& self;
            std::shared_ptr<StateData> state;

            InvokeGuard(DependencyBase& self) : self(self), state(self._state)
            {
                if (state != nullptr)
                    ++state->recursion_ctr;
            }
            InvokeGuard(const InvokeGuard&) = delete;
            InvokeGuard(DependencyBase& self, std::shared_ptr<StateData> state) :
                self(self), state(std::move(state))
            {
                if (this->state != nullptr)
                    ++this->state->recursion_ctr;
            }
            ~InvokeGuard()
            {
                if (state == nullptr)
                    return;
                --state->recursion_ctr;
                if (state->recursion_ctr == 0 && state->owner == &self && !state->destroying)
                    self._flush_pending_local();
            }

            InvokeGuard& operator=(const InvokeGuard&) = delete;
        };

        DependencyBase() = default;
        DependencyBase(const DependencyBase&) {}
        DependencyBase(DependencyBase&&) = delete;
        ~DependencyBase()
        {
            IOContext::get()->sync(
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

        std::shared_ptr<StateData> _ensure_state()
        {
            if (_state == nullptr)
            {
                _state = std::make_shared<StateData>();
                _state->owner = this;
            }
            return _state;
        }
        std::uint32_t _next_generation()
        {
            auto generation = _state->next_generation++;
            if (_state->next_generation == 0)
                _state->next_generation = 1;
            return generation;
        }
        Handle _make_handle(std::uint32_t index)
        {
            return Handle{_state, index, _state->dependencies[index].generation};
        }
        Handle _insert_slot(Deps dependency)
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
        void _close_slot_local(std::uint32_t index, std::uint32_t generation)
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
        void _cleanup_local()
        {
            if (_state == nullptr)
                return;
            auto state = _state;
            if (state->recursion_ctr != 0 || state->destroying)
                return;
            while (!state->dependencies.empty() && state->dependencies.back().func == nullptr)
                state->dependencies.pop_back();
        }
        void _flush_pending_local()
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
        void _destroy_state_local()
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

        virtual void _destroy(Deps dependency) = 0;
    };
    using DependencyHandle = DependencyBase::Handle;

    template<typename Type> class Dependency : public DependencyBase
    {
        D2_TAG_MODULE(react)
    public:
        using Query = DepQuery;
        using value_type = Type;
        using manager_callback = bool (*)(std::shared_ptr<void>, void*, const Type&, Query);
        using Handle = DependencyBase::Handle;
    private:
        struct Dummy
        {
        };
        // trampoline | callback | ptr
        using dynalink =
            // callback | provider | this
            std::tuple<Type (*)(void (*)(), void*, void*, bool), void (*)(), void*>;
    private:
        Handle _link{};
        std::variant<Type, Dependency<Type>*, dynalink> _value{Type()};

        Dependency& _source()
        {
            auto* current = this;
            while (std::holds_alternative<Dependency<Type>*>(current->_value))
            {
                auto* next = std::get<Dependency<Type>*>(current->_value);
                if (next == nullptr)
                    D2_THRW("Dependency link is null");
                current = next;
            }
            return *current;
        }
        const Dependency& _source() const
        {
            auto* current = this;
            while (std::holds_alternative<Dependency<Type>*>(current->_value))
            {
                auto* next = std::get<Dependency<Type>*>(current->_value);
                if (next == nullptr)
                    D2_THRW("Dependency link is null");
                current = next;
            }
            return *current;
        }

        Type _read() const
        {
            const auto& src = _source();
            if (std::holds_alternative<Type>(src._value))
                return std::get<Type>(src._value);
            const auto& [callback, icb, ptr] = std::get<dynalink>(src._value);
            return callback(icb, ptr, const_cast<Dependency<Type>*>(&src), false);
        }

        Type& _value_local()
        {
            auto& src = _source();
            if (!std::holds_alternative<Type>(src._value))
                D2_THRW("Dependency value is dynamic and cannot be referenced");
            return std::get<Type>(src._value);
        }
        const Type& _value_local() const
        {
            const auto& src = _source();
            if (!std::holds_alternative<Type>(src._value))
                D2_THRW("Dependency value is dynamic and cannot be referenced");
            return std::get<Type>(src._value);
        }

        template<typename Func> decltype(auto) _with_value(Func&& func) const
        {
            const auto& src = _source();
            if (std::holds_alternative<Type>(src._value))
                return std::forward<Func>(func)(std::get<Type>(src._value));
            const auto v = _read();
            return std::forward<Func>(func)(v);
        }

        bool _invoke_raw(const Deps& dependency, Query query) const
        {
            const auto obj = dependency.obj;
            auto* const base = dependency.base;
            const auto func = reinterpret_cast<manager_callback>(dependency.func);
            if (func == nullptr)
                return false;
            return _with_value([&](const Type& v) { return func(obj.lock(), base, v, query); });
        }

        void _destroy(Deps dependency) override
        {
            const auto obj = dependency.obj;
            auto* const base = dependency.base;
            const auto func = reinterpret_cast<manager_callback>(dependency.func);
            if (func == nullptr)
                return;
            _with_value([&](const Type& v) { func(obj.lock(), base, v, Query::Destroy); });
        }
        void _provider_destroyed()
        {
            _link.release();
            _value = Type{};
        }

        Handle _subscribe_base_local(std::weak_ptr<void> handle, void* base, manager_callback func)
        {
            Deps dependency{handle, base, reinterpret_cast<void (*)()>(func)};
            bool keep = false;

            auto state = _ensure_state();
            {
                InvokeGuard guard(*this, state);
                keep = _invoke_raw(dependency, Query::Invoke);
            }

            if (!keep)
            {
                _destroy(dependency);
                return {};
            }

            return _insert_slot(std::move(dependency));
        }
        void _apply_local()
        {
            if (_state == nullptr)
                return;

            auto state = _state;
            InvokeGuard guard(*this, state);
            const auto end = state->dependencies.size();

            _with_value(
                [&](const Type& v)
                {
                    for (std::size_t i = 0; i < end && i < state->dependencies.size(); ++i)
                    {
                        auto& it = state->dependencies[i];
                        if (it.func == nullptr)
                            continue;

                        const auto obj = it.obj;
                        auto* const base = it.base;
                        const auto func = reinterpret_cast<manager_callback>(it.func);
                        const auto generation = it.generation;

                        if (!func(obj.lock(), base, v, Query::Invoke))
                            _close_slot_local(static_cast<std::uint32_t>(i), generation);
                    }
                }
            );
        }
        template<typename Value> void _set_local(Value&& value)
        {
            _unlink_local();
            _value = std::forward<Value>(value);
            _apply_local();
        }
        void _unlink_local()
        {
            if (std::holds_alternative<Type>(_value))
                return;
            _link.close();
            _value = Type{};
        }
    public:
        template<typename Other> friend class Dependency;

        Dependency() = default;
        Dependency(const Dependency& copy) :
            DependencyBase(), _value(IOContext::get()->sync([&copy] { return copy._read(); }))
        {
        }
        Dependency(Dependency&&) = delete;
        ~Dependency()
        {
            IOContext::get()->sync(
                [this]
                {
                    _unlink_local();
                    _destroy_state_local();
                }
            );
        }

        template<typename Data, typename Elem, typename Func>
        Handle subscribe(std::weak_ptr<Elem> handle, Data&& def, Func&& callback)
        {
            using DataT = std::remove_cvref_t<Data>;
            using FuncT = std::remove_cvref_t<Func>;
            struct State
            {
                FuncT callback;
                [[no_unique_address]] DataT data;
            };
            auto* state = new State{
                .callback = std::forward<Func>(callback), .data = std::forward<Data>(def)
            };
            return subscribe_base(
                handle,
                state,
                +[](std::shared_ptr<void> handle, void* ptr, const Type& value, Query query) -> bool
                {
                    auto* state = static_cast<State*>(ptr);
                    if (query == Query::Destroy)
                    {
                        delete state;
                        return false;
                    }
                    if (handle == nullptr)
                        return false;
                    if constexpr (std::is_same_v<Dummy, DataT>)
                    {
                        return bool(state->callback(std::static_pointer_cast<Elem>(handle), value));
                    }
                    else
                    {
                        return bool(state->callback(
                            std::static_pointer_cast<Elem>(handle), state->data, value
                        ));
                    }
                }
            );
        }
        template<typename Elem, typename Func>
        Handle subscribe(std::weak_ptr<Elem> handle, Func&& callback)
        {
            return subscribe<Dummy>(handle, Dummy{}, std::forward<Func>(callback));
        }
        Handle subscribe_base(std::weak_ptr<void> handle, void* base, manager_callback func)
        {
            return IOContext::get()->sync(
                [this, handle = std::move(handle), base, func]() mutable
                { return _subscribe_base_local(std::move(handle), base, func); }
            );
        }
        void cleanup()
        {
            IOContext::get()->sync([this] { _cleanup_local(); });
        }
        void apply()
        {
            IOContext::get()->sync([this] { _apply_local(); });
        }

        template<typename Value> void set(Value&& value)
        {
            IOContext::get()->sync([this, value = std::forward<Value>(value)]() mutable
                                   { _set_local(std::move(value)); });
        }
        template<typename Value> void set_async(Value&& value)
        {
            auto state = _ensure_state();

            IOContext::get()->sync_async(
                [state, value = std::forward<Value>(value)]() mutable
                {
                    if (state->owner == nullptr || state->destroying)
                        return;
                    static_cast<Dependency<Type>*>(state->owner)->_set_local(std::move(value));
                }
            );
        }

        const Type& value() const
        {
            return IOContext::get()->sync([this]() -> const Type& { return _value_local(); });
        }

        Type read() const
        {
            return IOContext::get()->sync([this] { return _read(); });
        }

        void unlink()
        {
            IOContext::get()->sync([this] { _unlink_local(); });
        }
        void link(Dependency<Type>& provider)
        {
            IOContext::get()->sync(
                [this, &provider]
                {
                    _unlink_local();
                    _value = &provider;
                    _link = provider._subscribe_base_local(
                        std::weak_ptr<void>{},
                        this,
                        [](std::shared_ptr<void>, void* ptr, const Type&, Query query) -> bool
                        {
                            if (query == Query::Destroy)
                                static_cast<Dependency<Type>*>(ptr)->_provider_destroyed();
                            else
                                static_cast<Dependency<Type>*>(ptr)->_apply_local();
                            return true;
                        }
                    );
                }
            );
        }
        template<typename Other>
        void link(Dependency<Other>& provider, Type (*callback)(const Other&))
        {
            IOContext::get()->sync(
                [this, &provider, callback]
                {
                    _unlink_local();
                    _value = dynalink{
                        +[](void (*icb)(), void* ptr, void*, bool destroy)
                        {
                            const auto callback = reinterpret_cast<Type (*)(const Other&)>(icb);
                            const auto provider = static_cast<Dependency<Other>*>(ptr);
                            if (destroy)
                                return Type{};
                            return callback(provider->_read());
                        },
                        reinterpret_cast<void (*)()>(callback),
                        static_cast<void*>(&provider)
                    };
                    _link = provider._subscribe_base_local(
                        std::weak_ptr<void>{},
                        this,
                        [](std::shared_ptr<void>, void* ptr, const Other&, Query query) -> bool
                        {
                            if (query == Query::Destroy)
                                static_cast<Dependency<Type>*>(ptr)->_provider_destroyed();
                            else
                                static_cast<Dependency<Type>*>(ptr)->_apply_local();
                            return true;
                        }
                    );
                }
            );
        }

        template<typename Value>
        operator Value() const
            requires(!std::is_reference_v<Value> || std::is_const_v<Value>)
        {
            return IOContext::get()->sync(
                [this]() -> Value
                {
                    if constexpr (std::is_reference_v<Value>)
                        return static_cast<Value>(_value_local());
                    else
                        return static_cast<Value>(_read());
                }
            );
        }

        //
        // Operators
        //

        constexpr auto operator+()
            requires requires(const Type& v) { +v; }
        {
            return IOContext::get()->sync([this] { return +_value_local(); });
        }
        constexpr auto operator-()
            requires requires(const Type& v) { -v; }
        {
            return IOContext::get()->sync([this] { return -_value_local(); });
        }
        constexpr auto operator~()
            requires requires(const Type& v) { ~v; }
        {
            return IOContext::get()->sync([this] { return ~_value_local(); });
        }
        constexpr auto operator!()
            requires requires(const Type& v) { !v; }
        {
            return IOContext::get()->sync([this] { return !_value_local(); });
        }

        constexpr auto operator++()
            requires requires(Type& v) { ++v; }
        {
            return IOContext::get()->sync(
                [this]
                {
                    auto v = ++_value_local();
                    _apply_local();
                    return v;
                }
            );
        }
        constexpr auto operator--()
            requires requires(Type& v) { --v; }
        {
            return IOContext::get()->sync(
                [this]
                {
                    auto v = --_value_local();
                    _apply_local();
                    return v;
                }
            );
        }
        constexpr auto operator++(int)
            requires requires(Type& v) { v++; }
        {
            return IOContext::get()->sync(
                [this]
                {
                    auto v = _value_local()++;
                    _apply_local();
                    return v;
                }
            );
        }
        constexpr auto operator--(int)
            requires requires(Type& v) { v--; }
        {
            return IOContext::get()->sync(
                [this]
                {
                    auto v = _value_local()--;
                    _apply_local();
                    return v;
                }
            );
        }

#define FORWARD_ASSIGN_OP(op)                                                                      \
    template<typename Other>                                                                       \
    constexpr auto operator op(Other&& rhs)                                                        \
        requires requires(Type& v, Other&& o) { v op std::forward<Other>(o); }                     \
    {                                                                                              \
        return IOContext::get()->sync(                                                             \
            [this, rhs = std::forward<Other>(rhs)]() mutable                                       \
            {                                                                                      \
                auto v = (_value_local() op std::move(rhs));                                       \
                _apply_local();                                                                    \
                return v;                                                                          \
            }                                                                                      \
        );                                                                                         \
    }

        FORWARD_ASSIGN_OP(+=)
        FORWARD_ASSIGN_OP(-=)
        FORWARD_ASSIGN_OP(*=)
        FORWARD_ASSIGN_OP(/=)
        FORWARD_ASSIGN_OP(%=)
        FORWARD_ASSIGN_OP(&=)
        FORWARD_ASSIGN_OP(|=)
        FORWARD_ASSIGN_OP(^=)
        FORWARD_ASSIGN_OP(<<=)
        FORWARD_ASSIGN_OP(>>=)

#undef FORWARD_ASSIGN_OP

        template<class Index>
        constexpr auto operator[](Index&& index) const
            requires requires(const Type& v, Index&& i) { v[std::forward<Index>(i)]; }
        {
            return IOContext::get()->sync([this, index = std::forward<Index>(index)]() mutable
                                          { return _value_local()[std::move(index)]; });
        }

        template<class... Args>
        constexpr auto operator()(Args&&... args) const
            requires requires(const Type& v, Args&&... as) { v(std::forward<Args>(as)...); }
        {
            return IOContext::get()->sync([this, ... args = std::forward<Args>(args)]() mutable
                                          { return _value_local()(std::move(args)...); });
        }

        //
        //
        //

        Dependency& operator=(const Dependency& copy)
        {
            IOContext::get()->sync([this, &copy] { _set_local(copy._read()); });
            return *this;
        }
        Dependency& operator=(Dependency&&) = delete;

        template<typename Value>
            requires std::is_assignable_v<Type&, Value>
        Dependency& operator=(Value&& value)
        {
            set(std::forward<Value>(value));
            return *this;
        }
    };

    class Theme : public std::enable_shared_from_this<Theme>
    {
    public:
        using ptr = std::shared_ptr<Theme>;
    private:
        std::size_t _code{0x00};
    public:
        static std::size_t id_to_code(const std::string& id)
        {
            return std::hash<std::string>()(id);
        }

        template<typename Type> static auto make(std::vector<px::background> accents = {})
        {
            auto ptr = std::make_shared<Type>();
            ptr->_code = Type::tc;
            ptr->from(std::move(accents));
            return ptr;
        }
        template<typename Type>
        static auto make_named(const std::string& name, std::vector<px::background> accents = {})
        {
            auto ptr = std::make_shared<Type>();
            ptr->_code = std::hash<std::string>()(name);
            ptr->from(std::move(accents));
            return ptr;
        }

        Theme() = default;
        Theme(const Theme&) = default;
        Theme(Theme&&) = default;
        virtual ~Theme() = default;

        std::size_t code() const noexcept
        {
            return _code;
        }

        template<typename Theme> Theme& as()
        {
            return *static_cast<Theme*>(this);
        }

        Theme& operator=(const Theme&) = default;
        Theme& operator=(Theme&&) = default;
    };

    template<auto Func, typename Type> struct Dynavar
    {
        static constexpr auto filter = Func;
        Type& dependency;

        explicit Dynavar(Type& dep) : dependency(dep) {}

        template<typename Value>
        operator Value() const
            requires(!std::is_reference_v<Value> || std::is_const_v<Value>)
        {
            return Func(dependency.value());
        }
    };

    template<auto Func, typename Type> auto dynavar(Type& dep)
    {
        return Dynavar<Func, Type>(dep);
    }

    template<typename Type, meta::ConstString Name> struct Dep
    {
        using type = Type;
        static constexpr auto name = Name.view();
    };

    template<typename Base, typename Deps, typename... Ts>
    class DeclTheme : public Theme, public Deps
    {
    private:
        std::tuple<Dependency<typename Ts::type>...> _deps{};
    public:
        static inline const auto tc = typeid(typename Deps::Deps).hash_code();

        DeclTheme() = default;
        DeclTheme(const DeclTheme&) = default;
        DeclTheme(DeclTheme&&) = default;

        using Deps::Deps;

        template<Deps::Deps Idx> auto& get()
        {
            return std::get<std::size_t(Idx)>(_deps);
        }

        DeclTheme& operator=(const DeclTheme&) = default;
        DeclTheme& operator=(DeclTheme&&) = default;
    };

    template<typename Deps> struct ThemeRegistry
    {
        using type = void;
    };
} // namespace d2::style
