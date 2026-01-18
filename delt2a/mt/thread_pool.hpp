#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "task_ring.hpp"
#include "thread_future.hpp"
#include <functional>
#include <memory>
#include <chrono>
#include <vector>
#include <thread>
#include <any>

namespace mt
{
    namespace impl
    {
        class Futex
        {
        private:
            std::atomic_flag _flag{ false };
        public:
            Futex();

            void release() noexcept;
            void wait() noexcept;
        };
    }

	class ThreadPool;

	class context
	{
	public:
		using pool = std::shared_ptr<ThreadPool>;
	private:
		static inline thread_local pool _pool_ptr{ nullptr };
	public:
		static pool get() noexcept
		{
			return _pool_ptr;
		}
		static void set(pool ptr) noexcept
		{
			_pool_ptr = ptr;
		}
	};

	class ThreadPool : public std::enable_shared_from_this<ThreadPool>
	{
	public:
        using wptr = std::weak_ptr<ThreadPool>;
        using ptr = std::shared_ptr<ThreadPool>;
        enum class Distribution
        {
            Random,
            Fast,
            Optimal
        };
		struct Config
		{
			enum Flags : unsigned char
			{
				AutoShrink = 1 << 0,
				AutoGrow = 1 << 1,
                // All cyclic workers will be pushed onto the main thread
                MainCyclicWorker = 1 << 2,
				// Discards new tasks (return nullptr) if average tasks exceed pressure trigger ratio
                ManageOverload = 1 << 3,
                // <on overflow> Forces the thread pool to throw an exception
                OverflowGuardThrow = 1 << 4,
                // <on overflow> Forces the task to run even on an unsupported thread
                OverflowGuardRun = 1 << 5,
                // <on overflow> Discards the tasks
                OverflowGuardDiscard = 1 << 6,
			};
            unsigned char flags{ AutoShrink | AutoGrow | OverflowGuardRun };
            float growth_trigger_ratio{ 3.5f };
			float pressure_trigger_ratio{ 10.f };
            Distribution default_dist{ Distribution::Fast };
            std::size_t min_threads{ 1 };
            std::size_t max_threads{ std::thread::hardware_concurrency() };
			std::chrono::seconds max_idle_time{ 10 };
            std::function<std::size_t(std::size_t, std::size_t)> growth_callback{
                [](std::size_t cnt, std::size_t) {
                    return cnt > 0 ? cnt * 2 - 1 : 1;
                }
            };
			std::function<void(std::size_t)> event_grow{ nullptr };
			std::function<void(std::size_t)> event_shrink{ nullptr };
			std::function<void(std::size_t)> event_pressure{ nullptr };
		};
        class Worker
        {
        public:
            enum Flags
            {
                HandleTask = 1 << 0,
                HandleCyclicTask = 1 << 1,
                HandleDeferredTask = 1 << 2,
                MainWorker = 1 << 3,
            };
        private:
            wptr _ptr{};
            unsigned char _flags{ 0x00 };
            std::size_t _tid{ ~0ull };
        public:
            Worker() = default;
            Worker(std::nullptr_t) {}
            Worker(Worker&&) = default;
            Worker(const Worker&) = delete;
            Worker(wptr ptr, unsigned char flags = HandleTask | HandleCyclicTask);
            ~Worker();

            bool start() noexcept;
            void stop() noexcept;
            void wait(std::chrono::milliseconds max) const noexcept;
            void wait() const noexcept;
            void ping() const noexcept;
            bool active() const noexcept;

            Worker& operator=(Worker&&) = default;
            Worker& operator=(const Worker&) = delete;
        };
    private:
        static constexpr auto _cyclic_epsilon = std::chrono::milliseconds(10);
        static constexpr auto _thread_backlog = 16ull;
    private:
		struct Thread;
		struct Task
		{
			enum class Token
			{
				Continue,
				Discard,
			};
            enum class Query
            {
                Type,
                Task,
            };
            enum Type : unsigned char
            {
                Static = 1 << 0,
                Cyclic = 1 << 1,
                Deferred = 1 << 2,
                System = 1 << 3,
                Force = 1 << 4,
            };

            static constexpr auto oneoff = std::chrono::milliseconds::max();
            std::function<Token(Query, std::any&)> callback{};
            std::chrono::milliseconds timing{ oneoff };
		};
		struct CyclicTask : Task
		{
			std::chrono::steady_clock::time_point last{};

