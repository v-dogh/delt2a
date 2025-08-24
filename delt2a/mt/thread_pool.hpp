#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <condition_variable>
#include <shared_mutex>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <list>

#ifndef MT_TP_USE_CUSTOM_FUNCTION
#include <functional>
#endif

namespace util::mt
{
	namespace impl
	{
		enum BlockState : unsigned char
		{
			// Set when the task reaches it's EOL
			Discard = 1 << 0,
			// Set when the value has been updated
			Finished = 1 << 1,
		};

		template<typename Type>
		class AsyncControlBlock;

		template<>
		class AsyncControlBlock<void> : public std::enable_shared_from_this<AsyncControlBlock<void>>
		{
		protected:
			mutable std::mutex m_mtx{};
			mutable std::condition_variable m_cv{};
			mutable std::exception_ptr m_ex{ nullptr };
			mutable unsigned char m_flags{ 0x00 };

			bool _valid() const noexcept
			{
				return !(m_flags & BlockState::Discard && m_flags & BlockState::Finished);
			}
		public:
			AsyncControlBlock() = default;
			AsyncControlBlock(std::nullptr_t) { }
			AsyncControlBlock(const AsyncControlBlock&) = delete;
			AsyncControlBlock(AsyncControlBlock&&) = delete;

			// Access

			void discard() const noexcept
			{
				std::unique_lock lock(m_mtx);
				m_flags |= Discard;
			}

			void sync() const
			{
				std::unique_lock lock(m_mtx);
				m_cv.wait(lock, [this]() {
					return !_valid();
				});
			}

			void value() const
			{
				std::unique_lock lock(m_mtx);
				m_cv.wait(lock, [this]() {
					return (m_flags & BlockState::Finished);
				});
				if (m_ex != nullptr)
					std::rethrow_exception(m_ex);
			}

			void set_exception(std::exception_ptr ex)
			{
				{
					std::unique_lock lock(m_mtx);
					m_flags |= Finished;
					m_ex = ex;
				}
				set_value();
			}
			void set_value()
			{
				{
					std::unique_lock lock(m_mtx);
					m_flags |= Finished;
				}
				m_cv.notify_all();
			}
			void reset()
			{
				std::unique_lock lock(m_mtx);
				m_flags &= ~Finished;
			}

			bool valid() const noexcept
			{
				std::unique_lock lock(m_mtx);
				return _valid();
			}
		};

		template<typename Type>
		class AsyncControlBlock : public AsyncControlBlock<void>
		{
		private:
			using base = AsyncControlBlock<void>;
			using value_type = std::conditional_t<
				std::is_reference_v<Type>,
				std::remove_reference_t<Type>*,
				Type
			>;
		private:
			value_type m_value{};
		private:
			const Type& _value() const
			{
				if constexpr (std::is_reference_v<Type>) return *m_value;
				else return m_value;
			}
		public:
			using promise_type = std::shared_ptr<AsyncControlBlock<void>>;

			AsyncControlBlock() = default;
			AsyncControlBlock(const AsyncControlBlock&) = delete;
			AsyncControlBlock(AsyncControlBlock&&) = delete;
			template<typename Val>
			AsyncControlBlock(Val&& value)
				: m_value(std::forward<Val>(value)) { }

			// Access

			const Type& value() const
			{
				base::value();
				return _value();
			}

			template<typename Val>
			void set_value(Val&& value)
			{
				{
					std::unique_lock lock(m_mtx);
					if constexpr (std::is_reference_v<Type>)
					{
						static_assert(std::is_reference_v<Val>);
						m_value = &value;
					}
					else
					{
						m_value = std::forward<Val>(value);
					}
				}
				base::set_value();
			}

			// Coroutines support

			template<typename Val>
			void return_value(Val&& value)
			{
				set_value(std::forward<Val>(value));
			}
		};

		template<typename Val>
		struct ValueType
		{
			using result = const std::decay_t<Val>&;
			using result_ptr = const std::decay_t<Val>*;
		};
		template<>
		struct ValueType<void>
		{
			using result = void;
			using result_ptr = void;
		};

		template<typename Type>
		class AsyncResult
		{
		private:
			using value_type = typename ValueType<Type>::result;
			using value_type_ptr = typename ValueType<Type>::result_ptr;
		private:
			mutable std::shared_ptr<impl::AsyncControlBlock<Type>> m_control_block{ nullptr };

