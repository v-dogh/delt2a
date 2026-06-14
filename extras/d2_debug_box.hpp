#pragma once

#include <d2_std.hpp>
#include <elements/d2_draggable_box.hpp>

namespace d2::ex
{
    struct debug : TreeForward<debug, TreeState, dx::VirtualBox>
    {
        static void construct(TreeCtx<dx::VirtualBox, TreeState> ctx);
    };
} // namespace d2::ex