			bool ready() const noexcept
			{
                return std::chrono::steady_clock::now() - last >= timing - _cyclic_epsilon;
			}
			void update() noexcept
			{
				last = std::chrono::steady_clock::now();
			}
            std::chrono::milliseconds left() const noexcept
			{
                return std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - last
                );
			}
		};
		struct Thread
		{    
            TaskRing<Task, _thread_backlog> tasks{};
			std::size_t id{};
			std::vector<CyclicTask> cyclic_tasks{};
            std::chrono::milliseconds cyclic_wait{ std::chrono::milliseconds::max() };
			std::chrono::steady_clock::time_point last_task{};
			std::jthread handle{};
            std::atomic<std::size_t> task_cnt{ 0 };
            std::atomic<unsigned char> task_support{ 0x00 };
			std::atomic<bool> stop{ true };
		};
		struct Node : std::enable_shared_from_this<Node>
		{
			using ptr = std::shared_ptr<Node>;

            Node(std::size_t cnt, Distribution dist)
                : threads(cnt), dist(dist) {}

			std::vector<Thread> threads{};
			std::atomic<std::size_t> thread_scnt{ 0 };
            std::atomic<Distribution> dist{ Distribution::Fast };
			alignas (std::hardware_destructive_interference_size) std::atomic<std::size_t> thread_cnt{ 0 };
			alignas (std::hardware_destructive_interference_size) std::atomic<std::size_t> cyclic_task_cnt{ 0 };
            alignas (std::hardware_destructive_interference_size) std::atomic<std::size_t> task_cnt{ 0 };
			alignas (std::hardware_destructive_interference_size) std::atomic<std::size_t> thread_ctr{ 0 };
		};
	private:
		const Config _cfg{};

		std::vector<std::shared_ptr<Node>> _nodes{};

		// Node management

		void _node_shrink(std::size_t cnt, Node& node) noexcept;
		void _node_shutdown(Node& node) noexcept;
		void _node_shutdown_sync() noexcept;

		// Random distribution

		std::size_t _node_count() const noexcept;
		std::size_t _node_id() const noexcept;

        std::size_t _distribute_random(unsigned char support) noexcept;
        std::size_t _distribute_fast(unsigned char support) noexcept;
        std::size_t _distribute_optimal(unsigned char support) noexcept;
        std::size_t _distribute(Distribution algo, unsigned char support) noexcept;

        void _schedule(Task task, std::optional<Distribution> dist) noexcept;
		bool _check_overload() noexcept;

		// Thread handlers

		bool _worker_handle_task(Task& task, Node& node, Thread& state) noexcept;
        void _worker_wait_task(Node& node, Thread& state, std::chrono::milliseconds wait = std::chrono::milliseconds::max()) noexcept;

        void _worker_main(std::size_t id, wptr ptr, std::chrono::milliseconds wait = std::chrono::milliseconds::max()) noexcept;
        void _worker_normal(std::size_t id, wptr ptr, std::chrono::milliseconds wait = std::chrono::milliseconds::max()) noexcept;
        void _worker_close(Node::ptr node, Thread* state) noexcept;
        void _worker_manage(Node::ptr node, Thread* state) noexcept;

		// Helpers

        template<typename Type>
        Type _query_task(Task::Query query, const Task& task)
        {
            std::any out;
            out.emplace<Type>();
            task.callback(query, out);
            return std::any_cast<Type&>(out);
        }

		template<typename Func, typename... Argv>
		void _event(Func&& func, Argv&&... args) noexcept
		{
			try
			{
				if (func)
					func(std::forward<Argv>(args)...);
			}
			catch (...) {}
		}

		template<typename Func>
        void _launch_worker(std::size_t nid, Func&& func, std::size_t cnt = 1, unsigned char support = Task::Cyclic | Task::Static | Task::Deferred)
		{
			auto& node = _nodes[nid];
			while (cnt--)
			{
                for (std::size_t i = node->thread_cnt; i < _cfg.max_threads; i++)
				{
					if (bool expected = true;
						node->threads[i].stop.compare_exchange_strong(expected, false))
					{
						node->thread_cnt++;
						const auto id = i;
						auto& state = node->threads[id];
                        state.task_support = support | (i == 0 ? Task::System : 0x00);
						state.id = id;
						state.last_task = std::chrono::steady_clock::now();
                        state.handle = std::jthread(
                            std::forward<Func>(func),
                            this,
                            id,
                            weak_from_this(),
                            std::chrono::milliseconds::max()
                        );
						break;
					}
				}
			}
			_event(_cfg.event_grow, node->thread_cnt);
		}
        void _launch_worker_auto(std::size_t nid, std::size_t cnt = 1, unsigned char support = Task::Cyclic | Task::Static | Task::Deferred);

        template<typename Future, typename Ret>
        static consteval void _future_check()
        {
            static_assert(std::is_same_v<Ret, typename Future::ret>, "Future does not match function return type");
        }
    public:
		static ptr make(Config cfg);
		static ptr make();

