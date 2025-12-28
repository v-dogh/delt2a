#ifndef THREAD_DEFER_HPP
#define THREAD_DEFER_HPP

#include "thread_pool.hpp"

namespace mt
{
	template<typename Pool>
	struct DeferDiscard : Pool
	{
		using promise_type = DummyPromise<DeferDiscard<Pool>>;

		bool await_ready() const noexcept
		{
			return false;
		}
		void await_suspend(std::coroutine_handle<> h) const
		{
			Pool::enqueue_void([](std::coroutine_handle<> h){ h.resume(); }, h);
		}
		void await_resume() const noexcept {}
	};
	template<typename Pool>
	struct DeferDecision
	{
		using defer_type = DeferDiscard<Pool>;

		template<typename... Argv>
		static auto defer(Argv&&... args)
		{
			return DeferDiscard<Pool>(std::forward<Argv>(args)...);
		}
	};

	struct ThreadPoolDefer
	{
		ThreadPool::ptr pool;

		ThreadPoolDefer(ThreadPool::ptr ptr) : pool(ptr) {}

		template<typename Func, typename... Argv>
		auto enqueue(Func&& callback, Argv&&... args) const
		{
			return pool->launch(std::forward<Func>(callback), std::forward<Argv>(args)...);
		}
		template<typename Func, typename... Argv>
		void enqueue_void(Func&& callback, Argv&&... args) const
		{
			pool->launch_void(std::forward<Func>(callback), std::forward<Argv>(args)...);
		}
	};
	struct ContextDefer
	{
		template<typename Func, typename... Argv>
		auto enqueue(Func&& callback, Argv&&... args) const
		{
			auto pool = context::get();
			if (!pool)
				throw std::runtime_error{ "Invalid context" };
			return pool->launch(std::forward<Func>(callback), std::forward<Argv>(args)...);
		}
		template<typename Func, typename... Argv>
		void enqueue_void(Func&& callback, Argv&&... args) const
		{
			auto pool = context::get();
			if (!pool)
				throw std::runtime_error{ "Invalid context" };
			pool->launch_void(std::forward<Func>(callback), std::forward<Argv>(args)...);
		}
	};

	using tdefer = DeferDecision<ThreadPoolDefer>;
	using cdefer = DeferDecision<ContextDefer>;
}

#endif // THREAD_DEFER_HPP
