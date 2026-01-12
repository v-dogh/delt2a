#ifndef D2_SIGNAL_HANDLER_HPP
#define D2_SIGNAL_HANDLER_HPP

#include <absl/container/flat_hash_map.h>
#include <shared_mutex>
#include <typeindex>
#include <vector>
#include <atomic>
#include <tuple>
#include <memory>
#include <type_traits>
#include "mt/thread_pool.hpp"
#include "mt/task_ring.hpp"

namespace d2
{
    template<typename Type>
    class Ref
    {
        static_assert(!std::is_reference_v<Type>, "Ref<Type> cannot store a reference Type");
        Type* _value;
    public:
        explicit constexpr Ref(Type& value) noexcept : _value(std::addressof(value)) {}
        constexpr Type& get() const noexcept { return *_value; }

        constexpr operator Type&() const noexcept { return *_value; }

        constexpr Type* operator&() const noexcept { return _value; }
        constexpr Type* operator->() const noexcept { return _value; }
        constexpr Type& operator*()  const noexcept { return *_value; }
    };

    template<typename Type>
    Ref<Type> ref(Type& value)
    {
        return Ref<Type>(value);
    }
    template<typename Type>
    Ref<const Type> cref(const Type& value)
    {
        return Ref<const Type>(value);
    }

    namespace impl
    {
        template<typename>
        struct func_info;

        template<typename Ret, typename... Argv>
        struct func_info<Ret(Argv...)>
        {
            using ret_t = Ret;
            using args_t = std::tuple<Argv...>;
        };

        template<typename Ret, typename... Argv>
        struct func_info<Ret(*)(Argv...)> : func_info<Ret(Argv...)> {};

        template<typename State, typename Ret, typename... Argv>
        struct func_info<Ret(State::*)(Argv...) const> : func_info<Ret(Argv...)> {};

        template<typename State, typename Ret, typename... Argv>
        struct func_info<Ret(State::*)(Argv...)> : func_info<Ret(Argv...)> {};

        template<typename Func>
        struct func_info : func_info<decltype(&std::decay_t<Func>::operator())> {};

        template<typename Type>
        struct normalize { using type = std::decay_t<Type>; };

        template<typename Type>
        struct normalize<Type&> { using type = Ref<Type>; };

        template<typename... Argv>
        struct normalize<std::tuple<Argv...>> { using type = std::tuple<typename normalize<Argv>::type...>; };

        template<typename Type>
        struct args_normalize { using type = const Type&; };

        template<typename Type>
        struct args_normalize<Type&> { using type = Type&; };

        template<typename... Argv>
        struct args_normalize<std::tuple<Argv...>> { using type = std::tuple<typename args_normalize<Argv>::type...>; };

        template<typename Type>
        struct args_sig_normalize { using type = const Type&; };

        template<typename Type>
        struct args_sig_normalize<Ref<Type>> { using type = Type&; };

        template<typename... Argv>
        struct args_sig_normalize<std::tuple<Argv...>> { using type = std::tuple<typename args_sig_normalize<Argv>::type...>; };
    }

    class Signals : public std::enable_shared_from_this<Signals>
    {
    private:
        static constexpr auto _static_storage_length{ 16 };
        static constexpr auto _max_concurrent_signals{ 8 };

        using signal_id = std::uint64_t;
        using event_idx = std::uint64_t;
        using args_code = std::uint64_t;
    private:
        struct SignalInstance;
        struct HandleState
        {
            enum class State
            {
                Active,
                Inactive,
                Muted,
            };
            std::atomic<State> state{ State::Active };
            std::function<void(SignalInstance&)> callback{ nullptr };
            event_idx event{ ~0ull };

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
            void*(*_manager)(SignalInstance*, SignalInstance*, void*, void*, Action){ nullptr };
            union
            {
                std::array<unsigned char, _static_storage_length> _static_storage;
                unsigned char* _buffer;
            };