			void _check_block() const
			{
				if (m_control_block == nullptr)
					throw std::runtime_error{ "Attempt to access nullptr future" };
			}
		public:
			AsyncResult() = default;
			AsyncResult(std::nullptr_t) {}
			AsyncResult(const AsyncResult&) = default;
			AsyncResult(AsyncResult&&) = default;
			explicit AsyncResult(std::shared_ptr<impl::AsyncControlBlock<Type>> block)
				: m_control_block(block) { }
			~AsyncResult() = default;

			// Metadata

			bool running() const noexcept
			{
				return m_control_block->valid();
			}

			// Internals

			auto block() const
			{
				return m_control_block;
			}

			// Synchronization

			void sync() const
			{
				if (m_control_block == nullptr)
					return;
				m_control_block->sync();
			}

			value_type value() const
			{
				return m_control_block->value();
			}

			void stop(bool with_sync = true) const
			{
				if (m_control_block == nullptr)
					return;
				m_control_block->discard();
				if (with_sync)
					sync();
				m_control_block.reset();
			}

			AsyncResult& operator=(const AsyncResult&) = default;
			AsyncResult& operator=(AsyncResult&&) = default;

			bool operator==(std::nullptr_t) const noexcept
			{
				return m_control_block == nullptr;
			}
			bool operator!=(std::nullptr_t) const noexcept
			{
				return m_control_block != nullptr;
			}
			bool operator!() const noexcept
			{
				return m_control_block == nullptr;
			}

			value_type operator*() const
			{
				return value();
			}
			value_type_ptr operator->() const
			{
				return &m_control_block->value();
			}
		};
	}

	template<typename Type>
	using future = impl::AsyncResult<Type>;

	namespace impl
	{
		class AsynchronousScheduler : public std::enable_shared_from_this<AsynchronousScheduler>
		{
		public:
			enum Flags
			{
				AutoGrow = 1 << 0,
				AutoShrink = 1 << 1,
				RecycleCyclicThread = 1 << 2,
				ManualCyclicThread = 1 << 3,
			};
			struct Config
			{
				std::chrono::milliseconds maximum_idle{ std::chrono::milliseconds::max() };
				std::chrono::milliseconds resize_delay{ std::chrono::milliseconds(1000) };
				std::size_t resize_increment{ 2 };
				std::size_t minimum_threads{ 1 };
				std::size_t maximum_threads{ std::thread::hardware_concurrency() };
				float critical_load{ 5.f };
			};
			class CyclicWorkerHandle
			{
			private:
				std::shared_ptr<AsynchronousScheduler> m_ptr{ nullptr };
			public:
				CyclicWorkerHandle() = default;
				explicit CyclicWorkerHandle(std::nullptr_t) {}
				CyclicWorkerHandle(CyclicWorkerHandle&&) = default;
				CyclicWorkerHandle(const CyclicWorkerHandle&) = delete;
				explicit CyclicWorkerHandle(std::shared_ptr<AsynchronousScheduler> ptr)
					: m_ptr(ptr)
				{
					if (m_ptr->m_active_cyclic_thread)
						throw std::logic_error{ "Attempt to create cyclic worker handle when one is already running" };
					m_ptr->m_thread_counter++;
					m_ptr->m_active_cyclic_thread = true;
				}
				~CyclicWorkerHandle()
				{
					stop();
				}

				bool wait(std::chrono::milliseconds max) const
				{
					if (!m_ptr)
						return false;
					return m_ptr->_worker_handle_cyclic_task(max);
				}
				bool wait() const
				{
					if (!m_ptr)
						return false;
					return m_ptr->_worker_handle_cyclic_task();
				}
				void stop()
				{
					if (m_ptr)
					{
						m_ptr->m_active_cyclic_thread = false;
						m_ptr->m_thread_counter--;
						m_ptr->m_threads_cv.notify_all();
						m_ptr = nullptr;
					}
				}

				CyclicWorkerHandle& operator=(CyclicWorkerHandle&&) = default;
				CyclicWorkerHandle& operator=(const CyclicWorkerHandle&) = delete;
			};
		private:
#			ifndef MT_TP_USE_CUSTOM_FUNCTION
			using normal_task = std::function<void()>;
			using cyclic_task = std::function<bool()>;
#			else
			using normal_task = MT_TP_USE_CUSTOM_FUNCTION<void()>;
			using cyclic_task = MT_TP_USE_CUSTOM_FUNCTION<bool()>;
#			endif
			using control_block_ptr = std::shared_ptr<impl::AsyncControlBlock<void>>;
			template<typename Type>
			using typed_control_block = impl::AsyncControlBlock<Type>;