        explicit ThreadPool(Config cfg);
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;

		std::size_t threads(std::size_t node) const noexcept;
		std::size_t threads() const noexcept;

		std::size_t tasks(std::size_t node) const noexcept;
		std::size_t tasks() const noexcept;

        Distribution distribution(std::size_t node) const noexcept;
        Distribution distribution() const noexcept;

        void set_distribution(std::size_t node, Distribution dist) const noexcept;
        void set_distribution(Distribution dist) const noexcept;

        void start() noexcept;
		void stop() noexcept;
		void bind() noexcept;
		void resize(std::size_t size) noexcept;
		void resize(std::size_t node, std::size_t size) noexcept;

        Worker worker(unsigned char flags = Worker::Flags::HandleCyclicTask | Worker::Flags::HandleTask | Worker::Flags::HandleDeferredTask) noexcept;

        // Overrides

        // The type of the future will have to match the passed function's return type
        template<typename Future>
        struct TaskConfig
        {
            using future_type = Future;
            std::optional<Distribution> distribution{ std::nullopt };
            Future future{ nullptr };
        };

        template<typename Future, typename Func, typename... Argv>
        auto launchd(Func&& callback, TaskConfig<Future> cfg, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, Argv...>;
            _future_check<Future, ret>();

            if (_check_overload())
                return Future(nullptr);
            const auto task = cfg.future == nullptr ? Future::make() : cfg.future;

            _schedule({
                .callback = [
                    block = std::weak_ptr(task.block()),
                    callback = std::forward<Func>(callback),
                    ...args = std::forward<Argv>(args)
                ](Task::Query query, std::any& out) mutable -> Task::Token {
                    if (query == Task::Query::Type)
                    {
                        std::any_cast<unsigned char&>(out) = Task::Type::Static;
                        return Task::Token::Continue;
                    }
                    else if (query == Task::Query::Task)
                    {
                        const auto ptr = block.lock();
                        if (!(ptr && ptr->is_discarded()))
                        {
                            try
                            {
                                if (ptr)
                                {
                                    if constexpr (std::is_same_v<void, ret>)
                                    {
                                        callback(std::forward<Argv>(args)...);
                                        ptr->set_value();
                                    }
                                    else
                                        ptr->set_value(callback(std::forward<Argv>(args)...));
                                }
                                else
                                {
                                    callback(std::forward<Argv>(args)...);
                                }
                            }
                            catch (...)
                            {
                                if (ptr)
                                    ptr->set_exception(std::current_exception());
                            }
                        }
                    }
                    return Task::Token::Discard;
                }
            }, cfg.distribution);
            return task;
        }

        template<typename Func, typename... Argv>
        void launchd_void(Func&& callback, std::optional<Distribution> dist, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, Argv...>;

            if (_check_overload())
                return;

            _schedule({
                .callback = [
                    callback = std::forward<Func>(callback),
                    ...args = std::forward<Argv>(args)
                ](Task::Query query, std::any& out) mutable -> Task::Token {
                    if (query == Task::Query::Type)
                    {
                        std::any_cast<unsigned char&>(out) = Task::Type::Static;
                        return Task::Token::Continue;
                    }
                    else if (query == Task::Query::Task)
                    {
                        try { callback(std::forward<Argv>(args)...); }
                        catch (...) {}
                    }
                    return Task::Token::Discard;
                }
            }, dist);
        }

        template<typename Future, typename Func, typename... Argv>
        auto launchd_cyclic(std::chrono::milliseconds time, Func&& callback, TaskConfig<Future> cfg, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, std::nullptr_t, Argv...>;
            _future_check<Future, ret>();

            if (_check_overload())
                return Future(nullptr);
            const auto task = cfg.future == nullptr ? Future::make() : cfg.future;

            _schedule({
                .callback = [
                    block = task.block(),
                    callback = std::forward<Func>(callback),
                    ...args = std::forward<Argv>(args)
                ](Task::Query query, std::any& out) mutable -> Task::Token {
                    if (query == Task::Query::Type)
                    {
                        std::any_cast<unsigned char&>(out) = Task::Type::Cyclic;
                        return Task::Token::Continue;
                    }
                    else if (query == Task::Query::Task)
                    {
                        const auto fut = future<ret>(block);
                        if (!block->is_discarded() && !block->is_paused())
                        {
                            try
                            {
                                if constexpr (std::is_same_v<void, ret>)
                                {
                                    callback(fut, args...);
                                    block->set_tick();
                                }
                                else
                                    block->set_tick(callback(fut, args...));
                            }
                            catch (...)
                            {
                                block->set_tick_exception(std::current_exception());
                            }
                        }
                        return block->is_discarded() ? Task::Token::Discard : Task::Token::Continue;
                    }
                    return Task::Token::Discard;
                },
                .timing = time
            }, cfg.distribution);
            return task;
        }

