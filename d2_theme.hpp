#pragma once

#include <algorithm>
#include <d2_meta.hpp>
#include <d2_pixel.hpp>
#include <memory>
#include <variant>
#include <vector>

namespace d2::style
{
    class Theme : public std::enable_shared_from_this<Theme>
    {
    public:
        enum class Query
        {
            Invoke,
            Destroy,
        };
        template<typename Type> class Dependency
        {
        public:
            using value_type = Type;
            using manager_callback = bool (*)(std::shared_ptr<void>, void*, const Type&, Query);
        private:
            struct Dummy
            {
            };
            struct Deps
            {
                std::weak_ptr<void> obj{};
                void* base{nullptr};
                manager_callback func{nullptr};
            };
            // trampoline | callback | ptr
            using dynalink =
                // callback | provider | this
                std::tuple<Type (*)(void (*)(), void*, void*, bool), void (*)(), void*>;
        private:
            std::size_t _recursion_ctr{0};
            std::variant<Type, Dependency<Type>*, dynalink> _value{Type()};
            std::vector<Deps> _dependencies{};
            std::vector<Deps> _pending_sub{};
            std::vector<Deps> _pending_unsub{};

            struct InvokeGuard
            {
                Dependency& self;

                explicit InvokeGuard(Dependency& self) : self(self)
                {
                    ++self._recursion_ctr;
                }
                ~InvokeGuard()
                {
                    --self._recursion_ctr;
                    if (self._recursion_ctr == 0)
                        self._flush_pending();
                }

                InvokeGuard(const InvokeGuard&) = delete;
                InvokeGuard& operator=(const InvokeGuard&) = delete;
            };

            Dependency& _source()
            {
                auto* current = this;
                while (std::holds_alternative<Dependency<Type>*>(current->_value))
                {
                    auto* next = std::get<Dependency<Type>*>(current->_value);
                    if (next == nullptr)
                        throw std::logic_error("Dependency link is null");
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
                        throw std::logic_error("Dependency link is null");
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
                const auto func = dependency.func;
                if (func == nullptr)
                    return false;
                return _with_value([&](const Type& v) { return func(obj.lock(), base, v, query); });
            }
            bool _invoke(Deps& dependency, Query query)
            {
                InvokeGuard guard(*this);
                return _invoke_raw(dependency, query);
            }

            void _destroy(Deps dependency)
            {
                const auto obj = dependency.obj;
                auto* const base = dependency.base;
                const auto func = dependency.func;
                if (func == nullptr)
                    return;
                _with_value([&](const Type& v) { func(obj.lock(), base, v, Query::Destroy); });
            }
            void _queue_sub(Deps dependency)
            {
                if (_recursion_ctr != 0)
                    _pending_sub.push_back(std::move(dependency));
                else
                    _dependencies.push_back(std::move(dependency));
            }
            void _queue_unsub(Deps& dependency)
            {
                if (dependency.func == nullptr)
                    return;

                if (_recursion_ctr != 0)
                {
                    _pending_unsub.push_back(dependency);
                    dependency.obj = std::weak_ptr<void>();
                    dependency.base = nullptr;
                    dependency.func = nullptr;
                }
                else
                {
                    auto copy = dependency;
                    dependency.obj = std::weak_ptr<void>();
                    dependency.base = nullptr;
                    dependency.func = nullptr;
                    _destroy(copy);
                }
            }

            template<typename Pred> void _unsubscribe_if(Pred&& pred)
            {
                std::vector<Deps> destroy{};
                for (auto it = _pending_sub.begin(); it != _pending_sub.end();)
                {
                    if (std::forward<Pred>(pred)(*it))
                    {
                        destroy.push_back(*it);
                        it = _pending_sub.erase(it);
                    }
                    else
                        ++it;
                }
                if (_recursion_ctr != 0)
                {
                    for (auto& it : _dependencies)
                    {
                        if (it.func != nullptr && std::forward<Pred>(pred)(it))
                            _queue_unsub(it);
                    }
                }
                else
                {
                    for (auto it = _dependencies.begin(); it != _dependencies.end();)
                    {
                        if (it->func != nullptr && std::forward<Pred>(pred)(*it))
                        {
                            destroy.push_back(*it);
                            it = _dependencies.erase(it);
                        }
                        else
                            ++it;
                    }
                }
                for (auto& it : destroy)
                    _destroy(it);
            }
            void _flush_pending()
            {
                if (_recursion_ctr != 0)
                    return;
                if (!_pending_unsub.empty())
                {
                    _dependencies.erase(
                        std::remove_if(
                            _dependencies.begin(),
                            _dependencies.end(),
                            [&](const auto& v) { return v.func == nullptr; }
                        ),
                        _dependencies.end()
                    );

                    auto pending = std::move(_pending_unsub);
                    _pending_unsub.clear();

                    for (auto& it : pending)
                        _destroy(it);
                }
                if (!_pending_sub.empty())
                {
                    _dependencies.reserve(_dependencies.size() + _pending_sub.size());
                    std::move(
                        _pending_sub.begin(), _pending_sub.end(), std::back_inserter(_dependencies)
                    );
                    _pending_sub.clear();
                }
            }
        public:
            template<typename Other> friend class Dependency;

            Dependency() = default;
            Dependency(const Dependency& copy) : _value(copy._read()) {}
            Dependency(Dependency&&) = default;
            ~Dependency()
            {
                unlink();

                auto dependencies = std::move(_dependencies);
                auto pending_sub = std::move(_pending_sub);
                auto pending_unsub = std::move(_pending_unsub);

                _dependencies.clear();
                _pending_sub.clear();
                _pending_unsub.clear();

                for (auto& it : dependencies)
                    _destroy(it);
                for (auto& it : pending_sub)
                    _destroy(it);
                for (auto& it : pending_unsub)
                    _destroy(it);
            }

            template<typename Data, typename Elem, typename Func>
            void subscribe(std::weak_ptr<Elem> handle, Data&& def, Func&& callback)
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
                Deps dependency{
                    handle,
                    state,
                    +[](std::shared_ptr<void> handle, void* ptr, const Type& value, Query query)
                        -> bool
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
                            return bool(
                                state->callback(std::static_pointer_cast<Elem>(handle), value)
                            );
                        }
                        else
                        {
                            return bool(state->callback(
                                std::static_pointer_cast<Elem>(handle), &state->data, value
                            ));
                        }
                    }
                };
                bool keep = false;
                {
                    InvokeGuard guard(*this);
                    keep = _invoke_raw(dependency, Query::Invoke);
                    if (keep)
                        _pending_sub.push_back(dependency);
                }
                if (!keep)
                    _destroy(dependency);
            }
            template<typename Elem, typename Func>
            void subscribe(std::weak_ptr<Elem> handle, Func&& callback)
            {
                subscribe<Dummy>(handle, Dummy{}, std::forward<Func>(callback));
            }
            void subscribe_base(std::weak_ptr<void> handle, void* base, manager_callback func)
            {
                Deps dependency{handle, base, func};
                bool keep = false;
                {
                    InvokeGuard guard(*this);
                    keep = _invoke_raw(dependency, Query::Invoke);
                    if (keep)
                        _pending_sub.push_back(dependency);
                }
                if (!keep)
                    _destroy(dependency);
            }
            void apply()
            {
                InvokeGuard guard(*this);

                _with_value(
                    [&](const Type& v)
                    {
                        for (auto& it : _dependencies)
                        {
                            if (it.func == nullptr)
                                continue;

                            const auto obj = it.obj;
                            auto* const base = it.base;
                            const auto func = it.func;

                            if (!func(obj.lock(), base, v, Query::Invoke))
                                _queue_unsub(it);
                        }
                    }
                );
            }