			struct Task
			{
				normal_task task{ nullptr };
				control_block_ptr ctr{ nullptr };
			};
			struct CyclicTask
			{
				cyclic_task task{ nullptr };
				control_block_ptr ctr{ nullptr };
				std::chrono::milliseconds timing{ 0 };
				std::chrono::steady_clock::time_point last_invocation{};

				void update()
				{
					last_invocation = std::chrono::steady_clock::now();
				}
				bool ready() const noexcept
				{
					return (std::chrono::steady_clock::now() - last_invocation) > timing;
				}
				auto time() const noexcept
				{
					if (ready())
						return std::chrono::milliseconds(0);
					return std::chrono::duration_cast<std::chrono::milliseconds>(timing - (std::chrono::steady_clock::now() - last_invocation));
				}
			};
		private:
			// Metadata

			std::atomic<std::chrono::milliseconds> m_resize_delay{ std::chrono::milliseconds(1000) };
			std::atomic<std::chrono::milliseconds> m_max_idle{ std::chrono::milliseconds::max() };
			std::atomic<std::size_t> m_resize_increment{ 2 };
			std::atomic<std::size_t> m_min_threads{ 1 };
			std::atomic<std::size_t> m_max_threads{ std::thread::hardware_concurrency() };
			std::atomic<float> m_critical_load{ 5.f };

			std::atomic<std::chrono::steady_clock::time_point> m_last_resize{};
			std::atomic<std::size_t> m_thread_counter{ 0 };
			std::atomic<std::size_t> m_shrink_counter{ 0 };
			std::atomic<std::size_t> m_active{ 0 };
			std::atomic<bool> m_terminate{ false };
			std::atomic<bool> m_terminate_cyclic{ false };
			std::atomic<bool> m_active_cyclic_thread{ false };
			std::atomic<bool> m_pushed_cyclic{ false };
			std::atomic<bool> m_cyclic_accepted_task{ false };
			std::atomic<unsigned char> m_flags{ 0x00 };

			// Task/thread management

			std::thread m_cyclic_thread{};
			std::vector<std::thread> m_threads{};
			std::list<CyclicTask> m_cyclic_tasks{};
			std::queue<Task> m_tasks{};

			// Synchronization

			mutable std::atomic_flag m_cyclic_tasks_append_flag{};
			mutable std::shared_mutex m_cyclic_tasks_mtx{};
			mutable std::mutex m_tasks_mtx{};
			mutable std::mutex m_threads_mtx{};

			mutable std::condition_variable_any m_cyclic_tasks_cv{};
			mutable std::condition_variable m_tasks_cv{};
			mutable std::condition_variable m_threads_cv{};

			void _set_append_flag() const noexcept
			{
				while (m_cyclic_tasks_append_flag.test_and_set(std::memory_order::acquire));
			}
			void _set_append_flag() noexcept
			{
				while (m_cyclic_tasks_append_flag.test_and_set(std::memory_order::acquire));
			}
			void _reset_append_flag() const noexcept
			{
				m_cyclic_tasks_append_flag.clear(std::memory_order::release);
			}
			void _reset_append_flag() noexcept
			{
				m_cyclic_tasks_append_flag.clear(std::memory_order::release);
			}

			// Worker threads

			std::thread _launch_thread(auto handler)
			{
				return std::thread([handler, this]() {
					(this->*handler)(weak_from_this());
				});
			}

			void _launch_cyclic_thread_if()
			{
				if (!(m_flags & ManualCyclicThread))
				{
					std::unique_lock lock(m_threads_mtx);
					if (!m_active_cyclic_thread)
					{
						m_cyclic_thread = _launch_thread(&AsynchronousScheduler::_cyclic_worker);
					}
				}
			}
			void _launch_workers_if()
			{
				if (
					m_flags & AutoGrow &&
					std::chrono::steady_clock::now() - m_last_resize.load() > m_resize_delay.load()
					)
				{
					std::unique_lock lock(m_threads_mtx);

					if (m_threads.size() != m_max_threads)
					{
						std::size_t count = 0;
						if (!m_threads.empty())
						{
							if (
								const auto load = static_cast<float>(tasks()) / static_cast<float>(m_threads.size());
								load > m_critical_load
								)
							{
								count = std::min(
											m_max_threads.load() - m_threads.size(),
											static_cast<std::size_t>(load)
											);
							}
						}
						else if (
							!(m_flags & RecycleCyclicThread) ||
							!m_active_cyclic_thread
							)
						{
							count = 1;
						}

						if (!count)
						{
							return;
						}

						for (std::size_t i = 0; i < count; i++)
						{
							if (m_threads.size() == m_max_threads)
								break;
							m_threads.push_back(
										_launch_thread(&AsynchronousScheduler::_worker)
										);
						}

						m_last_resize = std::chrono::steady_clock::now();
					}
				}
			}

