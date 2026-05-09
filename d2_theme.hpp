#ifndef D2_THEME_HPP
#define D2_THEME_HPP

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
            std::variant<Type, Dependency<Type>*, dynalink> _value{Type()};
            std::vector<Deps> _dependencies{};

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
            bool _invoke(Deps& dependency, Query query) const
            {
                return _with_value(
                    [&](const Type& v)
                    { return dependency.func(dependency.obj.lock(), dependency.base, v, query); }
                );
            }
        public:
            template<typename Other> friend class Dependency;

            Dependency() = default;
            Dependency(const Dependency& copy) : _value(copy._read()) {}
            Dependency(Dependency&&) = default;
            ~Dependency()
            {
                unlink();
                _with_value(
                    [&](const Type& v)
                    {
                        for (decltype(auto) it : _dependencies)
                        {
                            it.func(it.obj.lock(), it.base, v, Query::Destroy);
                            it.obj = std::weak_ptr<void>();
                            it.base = nullptr;
                            it.func = nullptr;
                        }
                    }
                );
            }

            template<typename Data, typename Elem, typename Func>
            void subscribe(std::weak_ptr<Elem> handle, Func&& callback, Data&& def = {})
            {
                struct State
                {
                    Func callback;
                    [[no_unique_address]] Data data;
                };
                _dependencies.push_back(
                    Deps{
                        handle,
                        new State{
                            .callback = std::forward<Func>(callback),
                            .data = std::forward<Data>(def)
                        },
                        +[](std::shared_ptr<void> handle, void* ptr, const Type& value, Query query)
                            -> bool
                        {
                            auto* state = static_cast<State*>(ptr);
                            if (handle == nullptr || query == Query::Destroy)
                            {
                                delete state;
                                return false;
                            }
                            if constexpr (std::is_same_v<Dummy, Data>)
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
                    }
                );
                if (!_invoke(_dependencies.back(), Query::Invoke))
                    _dependencies.pop_back();
            }
            template<typename Elem, typename Func>
            void subscribe(std::weak_ptr<Elem> handle, Func&& callback)
            {
                subscribe<Dummy>(handle, std::forward<Func>(callback));
            }
            void subscribe(std::weak_ptr<void> handle, void* base, manager_callback func)
            {
                _dependencies.push_back(Deps{handle, base, func});
                if (!_invoke(_dependencies.back(), Query::Invoke))
                    _dependencies.pop_back();
            }
            void apply()
            {
                bool has_expired = false;
                _with_value(
                    [&](const Type& v)
                    {
                        for (decltype(auto) it : _dependencies)
                        {
                            const auto ptr = it.obj.lock();
                            if (!it.func(ptr, it.base, v, Query::Invoke))
                            {
                                it.obj = std::weak_ptr<void>();
                                it.base = nullptr;
                                it.func = nullptr;
                                has_expired = true;
                            }
                        }
                    }
                );

                if (has_expired)
                {
                    _dependencies.erase(
                        std::remove_if(
                            _dependencies.begin(),
                            _dependencies.end(),
                            [&](const auto& v) { return v.func == nullptr; }
                        ),
                        _dependencies.end()
                    );
                }
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
                        auto& deps = provider->_dependencies;
                        deps.erase(
                            std::remove_if(
                                deps.begin(),
                                deps.end(),
                                [&](const auto& v) { return v.base == this; }
                            ),
                            deps.end()
                        );
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
                            auto& deps = provider->_dependencies;
                            deps.erase(
                                std::remove_if(
                                    deps.begin(),
                                    deps.end(),
                                    [&](const auto& v) { return v.base == tptr; }
                                ),
                                deps.end()
                            );
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
        std::tuple<typename Ts::type...> _deps{};
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

#endif
