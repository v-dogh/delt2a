#ifndef D2_TREE_STATE_HPP
#define D2_TREE_STATE_HPP

#include <absl/container/flat_hash_map.h>
#include <d2_exceptions.hpp>
#include <d2_io_handler_frwd.hpp>
#include <d2_tree_element_frwd.hpp>
#include <memory>
#include <mods/d2_core.hpp>

namespace d2
{
    class TreeState : public std::enable_shared_from_this<TreeState>
    {
        D2_TAG_MODULE(tree)
    public:
        using ptr = std::shared_ptr<TreeState>;
    private:
        std::shared_ptr<ParentElement> _root_ptr{nullptr};
        std::weak_ptr<ParentElement> _core_ptr{};
        std::weak_ptr<IOContext> _ctx{};
    public:
        template<typename Type, typename... Argv>
        static auto make(
            std::shared_ptr<ParentElement> rptr,
            std::shared_ptr<ParentElement> coreptr,
            std::shared_ptr<IOContext> ctx,
            Argv&&... args
        )
        {
            return std::make_shared<Type>(rptr, coreptr, ctx, std::forward<Argv>(args)...);
        }

        TreeState(
            std::shared_ptr<ParentElement> rptr,
            std::shared_ptr<ParentElement> coreptr,
            std::shared_ptr<IOContext> ctx
        );
        TreeState(const TreeState&) = delete;
        TreeState(TreeState&&) = default;
        virtual ~TreeState() = default;

        virtual void construct() {}
        virtual void swap_in() {}
        virtual void swap_out() {}
        virtual void update() {}

        void set_context(std::shared_ptr<IOContext> ctx);
        void set_root(std::shared_ptr<ParentElement> ptr);
        void set_core(std::shared_ptr<ParentElement> ptr);

        std::shared_ptr<IOContext> context() const;
        std::shared_ptr<ParentElement> root() const;
        std::shared_ptr<TreeState> root_state() const;
        std::shared_ptr<ParentElement> core() const;
        sys::module<sys::SystemScreen> screen() const;

        template<typename Type> std::shared_ptr<const Type> as() const
        {
            auto p = std::dynamic_pointer_cast<Type>(shared_from_this());
            if (p == nullptr)
                D2_THRW("Attempt to access an the tree state through an invalid object");
            return p;
        }
        template<typename Type> std::shared_ptr<Type> as()
        {
            auto p = std::dynamic_pointer_cast<Type>(shared_from_this());
            if (p == nullptr)
                D2_THRW("Attempt to access an the tree state through an invalid object");
            return p;
        }

        TreeState& operator=(const TreeState&) = delete;
        TreeState& operator=(TreeState&&) = default;
    };

    template<typename... Argv> struct TupTreeState : public TreeState
    {
        std::tuple<Argv...> params{};
        TupTreeState(
            std::shared_ptr<ParentElement> rptr,
            std::shared_ptr<ParentElement> coreptr,
            std::shared_ptr<IOContext> ctx,
            Argv... args
        ) : TreeState(rptr, coreptr, ctx), params(std::move(args...))
        {
        }
        using TreeState::TreeState;
    };

    template<typename Type> using tree_state_ptr = std::shared_ptr<Type>;
} // namespace d2

#endif // D2_TREE_STATE_HPP
