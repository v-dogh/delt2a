#ifndef D2_FLOW_BOX_HPP
#define D2_FLOW_BOX_HPP

#include "d2_box.hpp"
#include "d2_scrollbox.hpp"
#include "d2_draggable_box.hpp"

namespace d2::dx
{
    class FlowBox : public Box
    {
    protected:
        mutable bool _layout_dirty{ true };
        virtual void _layout_for_impl(Element::Layout type, cptr ptr) const override;
    public:
        using Box::Box;
    };
    class ScrollFlowBox : public ScrollBox
    {
    protected:
        mutable bool _layout_dirty{ true };
        virtual void _layout_for_impl(Element::Layout type, cptr ptr) const override;
    public:
        using ScrollBox::ScrollBox;
    };
    class VirtualFlowBox : public VirtualBox
    {
    protected:
        mutable bool _layout_dirty{ true };
        virtual void _layout_for_impl(Element::Layout type, cptr ptr) const override;
    public:
        using VirtualBox::VirtualBox;
    };
}


#endif // D2_FLOW_BOX_H
