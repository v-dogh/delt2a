#include "d2_demand_scrollbox.hpp"

namespace d2::dx
{
    // implement preloading
    // resize the slider thumb width based on length/window ratio etc. etc.

    int DemandScrollBox::_get_border_impl(BorderType type, cptr elem) const
    {
        const auto bw = data::container_options & EnableBorder ? resolve_units(data::border_width) : 0;
        if (elem != _scrollbar && type == BorderType::Right && _scrollbar->getstate(Display))
        {
            const auto sw = _scrollbar->box().width;
            return bw + sw;
        }
        return bw;
    }
    Unit DemandScrollBox::_layout_impl(enum Element::Layout type) const
    {
        switch (type)
        {
        case Element::Layout::X: return data::x;
        case Element::Layout::Y: return data::y;
        case Element::Layout::Width: return data::width;
        case Element::Layout::Height: return data::height;
        }
    }
    void DemandScrollBox::_layout_for_impl(enum Element::Layout type, cptr ptr) const
    {
        if (ptr == _scrollbar)
        {
            ParentElement::_layout_for_impl(type, ptr);
        }
        else
        {
            switch (type)
            {
            case Element::Layout::Y: ptr->override_layout(type, 0); return;
            case Element::Layout::X: [[ fallthrough ]];
            case Element::Layout::Width: [[ fallthrough ]];
            case Element::Layout::Height: ParentElement::_layout_for_impl(type, ptr);
            }
        }
    }

    void DemandScrollBox::_signal_write_impl(write_flag type, unsigned int prop, ptr)
    {
        if (prop == initial_property || prop == Length)
            _scrollbar->set<Slider::Max>(data::length);
    }
    void DemandScrollBox::_state_change_impl(State state, bool value)
    {
        if (state == State::Created && value && _scrollbar == nullptr)
        {
            _scrollbar = d2::Element::make<VerticalSlider>(
                "",
                this->state(),
                std::static_pointer_cast<ParentElement>(shared_from_this())
            );
            _scrollbar->setstate(State::Swapped, false);
            _scrollbar->set<Slider::X>(0.0_pxi);
            _scrollbar->set<Slider::Y>(0.0_px);
            _scrollbar->set<Slider::Height>(1.0_pc);
            _scrollbar->set<Slider::OnSubmit>([this](auto ptr, auto, auto abs)
            {
                _offset = abs;
                _signal_write(WriteType::Style);
            });
            internal::ElementView::from(_scrollbar)
                .signal_write(WriteType::Masked);
        }
        else if (state == State::Swapped && data::swap_out)
        {
            _view.clear();
            _view.shrink_to_fit();
        }
    }

    TypedTreeIter<VerticalSlider> DemandScrollBox::scrollbar() const
    {
        return _scrollbar;
    }
    void DemandScrollBox::scroll_to(int offset)
    {
        _scrollbar->reset_absolute(offset);
    }

    void DemandScrollBox::foreach(foreach_callback callback) const noexcept
    {
        for (decltype(auto) it : _view)
            callback(it);
    }
    void DemandScrollBox::foreach_internal(foreach_internal_callback callback) const
    {
        callback(_scrollbar);
        for (decltype(auto) it : _view)
            callback(it);
    }

    void DemandScrollBox::_update_view()
    {
        // Pre disable scrollbar if we have hint
        const auto bw = (data::container_options & EnableBorder) ? resolve_units(data::border_width) : 0;
        const auto height = layout(d2::Element::Layout::Height) - bw * 2;
        const auto xpad = resolve_units(data::padding_horizontal);
        const auto ypad = resolve_units(data::padding_vertical);
        if (data::static_height_hint.raw() != 0.0 && data::hide_slider)
        {
            const auto stat = resolve_units(data::static_height_hint);
            const auto est_height = (stat + ypad) * data::length;
            if (est_height <= height)
                _scrollbar->setstate(Display, false);
        }
        // Update the view
        {
            if (_offset != _prev_offset || _view.empty())
            {
                const auto ptr = std::static_pointer_cast<DemandScrollBox>(shared_from_this());
                auto push = [&](std::size_t off, std::size_t idx) {
                    seed seed(off, ptr);
                    data::fetch_callback(seed);
                    _view[idx] = seed._get();
                };

                if (_view.empty())
                {
                    _view.reserve(height + data::preload * 2);

                    std::size_t accum = ypad;
                    std::size_t idx = 0;
                    while (accum < height && idx < data::length)
                    {
                        _view.push_back(nullptr);
                        push(_offset + idx++, _view.size() - 1);
                        if (_view.back())
                        {
                            accum += _view.back()
                            ->layout(d2::Element::Layout::Height);
                            accum += ypad;
                        }
                    }
                }
                else
                {

                }
            }
            _prev_offset = _offset;
        }
    }
    void DemandScrollBox::_frame_impl(PixelBuffer::View buffer)
    {
        _update_view();

        buffer.fill(
            Pixel::combine(
                data::foreground_color,
                data::background_color
            )
        );

        // Render elements
        const auto bw = (data::container_options & EnableBorder) ? resolve_units(data::border_width) : 0;
        const auto height = layout(d2::Element::Layout::Height) - bw * 2;
        const auto xpad = resolve_units(data::padding_horizontal);
        const auto ypad = resolve_units(data::padding_vertical);
        std::size_t logical_height = 0;
        {
            for (std::size_t i = data::preload; i < _view.size() - data::preload; i++)
            {
                logical_height += ypad;
                auto& it = _view[i];
                const auto frame = it->frame();
                const auto [ x, y ] = it->position();
                const auto [ width, height ] = it->box();
                it->override_position(x + xpad, logical_height);
                buffer.inscribe(
                    x + xpad, logical_height,
                    frame.buffer()
                );
                logical_height += height;
            }
        }
        // Render scrollbar
        {
            // Update the slider width
            const auto logical_count = _view.size() - (data::preload * 2);
            const auto slider_height = layout(Element::Layout::Height);
            const auto sw = _scrollbar->get<Slider::SliderWidth>();
            const auto est_height = ((float(logical_height) / logical_count) * data::length);
            const auto rsw = std::max<int>(
                slider_height,
                slider_height * height / est_height
            );
            // Update slider and render scrollbar
            if (est_height > height || !data::hide_slider)
            {
                _scrollbar->setstate(Display, true);
                if (sw != rsw)
                    _scrollbar->set<Slider::SliderWidth>(rsw);
                const auto frame = _scrollbar->frame();
                const auto [ x, y ] = _scrollbar->position();
                buffer.inscribe(x, y, frame.buffer());
            }
            else
                _scrollbar->setstate(Display, false);
        }
        ContainerHelper::_render_border(buffer);
    }
}
