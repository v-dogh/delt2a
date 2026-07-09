#pragma once

#include <absl/container/flat_hash_map.h>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <core/utils/d2_exceptions.hpp>
#include <functional>
#include <memory>
#include <mt/pool.hpp>
#include <mt/task_ring.hpp>
#include <new>
#include <shared_mutex>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

namespace d2
{
    template<typename Type> class Ref
    {
    private:
        static_assert(!std::is_reference_v<Type>, "Ref<Type> cannot store a reference Type");
        Type* _value;
    public:
        explicit constexpr Ref(Type& value) noexcept : _value(std::addressof(value)) {}

        constexpr Type& get() const noexcept
        {
            return *_value;
        }

        constexpr operator Type&() const noexcept
        {
            return *_value;
        }

        constexpr Type* operator&() const noexcept
        {
            return _value;
        }
        constexpr Type* operator->() const noexcept
        {
            return _value;
        }
        constexpr Type& operator*() const noexcept
        {
            return *_value;
        }
    };

    template<typename Type> Ref<Type> ref(Type& value)
    {
        return Ref<Type>(value);
    }
    template<typename Type> Ref<const Type> cref(const Type& value)
    {
        return Ref<const Type>(value);
    }

    namespace impl
    {
        template<typename> inline constexpr bool dependent_false_v = false;

        template<typename> struct func_info;
        template<typename Ret, typename... Argv> struct func_info<Ret(Argv...)>
        {
            using ret_t = Ret;
            using args_t = std::tuple<Argv...>;
        };
        template<typename Ret, typename... Argv>
        struct func_info<Ret (*)(Argv...)> : func_info<Ret(Argv...)>
        {
        };
        template<typename State, typename Ret, typename... Argv>
        struct func_info<Ret (State::*)(Argv...) const> : func_info<Ret(Argv...)>
        {
        };
        template<typename State, typename Ret, typename... Argv>
        struct func_info<Ret (State::*)(Argv...)> : func_info<Ret(Argv...)>
        {
        };
        template<typename Func>
        struct func_info : func_info<decltype(&std::decay_t<Func>::operator())>
        {
        };

        template<typename Type> struct normalize
        {
            using type = std::decay_t<Type>;
        };
        template<typename Type> struct normalize<Type&>
        {
            using type = Ref<Type>;
        };
        template<typename Type> struct normalize<Ref<Type>>
        {
            using type = Ref<Type>;
        };
        template<typename... Argv> struct normalize<std::tuple<Argv...>>
        {
            using type = std::tuple<typename normalize<Argv>::type...>;
        };
        template<typename Type> struct args_normalize
        {
        private:
            using no_ref_t = std::remove_reference_t<Type>;
            using value_t = std::remove_cv_t<no_ref_t>;
        public:
            using type = std::conditional_t<std::is_lvalue_reference_v<Type>, Type, const value_t&>;
        };
        template<typename Type> struct args_normalize<Ref<Type>>
        {
            using type = Type&;
        };
        template<typename... Argv> struct args_normalize<std::tuple<Argv...>>
        {
            using type = std::tuple<typename args_normalize<Argv>::type...>;
        };
        template<typename Type> struct args_sig_normalize
        {
        private:
            using no_ref_t = std::remove_reference_t<Type>;
            using value_t = std::remove_cv_t<no_ref_t>;
        public:
            using type = std::conditional_t<std::is_lvalue_reference_v<Type>, Type, const value_t&>;
        };

        template<typename Type> struct args_sig_normalize<Ref<Type>>
        {
            using type = Type&;
        };
        template<typename... Argv> struct args_sig_normalize<std::tuple<Argv...>>
        {
            using type = std::tuple<typename args_sig_normalize<Argv>::type...>;
        };

        struct RuntimeArg
        {
            const std::type_info* type{nullptr};
            bool lvalue_ref{false};
            bool is_const{false};
            bool is_volatile{false};

            template<typename Type> static RuntimeArg make() noexcept
            {
                using no_ref_t = std::remove_reference_t<Type>;
                using base_t = std::remove_cv_t<no_ref_t>;
                return RuntimeArg{
                    &typeid(base_t),
                    std::is_lvalue_reference_v<Type>,
                    std::is_const_v<no_ref_t>,
                    std::is_volatile_v<no_ref_t>,
                };
            }

            bool operator==(const RuntimeArg& other) const noexcept;
            bool operator!=(const RuntimeArg& other) const noexcept;
        };

        struct RuntimeSignature
        {
            const RuntimeArg* args{nullptr};
            std::size_t size{0};

            bool operator==(const RuntimeSignature& other) const noexcept;
        };

        template<typename... Argv> RuntimeSignature runtime_signature_for() noexcept
        {
            static const std::array<RuntimeArg, sizeof...(Argv)> value{RuntimeArg::make<Argv>()...};
            return RuntimeSignature{value.data(), value.size()};
        }

        template<typename Tuple> struct runtime_signature_for_tuple;
        template<typename... Argv> struct runtime_signature_for_tuple<std::tuple<Argv...>>
        {
            static RuntimeSignature get() noexcept
            {
                return runtime_signature_for<Argv...>();
            }
        };
    } // namespace impl

