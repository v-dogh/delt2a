#include "d2_tree_state.hpp"
#include "d2_tree_parent.hpp"

namespace d2
{
    TreeState::TreeState(
        std::shared_ptr<ParentElement> rptr,
        std::shared_ptr<ParentElement> coreptr
        ) : _root_ptr(rptr), _core_ptr(coreptr)
    {
        if (rptr)
            _ctx = rptr->context();
    }

    void TreeState::set_context(std::shared_ptr<IOContext> ctx)
    {
        _ctx = ctx;
    }
    void TreeState::set_root(std::shared_ptr<ParentElement> ptr)
    {
        _root_ptr = ptr;
    }
    void TreeState::set_core(std::shared_ptr<ParentElement> ptr)
    {
        _core_ptr = ptr;
    }

    std::shared_ptr<IOContext> TreeState::context() const
    {
        return _ctx.lock();
    }
    sys::module<sys::SystemScreen> TreeState::screen() const
    {
        return context()->screen();
    }
    std::shared_ptr<ParentElement> TreeState::root() const
    {
        return _root_ptr;
    }
    std::shared_ptr<TreeState> TreeState::root_state() const
    {
        return root()->state();
    }
    std::shared_ptr<ParentElement> TreeState::core() const
    {
        return _core_ptr.lock();
    }
}