			// Checks whether the worker should automatically exit (due to resize)
			[[ nodiscard ]] bool _auto_shrink_check(bool idle) noexcept
			{
				if (m_terminate)
					return true;

				if (
					idle &&
					m_flags & AutoShrink &&
					((std::chrono::steady_clock::now() - m_last_resize.load()) > m_resize_delay.load())
					)
				{
					std::unique_lock lock(m_threads_mtx);
					m_last_resize = std::chrono::steady_clock::now();
					return m_threads.size() > m_min_threads;
				}
				else if (m_shrink_counter)
				{
					std::unique_lock lock(m_threads_mtx);
					if (m_threads.size() > m_min_threads)
					{
						m_shrink_counter--;
						return true;
					}
					m_shrink_counter = 0;
					return false;
				}

				return false;
			}
			// Removes the exitting thread from the thread pool
			void _cleanup_thread(std::thread::id id) noexcept
			{
				{
					std::scoped_lock lock(m_threads_mtx, m_tasks_mtx);
					auto f = std::find_if(m_threads.begin(), m_threads.end(), [&id](const auto& v) {
						return v.get_id() == id;
					});

					if (f != m_threads.end())
					{
						f->detach();
						m_threads.erase(f);
						m_last_resize = std::chrono::steady_clock::now();
					}
				}
				m_threads_cv.notify_all();
			}

			// Invokes the task and manages the active counter
			auto _invoke_task(const auto& callback)
			{
				if constexpr (std::is_same_v<void, decltype(callback())>)
				{
					m_active++;
					callback();
					m_active--;
				}
				else
				{
					m_active++;
					const auto res = callback();
					m_active--;
					return res;
				}
			}

			// Handles an incoming task from the pool (invoked by a worker)
			bool _worker_handle_task()
			{
				const auto timestamp = std::chrono::steady_clock::now();
				std::unique_lock lock(m_tasks_mtx);
				if (m_max_idle.load() == std::chrono::milliseconds::max())
				{
					m_tasks_cv.wait(lock, [this, &timestamp]() {
						return
								m_terminate ||
								((m_flags & AutoShrink) && (std::chrono::steady_clock::now() - timestamp > m_max_idle.load())) ||
								!m_tasks.empty();
					});
				}
				else
				{
					m_tasks_cv.wait_for(lock, m_max_idle.load(), [this, &timestamp]() {
						return
								m_terminate ||
								((m_flags & AutoShrink) && (std::chrono::steady_clock::now() - timestamp > m_max_idle.load())) ||
								!m_tasks.empty();
					});
				}

				// Auto shrink check doesn't need idle time here
				// Since the cv is only satisfied if tasks are not empty or termination is ordered or the idle time was exceeded
				// This means that if tasks are empty idle time must have been exceeded

				if (m_tasks.empty())
					return !_auto_shrink_check(true);

				auto task = std::move(m_tasks.front());
				m_tasks.pop();
				lock.unlock();
				_invoke_task(task.task);

				return !_auto_shrink_check(false);
			}
			// Periodically handles cyclic tasks from the pool (invoked by cyclic_worker)
			bool _worker_handle_cyclic_task()
			{
				return _worker_handle_cyclic_task(m_max_idle);
			}
			bool _worker_handle_cyclic_task(std::chrono::milliseconds max)
			{
				std::chrono::milliseconds max_wait_time{ max };
				std::shared_lock lock(m_cyclic_tasks_mtx);

				decltype(m_cyclic_tasks)::iterator beg;
				decltype(m_cyclic_tasks)::iterator end;

				_set_append_flag();
				{
					beg = m_cyclic_tasks.begin();
					end = m_cyclic_tasks.end();
					m_pushed_cyclic = false;
				}
				_reset_append_flag();

				for (auto it = beg; it != end;)
				{
					if (it->ready())
					{
						it->update();
						if (!_invoke_task(it->task))
						{
							auto del = it;
							++it;

							lock.unlock();
							{
								std::unique_lock lock(m_cyclic_tasks_mtx);
								m_cyclic_tasks.erase(del);
							}
							lock.lock();

							continue;
						}
					}

					max_wait_time = std::min(
								max_wait_time,
								it->time()
								);

					++it;
				}

				// Here we have to wait for either a task or until the next cyclic
				if (m_flags & RecycleCyclicThread)
				{
					std::unique_lock lock(m_tasks_mtx);

					if (max_wait_time == std::chrono::milliseconds::max())
					{
						m_tasks_cv.wait(lock, [this]() {
							return
								m_terminate_cyclic ||
								m_terminate ||
								m_pushed_cyclic;
						});
					}
					else
					{
						m_tasks_cv.wait_for(lock, max_wait_time, [this]() {
							return
								m_terminate_cyclic ||
								m_terminate ||
								m_pushed_cyclic;
						});
					}

					// If it's empty, then we have a new cyclic task
					// Or we need to handle the cyclic tasks
					if (!m_tasks.empty())
					{
						auto task = std::move(m_tasks.front());
						m_tasks.pop();
						lock.unlock();

						_invoke_task(task.task);
					}
					else
						m_cyclic_accepted_task = true;
				}
				// Here we just wait for a cyclic
				else
				{
					if (max_wait_time == std::chrono::milliseconds::max())
					{
						m_cyclic_tasks_cv.wait(lock, [this]() {
							return
								m_terminate_cyclic ||
								m_terminate ||
								m_pushed_cyclic;
						});
					}
					else
					{
						m_cyclic_tasks_cv.wait_for(lock, max_wait_time, [this]() {
							return
								m_terminate_cyclic ||
								m_terminate ||
								m_pushed_cyclic;
						});
					}
				}

				if (m_terminate_cyclic)
					return false;
				return !_auto_shrink_check(false);
			}