    template<auto Ev, typename... Argv> struct EvCase
    {
        static constexpr auto event = Ev;
        static constexpr bool def = false;
        using raw_signature_t = std::tuple<Argv...>;
        using signature_t = typename impl::args_normalize<raw_signature_t>::type;
        using event_t = decltype(Ev);
    };

    template<typename Ev, typename... Argv> struct EvDefault
    {
        static constexpr bool def = true;
        using raw_signature_t = std::tuple<Argv...>;
        using signature_t = typename impl::args_normalize<raw_signature_t>::type;
        using event_t = Ev;
    };

    template<typename> struct SignalRegistry
    {
        using manifest_t = void;
    };

    template<typename Spec, typename... Specs> class SignalManifest
    {
    private:
        using event_idx = std::uint64_t;

        template<auto Ev, typename Candidate, bool Default = Candidate::def> struct CaseMatches;

        template<auto Ev, typename Candidate>
        struct CaseMatches<Ev, Candidate, true> : std::false_type
        {
        };

        template<auto Ev, typename Candidate>
        struct CaseMatches<Ev, Candidate, false> : std::bool_constant<Ev == Candidate::event>
        {
        };

        template<auto Ev, typename... Items> struct FindExactImpl
        {
            using type = void;
        };

        template<auto Ev, typename First, typename... Rest> struct FindExactImpl<Ev, First, Rest...>
        {
        private:
            using rest_t = typename FindExactImpl<Ev, Rest...>::type;
        public:
            using type = std::conditional_t<CaseMatches<Ev, First>::value, First, rest_t>;
        };

        template<typename... Items> struct FindDefaultImpl
        {
            using type = void;
        };
        template<typename First, typename... Rest> struct FindDefaultImpl<First, Rest...>
        {
        private:
            using rest_t = typename FindDefaultImpl<Rest...>::type;
        public:
            using type = std::conditional_t<First::def, First, rest_t>;
        };

        template<auto Ev> struct FindCase
        {
        private:
            using exact_t = typename FindExactImpl<Ev, Spec, Specs...>::type;
            using default_t = typename FindDefaultImpl<Spec, Specs...>::type;
        public:
            using case_t = std::conditional_t<std::is_void_v<exact_t>, default_t, exact_t>;
        };

        template<typename Candidate> static bool _runtime_event_matches(event_idx ev) noexcept
        {
            if constexpr (Candidate::def)
                return false;
            else
                return ev == static_cast<event_idx>(Candidate::event);
        }

        template<typename Candidate>
        static bool _runtime_signature_matches(impl::RuntimeSignature signature) noexcept
        {
            return signature ==
                   impl::runtime_signature_for_tuple<typename Candidate::signature_t>::get();
        }
        template<typename Candidate>
        static bool _runtime_case_matches(event_idx ev, impl::RuntimeSignature signature) noexcept
        {
            return _runtime_event_matches<Candidate>(ev) &&
                   _runtime_signature_matches<Candidate>(signature);
        }
    public:
        using event_t = typename Spec::event_t;
        using manifest_t = SignalManifest<Spec, Specs...>;

        template<auto Ev> using case_for_t = typename FindCase<Ev>::case_t;

        static_assert(std::is_enum_v<event_t>, "Signal ID must be an enum");
        static_assert(
            (std::is_same_v<event_t, typename Specs::event_t> && ...),
            "All signal IDs of a manifest must share an enum"
        );
        static_assert(
            (std::size_t{Spec::def} + ... + std::size_t{Specs::def}) <= 1,
            "A signal manifest can only have one default case"
        );

        static bool verify(event_idx ev, impl::RuntimeSignature signature) noexcept
        {
            const bool exact_event =
                (_runtime_event_matches<Spec>(ev) || ... || _runtime_event_matches<Specs>(ev));

            if (exact_event)
            {
                return (
                    _runtime_case_matches<Spec>(ev, signature) || ... ||
                    _runtime_case_matches<Specs>(ev, signature)
                );
            }

            using default_t = typename FindDefaultImpl<Spec, Specs...>::type;
            if constexpr (std::is_void_v<default_t>)
                return false;
            else
                return _runtime_signature_matches<default_t>(signature);
        }
    };

    template<> struct SignalRegistry<void>
    {
    private:
        template<auto Ev, typename Manifest> struct SignatureLookupImpl
        {
            static_assert(!std::is_void_v<Manifest>, "Signal manifest not registered");
        };

        template<auto Ev, typename Manifest>
            requires(!std::is_void_v<Manifest>)
        struct SignatureLookupImpl<Ev, Manifest>
        {
        private:
            using case_t = typename Manifest::template case_for_t<Ev>;
            static_assert(!std::is_void_v<case_t>, "Signal event not handled in manifest");
        public:
            using signature_t = typename case_t::signature_t;
        };

        template<auto Ev, typename Tuple> struct SatisfiedTupleImpl;
        template<auto Ev, typename... Argv>
        struct SatisfiedTupleImpl<Ev, std::tuple<Argv...>>
            : std::bool_constant<std::is_same_v<
                  std::tuple<Argv...>,
                  typename SignatureLookupImpl<
                      Ev,
                      typename SignalRegistry<decltype(Ev)>::manifest_t
                  >::signature_t
              >>
        {
        };
    public:
        template<auto Ev>
        using signature_for_t =
            typename SignatureLookupImpl<Ev, typename SignalRegistry<decltype(Ev)>::manifest_t>::
                signature_t;

        template<auto Ev, typename... Argv>
        static constexpr bool satisfied = std::is_same_v<std::tuple<Argv...>, signature_for_t<Ev>>;

        template<auto Ev, typename Tuple>
        static constexpr bool satisfied_tuple = SatisfiedTupleImpl<Ev, Tuple>::value;

        template<auto Ev, typename... Argv> static consteval void ensure_satisfied()
        {
            static_assert(satisfied<Ev, Argv...>, "Invalid signal event signature");
        }
        template<auto Ev, typename Tuple> static consteval void ensure_tuple_satisfied()
        {
            static_assert(satisfied_tuple<Ev, Tuple>, "Invalid signal event signature");
        }
    };

