#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <semaphore>

namespace mt
{
    namespace impl
    {
        inline void yield() noexcept
        {
#if defined(_MSC_VER)
            _mm_pause();
#elif defined(__GNUG__)
            __builtin_ia32_pause();
#else
            std::this_thread::yield();
#endif
        }
    } // namespace impl

    template<typename Type, std::size_t Size> class TaskRing
    {
    private:
        static_assert((Size & (Size - 1)) == 0, "TaskRing size must be power of 2");

        static constexpr auto _mask = Size - 1;

        /* data     */ std::array<Type, Size> _ring{};
        /* consumer */ std::atomic<std::size_t> _tail{0};
        /* producer */ std::atomic<std::size_t> _head{0};
        /* producer */ std::atomic<std::size_t> _head_check{0};
        /* notify   */ std::counting_semaphore<Size> _available{0};

        bool _enqueue_impl(bool wait, Type& value) noexcept
        {
            std::size_t head = 0;
            while (true)
            {
                head = _head.load(std::memory_order::relaxed);
                if (head - _tail.load(std::memory_order::acquire) >= Size)
                {
                    if (wait)
                    {
                        impl::yield();
                        continue;
                    }
                    return false;
                }
                if (_head.compare_exchange_weak(
                        head, head + 1, std::memory_order::acq_rel, std::memory_order::relaxed
                    ))
                    break;
            }

            _ring[head & _mask] = Type{std::move(value)};
            while (_head_check.load(std::memory_order::acquire) != head)
                impl::yield();

            _head_check.store(head + 1, std::memory_order::release);
            _head_check.notify_one();
            _available.release();

            return true;
        }
        void _dequeue_ready(Type& value) noexcept
        {
            const auto tail = _tail.load(std::memory_order::relaxed);
            while (_head_check.load(std::memory_order::acquire) <= tail)
                _head_check.wait(tail, std::memory_order::acquire);
            value = std::move(_ring[tail & _mask]);
            _tail.store(tail + 1, std::memory_order::release);
        }
    public:
        TaskRing() = default;
        TaskRing(const TaskRing&) = delete;
        TaskRing(TaskRing&& copy) noexcept {}

        template<typename... Argv> void enqueue(Argv&&... args) noexcept
        {
            auto value = Type{std::forward<Argv>(args)...};
            _enqueue_impl(true, value);
        }
        template<typename... Argv> bool try_enqueue(Argv&&... args) noexcept
        {
            auto value = Type{std::forward<Argv>(args)...};
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
            if (!_available.try_acquire())
                return false;
            _dequeue_ready(value);
            return true;
        }
        bool dequeue(Type& value) noexcept
        {
            _available.acquire();
            _dequeue_ready(value);
            return true;
        }
        bool dequeue(Type& value, std::chrono::microseconds max) noexcept
        {
            if (!_available.try_acquire_for(max))
                return false;
            _dequeue_ready(value);
            return true;
        }

        bool empty() const noexcept
        {
            const auto tail = _tail.load(std::memory_order::acquire);
            const auto head = _head_check.load(std::memory_order::acquire);
            return head <= tail;
        }

        TaskRing& operator=(const TaskRing&) = delete;
        TaskRing& operator=(TaskRing&&) noexcept
        {
            return *this;
        }
    };
} // namespace mt
