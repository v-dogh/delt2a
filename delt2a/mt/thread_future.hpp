#ifndef THREAD_FUTURE_HPP
#define THREAD_FUTURE_HPP

#include "thread_control.hpp"
#include <memory>
#include <optional>
#include <coroutine>

namespace mt
{
	template<typename Type>
	class Promise;

	template<typename Type, template<typename> typename Block>
	class Future
	{
	public:
		using promise_type = Promise<Type>;
		using block_ptr = std::shared_ptr<Block<Type>>;
        using ret = Type;
	private:
		block_ptr _block{ nullptr };

		void _check_block() const
		{
			[[ unlikely ]] if (_block == nullptr)
				throw std::runtime_error{ "Invalid control block" };
		}
	public:
        template<typename... Argv>
        static auto make(Argv&&... args)
		{
            return Future(std::make_shared<Block<Type>>(
                std::forward<Argv>(args)...
            ));
		}

		Future() = default;
		Future(std::nullptr_t) {}
		Future(block_ptr ptr) : _block(ptr) {}
		Future(const Future&) = default;
		Future(Future&&) = default;

		Type value() const
		{
			_check_block();
			return _block->value();
		}
		std::optional<Type> value_or(auto&& callback) const
		{
			_check_block();
			_block->sync_value();
			if (!_block->has_value())
			{
				callback();
				return std::nullopt;
			}
			return _block->value();
		}

        void pause() const noexcept
        {
            if (!_block)
                return;
            _block->pause();
        }
        void unpause() const noexcept
        {
            if (!_block)
                return;
            _block->unpause();
        }

		void sync_value() const noexcept
		{
			if (!_block)
				return;
			_block->sync_value();
		}
		void sync() const noexcept
		{
			if (!_block)
				return;
			_block->sync();
		}
		void discard() const noexcept
		{
			if (!_block)
				return;
			_block->discard();
		}

        bool is_discarded() const noexcept
        {
            if (!_block)
                return true;
            return _block->is_discarded();
        }
        bool is_paused() const noexcept
        {
            if (!_block)
                return false;
            return _block->is_paused();
        }

		bool has_exception() const noexcept
		{
			return _block && _block->has_exception();
		}
		bool has_value() const noexcept
		{
			return _block && _block->has_value();
		}

		std::exception_ptr exception() const
		{
			return _block->exception();
		}

		block_ptr block() const noexcept
		{
			return _block;
		}

		bool await_ready() const noexcept
		{
			return has_value();
		}
		bool await_suspend(std::coroutine_handle<> handle)
		{
			// Try deferring the handle
			// We could have experienced the write in the meantime
			// If that is the case (and the write did not take the handle) we need to run it
			return !(!_block->try_set_handle(handle) ||
					 (has_value() &&
					  _block->has_handle()));
		}
		Type await_resume()
		{
			return value();
		}

		std::weak_ordering operator<=>(const Future&) const = default;

		Future& operator=(const Future&) = default;
		Future& operator=(Future&&) = default;
	};

	template<typename Type>
	using future = Future<Type, ControlBlock>;

    template<typename Type>
    using man_future = Future<Type, ManagedControlBlock>;

	template<typename Type>
	using co_future = Future<Type, CoroutineControlBlock>;

	template<typename Base, typename Type>
	struct PromiseBase
	{
		template<typename... Argv>
		void return_value(Argv&&... args) noexcept
		{
			if (!static_cast<Base*>(this)->_block)
				return;
			static_cast<Base*>(this)->
				_block->set_value(std::forward<Argv>(args)...);
		}
	};
	template<typename Base>
	struct PromiseBase<Base, void>
	{
		void return_void() noexcept
		{
			if (!static_cast<Base*>(this)->_block)
				return;
			static_cast<Base*>(this)->
				_block->set_value();
		}
	};

	template<typename Type>
	class Promise : public PromiseBase<Promise<Type>, Type>
	{
	public:
		using future_type = co_future<Type>;
		using block_ptr = std::shared_ptr<CoroutineControlBlock<Type>>;
	private:
		block_ptr _block{ nullptr };
	public:
		friend class PromiseBase<Promise<Type>, Type>;

		Promise()
		{
			_block = std::make_shared<CoroutineControlBlock<Type>>();
		}
		Promise(std::nullptr_t) {}
		Promise(const Promise&) = default;
		Promise(Promise&&) = default;

		future_type get_return_object() noexcept
		{
			return future_type(_block);
		}

		std::suspend_never initial_suspend() noexcept { return {}; }
		auto final_suspend() noexcept
		{
			struct awaitable
			{
				block_ptr block;

				bool await_ready() const noexcept
				{
					return block->has_value();
				}
				bool await_suspend(std::coroutine_handle<> handle) noexcept
				{
					return !(!block->try_set_handle(handle) ||
							 (block->has_value() &&
							  block->has_handle()));
				}
				Type await_resume() noexcept
				{
					return block->value();
				}
			};
			return awaitable{ _block };
		}

		void unhandled_exception() noexcept
		{
			if (!_block)
				return;
			_block->set_exception();
		}

		std::weak_ordering operator<=>(const Promise&) const = default;

		Promise& operator=(const Promise&) = default;
		Promise& operator=(Promise&&) = default;
	};

	template<typename Base>
	class DummyPromise
	{
	public:
		Base get_return_object() noexcept
		{
			return Base();
		}

		void return_void() noexcept {}

		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }

		void unhandled_exception() noexcept {}
	};
}

#endif // THREAD_FUTURE_HPP