            template<typename Value> void set(Value&& value)
            {
                unlink();
                _value = std::forward<Value>(value);
                apply();
            }

            Type& value()
            {
                auto& src = _source();
                if (!std::holds_alternative<Type>(src._value))
                    throw std::logic_error("Dependency value is dynamic and cannot be referenced");
                return std::get<Type>(src._value);
            }
            const Type& value() const
            {
                const auto& src = _source();
                if (!std::holds_alternative<Type>(src._value))
                    throw std::logic_error("Dependency value is dynamic and cannot be referenced");
                return std::get<Type>(src._value);
            }

            void unlink()
            {
                if (std::holds_alternative<Type>(_value))
                    return;
                if (std::holds_alternative<Dependency<Type>*>(_value))
                {
                    auto* provider = std::get<Dependency<Type>*>(_value);
                    if (provider != nullptr)
                    {
                        provider->_unsubscribe_if([&](const auto& v) { return v.base == this; });
                    }
                }
                else if (std::holds_alternative<dynalink>(_value))
                {
                    const auto& [callback, icb, ptr] = std::get<dynalink>(_value);
                    callback(icb, ptr, this, true);
                }
                _value = Type{};
            }
            void link(Dependency<Type>& provider)
            {
                unlink();
                _value = &provider;
                provider.subscribe(
                    std::weak_ptr<void>{},
                    this,
                    [](std::shared_ptr<void>, void* ptr, const Type&, Query query) -> bool
                    {
                        if (query == Query::Destroy)
                            static_cast<Dependency<Type>*>(ptr)->unlink();
                        else
                            static_cast<Dependency<Type>*>(ptr)->apply();
                        return true;
                    }
                );
            }
            template<typename Other>
            void link(Dependency<Other>& provider, Type (*callback)(const Other&))
            {
                unlink();
                _value = dynalink{
                    +[](void (*icb)(), void* ptr, void* tptr, bool destroy)
                    {
                        auto callback = reinterpret_cast<Type (*)(const Other&)>(icb);
                        auto provider = static_cast<Dependency<Other>*>(ptr);
                        if (destroy)
                        {
                            provider->_unsubscribe_if([&](const auto& v)
                                                      { return v.base == tptr; });
                            return Type{};
                        }
                        return callback(provider->_read());
                    },
                    reinterpret_cast<void (*)()>(callback),
                    static_cast<void*>(&provider)
                };
                provider.subscribe(
                    std::weak_ptr<void>{},
                    this,
                    [](std::shared_ptr<void>, void* ptr, const Other&, Query query) -> bool
                    {
                        if (query == Query::Destroy)
                            static_cast<Dependency<Type>*>(ptr)->unlink();
                        else
                            static_cast<Dependency<Type>*>(ptr)->apply();
                        return true;
                    }
                );
            }