    template<typename Type> struct SignalSpecifier
    {
        using value_t = std::decay_t<Type>;
        value_t value;
    };
    template<typename Type> struct DispatchSpecifier
    {
        using value_t = std::decay_t<Type>;
        value_t value;
    };

    template<typename Type> SignalSpecifier<std::decay_t<Type>> sigspec(Type&& value)
    {
        return SignalSpecifier<std::decay_t<Type>>{std::forward<Type>(value)};
    }
    template<typename Type> DispatchSpecifier<std::decay_t<Type>> disspec(Type&& value)
    {
        return DispatchSpecifier<std::decay_t<Type>>{std::forward<Type>(value)};
    }

    // So I guess I should probably start adding some comments
    // And so we start with this one
    // -------------------------------------------------------------
    // This is one handles event signals
    // It supports basically two classes of identifiers + three identifier types
    // We have signal identifiers and bucket identifiers
    // For the signal IDs we got:
    // 1. Event enum type
    // 2. <optional> Signal specifier type (runtime)
    // And for the buckets we have (these simply sort inside a specific signal):
    // 1. Event enum value (runtime or compile time)
    // 2. <optional> Event specifier (runtime)
    // We have RAII handles for Signal (returned by connect) + Handle (returned by listen)
    // The backend is type erased so in the future we will get easier scripting integration etc.
    // What I mean by that is that the Signals class is global and handles all signal types no
    // matter the signature
    // -------------------------------------------------------------
    // When it comes to declaration the user facing API is:
    // Event enum/any optional specifiers (provides actual events) +
    // SignalManifest (defines signatures, tied to the enum) +
    // SignalRegistry (keyed by the enum, points to the manifest)
    // The optional specifiers are passed through:
    // sigspec(value) or disspec(value)
    class Signals : public std::enable_shared_from_this<Signals>
    {
        D2_TAG_MODULE(sig)
    private:
        static constexpr auto _static_storage_length{16};
        static constexpr auto _max_concurrent_signals{8};

        using signal_id = std::type_index;
        using event_idx = std::uint64_t;
        using signature_verifier = bool (*)(event_idx, impl::RuntimeSignature);
    private:
        class SpecifierKey
        {
        private:
            static constexpr auto _specifier_storage_length{16};
            struct Ops
            {
                const void* type{nullptr};
                std::size_t size{0};
                std::size_t alignment{0};
                bool local{true};
                std::size_t (*hash)(const void*){nullptr};
                bool (*equal)(const void*, const void*){nullptr};
                void (*copy)(void*, const void*){nullptr};
                void (*move)(void*, void*){nullptr};
                void (*destroy)(void*){nullptr};
            };

            const Ops* _ops{nullptr};
            std::size_t _hash{0};
            union
            {
                alignas(
                    std::max_align_t
                ) std::array<unsigned char, _specifier_storage_length> _storage;
                void* _buffer;
            };

            template<typename Type> static const void* _type_tag() noexcept
            {
                static const int tag{};
                return &tag;
            }
            template<typename Type> static const Ops* _ops_for() noexcept
            {
                using type = std::decay_t<Type>;
                constexpr bool local = sizeof(type) <= _specifier_storage_length &&
                                       alignof(type) <= alignof(std::max_align_t) &&
                                       std::is_nothrow_move_constructible_v<type>;
                static const Ops ops{
                    _type_tag<type>(),
                    sizeof(type),
                    alignof(type),
                    local,
                    +[](const void* ptr) -> std::size_t
                    {
                        const auto* value = reinterpret_cast<const type*>(ptr);
                        return absl::HashOf(_type_tag<type>(), *value);
                    },
                    +[](const void* lhs, const void* rhs) -> bool
                    {
                        return *reinterpret_cast<const type*>(lhs) ==
                               *reinterpret_cast<const type*>(rhs);
                    },
                    +[](void* dest, const void* src)
                    { new (dest) type(*reinterpret_cast<const type*>(src)); },
                    +[](void* dest, void* src)
                    {
                        new (dest) type(std::move(*reinterpret_cast<type*>(src)));
                        std::destroy_at(reinterpret_cast<type*>(src));
                    },
                    +[](void* ptr) { std::destroy_at(reinterpret_cast<type*>(ptr)); },
                };
                return &ops;
            }

            void* _ptr() noexcept;
            const void* _ptr() const noexcept;

            void _destroy() noexcept;

            void _copy_from(const SpecifierKey& copy);
            void _move_from(SpecifierKey&& copy) noexcept;
        public:
            SpecifierKey() noexcept = default;

