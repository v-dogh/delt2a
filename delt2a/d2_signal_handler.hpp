#ifndef D2_SIGNAL_HANDLER_HPP
#define D2_SIGNAL_HANDLER_HPP

#include <absl/container/flat_hash_map.h>
#include <shared_mutex>
#include <typeindex>
#include <vector>
#include <atomic>
#include <tuple>
#include <typeinfo>
#include <memory>
#include <type_traits>
#include "mt/thread_pool.hpp"
#include "mt/task_ring.hpp"

namespace d2
{
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
            std::size_t _alignment{ 0 };
            std::size_t _length{ 0 };
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
                using type = std::tuple<Argv...>;
                constexpr auto alignment = alignof(type);
                const auto total = (sizeof(Argv) + ... + 0);

                auto aligned = static_cast<void*>(_static_storage.data());
                auto aligned_size = _static_storage.size();
                if (const auto ptr = std::align(
                        alignment,
                        total,
                        aligned,
                        aligned_size
                    ); ptr)
                {
                    const auto diff = static_cast<unsigned char*>(ptr) - _static_storage.data();
                    _alignment = diff;
                    new (ptr) type{ std::make_tuple(std::forward<Argv>(pack)...) };
                }
                else
                {
                    _buffer = static_cast<unsigned char*>(
                        ::operator new(total, std::align_val_t{ alignment })
                    );
                    new (_buffer) type{ std::make_tuple(std::forward<Argv>(pack)...) };
                }
                _length = total;
            }
            SignalInstance(SignalInstance&& copy);
            SignalInstance(const SignalInstance&) = delete;
            ~SignalInstance()
            {
                if (_length > _static_storage_length)
                    delete[] _buffer;
            }

            template<typename Tuple>
            const Tuple& arguments_tuple() const
            {
                if (_length + _alignment > _static_storage_length)
                    return *reinterpret_cast<const Tuple*>(_buffer + _alignment);
                return *reinterpret_cast<const Tuple*>(_static_storage.data() + _alignment);
            }
            template<typename... Argv>
            const std::tuple<Argv...>& arguments() const
            {
                return arguments_tuple<std::tuple<Argv...>>();
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
        constexpr signal_id _sig_id()
        {
            return typeid(Sig).hash_code();
        }
        template<typename... Argv>
        constexpr args_code _sig_args_code(std::tuple<Argv...>)
        {
            using tup = decltype(std::tuple{ std::type_index(typeid(Argv))... });
            return absl::Hash<tup>()(
                std::tuple{ std::type_index(typeid(Argv))... }
            );
        }
        template<typename... Argv>
        constexpr args_code _sig_args_code()
        {
            using tup = decltype(std::tuple{ std::type_index(typeid(Argv))... });
            return absl::Hash<tup>()(
                std::tuple{ std::type_index(typeid(Argv))... }
            );
        }

        Handle _sig_listen(signal_id id, event_idx ev, args_code code, std::function<void(SignalInstance&)> callback);
        void _sig_apply(SignalStorage& storage, event_idx ev, SignalInstance& args);
        void _sig_apply_all(SignalStorage& storage, event_idx ev);
        void _sig_trigger(signal_id id, event_idx ev, args_code code, SignalInstance sig);

        Signal _sig_register(signal_id id, args_code code, std::size_t size, unsigned char flags);
        void _sig_deregister(signal_id id);
    public:
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
                _sig_args_code<Argv...>(),
                (sizeof(Argv) + ... + 0),
                flags
            );
        }
        template<typename Signal>
        void disconnect()
        {
            static_assert(std::is_enum_v<Signal>, "Signal must be an enum");
            _sig_deregister(_sig_id<Signal>());
        }

        template<typename Signal, typename Func>
        Handle listen(Signal sig, Func&& callback)
        {
            static_assert(std::is_enum_v<Signal>, "Signal must be an enum");
            using func_info = impl::func_info<Func>;
            return _sig_listen(
                _sig_id<Signal>(),
                event_idx(sig),
                _sig_args_code(typename func_info::args_t()),
                [callback = std::move(callback)](SignalInstance& args) {
                    std::apply([&](const auto&... args) {
                        callback(args...);
                    }, args.arguments_tuple<typename func_info::args_t>());
                }
            );
        }

        template<typename Signal, typename... Argv>
        void signal(Signal sig, Argv&&... args)
        {
            static_assert(std::is_enum_v<Signal>, "Signal must be an enum");
            _sig_trigger(
                _sig_id<Signal>(),
                event_idx(sig),
                _sig_args_code<Argv...>(),
                SignalInstance(std::forward<Argv>(args)...)
            );
        }

        Signals& operator=(const Signals&) = delete;
        Signals& operator=(Signals&&) = delete;
    };
}

#endif // D2_SIGNAL_HANDLER_HPP