			// Manages a thread that accepts tasks
			void _worker(std::weak_ptr<AsynchronousScheduler> pool)
			{
				m_thread_counter++;
				while (_worker_handle_task());
				//const auto lock = pool.lock();
				//if (lock != nullptr)
				_cleanup_thread(std::this_thread::get_id());
				m_thread_counter--;
			}
			// Manages a thread that handles cyclic tasks (max one per pool)
			void _cyclic_worker(std::weak_ptr<AsynchronousScheduler> pool)
			{
				m_thread_counter++;
				m_active_cyclic_thread = true;
				while (_worker_handle_cyclic_task());
				m_active_cyclic_thread = false;
				//const auto lock = pool.lock();
				//if (lock != nullptr)
				_cleanup_thread(std::this_thread::get_id());
				m_thread_counter--;
			}

			// Task generation

			template<typename Func, typename... Argv>
			[[ nodiscard ]] auto _anonymize_task(Func&& callback, Argv&&... args)
			{
				using ret_type = decltype(callback(std::forward<Argv>(args)...));
				auto block = std::dynamic_pointer_cast<typed_control_block<ret_type>>(
										std::make_shared<impl::AsyncControlBlock<ret_type>>()
									);
				return std::make_pair([
									  block,
									  callback = std::forward<Func>(callback),
									  ...args = std::forward<Argv>(args)
									  ]() mutable {
					block->discard();
					try
					{
						if constexpr (std::is_same_v<ret_type, void>)
						{
							callback(std::forward<Argv>(args)...);
							block->set_value();
						}
						else
						{
							block->set_value(callback(std::forward<Argv>(args)...));
						}
					}
					catch (...)
					{
						block->set_exception(std::current_exception());
					}
				}, future<ret_type>(block));
			}

			template<typename Func, typename... Argv>
			auto _anonymize_deferred_task(Func&& callback, Argv&&... args)
			{
				using ret_type = decltype(callback(std::forward<Argv>(args)...));
				auto block = std::dynamic_pointer_cast<typed_control_block<ret_type>>(
										std::make_shared<impl::AsyncControlBlock<ret_type>>()
									);
				return std::make_pair([
									  block,
									  callback = std::forward<Func>(callback),
									  ...args = std::forward<Argv>(args)
									  ]() mutable -> bool {
					if (!block->valid())
						return false;
					try
					{
						if constexpr (std::is_same_v<ret_type, void>)
						{
							block->reset();
							callback(std::forward<Argv>(args)...);
							block->set_value();
						}
						else
						{
							block->reset();
							block->set_value(callback(std::forward<Argv>(args)...));
						}
					}
					catch (...)
					{
						block->set_exception(std::current_exception());
					}
					return false;
				}, future<ret_type>(block));
			}

			template<typename Func, typename... Argv>
			auto _anonymize_cyclic_task(Func&& callback, Argv&&... args)
			{
				using ret_type = decltype(callback(mt::future<void>(nullptr), std::forward<Argv>(args)...));
				auto block = std::dynamic_pointer_cast<typed_control_block<ret_type>>(
										std::make_shared<impl::AsyncControlBlock<ret_type>>()
									);
				return std::make_pair([
									  block,
									  callback = std::forward<Func>(callback),
									  ...args = std::forward<Argv>(args)
									  ]() mutable -> bool {
					if (!block->valid())
						return false;
					try
					{
						if constexpr (std::is_same_v<ret_type, void>)
						{
							block->reset();
							callback(mt::future<ret_type>(block), std::forward<Argv>(args)...);
							block->set_value();
						}
						else
						{
							block->reset();
							block->set_value(callback(std::forward<Argv>(args)...));
						}
					}
					catch (...)
					{
						block->set_exception(std::current_exception());
					}
					return true;
				}, future<ret_type>(block));
			}