            template<typename Type>
                requires(!std::is_same_v<std::decay_t<Type>, SpecifierKey>)
            explicit SpecifierKey(Type&& value)
            {
                using type = std::decay_t<Type>;
                static_assert(!std::is_void_v<type>, "Signal specifier cannot be void");
                static_assert(
                    std::is_copy_constructible_v<type>,
                    "Signal specifier must be copy constructible"
                );
                static_assert(
                    std::is_move_constructible_v<type>,
                    "Signal specifier must be move constructible"
                );

                const auto* ops = _ops_for<type>();
                const auto hash = ops->hash(std::addressof(value));

                if (ops->local)
                {
                    new (_storage.data()) type(std::forward<Type>(value));
                }
                else
                {
                    _buffer = ::operator new(ops->size, std::align_val_t{ops->alignment});
                    try
                    {
                        new (_buffer) type(std::forward<Type>(value));
                    }
                    catch (...)
                    {
                        ::operator delete(_buffer, std::align_val_t{ops->alignment});
                        throw;
                    }
                }

                _ops = ops;
                _hash = hash;
            }

            SpecifierKey(const SpecifierKey& copy);
            SpecifierKey(SpecifierKey&& copy) noexcept;
            ~SpecifierKey();

            SpecifierKey& operator=(const SpecifierKey& copy);
            SpecifierKey& operator=(SpecifierKey&& copy) noexcept;

            bool empty() const noexcept;
            std::size_t hash() const noexcept;

            bool operator==(const SpecifierKey& other) const;
            bool operator!=(const SpecifierKey& other) const;
        };

        struct SpecifierKeyHash
        {
            std::size_t operator()(const SpecifierKey& key) const noexcept;
        };
        struct SpecifierKeyEqual
        {
            bool operator()(const SpecifierKey& lhs, const SpecifierKey& rhs) const;
        };

        struct SignalKey
        {
            signal_id id{typeid(void)};
            SpecifierKey specifier{};

            SignalKey() = default;
            SignalKey(signal_id id, SpecifierKey specifier) :
                id(id), specifier(std::move(specifier))
            {
            }
        };
        struct SignalKeyHash
        {
            std::size_t operator()(const SignalKey& key) const noexcept;
        };
        struct SignalKeyEqual
        {
            bool operator()(const SignalKey& lhs, const SignalKey& rhs) const;
        };

        struct SignalInstance;
        struct HandleState
        {
            enum class State
            {
                Active,
                Inactive,
                Muted,
            };
            std::atomic<State> state{State::Active};
            std::function<void(SignalInstance&)> callback{nullptr};
            event_idx event{~0ull};
            SpecifierKey dispatch{};

            HandleState(std::function<void(SignalInstance&)> callback);
        };

        struct SignalInstance
        {
        private:
            enum class Action
            {
                Destroy,
                Ptr,
                Move,
                Apply,
            };
        private:
            void* (*_manager)(SignalInstance*, SignalInstance*, void*, void*, Action){nullptr};
            union
            {
                std::array<unsigned char, _static_storage_length> _static_storage;
                unsigned char* _buffer;
            };

            void _move_from(SignalInstance&& copy);
        public:
            template<typename... Argv> SignalInstance(Argv&&... pack)
            {
                using func = void (*)(void*, typename impl::args_sig_normalize<Argv>::type...);
                using type = std::tuple<Argv...>;
                constexpr auto alignment = alignof(type);
                constexpr auto total = sizeof(type);

                auto aligned = static_cast<void*>(_static_storage.data());
                auto aligned_size = _static_storage.size();
                if (const auto ptr = std::align(alignment, total, aligned, aligned_size); ptr)
                {
                    new (ptr) type{std::forward<Argv>(pack)...};
                    _manager = +[](SignalInstance* ptr,
                                   SignalInstance* dest,
                                   void* callback,
                                   void* block,
                                   Action action) -> void*
                    {
                        auto aligned = static_cast<void*>(ptr->_static_storage.data());
                        auto aligned_size = ptr->_static_storage.size();
                        const auto data = std::align(alignment, total, aligned, aligned_size);

                        if (action == Action::Destroy)
                        {
                            std::destroy_at(reinterpret_cast<type*>(data));
                        }
                        else if (action == Action::Ptr)
                        {
                            return data;
                        }
                        else if (action == Action::Move)
                        {
                            auto* dest_ptr = reinterpret_cast<type*>(
                                ptr->_manager(dest, nullptr, nullptr, nullptr, Action::Ptr)
                            );
                            new (dest_ptr) type{std::move(*reinterpret_cast<type*>(data))};
                            ptr->_manager(ptr, nullptr, nullptr, nullptr, Action::Destroy);
                            dest->_manager = ptr->_manager;
                            ptr->_manager = nullptr;
                        }
                        else if (action == Action::Apply)
                        {
                            auto* args = reinterpret_cast<type*>(data);
                            auto* cb = *reinterpret_cast<func*>(callback);
                            std::apply([&](auto&... args) { cb(block, args...); }, *args);
                        }

                        return nullptr;
                    };
                }
                else
                {
                    _buffer = static_cast<unsigned char*>(
                        ::operator new(total, std::align_val_t{alignment})
                    );
                    new (_buffer) type{std::forward<Argv>(pack)...};

                    _manager = +[](SignalInstance* ptr,
                                   SignalInstance* dest,
                                   void* callback,
                                   void* block,
                                   Action action) -> void*
                    {
                        if (action == Action::Destroy)
                        {
                            std::destroy_at(reinterpret_cast<type*>(ptr->_buffer));
                            ::operator delete(ptr->_buffer, std::align_val_t{alignof(type)});
                        }
                        else if (action == Action::Ptr)
                        {
                            return ptr->_buffer;
                        }
                        else if (action == Action::Move)
                        {
                            dest->_buffer = ptr->_buffer;
                            dest->_manager = ptr->_manager;
                            ptr->_manager = nullptr;
                        }
                        else if (action == Action::Apply)
                        {
                            auto* args = reinterpret_cast<type*>(ptr->_buffer);
                            auto* cb = *reinterpret_cast<func*>(callback);
                            std::apply([&](auto&... args) { cb(block, args...); }, *args);
                        }

                        return nullptr;
                    };
                }
            }

