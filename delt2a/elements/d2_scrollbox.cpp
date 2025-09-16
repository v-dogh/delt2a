#include "d2_scrollbox.hpp"

namespace d2::dx
{
    void ScrollBox::_recompute_height() const noexcept
    {
        computed_height_ = layout(Element::Layout::Height);
        foreach_internal([this](TreeIter it)
        {
            if (it != scrollbar_ && it->getstate(Display))
            {
                computed_height_ =
                    std::max(computed_height_, it->layout(Element::Layout::Y) + offset_ + it->layout(Element::Layout::Height));
            }
        });
    }

    int ScrollBox::_get_border_impl(BorderType type, Element::cptr elem) const noexcept
    {
        if (elem != scrollbar_ && type == BorderType::Right)
        {
            const auto sw = scrollbar_->box().width;
            return Box::_get_border_impl(type, elem) + sw;
        }
        return Box::_get_border_impl(type, elem);
    }
    void ScrollBox::_layout_for_impl(Element::Layout type, cptr ptr) const
    {
        const auto off = resolve_units(ptr->internal_layout(type), ptr);
        ptr->override_layout(type,
            off - (offset_ * (type == Element::Layout::Y && ptr != scrollbar_)
        ));
    }

    void ScrollBox::_state_change_impl(State state, bool value)
    {
        if (state == State::Created && value)
        {
            if (scrollbar_ == nullptr)
            {
                scrollbar_ = std::make_shared<VerticalSlider>(
                                 "", this->state(), std::static_pointer_cast<ParentElement>(shared_from_this())
                             );
                scrollbar_->setstate(State::Swapped, false);
                scrollbar_->zindex = 5;
                scrollbar_->x = 0.0_pxi;
                scrollbar_->y = 0.0_px;
                scrollbar_->height = 1.0_pc;
                scrollbar_->on_change = [this](auto ptr, auto, auto abs)
                {
                    offset_ = abs;
                    _signal_write(WriteType::Style);
                    foreach([](TreeIter ptr) {
                        internal::ElementView::from(ptr)
                            .signal_write(WriteType::LayoutYPos);
                    });
                };
                internal::ElementView::from(scrollbar_)
                    .signal_write(WriteType::Masked);
            }
        }
    }
    void ScrollBox::_update_layout_impl() noexcept
    {
        const auto height = layout(Element::Layout::Height);
        const auto bw = (Box::container_options & Box::EnableBorder) ?
                        resolve_units(Box::border_width) : 0;

        _recompute_height();

        bool written = false;
        if (computed_height_ > height)
        {
            const auto sw = computed_height_ ? ((float(height) / computed_height_) * height) : 0;
            scrollbar_->setstate(Display, true);
            scrollbar_->slider_width = sw;
            written = scrollbar_->slider_width != sw;
        }
        else
            scrollbar_->setstate(Display, false);

        if (const auto ch = computed_height_ - (bw * 2) - height;
            scrollbar_->max != ch)
        {
            scrollbar_->min = 0;
            scrollbar_->max = ch;
            written = true;
        }
        if (written)
            internal::ElementView::from(scrollbar_)
                .signal_write(WriteType::Style);
    }

    TypedTreeIter<VerticalSlider> ScrollBox::scrollbar() const noexcept
    {
        return scrollbar_;
    }

    void ScrollBox::foreach_internal(foreach_internal_callback callback) const
    {
        for (decltype(auto) it : _elements)
            callback(it);
        if (scrollbar_)
            callback(scrollbar_);
    }
}