			void _launch_task(normal_task&& callback, control_block_ptr block)
			{
				_launch_workers_if();
				{
					if (threads() < m_min_threads)
					{
						launch_threads(m_min_threads);
					}
				}
				{
					std::unique_lock lock(m_tasks_mtx);
					m_tasks.emplace(Task{ std::move(callback), block });
				}
				m_tasks_cv.notify_one();
			}
			void _launch_cyclic_task(cyclic_task&& callback, control_block_ptr block, std::chrono::milliseconds timing, bool delay = false)
			{
				_launch_cyclic_thread_if();
				_set_append_flag();
				{
					// This can be shared because we write at the end (which is not destructive)
					// The cyclic handler accounts for that and when iterating saves the begin/end iterators
					std::shared_lock lock2(m_cyclic_tasks_mtx);
					m_pushed_cyclic = true;
					m_cyclic_tasks.emplace_back(CyclicTask{
						std::move(callback),
						block,
						timing
					});
					if (delay) m_cyclic_tasks.back().last_invocation = std::chrono::steady_clock::now();
				}
				_reset_append_flag();
				if (m_flags & RecycleCyclicThread)
				{
					m_cyclic_accepted_task = false;
					while (!m_cyclic_accepted_task) { m_tasks_cv.notify_one(); }
				}
				else
					m_cyclic_tasks_cv.notify_one();
			}

			// Object management

			void _move_from(AsynchronousScheduler&& copy)
			{
				bool launch_cthread = false;
				copy.terminate();
				{
					std::scoped_lock lock1(copy.m_threads_mtx, copy.m_tasks_mtx, copy.m_cyclic_tasks_mtx);
					std::scoped_lock lock2(m_threads_mtx, m_tasks_mtx, m_cyclic_tasks_mtx);

					m_max_idle = copy.m_max_idle.load();
					m_resize_delay = copy.m_resize_delay.load();
					m_critical_load = copy.m_critical_load.load();
					m_last_resize = copy.m_last_resize.load();
					m_resize_increment = copy.m_resize_increment.load();
					m_max_threads = copy.m_max_threads.load();
					m_min_threads = copy.m_min_threads.load();

					m_tasks = std::move(copy.m_tasks);
					m_cyclic_tasks = std::move(copy.m_cyclic_tasks);
					launch_cthread = !m_cyclic_tasks.empty();
				}

				if (launch_cthread)
				{
					m_pushed_cyclic = true;
					_launch_cyclic_thread_if();
				}
			}
		public:
			using AsynchronousSchedulerPtr = std::shared_ptr<AsynchronousScheduler>;

			template<typename Func, typename... Argv>
			static auto parallel_if(
					std::shared_ptr<AsynchronousScheduler> sh,
					Func&& callback,
					Argv&&... args
					)
			{
				if (sh)
				{
					return sh->launch_task(std::forward<Func>(callback), std::forward<Argv>(args)...);
				}
				else
				{
					using ret_type = decltype(callback(std::forward<Argv>(args)...));
					if constexpr (std::is_same_v<ret_type, void>)
					{
						callback(std::forward<Argv>(args)...);
						return future<void>(
									std::make_shared<impl::AsyncControlBlock<void>>()
									);
					}
					else
					{
						return future<ret_type>(std::make_shared<impl::AsyncControlBlock<ret_type>>(
													callback(std::forward<Argv>(args)...)
													));
					}
				}
			}

			template<typename Func, typename... Argv>
			static auto schedule_if(
					std::shared_ptr<AsynchronousScheduler> sh,
					Func&& callback,
					std::chrono::milliseconds timing,
					Argv&&... args
					)
			{
				if (sh == nullptr)
					throw std::runtime_error{ "Failed to schedule task; the AsynchronousScheduler is nullptr" };
				return sh->launch_cyclic_task(std::forward<Func>(callback), timing, std::forward<Argv>(args)...);
			}