            SignalInstance(SignalInstance&& copy) noexcept;
            SignalInstance(const SignalInstance&) = delete;
            ~SignalInstance();

            template<typename Func, typename... Argv>
            void apply(void (*acceptor)(void*, Argv...), Func& callback)
            {
                _manager(
                    this,
                    nullptr,
                    reinterpret_cast<void*>(&acceptor),
                    reinterpret_cast<void*>(&callback),
                    Action::Apply
                );
            }

            SignalInstance& operator=(SignalInstance&& copy) noexcept;
            SignalInstance& operator=(const SignalInstance&) = delete;

            bool operator==(std::nullptr_t) const;
            bool operator!=(std::nullptr_t) const;
        };
        struct SignalEnvelope
        {
            event_idx event{~0ull};
            SpecifierKey dispatch{};
            SignalInstance instance{};

            SignalEnvelope() = default;
            SignalEnvelope(event_idx event, SpecifierKey dispatch, SignalInstance instance) :
                event(event), dispatch(std::move(dispatch)), instance(std::move(instance))
            {
            }
            SignalEnvelope(SignalEnvelope&&) noexcept = default;
            SignalEnvelope(const SignalEnvelope&) = delete;

            SignalEnvelope& operator=(SignalEnvelope&&) noexcept = default;
            SignalEnvelope& operator=(const SignalEnvelope&) = delete;
        };
        struct SignalStorage
        {
            using HandleList = std::vector<std::shared_ptr<HandleState>>;

            struct EventBucket
            {
                HandleList generic{};
                absl::flat_hash_map<SpecifierKey, HandleList, SpecifierKeyHash, SpecifierKeyEqual>
                    keyed{};
            };

            std::shared_mutex mtx{};
            mt::TaskRing<SignalEnvelope, _max_concurrent_signals> instances{};
            mt::TaskRing<SignalEnvelope, 1> combined{};
            absl::flat_hash_map<event_idx, EventBucket> handles{};
            signature_verifier verify{nullptr};
            std::atomic<bool> is_queued{false};
            unsigned char flags{0x00};
        };

        std::shared_mutex _mtx{};
        absl::
            flat_hash_map<SignalKey, std::shared_ptr<SignalStorage>, SignalKeyHash, SignalKeyEqual>
                _sigs{};
        mt::ConcurrentPool::ptr _pool{nullptr};
    public:
        enum SignalFlags : unsigned char
        {
            Async = 1 << 0,
            Deferred = 1 << 1,
            Combine = 1 << 3,
            DropIfFull = 1 << 4,
        };

        struct Handle
        {
        public:
            using State = HandleState::State;
        private:
            std::shared_ptr<SignalStorage> _storage{};
            std::shared_ptr<HandleState> _state{nullptr};
        public:
            Handle() = default;
            Handle(const Handle&) = delete;
            Handle(Handle&& other) noexcept;
            Handle(std::shared_ptr<HandleState> ptr, std::shared_ptr<SignalStorage> storage);
            Handle(std::nullptr_t) {}
            ~Handle();

            void mute();
            void unmute();
            void release();
            void close();
            State state() const;

            Handle& operator=(const Handle&) = delete;
            Handle& operator=(Handle&& other) noexcept;

            bool operator==(std::nullptr_t) const;
            bool operator!=(std::nullptr_t) const;
        };
        struct Signal
        {
        private:
            std::weak_ptr<Signals> _state{};
            std::shared_ptr<SignalStorage> _storage{};
            SignalKey _id{};
        public:
            Signal() = default;
            Signal(
                std::weak_ptr<Signals> ptr, std::shared_ptr<SignalStorage> storage, SignalKey id
            );
            Signal(std::nullptr_t) {}
            Signal(const Signal&) = delete;
            Signal(Signal&& other) noexcept;
            ~Signal();

            void disconnect();

            Signal& operator=(const Signal&) = delete;
            Signal& operator=(Signal&& other) noexcept;

            bool operator==(std::nullptr_t) const;
            bool operator!=(std::nullptr_t) const;
        };
        struct Config
        {
            unsigned char flags;