            template<typename Value>
            operator Value() const
                requires(!std::is_reference_v<Value> || std::is_const_v<Value>)
            {
                if constexpr (std::is_reference_v<Value>)
                    return static_cast<Value>(value());
                else
                    return static_cast<Value>(_read());
            }

            //
            // Operators
            //

            constexpr auto operator+()
                requires requires(Type& v) { +v; }
            {
                return +value();
            }
            constexpr auto operator-()
                requires requires(Type& v) { -v; }
            {
                return -value();
            }
            constexpr auto operator~()
                requires requires(Type& v) { ~v; }
            {
                return ~value();
            }
            constexpr auto operator!()
                requires requires(Type& v) { !v; }
            {
                return !value();
            }

            constexpr auto operator++()
                requires requires(Type& v) { ++v; }
            {
                auto v = ++value();
                apply();
                return v;
            }
            constexpr auto operator--()
                requires requires(Type& v) { --v; }
            {
                auto v = --value();
                apply();
                return v;
            }
            constexpr auto operator++(int)
                requires requires(Type& v) { v++; }
            {
                auto v = value()++;
                apply();
                return v;
            }
            constexpr auto operator--(int)
                requires requires(Type& v) { v--; }
            {
                auto v = value()--;
                apply();
                return v;
            }

#define FORWARD_ASSIGN_OP(op)                                                                      \
    template<typename Other>                                                                       \
    constexpr auto operator op(Other&& rhs)                                                        \
        requires requires(Type& v, Other&& o) { v op std::forward<Other>(o); }                     \
    {                                                                                              \
        auto v = (value() op std::forward<Other>(rhs));                                            \
        apply();                                                                                   \
        return v;                                                                                  \
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
            constexpr auto operator[](Index&& index)
                requires requires(Type& v, Index&& i) { v[std::forward<Index>(i)]; }
            {
                return value()[std::forward<Index>(index)];
            }

            template<class... Args>
            constexpr auto operator()(Args&&... args)
                requires requires(Type& v, Args&&... as) { v(std::forward<Args>(as)...); }
            {
                return value()(std::forward<Args>(args)...);
            }

            //
            //
            //

            Dependency& operator=(const Dependency& copy)
            {
                set(copy._read());
                return *this;
            }
            Dependency& operator=(Dependency&&) = default;

            template<typename Value>
                requires std::is_assignable_v<Type&, Value>
            Dependency& operator=(Value&& value)
            {
                set(std::forward<Value>(value));
                return *this;
            }
        };
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
