#ifndef D2_SCREEN_FRWD_HPP
#define D2_SCREEN_FRWD_HPP

#include <memory>
#include "d2_tree_element_frwd.hpp"
#include "d2_io_handler_frwd.hpp"

namespace d2
{
	class Screen;
	class TreeState
	{
	public:
		using ptr = std::shared_ptr<TreeState>;
	private:
		std::shared_ptr<Screen> src_{ nullptr };
		std::shared_ptr<IOContext> ctx_{ nullptr };
		std::shared_ptr<ParentElement> root_ptr_{ nullptr };
		std::weak_ptr<ParentElement> core_ptr_{};
	public:
		template<typename Type, typename... Argv>
		static auto make(
			std::shared_ptr<Screen> src,
			std::shared_ptr<IOContext> ctx,
			std::shared_ptr<ParentElement> rptr,
			std::shared_ptr<ParentElement> coreptr,
			Argv&&... args
		)
		{
			return std::make_shared<Type>(
				src, ctx, rptr, coreptr, std::forward<Argv>(args)...
			);
		}

		TreeState(
			std::shared_ptr<Screen> src,
			std::shared_ptr<IOContext> ctx,
			std::shared_ptr<ParentElement> rptr,
			std::shared_ptr<ParentElement> coreptr = nullptr
			) : src_(src), ctx_(ctx), root_ptr_(rptr), core_ptr_(coreptr)
		{}
		virtual ~TreeState() {}

		virtual void swap_in() {}
		virtual void swap_out() {}
		virtual void update() {}

		void set_root(std::shared_ptr<ParentElement> ptr) noexcept
		{
			root_ptr_ = ptr;
		}
		void set_core(std::shared_ptr<ParentElement> ptr) noexcept
		{
			core_ptr_ = ptr;
		}

		std::shared_ptr<IOContext> context() const noexcept
		{
			return ctx_;
		}
		std::shared_ptr<Screen> screen() const noexcept
		{
			return src_;
		}
		std::shared_ptr<ParentElement> root() const noexcept
		{
			return root_ptr_;
		}
		std::shared_ptr<ParentElement> core() const noexcept
		{
			return core_ptr_.lock();
		}

		template<typename Type>
		const auto* as() const noexcept
		{
			return static_cast<const Type*>(this);
		}
		template<typename Type>
		auto* as() noexcept
		{
			return static_cast<Type*>(this);
		}
	};

} // d2

#endif // D2_SCREEN_FRWD_HPP