            constexpr Config(unsigned char flags = 0x00) noexcept : flags(flags) {}
        };
        using ptr = std::shared_ptr<Signals>;
    private:
        template<typename Sig> signal_id _sig_id() const noexcept
        {
            return signal_id(typeid(Sig));
        }
        template<typename... Argv>
        static bool _verify_single_signature(event_idx, impl::RuntimeSignature signature) noexcept
        {
            using normalized_t = typename impl::args_normalize<std::tuple<Argv...>>::type;
            return signature == impl::runtime_signature_for_tuple<normalized_t>::get();
        }
        template<typename... Argv> static signature_verifier _single_signature_verifier() noexcept
        {
            return &_verify_single_signature<Argv...>;
        }
        template<typename Func, typename... Argv>
        static std::function<void(SignalInstance&)>
        _make_listener_callback(Func&& callback, std::tuple<Argv...>*)
        {
            using callback_t = std::decay_t<Func>;
            return
                [callback = callback_t(std::forward<Func>(callback))](SignalInstance& args) mutable
            {
                args.apply(
                    +[](void* ptr, Argv... args)
                    {
                        std::invoke(
                            *reinterpret_cast<callback_t*>(ptr), std::forward<Argv>(args)...
                        );
                    },
                    callback
                );
            };
        }

        Handle _sig_listen(
            signal_id id,
            SpecifierKey signal_specifier,
            event_idx ev,
            SpecifierKey dispatch_specifier,
            impl::RuntimeSignature signature,
            std::function<void(SignalInstance&)> callback
        );
        Handle _sig_listen_unchecked(
            signal_id id,
            SpecifierKey signal_specifier,
            event_idx ev,
            SpecifierKey dispatch_specifier,
            std::function<void(SignalInstance&)> callback
        );

        void _sig_apply(
            SignalStorage& storage,
            event_idx ev,
            const SpecifierKey& dispatch_specifier,
            SignalInstance& args
        );
        void _sig_apply_all(SignalStorage& storage);

        bool _sig_pending(const SignalStorage& storage) const;
        void _sig_drain_queued(std::shared_ptr<SignalStorage> storage);
        void _sig_schedule_drain(std::shared_ptr<SignalStorage> storage);

        void _sig_trigger(
            signal_id id,
            SpecifierKey signal_specifier,
            event_idx ev,
            SpecifierKey dispatch_specifier,
            impl::RuntimeSignature signature,
            SignalInstance sig
        );
        void _sig_trigger_unchecked(
            signal_id id,
            SpecifierKey signal_specifier,
            event_idx ev,
            SpecifierKey dispatch_specifier,
            SignalInstance sig
        );

        Signal _sig_register(
            signal_id id,
            SpecifierKey signal_specifier,
            signature_verifier verifier,
            unsigned char flags
        );
        void _sig_deregister(signal_id id, SpecifierKey signal_specifier);
    public:
        static ptr make(mt::ConcurrentPool::ptr pool = nullptr);

        Signals(mt::ConcurrentPool::ptr pool);
        Signals(const Signals&) = delete;
        Signals(Signals&&) = delete;

        template<typename Manifest> Signal connect(Config cfg = Config{})
        {
            using event_t = Manifest::event_t;
            return _sig_register(_sig_id<event_t>(), SpecifierKey{}, &Manifest::verify, cfg.flags);
        }
        template<typename Manifest, typename SigSpec>
        Signal connect(SignalSpecifier<SigSpec> specifier, Config cfg = Config{})
        {
            using event_t = Manifest::event_t;
            return _sig_register(
                _sig_id<event_t>(),
                SpecifierKey(std::move(specifier.value)),
                &Manifest::verify,
                cfg.flags
            );
        }
        template<typename Manifest> void disconnect()
        {
            using event_t = Manifest::event_t;
            _sig_deregister(_sig_id<event_t>(), SpecifierKey{});
        }
        template<typename Manifest, typename SigSpec>
        void disconnect(SignalSpecifier<SigSpec> specifier)
        {
            using event_t = Manifest::event_t;
            _sig_deregister(_sig_id<event_t>(), SpecifierKey(std::move(specifier.value)));
        }

