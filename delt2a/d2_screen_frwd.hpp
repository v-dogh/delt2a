#ifndef D2_SCREEN_FRWD_HPP
#define D2_SCREEN_FRWD_HPP

#include <memory>
#include <absl/container/flat_hash_map.h>
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
            std::shared_ptr<ParentElement> coreptr
        );
        TreeState(const TreeState&) = delete;
        TreeState(TreeState&&) = default;
        virtual ~TreeState() = default;

        virtual void construct() {}
		virtual void swap_in() {}
		virtual void swap_out() {}
		virtual void update() {}

        void set_root(std::shared_ptr<ParentElement> ptr);
        void set_core(std::shared_ptr<ParentElement> ptr);

        std::shared_ptr<IOContext> context() const;
        std::shared_ptr<Screen> screen() const;
        std::shared_ptr<ParentElement> root() const;
        std::shared_ptr<TreeState> root_state() const;
        std::shared_ptr<ParentElement> core() const;

		template<typename Type>
		const auto* as() const
		{
			return static_cast<const Type*>(this);
		}
		template<typename Type>
		auto* as()
		{
			return static_cast<Type*>(this);
		}

        TreeState& operator=(const TreeState&) = delete;
        TreeState& operator=(TreeState&&) = default;
    };

    template<typename... Argv>
    struct TupTreeState : public TreeState
    {
        std::tuple<Argv...> params{};
        TupTreeState(
            std::shared_ptr<Screen> src,
            std::shared_ptr<IOContext> ctx,
            std::shared_ptr<ParentElement> rptr,
            std::shared_ptr<ParentElement> coreptr,
            Argv... args
            ) : TreeState(src, ctx, rptr, coreptr), params(std::move(args...))
        {}
        using TreeState::TreeState;
    };
} // d2

#endif // D2_SCREEN_FRWD_HPP