        template<typename Future, typename Func, typename... Argv>
        auto launchd_deferred(std::chrono::milliseconds time, Func&& callback, TaskConfig<Future> cfg, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, Argv...>;
            _future_check<Future, ret>();

            if (_check_overload())
                return Future(nullptr);
            const auto task = cfg.future == nullptr ? Future::make() : cfg.future;

            _schedule({
                .callback = [
                    block = std::weak_ptr(task.block()),
                    callback = std::forward<Func>(callback),
                    ...args = std::forward<Argv>(args)
                ](Task::Query query, std::any& out) mutable -> Task::Token {
                    if (query == Task::Query::Type)
                    {
                        std::any_cast<unsigned char&>(out) = Task::Type::Deferred;
                        return Task::Token::Continue;
                    }
                    else if (query == Task::Query::Task)
                    {
                        const auto ptr = block.lock();
                        if (!(ptr && ptr->is_discarded()))
                        {
                            try
                            {
                                if (ptr)
                                {
                                    if (ptr->is_paused()) return Task::Token::Continue;
                                    if constexpr (std::is_same_v<void, ret>)
                                    {
                                        callback(std::forward<Argv>(args)...);
                                        ptr->set_value();
                                    }
                                    else
                                        ptr->set_value(callback(std::forward<Argv>(args)...));
                                }
                                else
                                {
                                    callback(std::forward<Argv>(args)...);
                                }
                            }
                            catch (...)
                            {
                                if (ptr)
                                    ptr->set_exception(std::current_exception());
                            }
                        }
                    }
                    return Task::Token::Discard;
                },
                .timing = time
            }, cfg.distribution);

            return task;
        }

        template<typename Func, typename... Argv>
        auto co_launchd(Func&& callback, std::optional<Distribution> dist, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, Argv...>;

            if (_check_overload())
                return co_future<ret>(nullptr);
            const auto task = co_future<ret>::make();

            _schedule({
                .callback = [
                    block = std::weak_ptr(task.block()),
                    callback = std::forward<Func>(callback),
                    ...args = std::forward<Argv>(args)
                ](Task::Query query, std::any& out) mutable -> Task::Token {
                    if (query == Task::Query::Type)
                    {
                        std::any_cast<unsigned char&>(out) = Task::Type::Static;
                        return Task::Token::Continue;
                    }
                    else if (query == Task::Query::Task)
                    {
                        const auto ptr = block.lock();
                        if (!(ptr && ptr->is_discarded()))
                        {
                            try
                            {
                                if (ptr)
                                {
                                    if constexpr (std::is_same_v<void, ret>)
                                    {
                                        callback(std::forward<Argv>(args)...);
                                        ptr->set_value();
                                    }
                                    else
                                        ptr->set_value(callback(std::forward<Argv>(args)...));
                                }
                                else
                                {
                                    callback(std::forward<Argv>(args)...);
                                }
                            }
                            catch (...)
                            {
                                if (ptr)
                                    ptr->set_exception(std::current_exception());
                            }
                        }
                    }
                    return Task::Token::Discard;
                }
            }, dist);

            return task;
        }

        // Defaults

        template<typename Func, typename... Argv>
        auto launch(Func&& callback, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, Argv...>;
            return launchd(
                std::forward<Func>(callback),
                TaskConfig<future<ret>>(),
                std::forward<Argv>(args)...
            );
        }

        template<typename Func, typename... Argv>
        void launch_void(Func&& callback, Argv&&... args)
        {
            launchd_void(
                std::forward<Func>(callback),
                std::nullopt,
                std::forward<Argv>(args)...
            );
        }

        template<typename Func, typename... Argv>
        auto launch_cyclic(std::chrono::milliseconds time, Func&& callback, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, std::nullptr_t, Argv...>;
            return launchd_cyclic(
                time,
                std::forward<Func>(callback),
                TaskConfig<future<ret>>(),
                std::forward<Argv>(args)...
            );
        }

        template<typename Func, typename... Argv>
        auto launch_deferred(std::chrono::milliseconds time, Func&& callback, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, Argv...>;
            return launchd_deferred(
                time,
                std::forward<Func>(callback),
                TaskConfig<future<ret>>(),
                std::forward<Argv>(args)...
            );
        }

        template<typename Func, typename... Argv>
        auto co_launch(Func&& callback, Argv&&... args)
        {
            return co_launchd(
                std::forward<Func>(callback),
                std::nullopt,
                std::forward<Argv>(args)...
            );
        }

		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;
	};
}

#endif // THREAD_POOL_HPP