        template<auto Ev, typename Func> Handle listen(Func&& callback)
        {
            using event_t = decltype(Ev);
            static_assert(std::is_enum_v<event_t>, "Signal must be an enum");

            using func_info = impl::func_info<std::decay_t<Func>>;
            using listener_args_t = typename impl::args_normalize<typename func_info::args_t>::type;
            SignalRegistry<void>::template ensure_tuple_satisfied<Ev, listener_args_t>();

            return _sig_listen_unchecked(
                _sig_id<event_t>(),
                SpecifierKey{},
                event_idx(Ev),
                SpecifierKey{},
                _make_listener_callback(
                    std::forward<Func>(callback), static_cast<listener_args_t*>(nullptr)
                )
            );
        }
        template<auto Ev, typename SigSpec, typename Func>
        Handle listen(SignalSpecifier<SigSpec> signal_specifier, Func&& callback)
        {
            using event_t = decltype(Ev);
            static_assert(std::is_enum_v<event_t>, "Signal must be an enum");

            using func_info = impl::func_info<std::decay_t<Func>>;
            using listener_args_t = typename impl::args_normalize<typename func_info::args_t>::type;
            SignalRegistry<void>::template ensure_tuple_satisfied<Ev, listener_args_t>();

            return _sig_listen_unchecked(
                _sig_id<event_t>(),
                SpecifierKey(std::move(signal_specifier.value)),
                event_idx(Ev),
                SpecifierKey{},
                _make_listener_callback(
                    std::forward<Func>(callback), static_cast<listener_args_t*>(nullptr)
                )
            );
        }
        template<auto Ev, typename DispatchSpec, typename Func>
        Handle listen(DispatchSpecifier<DispatchSpec> dispatch_specifier, Func&& callback)
        {
            using event_t = decltype(Ev);
            static_assert(std::is_enum_v<event_t>, "Signal must be an enum");

            using func_info = impl::func_info<std::decay_t<Func>>;
            using listener_args_t = typename impl::args_normalize<typename func_info::args_t>::type;
            SignalRegistry<void>::template ensure_tuple_satisfied<Ev, listener_args_t>();

            return _sig_listen_unchecked(
                _sig_id<event_t>(),
                SpecifierKey{},
                event_idx(Ev),
                SpecifierKey(std::move(dispatch_specifier.value)),
                _make_listener_callback(
                    std::forward<Func>(callback), static_cast<listener_args_t*>(nullptr)
                )
            );
        }
        template<auto Ev, typename SigSpec, typename DispatchSpec, typename Func>
        Handle listen(
            SignalSpecifier<SigSpec> signal_specifier,
            DispatchSpecifier<DispatchSpec> dispatch_specifier,
            Func&& callback
        )
        {
            using event_t = decltype(Ev);
            static_assert(std::is_enum_v<event_t>, "Signal must be an enum");

            using func_info = impl::func_info<std::decay_t<Func>>;
            using listener_args_t = typename impl::args_normalize<typename func_info::args_t>::type;
            SignalRegistry<void>::template ensure_tuple_satisfied<Ev, listener_args_t>();

            return _sig_listen_unchecked(
                _sig_id<event_t>(),
                SpecifierKey(std::move(signal_specifier.value)),
                event_idx(Ev),
                SpecifierKey(std::move(dispatch_specifier.value)),
                _make_listener_callback(
                    std::forward<Func>(callback), static_cast<listener_args_t*>(nullptr)
                )
            );
        }
        template<typename Event, typename Func> Handle listen(Event ev, Func&& callback)
        {
            static_assert(std::is_enum_v<Event>, "Signal must be an enum");

            using func_info = impl::func_info<std::decay_t<Func>>;
            using listener_args_t = typename impl::args_normalize<typename func_info::args_t>::type;

            return _sig_listen(
                _sig_id<Event>(),
                SpecifierKey{},
                event_idx(ev),
                SpecifierKey{},
                impl::runtime_signature_for_tuple<listener_args_t>::get(),
                _make_listener_callback(
                    std::forward<Func>(callback), static_cast<listener_args_t*>(nullptr)
                )
            );
        }
        template<typename Event, typename SigSpec, typename Func>
        Handle listen(Event ev, SignalSpecifier<SigSpec> signal_specifier, Func&& callback)
        {
            static_assert(std::is_enum_v<Event>, "Signal must be an enum");

            using func_info = impl::func_info<std::decay_t<Func>>;
            using listener_args_t = typename impl::args_normalize<typename func_info::args_t>::type;

            return _sig_listen(
                _sig_id<Event>(),
                SpecifierKey(std::move(signal_specifier.value)),
                event_idx(ev),
                SpecifierKey{},
                impl::runtime_signature_for_tuple<listener_args_t>::get(),
                _make_listener_callback(
                    std::forward<Func>(callback), static_cast<listener_args_t*>(nullptr)
                )
            );
        }
        template<typename Event, typename DispatchSpec, typename Func>
        Handle listen(Event ev, DispatchSpecifier<DispatchSpec> dispatch_specifier, Func&& callback)
        {
            static_assert(std::is_enum_v<Event>, "Signal must be an enum");

            using func_info = impl::func_info<std::decay_t<Func>>;
            using listener_args_t = typename impl::args_normalize<typename func_info::args_t>::type;

            return _sig_listen(
                _sig_id<Event>(),
                SpecifierKey{},
                event_idx(ev),
                SpecifierKey(std::move(dispatch_specifier.value)),
                impl::runtime_signature_for_tuple<listener_args_t>::get(),
                _make_listener_callback(
                    std::forward<Func>(callback), static_cast<listener_args_t*>(nullptr)
                )
            );
        }
        template<typename Event, typename SigSpec, typename DispatchSpec, typename Func>
        Handle listen(
            Event ev,
            SignalSpecifier<SigSpec> signal_specifier,
            DispatchSpecifier<DispatchSpec> dispatch_specifier,
            Func&& callback
        )
        {
            static_assert(std::is_enum_v<Event>, "Signal must be an enum");

            using func_info = impl::func_info<std::decay_t<Func>>;
            using listener_args_t = typename impl::args_normalize<typename func_info::args_t>::type;

            return _sig_listen(
                _sig_id<Event>(),
                SpecifierKey(std::move(signal_specifier.value)),
                event_idx(ev),
                SpecifierKey(std::move(dispatch_specifier.value)),
                impl::runtime_signature_for_tuple<listener_args_t>::get(),
                _make_listener_callback(
                    std::forward<Func>(callback), static_cast<listener_args_t*>(nullptr)
                )
            );
        }