            void _move_from(SignalInstance&& copy);
        public:
            template<typename... Argv>
            SignalInstance(Argv&&... pack)
            {
                using func = void(*)(void*, typename impl::args_sig_normalize<Argv>::type...);
                using type = std::tuple<Argv...>;
                constexpr auto alignment = alignof(type);
                constexpr auto total = sizeof(type);

                auto aligned = static_cast<void*>(_static_storage.data());
                auto aligned_size = _static_storage.size();
                if (const auto ptr = std::align(
                        alignment,
                        total,
                        aligned,
                        aligned_size
                    ); ptr)
                {
                    new (ptr) type{ std::forward<Argv>(pack)... };
                    _manager = +[](SignalInstance* ptr, SignalInstance* dest, void* callback, void* block, Action action) -> void* {
                        auto aligned = static_cast<void*>(ptr->_static_storage.data());
                        auto aligned_size = ptr->_static_storage.size();
                        const auto data = std::align(
                            alignment,
                            total,
                            aligned,
                            aligned_size
                        );
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
                            auto* dest_ptr = reinterpret_cast<type*>(ptr->_manager(
                                dest,
                                nullptr,
                                nullptr,
                                nullptr,
                                Action::Ptr
                            ));
                            new (dest_ptr) type{std::move(
                                *reinterpret_cast<type*>(data)
                            )};
                            ptr->_manager(ptr, nullptr, nullptr, nullptr, Action::Destroy);
                            dest->_manager = ptr->_manager;
                            ptr->_manager = nullptr;
                        }
                        else if (action == Action::Apply)
                        {
                            auto* args = reinterpret_cast<type*>(data);
                            auto* cb = *reinterpret_cast<func*>(callback);
                            std::apply([&](auto&... args) {
                                cb(block, args...);
                            }, *args);
                        }
                        return nullptr;
                    };
                }
                else
                {
                    _buffer = static_cast<unsigned char*>(
                        ::operator new(total, std::align_val_t{ alignment })
                    );
                    new (_buffer) type{ std::forward<Argv>(pack)... };
                    _manager = +[](SignalInstance* ptr, SignalInstance* dest, void* callback, void* block, Action action) -> void* {
                        if (action == Action::Destroy)
                        {
                            std::destroy_at(reinterpret_cast<type*>(ptr->_buffer));
                            ::operator delete(
                                ptr->_buffer,
                                std::align_val_t{ alignof(type) }
                            );
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
                            std::apply([&](auto&... args) {
                                cb(block, args...);
                            }, *args);
                        }
                        return nullptr;
                    };
                }
            }
            SignalInstance(SignalInstance&& copy);
            SignalInstance(const SignalInstance&) = delete;
            ~SignalInstance()
            {
                if (_manager != nullptr)
                    _manager(this, nullptr, nullptr, nullptr, Action::Destroy);
            }

            template<typename Func, typename... Argv>
            void apply(void(*acceptor)(void*, Argv...), Func& callback)
            {
                _manager(
                    this,
                    nullptr,
                    reinterpret_cast<void*>(&acceptor),
                    reinterpret_cast<void*>(&callback),
                    Action::Apply
                );
            }

            SignalInstance& operator=(SignalInstance&& copy);
            SignalInstance& operator=(const SignalInstance&) = delete;

            bool operator==(std::nullptr_t) const;
            bool operator!=(std::nullptr_t) const;
        };
        struct SignalStorage
        {
            std::shared_mutex mtx{};
            mt::TaskRing<SignalInstance, _max_concurrent_signals> instances{};
            mt::TaskRing<SignalInstance, 1> combined{};
            absl::flat_hash_map<event_idx, std::vector<std::weak_ptr<HandleState>>> handles{};
            std::size_t packed_size{ 0 };
            args_code argument_code{ 0 };
            std::atomic<bool> is_queued{ false };
            unsigned char flags{ 0x00 };
        };

        std::shared_mutex _mtx{};
        absl::flat_hash_map<signal_id, std::unique_ptr<SignalStorage>> _sigs{};
        mt::ThreadPool::ptr _pool{ nullptr };
    public:
        enum SignalFlags : unsigned char
        {
            Async = 1 << 0,
            Deferred = 1 << 1,
            Combine = 1 << 2,
            DropIfFull = 1 << 3,
        };
        struct Handle
        {
        public:
            using State = HandleState::State;
        private:
            std::shared_ptr<HandleState> _state{ nullptr };
        public:
            Handle() = default;
            Handle(const Handle&) = default;
            Handle(Handle&&) = default;
            Handle(std::shared_ptr<HandleState> ptr);
            Handle(std::nullptr_t) {}

            void mute();
            void unmute();
            void close();
            State state() const;

            Handle& operator=(const Handle&) = default;
            Handle& operator=(Handle&&) = default;

            bool operator==(std::nullptr_t) const;
            bool operator!=(std::nullptr_t) const;
        };
        struct Signal
        {
        private:
            std::weak_ptr<Signals> _state;
            signal_id _id{ 0 };
        public:
            Signal() = default;
            Signal(std::weak_ptr<Signals> ptr, signal_id id);
            Signal(std::nullptr_t) {}

            void disconnect();

            Signal& operator=(const Signal&) = default;
            Signal& operator=(Signal&&) = default;

            bool operator==(std::nullptr_t) const;
            bool operator!=(std::nullptr_t) const;
        };
        using ptr = std::shared_ptr<Signals>;
    private:
        template<typename Sig>
        constexpr signal_id _sig_id(std::size_t runtime)
        {
            return absl::HashOf(
                absl::Hash<std::type_index>()(typeid(Sig)),
                runtime
            );
        }
        template<typename Sig>
        constexpr signal_id _sig_id()
        {
            return absl::Hash<std::type_index>()(typeid(Sig));
        }

        template<typename... Argv>
        struct args_sig
        {
            static constexpr args_code code()
            {
                using tup = decltype(std::tuple{ std::type_index(typeid(Argv))... });
                return absl::Hash<tup>()(
                    std::tuple{ std::type_index(typeid(Argv))... }
                );
            }
        };
        template<typename... Argv>
        struct args_sig<std::tuple<Argv...>>
        {
            static constexpr args_code code()
            {
                using tup = decltype(std::tuple{ std::type_index(typeid(Argv))... });
                return absl::Hash<tup>()(
                    std::tuple{ std::type_index(typeid(Argv))... }
                );
            }
        };

