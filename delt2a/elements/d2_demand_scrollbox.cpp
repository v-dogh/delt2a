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
    bool DemandScrollBox::_provides_input_impl() const
    {
        return true;
    }
    void DemandScrollBox::_event_impl(sys::screen::Event ev)
    {
        const auto ch = scrollbar()->get<Slider::Max>();
        if (ev == sys::screen::Event::MouseInput)
        {
            if (getstate(Hovered))
            {
                if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollUp))
                {
                    if (_offset <= ch)
                        scroll_to(_offset + 1);
                }
                else if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollDown))
                {
                    if (_offset > 0)
                        scroll_to(_offset - 1);
                }
            }
        }
        else if (ev == sys::screen::Event::KeyInput)
        {
            if (context()->input()->is_pressed(sys::SystemInput::ArrowLeft) ||
                context()->input()->is_pressed(sys::SystemInput::ArrowDown))
            {
                if (_offset <= ch)
                    scroll_to(_offset + 1);
            }
            else if (context()->input()->is_pressed(sys::SystemInput::ArrowRight) ||
                     context()->input()->is_pressed(sys::SystemInput::ArrowUp))
            {
                if (_offset > 0)
                    scroll_to(_offset - 1);
            }
        }
    }

    TreeIter<VerticalSlider> DemandScrollBox::scrollbar() const
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
        const auto bw = (data::container_options & EnableBorder) ? resolve_units(data::border_width) : 0;
        const int height = int(layout(d2::Element::Layout::Height)) - int(bw) * 2;
        const int ypad   = int(resolve_units(data::padding_vertical));

        if (data::length == 0 || height <= 0)
        {
            _view.clear();
            _view_base = 0;
            _top_index = 0;
            _top_inset = 0;
            _zero_start = ypad;
            _prev_offset = int(_offset);
            return;
        }
        if (_height_cache.size() != data::length)
            _height_cache.assign(data::length, -1);

        // Get new element in view
        auto fetch = [&](std::size_t i) -> Element::ptr
        {
            seed seed(i, std::static_pointer_cast<DemandScrollBox>(shared_from_this()));
            data::fetch_callback(seed);
            auto elem = seed._get();
            if (elem)
            {
                const auto h = int(elem->layout(d2::Element::Layout::Height));
                _height_cache[i] = h;
                _avg_height =
                    (_avg_height * double(_avg_height_cnt) + double(h)) /
                    double(_avg_height_cnt + 1);
                ++_avg_height_cnt;
            }
            return elem;
        };
        auto estimated_height = [&](std::size_t i) -> int
        {
            const auto m = _height_cache[i];
            if (m >= 0)
                return m;
            if (data::static_height_hint.raw() != 0.0)
                return int(resolve_units(data::static_height_hint));
            if (_avg_height_cnt)
                return int(_avg_height);
            return 1;
        };
        auto span = [&](std::size_t i) -> int
        {
            return estimated_height(i) + ypad;
        };

        const auto new_off_px = int(_offset);
        const auto dy = new_off_px - _prev_offset;

        if (data::static_height_hint.raw() != 0.f)
        {
            const auto stat = int(resolve_units(data::static_height_hint));
            const auto item_span = stat + ypad;

            auto off = new_off_px;
            if (off < 0)
                off = 0;

            auto idx = std::size_t(off / item_span);
            if (idx >= data::length)
                idx = data::length - 1;

            auto inset = int(off - int(idx) * item_span);
            if (inset < 0)
                inset = 0;
            if (inset >= item_span)
                inset = item_span - 1;

            _top_index = idx;
            _top_inset = inset;
        }
        else
        {
            _top_inset += dy;
            while (_top_index + 1 < data::length)
            {
                if (_height_cache[_top_index] < 0)
                    fetch(_top_index);

                const auto sp = span(_top_index);
                if (_top_inset < sp)
                    break;

                _top_inset -= sp;
                ++_top_index;
            }
            while (_top_inset < 0 && _top_index > 0)
            {
                --_top_index;
                if (_height_cache[_top_index] < 0)
                    fetch(_top_index);
                _top_inset += span(_top_index);
            }

            const auto sp = span(_top_index);
            if (_top_inset < 0) _top_inset = 0;
            if (_top_inset >= sp) _top_inset = sp - 1;
        }

        std::size_t vis_first = _top_index;
        std::size_t vis_last_excl = vis_first;
        int y = -_top_inset;
        while (vis_last_excl < data::length && y < height)
        {
            if (_height_cache[vis_last_excl] < 0)
                fetch(vis_last_excl);
            y += span(vis_last_excl);
            ++vis_last_excl;
        }

        if (vis_last_excl == vis_first)
            vis_last_excl = std::min<std::size_t>(vis_first + 1, data::length);

        const auto vis_count = vis_last_excl - vis_first;
        const auto view_count = vis_count + 2 * data::preload;
        const auto new_base = int(vis_first) - int(data::preload);

        // Reuse old views
        const auto old_base = _view_base;
        const auto old_last = old_base + int(_view.size());

        auto old_at = [&](int i) -> std::shared_ptr<d2::Element>
        {
            if (i < old_base || i >= old_last)
                return nullptr;
            return _view[std::size_t(i - old_base)];
        };

        std::vector<std::shared_ptr<d2::Element>> new_view;
        new_view.resize(view_count);

        for (std::size_t i = 0; i < view_count; ++i)
        {
            const auto idx = new_base + int(i);
            if (idx < 0 || idx >= int(data::length))
            {
                new_view[i] = nullptr;
                continue;
            }

            if (auto reused = old_at(idx);
                reused != nullptr)
            {
                new_view[i] = reused;
            }
            else
            {
                new_view[i] = fetch(idx);
            }
        }

        _view = std::move(new_view);
        _view_base = new_base;

        int above = 0;
        for (std::size_t i = 0; i < data::preload; ++i)
        {
            const auto idx =
                int(vis_first) -
                int(data::preload) +
                int(i);
            if (idx < 0)
                continue;
            above += span(idx);
        }

        _zero_start = ypad - above - _top_inset;
        _prev_offset = new_off_px;
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
        if (!_view.empty())
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
            const auto sm = _scrollbar->get<Slider::Max>();
            const auto est_height = ((float(logical_height) / logical_count) * data::length);
            const auto rsw = std::max(1, std::min<int>(
                slider_height,
                slider_height * height / est_height
            ));
            // Update slider and render scrollbar
            if (est_height > height || !data::hide_slider)
            {
                _scrollbar->setstate(Display, true);
                if (sw != rsw)
                    _scrollbar->set<Slider::SliderWidth>(rsw);
                if (sm != est_height - height)
                    _scrollbar->set<Slider::Max>(est_height - height);
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
