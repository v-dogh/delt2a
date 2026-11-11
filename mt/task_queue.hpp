#pragma once

#include <blockingconcurrentqueue.h>
#include <atomic>
#include <chrono>
#include <new>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>

namespace mt
{
    template<typename Type, bool Ordered = false> class TaskQueue
    {
    private:
        static_assert(std::is_nothrow_move_constructible_v<Type>);
        static_assert(std::is_nothrow_move_assignable_v<Type>);

        moodycamel::BlockingConcurrentQueue<std::optional<Type>> _queue{};
        std::atomic<std::size_t> _pending{0};
        struct OrderedProducer
        {
            std::mutex mtx{};
            std::optional<moodycamel::ProducerToken> token{};
        };
        struct UnorderedProducer {};
        [[no_unique_address]] std::conditional_t<Ordered, OrderedProducer, UnorderedProducer> _producer{};

        bool _enqueue(std::optional<Type>& item)
        {
            if constexpr (Ordered)
            {
                std::lock_guard lock(_producer.mtx);
                return _queue.enqueue(*_producer.token, std::move(item));
            }
            else
                return _queue.enqueue(std::move(item));
        }

        bool _take(std::optional<Type>& item, Type& value)
        {
            if (!item)
                return false;
            value = std::move(*item);
            _pending.fetch_sub(1, std::memory_order_acq_rel);
            return true;
        }
    public:
        TaskQueue()
        {
            if constexpr (Ordered)
                _producer.token.emplace(_queue);
        }
        TaskQueue(const TaskQueue&) = delete;
        TaskQueue(TaskQueue&&) = delete;

        template<typename... Argv> void enqueue(Argv&&... args)
        {
            std::optional<Type> item(std::in_place, std::forward<Argv>(args)...);
            _pending.fetch_add(1, std::memory_order_acq_rel);
            if (!_enqueue(item))
            {
                _pending.fetch_sub(1, std::memory_order_acq_rel);
                throw std::bad_alloc();
            }
        }
        bool try_enqueue(Type& value)
        {
            std::optional<Type> item(std::in_place, std::move(value));
            _pending.fetch_add(1, std::memory_order_acq_rel);
            if (!_enqueue(item))
            {
                _pending.fetch_sub(1, std::memory_order_acq_rel);
                value = std::move(*item);
                return false;
            }
            return true;
        }

        bool try_dequeue(Type& value)
        {
            std::optional<Type> item;
            return _queue.try_dequeue(item) && _take(item, value);
        }
        bool dequeue(Type& value)
        {
            std::optional<Type> item;
            _queue.wait_dequeue(item);
            return _take(item, value);
        }
        template<typename Rep, typename Period>
        bool dequeue(Type& value, std::chrono::duration<Rep, Period> max)
        {
            std::optional<Type> item;
            return _queue.wait_dequeue_timed(item, max) && _take(item, value);
        }
        void interrupt()
        {
            if (!_queue.enqueue(std::nullopt))
                throw std::bad_alloc();
        }
        bool empty() const noexcept
        {
            return _pending.load(std::memory_order_acquire) == 0;
        }

        TaskQueue& operator=(const TaskQueue&) = delete;
        TaskQueue& operator=(TaskQueue&&) = delete;
    };
} // namespace mt