        Handle _sig_listen(signal_id id, event_idx ev, args_code code, std::function<void(SignalInstance&)> callback);
        void _sig_apply(SignalStorage& storage, event_idx ev, SignalInstance& args);
        void _sig_apply_all(SignalStorage& storage, event_idx ev);
        void _sig_trigger(signal_id id, event_idx ev, args_code code, SignalInstance sig);

        Signal _sig_register(signal_id id, args_code code, std::size_t size, unsigned char flags);
        void _sig_deregister(signal_id id);
    public:
        template<typename... Argv>
        static signal_id id(Argv&&... args)
        {
            return absl::HashOf(std::forward<Argv>(args)...);
        }
        static ptr make(mt::ThreadPool::ptr pool = nullptr);

        Signals(mt::ThreadPool::ptr pool);
        Signals(const Signals&) = delete;
        Signals(Signals&&) = delete;

        template<typename Sig, typename... Argv>
        Signal connect(unsigned char flags = 0x00)
        {
            static_assert(std::is_enum_v<Sig>, "Signal must be an enum");
            return _sig_register(
                _sig_id<Sig>(),
                args_sig<typename impl::args_normalize<Argv>::type...>::code(),
                sizeof(std::tuple<typename impl::normalize<Argv>::type...>),
                flags
            );
        }
        template<typename Sig, typename... Argv>
        Signal connect(signal_id runtime, unsigned char flags = 0x00)
        {
            static_assert(std::is_enum_v<Sig>, "Signal must be an enum");
            return _sig_register(
                _sig_id<Sig>(runtime),
                args_sig<typename impl::args_normalize<Argv>::type...>::code(),
                sizeof(std::tuple<typename impl::normalize<Argv>::type...>),
                flags
            );
        }

        template<typename Signal>
        void disconnect()
        {
            static_assert(std::is_enum_v<Signal>, "Signal must be an enum");
            _sig_deregister(_sig_id<Signal>());
        }
        template<typename Signal>
        void disconnect(signal_id runtime)
        {
            static_assert(std::is_enum_v<Signal>, "Signal must be an enum");
            _sig_deregister(_sig_id<Signal>(runtime));
        }

        template<typename Signal, typename Func>
        Handle listen(Signal sig, Func&& callback)
        {
            static_assert(std::is_enum_v<Signal>, "Signal must be an enum");
            using func_info = impl::func_info<Func>;
            using norm_args = typename impl::args_normalize<typename func_info::args_t>::type;
            return _sig_listen(
                _sig_id<Signal>(),
                event_idx(sig),
                args_sig<norm_args>::code(),
                [&]<typename... Argv>(std::tuple<Argv...>*)
                {
                    return [callback = std::move(callback)](SignalInstance& args) mutable {
                        args.apply(+[](void* ptr, Argv... args) {
                            (*reinterpret_cast<Func*>(ptr))(args...);
                        }, callback);
                    };
                }.template operator()(static_cast<norm_args*>(nullptr))
            );
        }
        template<typename Signal, typename Func>
        Handle listen(Signal sig, signal_id runtime, Func&& callback)
        {
            static_assert(std::is_enum_v<Signal>, "Signal must be an enum");
            using func_info = impl::func_info<Func>;
            using unorm_args = func_info::args_t;
            using norm_args = typename impl::normalize<unorm_args>::type;
            return _sig_listen(
                _sig_id<Signal>(runtime),
                event_idx(sig),
                args_sig<unorm_args>::code(),
                [&]<typename... Argv>(std::tuple<Argv...>*)
                {
                    return [callback = std::move(callback)](SignalInstance& args) mutable {
                        args.apply(+[](void* ptr, typename impl::args_normalize<Argv>::type... args) {
                            (*reinterpret_cast<Func*>(ptr))(args...);
                        }, callback);
                    };
                }.template operator()(static_cast<unorm_args*>(nullptr))
            );
        }

        template<typename Signal, typename... Argv>
        void signal(Signal sig, Argv&&... args)
        {
            static_assert(std::is_enum_v<Signal>, "Signal must be an enum");
            _sig_trigger(
                _sig_id<Signal>(),
                event_idx(sig),
                args_sig<typename impl::args_sig_normalize<Argv>::type...>::code(),
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }
        template<typename Signal, typename... Argv>
        void signal(Signal sig, signal_id runtime, Argv&&... args)
        {
            static_assert(std::is_enum_v<Signal>, "Signal must be an enum");
            _sig_trigger(
                _sig_id<Signal>(runtime),
                event_idx(sig),
                args_sig<typename impl::args_sig_normalize<Argv>::type...>::code(),
                SignalInstance(typename impl::normalize<Argv>::type(std::forward<Argv>(args))...)
            );
        }

        Signals& operator=(const Signals&) = delete;
        Signals& operator=(Signals&&) = delete;
    };
}

#endif // D2_SIGNAL_HANDLER_HPP
