#ifndef THREAD_CONTROL_HPP
#define THREAD_CONTROL_HPP

#include <coroutine>
#include <memory>
#include <variant>
#include <atomic>

namespace mt
{
	template<typename Type>
	class ControlBlock : public std::enable_shared_from_this<ControlBlock<Type>>
	{
	protected:
		enum Flags : unsigned char
		{
			// Set when the task returns a value
			Updated = 1 << 0,
			// Set when the task is finished
			Discarded = 1 << 1,
			// Set when an exception occured
			Exception = 1 << 2,
		};
	private:
		std::variant<
			std::conditional_t<
				std::is_same_v<Type, void>,
				std::uint8_t, Type
			>,
			std::exception_ptr
		> _val{};
		std::atomic<unsigned char> _flags{ 0x00 };
	protected:
		template<typename... Argv>
		void _set_value(Argv&&... args)
		{
			if constexpr (!std::is_same_v<void, Type>)
				_val.template emplace<0>(std::forward<Argv>(args)...);
		}
		void _sync_flags(unsigned char flags) const noexcept
		{
			auto value = _flags.load();
			while (!(value & flags))
			{
				_flags.wait(value);
				value = _flags.load();
			}
		}
		void _sync_flags(unsigned char flags, unsigned char rem) noexcept
		{
			auto value = _flags.load();
			while (!(value & flags))
			{
				_flags.wait(value);
				value = _flags.load();
			}
			_flags &= rem;
		}
        void _sum_flags(unsigned char flags) noexcept
        {
            _flags |= flags;
            _flags.notify_all();
        }
    public:
		ControlBlock() = default;
		ControlBlock(const ControlBlock&) = delete;
		ControlBlock(ControlBlock&&) = delete;

		template<typename... Argv>
		void set_tick(Argv&&... args) noexcept
		{
			if constexpr (!std::is_same_v<void, Type>)
				_val.template emplace<0>(std::forward<Argv>(args)...);
            _sum_flags(Updated);
		}
        void set_tick_exception(std::exception_ptr ex) noexcept
        {
            _val = ex;
            _sum_flags(Exception);
        }
		template<typename... Argv>
		void set_value(Argv&&... args) noexcept
		{
			if constexpr (!std::is_same_v<void, Type>)
				_val.template emplace<0>(std::forward<Argv>(args)...);
            _sum_flags(Updated | Discarded);
		}
		void set_exception(std::exception_ptr ex) noexcept
		{
			_val = ex;
            _sum_flags(Discarded | Exception);
		}
		void set_exception() noexcept
		{
			_val = std::current_exception();
            _sum_flags(Discarded | Exception);
		}

		void sync_value() const noexcept
		{
            _sync_flags(Updated | Exception | Discarded);
		}
		void sync() const noexcept
		{
			_sync_flags(Discarded);
		}
		void discard() noexcept
		{
            _sum_flags(Discarded);
		}
		Type value()
		{
            _sync_flags(Updated | Exception, Updated);
			rethrow();
			if constexpr (!std::is_same_v<Type, void>)
				return std::move(std::get<0>(_val));
		}

		bool is_discarded() const noexcept
		{
			return _flags & Discarded;
		}
		bool has_value() const noexcept
		{
			return _flags & Updated;
		}
		bool has_exception() const noexcept
		{
			return _flags & Exception;
		}

		std::exception_ptr exception() const noexcept
		{
			return std::get<1>(_val);
		}
		void rethrow() const
		{
			if (has_exception())
				throw std::get<1>(_val);
		}

		bool try_set_handle(std::coroutine_handle<> handle) noexcept
		{
			return false;
		}
		bool has_handle()
		{
			return false;
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
        std::array<void(*)(State&..., std::shared_ptr<ManagedControlBlock>), 3> _hooks{};

        void _trigger(Event ev) noexcept
        {
            if (const auto hook = _hooks[std::size_t(ev)])
            {
                try
                {
                    std::apply([&](auto&... args) {
                        hook(
                            args...,
                            std::static_pointer_cast<ManagedControlBlock>(
                                base::shared_from_this()
                            )
                        );
                    }, _state);
                }
                catch (...)
                {
                    base::set_exception(
                        std::current_exception()
                    );
                }
            }
        }
    public:
        using ControlBlock<Type>::ControlBlock;
        template<typename... Argv>
        ManagedControlBlock(Argv&&... args) :
            _state(std::forward<Argv>(args)...)
        {
            static_assert(sizeof...(args) == sizeof...(State) || sizeof...(args) == 0);
        }
        ~ManagedControlBlock()
        {
            _trigger(Event::Destroyed);
        }

        template<typename... Argv>
        void set_tick(Argv&&... args) noexcept
        {
            base::set_tick(std::forward<Argv>(args)...);
            _trigger(Event::Updated);
        }
        void set_tick_exception(std::exception_ptr ex) noexcept
        {
            base::set_tick_exception(ex);
            _trigger(Event::Exception);
        }
        template<typename... Argv>
        void set_value(Argv&&... args) noexcept
        {
            base::set_value(std::forward<Argv>(args)...);
            _trigger(Event::Updated);
            _trigger(Event::Discarded);
        }
        void set_exception(std::exception_ptr ex) noexcept
        {
            base::set_exception(ex);
            _trigger(Event::Exception);
            _trigger(Event::Discarded);
        }
        void set_exception() noexcept
        {
            base::set_exception();
            _trigger(Event::Exception);
            _trigger(Event::Discarded);
        }
    };

	template<typename Type>
	class CoroutineControlBlock : public ControlBlock<Type>
	{
	private:
		using base = ControlBlock<Type>;
	private:
		std::atomic<std::coroutine_handle<>> _handle{};

		void _try_run_handle() noexcept
		{
			if (auto handle = _handle.exchange(nullptr);
				handle != nullptr)
				handle.resume();
		}
	public:
		using ControlBlock<Type>::ControlBlock;

		~CoroutineControlBlock()
		{
			if (auto handle = _handle.exchange(nullptr);
				handle != nullptr)
				handle.destroy();
		}

		template<typename... Argv>
		void set_value(Argv&&... args) noexcept
		{
			base::set_value(std::forward<Argv>(args)...);
			_try_run_handle();
		}
		void set_exception(std::exception_ptr ex) noexcept
		{
			base::set_exception(ex);
			_try_run_handle();
		}
		void set_exception() noexcept
		{
			base::set_exception();
			_try_run_handle();
		}

		bool try_set_handle(std::coroutine_handle<> handle) noexcept
		{
			std::coroutine_handle<> dummy = nullptr;
			return _handle.compare_exchange_strong(dummy, handle);
		}
		bool has_handle()
		{
			return _handle != nullptr;
		}
	};
}

#endif // THREAD_CONTROL_HPP
