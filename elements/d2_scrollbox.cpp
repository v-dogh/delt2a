#include "d2_scrollbox.hpp"

namespace d2::dx
{
    void ScrollBox::_recompute_height() const
    {
        if (needs_recompute_)
        {
            computed_height_ = 0;
            foreach_internal(
                [this](TreeIter<> it)
                {
                    if (it != _scrollbar && it->getstate(Display))
                    {
                        computed_height_ = std::max(
                            computed_height_,
                            it->layout(Element::Layout::Y) + offset_ +
                                it->layout(Element::Layout::Height)
                        );
                    }
                    return true;
                }
            );
        }
    }

    int ScrollBox::_get_border_impl(BorderType type, Element::cptr elem) const
    {
        if (elem != _scrollbar && type == BorderType::Right)
        {
            const auto sw = _scrollbar->box().width;
            return Box::_get_border_impl(type, elem) + sw;
        }
        return Box::_get_border_impl(type, elem);
    }
    void ScrollBox::_layout_for_impl(Element::Layout type, cptr ptr) const
    {
        const auto off = resolve_units(ptr->internal_layout(type), ptr);
        ptr->override_layout(
            type, off - (offset_ * (type == Element::Layout::Y && ptr != _scrollbar))
        );
    }

    void ScrollBox::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        if (type & WriteType::LayoutHeight)
            needs_recompute_ = true;
    }
    void ScrollBox::_state_change_impl(State state, bool value)
    {
        if (state == State::Created && value)
        {
            if (_scrollbar == nullptr)
            {
                _scrollbar = Element::make<VerticalSlider>("", this->state());
                _insert_setstate(_scrollbar);
                _scrollbar->set<Slider::ZIndex>(5);
                _scrollbar->set<Slider::X>(0.0_pxi);
                _scrollbar->set<Slider::Y>(0.0_px);
                _scrollbar->set<Slider::Height>(1.0_pc);
                _scrollbar->set<Slider::OnSubmit>(
                    [this](auto ptr, auto, auto abs)
                    {
                        offset_ = abs;
                        _signal_write(WriteType::Style);
                        foreach (
                            [](TreeIter<> ptr)
                            {
                                internal::ElementView::from(ptr).signal_write(
                                    WriteType::LayoutYPos
                                );
                                return true;
                            }
                        )
                            ;
                    }
                );
                internal::ElementView::from(_scrollbar).signal_write(WriteType::Masked);
            }
        }
    }
    bool ScrollBox::_provides_input_impl() const
    {
        return true;
    }
    void ScrollBox::_event_impl(in::InputFrame& frame)
    {
        const auto height = layout(Element::Layout::Height);
        const auto bw =
            (Box::container_options & Box::EnableBorder) ? resolve_units(Box::border_width) : 0;
        const auto wh = height - (bw * 2);
        const auto ch = std::max(0, computed_height_ - wh);
        if (frame.had_event(in::Event::ScrollWheelMovement))
        {
            if (getstate(RcHover))
            {
                const auto scroll = frame.scroll_delta().second;
                scroll_to(std::clamp(offset_ + scroll, 0, ch));
            }
        }
        if (frame.had_event(in::Event::KeyInput))
        {
            if (frame.active(in::special::ArrowLeft, in::mode::Hold) ||
                frame.active(in::special::ArrowDown, in::mode::Hold))
            {
                if (offset_ <= ch)
                    scroll_to(offset_ + 1);
            }
            else if (
                frame.active(in::special::ArrowRight, in::mode::Hold) ||
                frame.active(in::special::ArrowUp, in::mode::Hold)
            )
            {
                if (offset_ > 0)
                    scroll_to(offset_ - 1);
            }
        }
    }

    void ScrollBox::_update_layout_impl()
    {
        const auto height = layout(Element::Layout::Height);
        const auto bw =
            (Box::container_options & Box::EnableBorder) ? resolve_units(Box::border_width) : 0;
        const auto wh = height - (bw * 2);

        _recompute_height();

        bool written = false;
        if (computed_height_ > wh)
        {
            const auto sw =
                computed_height_ ? std::max<int>(1, ((float(wh) / computed_height_) * wh)) : 0;
            _scrollbar->setstate(Display, true);
            if (_scrollbar->get<Slider::SliderWidth>() != sw)
            {
                _scrollbar->set<Slider::SliderWidth>(sw);
                written = true;
            }
        }
        else
            _scrollbar->setstate(Display, false);

        if (const auto ch = std::max(0, computed_height_ - wh);
            _scrollbar->get<Slider::Max>() != ch)
        {
            _scrollbar->set<Slider::Min>(0);
            _scrollbar->set<Slider::Max>(ch);
            written = true;
        }
        if (written)
            internal::ElementView::from(_scrollbar).signal_write(WriteType::Style);
    }

    TreeIter<VerticalSlider> ScrollBox::scrollbar() const
    {
        return _scrollbar;
    }
    void ScrollBox::scroll_to(int xoffset)
    {
        _recompute_height();
        const auto height = layout(Element::Layout::Height);
        const auto bw =
            (Box::container_options & Box::EnableBorder) ? resolve_units(Box::border_width) : 0;
        const auto wh = height - (bw * 2);
        const auto ch = std::max(0, computed_height_ - wh);
        D2_EXPECT(xoffset >= 0 && xoffset <= ch)
        xoffset = std::clamp(xoffset, 0, ch);
        _scrollbar->reset_absolute(xoffset);
        _signal_write(WriteType::Style);
        foreach (
            [](TreeIter<> ptr)
            {
                internal::ElementView::from(ptr).signal_write(WriteType::LayoutYPos);
                return true;
            }
        )
            ;
    }
    int ScrollBox::internal_height() const
    {
        _recompute_height();
        return computed_height_;
    }
    int ScrollBox::window_height() const
    {
        const auto height = layout(Element::Layout::Height);
        const auto bw =
            (Box::container_options & Box::EnableBorder) ? resolve_units(Box::border_width) : 0;
        return height - (bw * 2);
    }
    int ScrollBox::scroll_offset() const noexcept
    {
        return offset_;
    }
    void ScrollBox::sync()
    {
        _recompute_height();
        const auto height = layout(Element::Layout::Height);
        const auto bw =
            (Box::container_options & Box::EnableBorder) ? resolve_units(Box::border_width) : 0;
        const auto wh = height - (bw * 2);
        const auto ch = std::max(0, computed_height_ - wh);
        scroll_to(ch);
    }

    void ScrollBox::foreach_internal(foreach_internal_callback callback) const
    {
        for (decltype(auto) it : _elements)
            callback(it);
        if (_scrollbar)
            callback(_scrollbar);
    }
} // namespace d2::dx