			static AsynchronousSchedulerPtr from()
			{
				return std::make_shared<AsynchronousScheduler>();
			}
			static AsynchronousSchedulerPtr from(AsynchronousScheduler&& copy)
			{
				return std::make_shared<AsynchronousScheduler>(std::move(copy));
			}
			static AsynchronousSchedulerPtr from(unsigned char flags)
			{
				return std::make_shared<AsynchronousScheduler>(flags);
			}
			static AsynchronousSchedulerPtr from(Config cfg, unsigned char flags = 0x00)
			{
				return std::make_shared<AsynchronousScheduler>(cfg, flags);
			}

			AsynchronousScheduler() = default;
			AsynchronousScheduler(const AsynchronousScheduler&) = delete;
			AsynchronousScheduler(AsynchronousScheduler&& copy) { _move_from(std::move(copy)); }
			explicit AsynchronousScheduler(unsigned char flags) : m_flags(flags) { }
			explicit AsynchronousScheduler(Config cfg, unsigned char flags = 0x00) : m_flags(flags)
			{
				m_max_idle = cfg.maximum_idle;
				m_critical_load = cfg.critical_load;
				m_max_threads = cfg.maximum_threads;
				m_min_threads = cfg.minimum_threads;
				m_resize_delay = cfg.resize_delay;
				m_resize_increment = cfg.resize_increment;
			}
			~AsynchronousScheduler()
			{
				terminate();
			}

			[[ nodiscard ]] std::size_t tasks() const noexcept
			{
				std::unique_lock lock(m_tasks_mtx);
				return m_active + m_tasks.size();
			}
			[[ nodiscard ]] std::size_t threads() const noexcept
			{
				std::unique_lock lock(m_threads_mtx);
				return m_threads.size();
			}
			[[ nodiscard ]] std::size_t max_threads() const noexcept
			{
				return m_max_threads;
			}

			template<typename Func, typename... Argv>
			auto launch_task(Func&& callback, Argv&&... args) noexcept
			{
				auto [ task, future ] = _anonymize_task(
								 std::forward<Func>(callback),
								 std::forward<Argv>(args)...
								 );
				_launch_task(std::move(task), future.block());
				return future;
			}

			template<typename Func, typename... Argv>
			auto launch_cyclic_task(Func&& callback, std::chrono::milliseconds timing, Argv&&... args) noexcept
			{
				auto [ task, future ] = _anonymize_cyclic_task(
								 std::forward<Func>(callback),
								 std::forward<Argv>(args)...
								 );
				_launch_cyclic_task(std::move(task), future.block(), timing);
				return future;
			}

			template<typename Func, typename... Argv>
			auto launch_deferred_task(Func&& callback, std::chrono::milliseconds delay, Argv&&... args) noexcept
			{
				// Basically the same as launch_cyclic_task
				// The deferred task only returns false (to destroy the task) after first invocation
				// And also we have to delay the first invocation
				auto [ task, future ] = _anonymize_deferred_task(
								 std::forward<Func>(callback),
								 std::forward<Argv>(args)...
								 );
				_launch_cyclic_task(std::move(task), future.block(), delay, true);
				return future;
			}

			// Creates a cyclic worker handle which allows for manual cyclic task management
			CyclicWorkerHandle make_cyclic_worker()
			{
				return CyclicWorkerHandle(shared_from_this());
			}

			void launch_threads(std::size_t count) noexcept
			{
				std::unique_lock lock(m_threads_mtx);

				count = std::min(count + m_threads.size(), m_max_threads.load());
				if (m_threads.size() == m_max_threads)
				{
					return;
				}

				for (std::size_t i = 0; i < count; i++)
				{
					if (m_threads.size() == m_max_threads)
						break;
					m_threads.push_back(
								_launch_thread(&AsynchronousScheduler::_worker)
								);
				}
			}
			void shrink_threads(std::size_t count) noexcept
			{
				m_shrink_counter += count;
				m_tasks_cv.notify_all();
			}