        template<auto Ev, typename... Argv> void signal(Argv&&... args)
        {
            using event_t = decltype(Ev);
            static_assert(std::is_enum_v<event_t>, "Signal must be an enum");

            using emitted_args_t = std::tuple<typename impl::args_sig_normalize<Argv>::type...>;
            SignalRegistry<void>::template ensure_tuple_satisfied<Ev, emitted_args_t>();

            _sig_trigger_unchecked(
                _sig_id<event_t>(),
                SpecifierKey{},
                event_idx(Ev),
                SpecifierKey{},
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }
        template<auto Ev, typename SigSpec, typename... Argv>
        void signal(SignalSpecifier<SigSpec> signal_specifier, Argv&&... args)
        {
            using event_t = decltype(Ev);
            static_assert(std::is_enum_v<event_t>, "Signal must be an enum");

            using emitted_args_t = std::tuple<typename impl::args_sig_normalize<Argv>::type...>;
            SignalRegistry<void>::template ensure_tuple_satisfied<Ev, emitted_args_t>();

            _sig_trigger_unchecked(
                _sig_id<event_t>(),
                SpecifierKey(std::move(signal_specifier.value)),
                event_idx(Ev),
                SpecifierKey{},
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }
        template<auto Ev, typename DispatchSpec, typename... Argv>
        void signal(DispatchSpecifier<DispatchSpec> dispatch_specifier, Argv&&... args)
        {
            using event_t = decltype(Ev);
            static_assert(std::is_enum_v<event_t>, "Signal must be an enum");

            using emitted_args_t = std::tuple<typename impl::args_sig_normalize<Argv>::type...>;
            SignalRegistry<void>::template ensure_tuple_satisfied<Ev, emitted_args_t>();

            _sig_trigger_unchecked(
                _sig_id<event_t>(),
                SpecifierKey{},
                event_idx(Ev),
                SpecifierKey(std::move(dispatch_specifier.value)),
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }
        template<auto Ev, typename SigSpec, typename DispatchSpec, typename... Argv>
        void signal(
            SignalSpecifier<SigSpec> signal_specifier,
            DispatchSpecifier<DispatchSpec> dispatch_specifier,
            Argv&&... args
        )
        {
            using event_t = decltype(Ev);
            static_assert(std::is_enum_v<event_t>, "Signal must be an enum");

            using emitted_args_t = std::tuple<typename impl::args_sig_normalize<Argv>::type...>;
            SignalRegistry<void>::template ensure_tuple_satisfied<Ev, emitted_args_t>();

            _sig_trigger_unchecked(
                _sig_id<event_t>(),
                SpecifierKey(std::move(signal_specifier.value)),
                event_idx(Ev),
                SpecifierKey(std::move(dispatch_specifier.value)),
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }
        template<typename Event, typename... Argv> void signal(Event ev, Argv&&... args)
        {
            static_assert(std::is_enum_v<Event>, "Signal must be an enum");

            using emitted_args_t = std::tuple<typename impl::args_sig_normalize<Argv>::type...>;
            _sig_trigger(
                _sig_id<Event>(),
                SpecifierKey{},
                event_idx(ev),
                SpecifierKey{},
                impl::runtime_signature_for_tuple<emitted_args_t>::get(),
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }
        template<typename Event, typename SigSpec, typename... Argv>
        void signal(Event ev, SignalSpecifier<SigSpec> signal_specifier, Argv&&... args)
        {
            static_assert(std::is_enum_v<Event>, "Signal must be an enum");

            using emitted_args_t = std::tuple<typename impl::args_sig_normalize<Argv>::type...>;
            _sig_trigger(
                _sig_id<Event>(),
                SpecifierKey(std::move(signal_specifier.value)),
                event_idx(ev),
                SpecifierKey{},
                impl::runtime_signature_for_tuple<emitted_args_t>::get(),
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }
        template<typename Event, typename DispatchSpec, typename... Argv>
        void signal(Event ev, DispatchSpecifier<DispatchSpec> dispatch_specifier, Argv&&... args)
        {
            static_assert(std::is_enum_v<Event>, "Signal must be an enum");

            using emitted_args_t = std::tuple<typename impl::args_sig_normalize<Argv>::type...>;
            _sig_trigger(
                _sig_id<Event>(),
                SpecifierKey{},
                event_idx(ev),
                SpecifierKey(std::move(dispatch_specifier.value)),
                impl::runtime_signature_for_tuple<emitted_args_t>::get(),
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }
        template<typename Event, typename SigSpec, typename DispatchSpec, typename... Argv>
        void signal(
            Event ev,
            SignalSpecifier<SigSpec> signal_specifier,
            DispatchSpecifier<DispatchSpec> dispatch_specifier,
            Argv&&... args
        )
        {
            static_assert(std::is_enum_v<Event>, "Signal must be an enum");

            using emitted_args_t = std::tuple<typename impl::args_sig_normalize<Argv>::type...>;
            _sig_trigger(
                _sig_id<Event>(),
                SpecifierKey(std::move(signal_specifier.value)),
                event_idx(ev),
                SpecifierKey(std::move(dispatch_specifier.value)),
                impl::runtime_signature_for_tuple<emitted_args_t>::get(),
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }

        Signals& operator=(const Signals&) = delete;
        Signals& operator=(Signals&&) = delete;
    };
} // namespace d2
