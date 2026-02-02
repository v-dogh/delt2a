#include "d2_flow_box.hpp"

namespace d2::dx
{
    void recompute_layout(Element::cpptr ptr, std::vector<Element::ptr> elements, int basex = 0, int basey = 0)
    {
        // Division into virtual groups
        // Each sequence of elements with the same direction of alignment (vertical/horizontal)
        // belongs to the same virtual group.
        // The virtual group's alignment is the alignment of the next group
        // If we have a block with alignment A and the last element has alignment A and B then we introduce a break.
        // We proceed as usual by constructing a new block, but we only reset the offset corresponding to the previous
        // block's alignment
        // Examples:
        // Child 1,2 - horizontal
        // Child 3 - vertical
        // Child 4 - horizontal
        // Virtual Groups: {{{ 1H, 2H }V, 3V }H 4H }
        // Horizontal group
        // A - X
        // B - X
        // Break / horizontal group (carries from previous group)
        // C - X,Y
        // D - X
        // Result
        // AB
        // CD
        //
        // Size pooling
        // Each virtual group which contains an element with a relative size is marked as volatile.
        // If the relative size is aligned with it's positioning, the group receives a 'pool' of size
        // which it will allocate equally between it's elements (that have relative sizes on this axis).
        // Elements with absolute dimensions will substract their sizes from the pool.
        // If the relative size is not aligned with it's positioning, the entire group will be subject to this procedure
        // with respect to the rest of the groups on this axis.
        // Examples:
        // Q - X
        // A - XW
        // B - XW
        // C - XYW
        // D - XW
        // Layout:
        // Q(Q_width) A((width - Q_width) / 2) B((width - Q_width) / 2)
        // C(width / 2) C(width / 2)
        //
        // Q - Y
        // A - XYWH
        // B - XWH
        // C - XYWH
        // D - XWH
        // Layout:
        // Q(*, Q_height)
        // A(*, (height - Q_height) / 2) B(*, (height - Q_height) / 2)
        // C(*, (height - Q_height) / 2) D(*, (height - Q_height) / 2)

        auto cxpos = basex;
        auto cypos = basey;
        auto relh = false;
        auto relv = false;
        for (std::size_t i = 0; i < elements.size();)
        {
            auto& elem = elements[i];
            if (!elem->getstate(Element::Display))
            {
                i++;
                continue;
            }

            const auto relrep = elem->unit_report();
            const auto relht = bool(relrep & Element::RelativeXPos);
            const auto relvt = bool(relrep & Element::RelativeYPos);

            // Break the block if we had an opposite flag on the last element

            if (relht && relvt)
            {
                if (relh)
                {
                    relh = true;
                    relv = false;
                }
                else if (relv)
                {
                    relv = true;
                    relh = false;
                }
            }
            else
            {
                relh = relht;
                relv = relvt;
            }

            if (relh || relv)
            {
                // Calculate group

                auto beg = i;
                auto reldims_pool = 0;
                auto relcount = 0;
                auto is_pooled = false;
                auto is_pooled_vertically = false;

                auto cxpos_offset = 0;
                auto cypos_offset = 0;
                auto is_first = true;
                auto resetx = false;
                auto resety = false;

                const auto primary = elements[i];
                do
                {
                    auto sub = elements[i];
                    const auto relreps = sub->unit_report();
                    const auto relhs = bool(relreps & Element::RelativeXPos);
                    const auto relvs = bool(relreps & Element::RelativeYPos);

                    // Absolute object (skip)
                    if (!(relhs || relvs) ||
                        !sub->getstate(Element::State::Display))
                    {
                        i++;
                        continue;
                    }
                    // Break block
                    else if (
                        !is_first &&
                        ((relh && !relhs) ||
                         (relv && !relvs) ||
                         (relv && relhs) ||
                         (relh && relvs))
                        )
                    {

                        if (relv && relhs) resety = true;
                        else if (relh && relvs) resetx = true;
                        break;
                    }

                    // Relative size metadata

                    const auto pos = sub->internal_position();
                    const auto bbox = sub->internal_box();
                    if ((relreps & Element::RelativeWidth) && relhs)
                    {
                        if (!is_pooled)
                        {
                            reldims_pool += ptr->box().width;
                            is_pooled = true;
                            is_pooled_vertically = false;
                        }
                        relcount++;
                    }
                    if (relhs)
                    {
                        reldims_pool -= bbox.width + pos.x;
                    }

                    if ((relreps & Element::RelativeHeight) && relvs)
                    {
                        if (!is_pooled)
                        {
                            reldims_pool += ptr->box().height;
                            is_pooled = true;
                            is_pooled_vertically = true;
                        }
                        relcount++;
                    }
                    if (relvs)
                    {
                        reldims_pool -= bbox.height + pos.y;
                    }

                    is_first = false;
                    i++;
                }
                while (i < elements.size());

                if (is_pooled)
                {
                    if (is_pooled_vertically)
                    {
                        reldims_pool -=
                            ptr->border_for(ParentElement::BorderType::Top, primary) +
                            ptr->border_for(ParentElement::BorderType::Bottom, primary);
                    }
                    else
                    {
                        reldims_pool -=
                            ptr->border_for(ParentElement::BorderType::Left, primary) +
                            ptr->border_for(ParentElement::BorderType::Right, primary);
                    }
                }

                const auto rel_size = relcount ? reldims_pool / relcount : 0;
                is_first = true;
                do
                {
                    auto sub = elements[beg];
                    const auto relreps = sub->unit_report();
                    const auto relhs = bool(relreps & Element::RelativeXPos);
                    const auto relvs = bool(relreps & Element::RelativeYPos);

                    if (!(relhs || relvs) ||
                        !sub->getstate(Element::State::Display))
                    {
                        beg++;
                        continue;
                    }

                    const auto bbox = sub->internal_box();
                    const auto ipos = sub->internal_position();
                    auto pos = ipos;
                    pos.x += is_first && relht ? ptr->border_for(ParentElement::BorderType::Left, elem) : 0;
                    pos.y += is_first && relvt ? ptr->border_for(ParentElement::BorderType::Top, elem) : 0;
                    sub->override_position(
                        pos.x + cxpos,
                        pos.y + cypos
                    );

                    if (relv)
                    {
                        const auto rwidth = bbox.width;
                        const auto rheight = (rel_size * bool(relreps & Element::RelativeHeight)) + bbox.height;

                        cypos += rheight + pos.y;
                        cxpos_offset = std::max(
                            cxpos_offset,
                            rwidth + pos.x
                        );

                        if (relreps & Element::RelativeHeight)
                        {
                            sub->override_dimensions(rwidth, rheight);
                        }
                        else
                        {
                            sub->override_dimensions(bbox.width, bbox.height);
                        }
                    }
                    else if (relh)
                    {
                        const auto rwidth = (rel_size * bool(relreps & Element::RelativeWidth)) + bbox.width;
                        const auto rheight = bbox.height;

                        cxpos += rwidth + pos.x;
                        cypos_offset = std::max(
                            cypos_offset,
                            rheight + pos.y
                        );

                        if (relreps & Element::RelativeWidth)
                        {
                            sub->override_dimensions(rwidth, rheight);
                        }
                        else
                        {
                            sub->override_dimensions(bbox.width, bbox.height);
                        }
                    }

                    is_first = false;
                    beg++;
                } while (beg != i);

                if (resetx) cxpos = basex;
                else if (resety) cypos = basey;

                cxpos += cxpos_offset;
                cypos += cypos_offset;
            }
            else
                i++;
        }
    }
    void FlowBox::_layout_for_impl(enum Element::Layout type, cptr ptr) const
    {
        if (ptr->relative_layout(type))
        {
            _layout_dirty = true;
            recompute_layout(
                std::static_pointer_cast<const ParentElement>(shared_from_this()),
                _elements
            );
            _layout_dirty = false;
        }
        else
        {
            ParentElement::_layout_for_impl(type, ptr);
            if (!_layout_dirty &&
                (ptr->relative_layout(Layout::X) ||
                ptr->relative_layout(Layout::Y) ||
                ptr->relative_layout(Layout::Width) ||
                ptr->relative_layout(Layout::Height)))
            {
                _layout_dirty = true;
                foreach_internal([](Element::ptr ptr) -> bool {
                    auto flags = 0x00;
                    if (ptr->relative_layout(Layout::X)) flags |= WriteType::LayoutXPos;
                    if (ptr->relative_layout(Layout::Y)) flags |= WriteType::LayoutYPos;
                    if (ptr->relative_layout(Layout::Width)) flags |= WriteType::LayoutWidth;
                    if (ptr->relative_layout(Layout::Height)) flags |= WriteType::LayoutHeight;
                    if (flags != 0x00)
                        d2::internal::ElementView::from(ptr)
                            .signal_write(flags);
                    return true;
                });
            }
        }
    }
    void ScrollFlowBox::_layout_for_impl(enum Element::Layout type, cptr ptr) const
    {
        if (ptr->relative_layout(type))
        {
            _layout_dirty = true;
            recompute_layout(
                std::static_pointer_cast<const ParentElement>(shared_from_this()),
                _elements, 0, -offset_
            );
            _layout_dirty = false;
        }
        else
        {
            ParentElement::_layout_for_impl(type, ptr);
            if (!_layout_dirty &&
                (ptr->relative_layout(Layout::X) ||
                 ptr->relative_layout(Layout::Y) ||
                 ptr->relative_layout(Layout::Width) ||
                 ptr->relative_layout(Layout::Height)))
            {
                _layout_dirty = true;
                foreach_internal([](Element::ptr ptr) -> bool {
                    auto flags = 0x00;
                    if (ptr->relative_layout(Layout::X)) flags |= WriteType::LayoutXPos;
                    if (ptr->relative_layout(Layout::Y)) flags |= WriteType::LayoutYPos;
                    if (ptr->relative_layout(Layout::Width)) flags |= WriteType::LayoutWidth;
                    if (ptr->relative_layout(Layout::Height)) flags |= WriteType::LayoutHeight;
                    if (flags != 0x00)
                        d2::internal::ElementView::from(ptr)
                            .signal_write(flags);
                    return true;
                });
            }
        }
    }
    void VirtualFlowBox::_layout_for_impl(enum Element::Layout type, cptr ptr) const
    {
        if (ptr->relative_layout(type))
        {
            _layout_dirty = true;
            recompute_layout(
                std::static_pointer_cast<const ParentElement>(shared_from_this()),
                _elements
                );
            _layout_dirty = false;
        }
        else
        {
            ParentElement::_layout_for_impl(type, ptr);
            if (!_layout_dirty &&
                (ptr->relative_layout(Layout::X) ||
                 ptr->relative_layout(Layout::Y) ||
                 ptr->relative_layout(Layout::Width) ||
                 ptr->relative_layout(Layout::Height)))
            {
                _layout_dirty = true;
                foreach_internal([](Element::ptr ptr) -> bool {
                    auto flags = 0x00;
                    if (ptr->relative_layout(Layout::X)) flags |= WriteType::LayoutXPos;
                    if (ptr->relative_layout(Layout::Y)) flags |= WriteType::LayoutYPos;
                    if (ptr->relative_layout(Layout::Width)) flags |= WriteType::LayoutWidth;
                    if (ptr->relative_layout(Layout::Height)) flags |= WriteType::LayoutHeight;
                    if (flags != 0x00)
                        d2::internal::ElementView::from(ptr)
                            .signal_write(flags);
                    return true;
                });
            }
        }
    }
}