			// Synchronizes to all current active tasks
			// !!! If there are cyclic tasks running it will not synchronize until they stop
			void sync() noexcept
			{
				// Busy wait, I could setup another mutex for it,
				// but in general this function is rarely used anyways
				while (m_active)
				{
					std::scoped_lock lock(m_tasks_mtx, m_cyclic_tasks_mtx);
					if (m_tasks.size() && m_cyclic_tasks.size() && !m_active)
						break;
					std::this_thread::yield();
				}
			}
			// Removes all threads
			void terminate() noexcept
			{
				m_terminate = true;
				m_tasks_cv.notify_all();
				m_cyclic_tasks_cv.notify_all();

				std::unique_lock lock(m_threads_mtx);
				m_threads_cv.wait(lock, [this]() {
					return m_threads.empty() && !m_active_cyclic_thread;
				});

				if (m_cyclic_thread.joinable())
					m_cyclic_thread.join();
				while (m_thread_counter) std::this_thread::yield();
				m_terminate = false;				
			}
			// Removes all cyclic tasks and the cyclic thread
			void clear() noexcept
			{
				{
					std::unique_lock lock(m_cyclic_tasks_mtx);
					m_cyclic_tasks.clear();
				}

				if (m_active_cyclic_thread)
				{
					m_terminate_cyclic = true;
					m_cyclic_tasks_cv.notify_all();

					std::unique_lock lock(m_threads_mtx);
					m_threads_cv.wait(lock, [this]() {
						return !m_active_cyclic_thread;
					});

					if (m_cyclic_thread.joinable())
						m_cyclic_thread.join();
					m_terminate_cyclic = false;
				}
			}

			AsynchronousScheduler& operator=(const AsynchronousScheduler&) = delete;
			AsynchronousScheduler& operator=(AsynchronousScheduler&& copy) noexcept
			{
				_move_from(std::move(copy));
				return *this;
			}
		};
	}

	// Alias that makes sure that a shared_ptr<impl::AsynchronousScheduler> is constructed
	class AsynchronousScheduler
	{
	private:
		using Scheduler = impl::AsynchronousScheduler;
		using SchedulerPtr = Scheduler::AsynchronousSchedulerPtr;
	public:
		using CyclicWorkerHandle = Scheduler::CyclicWorkerHandle;
		using Flags = Scheduler::Flags;
		using Config = Scheduler::Config;
	private:
		SchedulerPtr m_scheduler{ nullptr };
	public:
		// Aliases the AsynchronousScheduler's static parallel_if
		// Automatically passes it's managed pointer to the thread pool
		template<typename Func, typename... Argv>
		auto parallel_if(
				Func&& callback,
				Argv&&... args
				)
		{
			return Scheduler::parallel_if(
						m_scheduler,
						std::forward<Func>(callback),
						std::forward<Argv>(args)...
						);
		}

		// Aliases the AsynchronousScheduler's static schedule_if
		// Automatically passes it's managed pointer to the thread pool
		template<typename Func, typename... Argv>
		auto schedule_if(
				Func&& callback,
				std::chrono::milliseconds timing,
				Argv&&... args
				)
		{
			return Scheduler::schedule_if(
						m_scheduler,
						timing,
						std::forward<Func>(callback),
						std::forward<Argv>(args)...
						);
		}

		static AsynchronousScheduler from()
		{
			return Scheduler::from();
		}
		static AsynchronousScheduler from(Scheduler&& copy)
		{
			return Scheduler::from(std::move(copy));
		}
		static AsynchronousScheduler from(unsigned char flags)
		{
			return Scheduler::from(flags);
		}
		static AsynchronousScheduler from(Scheduler::Config cfg, unsigned char flags = 0x00)
		{
			return Scheduler::from(cfg, flags);
		}

		AsynchronousScheduler() = default;
		AsynchronousScheduler(std::nullptr_t) {}
		AsynchronousScheduler(const AsynchronousScheduler&) = default;
		AsynchronousScheduler(AsynchronousScheduler&&) = default;
		AsynchronousScheduler(SchedulerPtr object) : m_scheduler(object) { }
		~AsynchronousScheduler() = default;

		AsynchronousScheduler& operator=(const AsynchronousScheduler&) = default;
		AsynchronousScheduler& operator=(AsynchronousScheduler&&) = default;
		AsynchronousScheduler& operator=(SchedulerPtr object)
		{
			m_scheduler = object;
			return *this;
		}

		bool operator==(std::nullptr_t) const noexcept
		{
			return m_scheduler == nullptr;
		}
		bool operator==(std::nullptr_t) noexcept
		{
			return m_scheduler == nullptr;
		}
		bool operator!=(std::nullptr_t) const noexcept
		{
			return m_scheduler != nullptr;
		}
		bool operator!=(std::nullptr_t) noexcept
		{
			return m_scheduler != nullptr;
		}
		bool operator!() const noexcept
		{
			return m_scheduler == nullptr;
		}
		bool operator!() noexcept
		{
			return m_scheduler == nullptr;
		}

		const auto& operator*() const noexcept
		{
			return *m_scheduler;
		}
		auto& operator*() noexcept
		{
			return *m_scheduler;
		}
		const auto& operator->() const noexcept
		{
			return m_scheduler;
		}
		auto& operator->() noexcept
		{
			return m_scheduler;
		}
	};
}

#endif // THREAD_POOL_HPP
