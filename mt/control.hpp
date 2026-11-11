#pragma once

#include <array>
#include <atomic>
#include <coroutine>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mt
{
    struct TaskDiscarded : std::runtime_error
    {
        TaskDiscarded() : std::runtime_error("Task was discarded without a result") {}
    };

    template<typename Type>
    class ControlBlock : public std::enable_shared_from_this<ControlBlock<Type>>
    {
    protected:
        enum Flags : unsigned char
        {
            Updated = 1 << 0,
            Discarded = 1 << 1,
            Exception = 1 << 2,
            Paused = 1 << 3,
        };
    private:
        using value_type = std::conditional_t<std::is_void_v<Type>, unsigned char, Type>;

        mutable std::mutex _mtx{};
        std::optional<value_type> _val{};
        std::exception_ptr _exception{};
        std::atomic<unsigned char> _flags{0};
        std::coroutine_handle<> _handle{};

        template<typename Func> void _publish(unsigned char flags, Func&& write)
        {
            std::coroutine_handle<> handle;
            {
                std::lock_guard lock(_mtx);
                if (_flags.load() & Discarded)
                    return;
                write();
                auto state = _flags.load();
                _flags.store((state & Paused) | flags);
                handle = std::exchange(_handle, nullptr);
            }
            _flags.notify_all();
            if (handle)
                handle.resume();
        }
        void _sync_flags(unsigned char flags) const noexcept
        {
            auto state = _flags.load();
            while (!(state & flags))
            {
                _flags.wait(state);
                state = _flags.load();
            }
        }
    public:
        ControlBlock() = default;
        ControlBlock(const ControlBlock&) = delete;
        ControlBlock(ControlBlock&&) = delete;

        template<typename... Argv> void set_tick(Argv&&... args)
        {
            _publish(Updated, [&]
            {
                if constexpr (!std::is_void_v<Type>)
                    _val.emplace(std::forward<Argv>(args)...);
                else
                    _val.emplace(0);
                _exception = nullptr;
            });
        }
        void set_tick_exception(std::exception_ptr ex)
        {
            _publish(Exception, [&] { _exception = ex; _val.reset(); });
        }
        template<typename... Argv> void set_value(Argv&&... args)
        {
            _publish(Updated | Discarded, [&]
            {
                if constexpr (!std::is_void_v<Type>)
                    _val.emplace(std::forward<Argv>(args)...);
                else
                    _val.emplace(0);
                _exception = nullptr;
            });
        }
        void set_exception(std::exception_ptr ex)
        {
            _publish(Exception | Discarded, [&] { _exception = ex; _val.reset(); });
        }
        void set_exception()
        {
            set_exception(std::current_exception());
        }

        void pause() noexcept
        {
            std::lock_guard lock(_mtx);
            _flags.fetch_or(Paused);
            _flags.notify_all();
        }
        void unpause() noexcept
        {
            std::lock_guard lock(_mtx);
            _flags.fetch_and(~Paused);
            _flags.notify_all();
        }
        void sync_value() const noexcept
        {
            _sync_flags(Updated | Exception | Discarded);
        }
        void sync() const noexcept
        {
            _sync_flags(Discarded);
        }
        void sync_pause() const noexcept
        {
            _sync_flags(Paused | Discarded);
        }
        void sync_unpause() const noexcept
        {
            auto state = _flags.load();
            while ((state & Paused) && !(state & Discarded))
            {
                _flags.wait(state);
                state = _flags.load();
            }
        }
        void discard() noexcept
        {
            std::coroutine_handle<> handle;
            {
                std::lock_guard lock(_mtx);
                _flags.fetch_or(Discarded);
                handle = std::exchange(_handle, nullptr);
            }
            _flags.notify_all();
            if (handle)
                handle.resume();
        }
        Type value()
        {
            while (true)
            {
                sync_value();
                std::lock_guard lock(_mtx);
                const auto flags = _flags.load();
                if (flags & Exception)
                    std::rethrow_exception(_exception);
                if (flags & Updated)
                {
                    _flags.fetch_and(~Updated);
                    if constexpr (!std::is_void_v<Type>)
                    {
                        auto value = std::move(*_val);
                        _val.reset();
                        return value;
                    }
                    else
                    {
                        _val.reset();
                        return;
                    }
                }
                if (flags & Discarded)
                    throw TaskDiscarded();
            }
        }

        bool is_paused() const noexcept { return _flags.load() & Paused; }
        bool is_discarded() const noexcept { return _flags.load() & Discarded; }
        bool has_value() const noexcept { return _flags.load() & Updated; }
        bool has_exception() const noexcept { return _flags.load() & Exception; }
        bool is_ready() const noexcept { return _flags.load() & (Updated | Exception | Discarded); }

        std::exception_ptr exception() const noexcept
        {
            std::lock_guard lock(_mtx);
            return _exception;
        }
        void rethrow() const
        {
            if (auto ex = exception())
                std::rethrow_exception(ex);
        }
        bool try_set_handle(std::coroutine_handle<> handle)
        {
            std::lock_guard lock(_mtx);
            if (is_ready())
                return false;
            if (_handle)
                throw std::logic_error("Future already has an awaiting coroutine");
            _handle = handle;
            return true;
        }

        ControlBlock& operator=(const ControlBlock&) = delete;
        ControlBlock& operator=(ControlBlock&&) = delete;
    };

    template<typename Type, typename... State>
    class ManagedControlBlock : public ControlBlock<Type>
    {
    private:
        using base = ControlBlock<Type>;
    public:
        enum class Event
        {
            Updated,
            Exception,
            Discarded,
            Destroyed
        };
    private:
        std::tuple<State...> _state{};
        std::array<void(*)(ManagedControlBlock&, State&...), 4> _hooks{};

        void _trigger(Event ev) noexcept
        {
            if (const auto hook = _hooks[std::size_t(ev)])
            {
                try
                {
                    std::apply([&](auto&... args) { hook(*this, args...); }, _state);
                }
                catch (...)
                {
                    if (ev != Event::Destroyed)
                        base::set_exception(std::current_exception());
                }
            }
        }
    public:
        template<typename... Argv>
        ManagedControlBlock(Argv&&... args) : _state(std::forward<Argv>(args)...)
        {
            static_assert(sizeof...(args) == sizeof...(State) || sizeof...(args) == 0);
        }
        ~ManagedControlBlock() { _trigger(Event::Destroyed); }

        template<typename... Argv> void set_tick(Argv&&... args)
        {
            base::set_tick(std::forward<Argv>(args)...);
            _trigger(Event::Updated);
        }
        void set_tick_exception(std::exception_ptr ex)
        {
            base::set_tick_exception(ex);
            _trigger(Event::Exception);
        }
        template<typename... Argv> void set_value(Argv&&... args)
        {
            base::set_value(std::forward<Argv>(args)...);
            _trigger(Event::Updated);
            _trigger(Event::Discarded);
        }
        void set_exception(std::exception_ptr ex)
        {
            base::set_exception(ex);
            _trigger(Event::Exception);
            _trigger(Event::Discarded);
        }
        void set_exception() { set_exception(std::current_exception()); }
        void discard() noexcept
        {
            base::discard();
            _trigger(Event::Discarded);
        }
        template<typename Func> void set_hook(Event ev, Func&& callback)
        {
            _hooks.at(std::size_t(ev)) = std::forward<Func>(callback);
        }
    };

    template<typename Type> using CoroutineControlBlock = ControlBlock<Type>;
} // namespace mt
