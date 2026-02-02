#ifndef TASK_RING_HPP
#define TASK_RING_HPP

#include <atomic>
#include <array>
#include <chrono>
#include <semaphore>

namespace mt
{
	template<typename Type, std::size_t Size>
	class TaskRing
	{
	private:
		static_assert((Size & (Size - 1)) == 0, "TaskRing size must be power of 2");

		static constexpr auto _mask = Size - 1;

		// _tail stores the pointer to the current task that is next in line for execution
		// _head stores the pointer of the next free task slot
		// _head_check makes sure that the consumer does not grab a task that was reserved but not written to

        /* data     */ std::array<Type, Size>        _ring{};
        /* consumer */ std::atomic<std::size_t>      _tail{ 0 };
        /* producer */ std::atomic<std::size_t>      _head{ 0 };
        /* producer */ std::atomic<std::size_t>      _head_check{ 0 };
        /* notify   */ std::counting_semaphore<Size> _available{ 0 };

		static inline void _yield() noexcept
		{
#           if defined(_MSC_VER)
				_mm_pause();
#           elif defined(__GNUG__)
				__builtin_ia32_pause();
#           else
				std::this_thread::yield();
#           endif
		}

        bool _enqueue_impl(bool wait, Type& value) noexcept
		{
			std::size_t head = 0;
			while (true)
			{
				head = _head.load();
				if (head - _tail >= Size)
				{
					if (wait)
					{
						_yield();
						continue;
					}
					return false;
				}
				if (_head.compare_exchange_weak(head, head + 1))
					break;
			}
            _ring[head & _mask] = Type{ std::move(value) };
			_head_check.fetch_add(1);
			_head_check.notify_one();
			_available.release();
			return true;
		}
        std::optional<std::size_t> _dequeue_impl(Type& value) noexcept
        {
            std::size_t tail = _tail.load();
            if (const auto head = _head_check.load();
                head <= tail)
                return head;
            value = std::move(_ring[tail & _mask]);
            _tail.fetch_add(1);
            return std::nullopt;
        }
    public:
        TaskRing() = default;
        TaskRing(const TaskRing&) = delete;
        TaskRing(TaskRing&& copy) noexcept {}

        template<typename... Argv>
        void enqueue(Argv&&... args) noexcept
        {
            auto value = Type{ std::forward<Argv>(args)... };
            _enqueue_impl(true, value);
        }
        template<typename... Argv>
        bool try_enqueue(Argv&&... args) noexcept
        {
            auto value = Type{ std::forward<Argv>(args)... };
            return _enqueue_impl(false, value);
        }

        void enqueue_ref(Type& value) noexcept
        {
            _enqueue_impl(true, value);
        }
        bool try_enqueue_ref(Type& value) noexcept
        {
            return _enqueue_impl(false, value);
        }

        bool try_dequeue(Type& value) noexcept
        {
            if (!_dequeue_impl(value).has_value())
            {
                _available.acquire();
                return true;
            }
            return false;
        }
        bool dequeue(Type& value) noexcept
        {
            auto head = _dequeue_impl(value);
            while (head.has_value())
            {
                _head_check.wait(head.value());
                head = _dequeue_impl(value);
            }
            _available.acquire();
            return true;
        }
        bool dequeue(Type& value, std::chrono::microseconds max) noexcept
        {
            const auto beg = std::chrono::steady_clock::now();
            do
            {
                if (std::chrono::steady_clock::now() - beg > max ||
                    !_available.try_acquire_for(max))
                {
                    return false;
                }
            }
            while (_dequeue_impl(value).has_value());
            return true;
        }

		TaskRing& operator=(const TaskRing&) = delete;
		TaskRing& operator=(TaskRing&&) noexcept { return *this; }
	};
}

#endif // TASK_RING_HPP
