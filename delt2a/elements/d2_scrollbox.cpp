#include "d2_scrollbox.hpp"

namespace d2::dx
{
    void ScrollBox::_recompute_height() const
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

    int ScrollBox::_get_border_impl(BorderType type, Element::cptr elem) const
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
                scrollbar_->set<Slider::ZIndex>(5);
                scrollbar_->set<Slider::X>(0.0_pxi);
                scrollbar_->set<Slider::Y>(0.0_px);
                scrollbar_->set<Slider::Height>(1.0_pc);
                scrollbar_->set<Slider::OnSubmit>([this](auto ptr, auto, auto abs)
                {
                    offset_ = abs;
                    _signal_write(WriteType::Style);
                    foreach([](TreeIter ptr) {
                        internal::ElementView::from(ptr)
                            .signal_write(WriteType::LayoutYPos);
                    });
                });
                internal::ElementView::from(scrollbar_)
                    .signal_write(WriteType::Masked);
            }
        }
    }
    void ScrollBox::_update_layout_impl()
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
            if (scrollbar_->get<Slider::SliderWidth>() == sw)
            {
                scrollbar_->set<Slider::SliderWidth>(sw);
                written = true;
            }
        }
        else
            scrollbar_->setstate(Display, false);

        if (const auto ch = computed_height_ - (bw * 2) - height;
            scrollbar_->get<Slider::Max>() != ch)
        {
            scrollbar_->set<Slider::Min>(0);
            scrollbar_->set<Slider::Max>(ch);
            written = true;
        }
        if (written)
            internal::ElementView::from(scrollbar_)
                .signal_write(WriteType::Style);
    }

    TypedTreeIter<VerticalSlider> ScrollBox::scrollbar() const
    {
        return scrollbar_;
    }
    void ScrollBox::scroll_to(int xoffset)
    {
        offset_ = xoffset;
        _signal_write(WriteType::Style);
        foreach([](TreeIter ptr) {
            internal::ElementView::from(ptr)
            .signal_write(WriteType::LayoutYPos);
        });
    }

    void ScrollBox::foreach_internal(foreach_internal_callback callback) const
    {
        for (decltype(auto) it : _elements)
            callback(it);
        if (scrollbar_)
            callback(scrollbar_);
    }
}
