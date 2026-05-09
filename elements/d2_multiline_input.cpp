#include "d2_multiline_input.hpp"

#include <d2_screen.hpp>
#include <d2_tree_element.hpp>
#include <mods/d2_ext.hpp>

#include <algorithm>
#include <cctype>

namespace d2::dx
{
    bool MultiInput::_ptr_overlapped() const
    {
        return bool(data::input_options & InputOptions::PtrOverlap);
    }

    bool MultiInput::_draw_ptr() const
    {
        return bool(data::input_options & InputOptions::DrawPtr) && getstate(Focused);
    }

    std::size_t MultiInput::_render_max_width(const DocModel& doc) const
    {
        auto width = doc.max_width;

        if (_draw_ptr() && !_ptr_overlapped())
        {
            const auto [ptr_line, ptr_col] = _line_col_from_abs(doc, _ptr_abs);
            if (ptr_line < _line_count(doc))
                width = std::max(width, _line_length(doc, ptr_line) + 1);
        }

        return std::max<std::size_t>(1, width);
    }

    void MultiInput::_write_ptr(Pixel& px)
    {
        if ((data::input_options & InputOptions::Blink) && !_ptr_blink)
        {
            px.v = ' ';
        }
        else
        {
            px = Pixel::combine(
                (data::input_options & InputOptions::AutoPtrColor)
                    ? data::foreground_color.extend(data::ptr_color.v)
                    : data::ptr_color,
                data::background_color
            );
        }
    }

    void MultiInput::_doc_dirty() const
    {
        _dirty_dm = true;
    }
    const MultiInput::DocModel& MultiInput::_doc_model() const
    {
        if (_dirty_dm)
        {
            DocModel doc{};

            _buffer.buf.iterate(
                [&](fb::Locale::Value v)
                {
                    const auto sv = v.value();

                    if (sv == "\n")
                    {
                        doc.length += sv.size();
                        doc.max_width = std::max(doc.max_width, doc.line_lengths.back());
                        doc.starts.push_back(doc.length);
                        doc.line_lengths.push_back(0);
                        return fb::FragmentedBuffer::It::Continue;
                    }

                    doc.line_lengths.back() += sv.size();
                    doc.length += sv.size();
                    doc.max_width = std::max(doc.max_width, doc.line_lengths.back());
                    return fb::FragmentedBuffer::It::Continue;
                }
            );

            doc.max_width = std::max<std::size_t>(1, doc.max_width);

            _dm_cache = std::move(doc);
            _dirty_dm = false;
        }

        return _dm_cache;
    }

    std::size_t MultiInput::_line_count(const DocModel& doc) const
    {
        return doc.starts.size();
    }

    std::size_t MultiInput::_line_length(const DocModel& doc, std::size_t line) const
    {
        if (line >= doc.line_lengths.size())
            return 0;

        return doc.line_lengths[line];
    }

    std::pair<std::size_t, std::size_t>
    MultiInput::_line_col_from_abs(const DocModel& doc, std::size_t abs) const
    {
        if (doc.starts.empty())
            return {0, 0};

        abs = std::min(abs, doc.length);

        auto it = std::upper_bound(doc.starts.begin(), doc.starts.end(), abs);
        if (it == doc.starts.begin())
            return {0, 0};

        const auto line = std::size_t(std::distance(doc.starts.begin(), it) - 1);
        const auto col = std::min<std::size_t>(abs - doc.starts[line], _line_length(doc, line));
        return {line, col};
    }

    std::size_t
    MultiInput::_abs_from_line_col(const DocModel& doc, std::size_t line, std::size_t col) const
    {
        if (doc.starts.empty())
            return 0;

        line = std::min(line, _line_count(doc) - 1);
        return doc.starts[line] + std::min<std::size_t>(col, _line_length(doc, line));
    }

    string MultiInput::_range_value(std::size_t pos, std::size_t len) const
    {
        string out{};

        if (!len)
            return out;

        out.reserve(len);

        std::size_t cursor = pos;
        _buffer.buf.iterate(
            [&](fb::Locale::Value v)
            {
                if (out.size() >= len)
                    return fb::FragmentedBuffer::It::Break;

                const auto sv = v.value();
                const auto left = len - out.size();
                const auto take = std::min(left, sv.size());

                out += string(sv.begin(), sv.begin() + take);
                cursor += take;

                return out.size() >= len ? fb::FragmentedBuffer::It::Break
                                         : fb::FragmentedBuffer::It::Continue;
            },
            pos
        );

        return out;
    }

    string MultiInput::_line_segment(
        const DocModel& doc, std::size_t line, std::size_t col, std::size_t len
    ) const
    {
        if (line >= _line_count(doc) || !len)
            return "";

        const auto line_len = _line_length(doc, line);

        if (col >= line_len)
            return "";

        len = std::min(len, line_len - col);
        return _range_value(doc.starts[line] + col, len);
    }

    bool MultiInput::_fixed_width() const
    {
        return data::width.getunits() != Unit::Auto;
    }

    bool MultiInput::_fixed_height() const
    {
        return data::height.getunits() != Unit::Auto;
    }

    int MultiInput::_vscroll_width() const
    {
        if (_vscrollbar == nullptr)
            return 1;

        const auto w = _vscrollbar->box().width;
        return std::max(1, w);
    }

    int MultiInput::_hscroll_height() const
    {
        if (_hscrollbar == nullptr)
            return 1;

        const auto h = _hscrollbar->box().height;
        return std::max(1, h);
    }

    MultiInput::Viewport MultiInput::_viewport(const DocModel& doc) const
    {
        const auto [boxw, boxh] = box();

        Viewport vp{std::size_t(std::max(0, boxw)), std::size_t(std::max(0, boxh)), false, false};

        const auto allow_v = data::enable_scrollbar_vertical && _fixed_height();
        const auto allow_h = data::enable_scrollbar_horizontal && _fixed_width();

        if (!allow_v && !allow_h)
            return vp;

        const auto vbar_w = std::size_t(_vscroll_width());
        const auto hbar_h = std::size_t(_hscroll_height());
        const auto render_width = _render_max_width(doc);

        for (int i = 0; i < 4; i++)
        {
            const auto avail_w = std::max<std::size_t>(1, vp.width);
            const auto avail_h = std::max<std::size_t>(1, vp.height);

            const auto next_v = allow_v && (_line_count(doc) > avail_h);
            const auto next_h = allow_h && (render_width > avail_w);

            if (next_v == vp.show_vscroll && next_h == vp.show_hscroll)
                break;

            vp.show_vscroll = next_v;
            vp.show_hscroll = next_h;

            vp.width = std::size_t(std::max(0, boxw - int(vp.show_vscroll ? vbar_w : 0)));
            vp.height = std::size_t(std::max(0, boxh - int(vp.show_hscroll ? hbar_h : 0)));
        }

        return vp;
    }

    void MultiInput::_scroll_x_to(std::size_t off)
    {
        const auto& doc = _doc_model();
        const auto vp = _viewport(doc);
        const auto render_width = _render_max_width(doc);
        const auto max_x = (render_width > std::max<std::size_t>(1, vp.width))
                               ? (render_width - std::max<std::size_t>(1, vp.width))
                               : 0;
        const auto next = std::min(off, max_x);

        if (_view_x == next)
            return;

        _view_x = next;

        if (_hscrollbar != nullptr)
        {
            _syncing_hscroll_ = true;
            _hscrollbar->reset_absolute(float(_view_x));
            _syncing_hscroll_ = false;
        }

        _signal_write(WriteType::Style);
    }

    void MultiInput::_scroll_y_to(std::size_t off)
    {
        const auto& doc = _doc_model();
        const auto vp = _viewport(doc);
        const auto max_y = (_line_count(doc) > std::max<std::size_t>(1, vp.height))
                               ? (_line_count(doc) - std::max<std::size_t>(1, vp.height))
                               : 0;
        const auto next = std::min(off, max_y);

        if (_view_y == next)
            return;

        _view_y = next;

        if (_vscrollbar != nullptr)
        {
            _syncing_vscroll_ = true;
            _vscrollbar->reset_absolute(float(_view_y));
            _syncing_vscroll_ = false;
        }

        _signal_write(WriteType::Style);
    }

    void MultiInput::_sync_scrollbars()
    {
        if (_vscrollbar == nullptr || _hscrollbar == nullptr)
            return;

        const auto& doc = _doc_model();
        const auto vp = _viewport(doc);
        const auto render_width = _render_max_width(doc);

        _vscrollbar->setstate(Display, vp.show_vscroll);
        _hscrollbar->setstate(Display, vp.show_hscroll);

        const auto max_y = (_line_count(doc) > std::max<std::size_t>(1, vp.height))
                               ? int(_line_count(doc) - std::max<std::size_t>(1, vp.height))
                               : 0;
        const auto max_x = (render_width > std::max<std::size_t>(1, vp.width))
                               ? int(render_width - std::max<std::size_t>(1, vp.width))
                               : 0;

        _view_y = std::min<std::size_t>(_view_y, std::size_t(std::max(0, max_y)));
        _view_x = std::min<std::size_t>(_view_x, std::size_t(std::max(0, max_x)));

        _vscrollbar->set<Slider::Min>(0);
        _vscrollbar->set<Slider::Max>(max_y);

        _hscrollbar->set<Slider::Min>(0);
        _hscrollbar->set<Slider::Max>(max_x);

        if (vp.show_vscroll)
        {
            const auto track_h = int(std::max<std::size_t>(1, vp.height));
            const auto doc_h = int(std::max<std::size_t>(1, _line_count(doc)));
            const auto thumb_h = std::max(1, (track_h * track_h) / std::max(1, doc_h));
            _vscrollbar->set<Slider::SliderWidth>(thumb_h);

            _syncing_vscroll_ = true;
            _vscrollbar->reset_absolute(float(_view_y));
            _syncing_vscroll_ = false;
        }

        if (vp.show_hscroll)
        {
            const auto track_w = int(std::max<std::size_t>(1, vp.width));
            const auto doc_w = int(std::max<std::size_t>(1, render_width));
            const auto thumb_w = std::max(1, (track_w * track_w) / std::max(1, doc_w));
            _hscrollbar->set<Slider::SliderWidth>(thumb_w);

            _syncing_hscroll_ = true;
            _hscrollbar->reset_absolute(float(_view_x));
            _syncing_hscroll_ = false;
        }
    }

    void MultiInput::_clamp_ptr()
    {
        const auto& doc = _doc_model();
        _ptr_abs = std::min(_ptr_abs, doc.length);
    }

    void MultiInput::_sync_ptr()
    {
        _clamp_ptr();
        _buffer.ptr.jump(_ptr_abs);
    }

    void MultiInput::_ensure_ptr_visible()
    {
        const auto& doc = _doc_model();
        const auto vp = _viewport(doc);
        const auto [line, col] = _line_col_from_abs(doc, _ptr_abs);

        const auto vw = std::max<std::size_t>(1, vp.width);
        const auto vh = std::max<std::size_t>(1, vp.height);

        if (line < _view_y)
            _scroll_y_to(line);
        else if (line >= _view_y + vh)
            _scroll_y_to(line - vh + 1);

        if (col < _view_x)
            _scroll_x_to(col);
        else if (col >= _view_x + vw)
            _scroll_x_to(col - vw + 1);

        if (_draw_ptr() && !_ptr_overlapped())
        {
            if (col + 1 > _view_x + vw)
                _scroll_x_to((col + 1) - vw);
        }

        _sync_scrollbars();
    }

    bool MultiInput::_is_highlighted() const
    {
        return _highlight_start != _highlight_end;
    }

    void MultiInput::_remove_highlighted()
    {
        if (_is_highlighted())
        {
            const auto lower = std::min(_highlight_start, _highlight_end);
            const auto upper = std::max(_highlight_start, _highlight_end);
            const auto diff = upper - lower;

            _buffer.act.erase(lower, diff);
            _doc_dirty();

            _ptr_abs = lower;
            _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;

            _sync_ptr();
            _reset_highlight();
            _ensure_ptr_visible();
            _signal_write(WriteType::Offset | WriteType::Style);
        }
    }

    void MultiInput::_begin_highlight_if()
    {
        if (!_is_highlighted())
        {
            _highlight_start = _ptr_abs;
            _highlight_end = _ptr_abs;
            _signal_write(WriteType::Style);
        }
    }

    void MultiInput::_reset_highlight()
    {
        if (_is_highlighted())
        {
            _highlight_start = 0;
            _highlight_end = 0;
            _signal_write(WriteType::Style);
        }
    }

    string MultiInput::_highlighted_value() const
    {
        if (!_is_highlighted())
            return "";

        const auto lower = std::min(_highlight_start, _highlight_end);
        const auto upper = std::max(_highlight_start, _highlight_end);

        return _range_value(lower, upper - lower);
    }

    void MultiInput::_start_blink()
    {
        if ((data::input_options & InputOptions::Blink) && getstate(State::Focused) &&
            _btask == nullptr)
        {
            _btask = context()->scheduler()->launch_cyclic(
                std::chrono::milliseconds(500),
                [this](auto)
                {
                    _ptr_blink = !_ptr_blink;
                    _signal_write(WriteType::Style);
                }
            );
        }
    }

    void MultiInput::_stop_blink()
    {
        if (_btask != nullptr)
        {
            _btask.discard();
            _btask.sync();
            _btask = nullptr;
        }

        _ptr_blink = false;
        _signal_write(WriteType::Style);
    }

    void MultiInput::_freeze_blink()
    {
        if (_btask != nullptr)
        {
            _btask.discard();
            _btask = nullptr;
        }
    }

    void MultiInput::_unfreeze_blink()
    {
        if (getstate(State::Focused) && _btask == nullptr)
            _start_blink();
    }

    std::size_t MultiInput::_mouse_abs_pos() const
    {
        const auto& doc = _doc_model();
        const auto vp = _viewport(doc);
        const auto [mx, my] = mouse_object_space();

        if (!_line_count(doc) || vp.width == 0 || vp.height == 0)
            return 0;

        const auto lx = std::size_t(std::clamp(mx, 0, int(vp.width) - 1));
        const auto ly = std::size_t(std::clamp(my, 0, int(vp.height) - 1));

        const auto line = std::min<std::size_t>(_view_y + ly, _line_count(doc) - 1);

        auto col = _view_x + lx;

        if (_draw_ptr() && !_ptr_overlapped())
        {
            const auto [ptr_line, ptr_col] = _line_col_from_abs(doc, _ptr_abs);
            if (line == ptr_line && col > ptr_col)
                col--;
        }

        col = std::min<std::size_t>(col, _line_length(doc, line));
        return _abs_from_line_col(doc, line, col);
    }

    void MultiInput::_move_to(std::size_t pos, bool keep_highlight)
    {
        if (!keep_highlight)
            _reset_highlight();

        _ptr_abs = pos;
        _sync_ptr();

        const auto& doc = _doc_model();
        _preferred_x = _line_col_from_abs(doc, _ptr_abs).second;

        _ensure_ptr_visible();
        _signal_write(WriteType::Style);
    }

    void MultiInput::_move_right(std::size_t cnt)
    {
        const auto& doc = _doc_model();
        _move_to(std::min(_ptr_abs + cnt, doc.length));
    }

    void MultiInput::_move_left(std::size_t cnt)
    {
        _move_to(_ptr_abs >= cnt ? _ptr_abs - cnt : 0);
    }

    void MultiInput::_move_up(std::size_t cnt)
    {
        const auto& doc = _doc_model();
        auto [line, col] = _line_col_from_abs(doc, _ptr_abs);

        if (line == 0)
            return;

        line = (cnt > line) ? 0 : line - cnt;
        _move_to(_abs_from_line_col(doc, line, _preferred_x));
    }

    void MultiInput::_move_down(std::size_t cnt)
    {
        const auto& doc = _doc_model();
        auto [line, col] = _line_col_from_abs(doc, _ptr_abs);

        if (!_line_count(doc))
            return;

        line = std::min<std::size_t>(line + cnt, _line_count(doc) - 1);
        _move_to(_abs_from_line_col(doc, line, _preferred_x));
    }

    void MultiInput::_insert(std::string_view value)
    {
        if (_is_highlighted())
            _remove_highlighted();

        _buffer.act.insert(_ptr_abs, value);
        _doc_dirty();

        _ptr_abs += value.size();

        _sync_ptr();
        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
        _ensure_ptr_visible();

        _signal_write(WriteType::Masked);
    }

    void MultiInput::_erase_before(std::size_t len)
    {
        if (_is_highlighted())
        {
            _remove_highlighted();
            return;
        }

        if (!len || !_ptr_abs)
            return;

        len = std::min(len, _ptr_abs);
        _buffer.act.erase(_ptr_abs - len, len);
        _doc_dirty();

        _ptr_abs -= len;

        _sync_ptr();
        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
        _ensure_ptr_visible();

        _signal_write(WriteType::Masked);
    }

    void MultiInput::_erase_after(std::size_t len)
    {
        if (_is_highlighted())
        {
            _remove_highlighted();
            return;
        }

        const auto& doc = _doc_model();
        if (!len || _ptr_abs >= doc.length)
            return;

        len = std::min(len, doc.length - _ptr_abs);
        _buffer.act.erase(_ptr_abs, len);
        _doc_dirty();

        _sync_ptr();
        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
        _ensure_ptr_visible();

        _signal_write(WriteType::Masked);
    }

    void MultiInput::_signal_write_impl(write_flag type, unsigned int prop, ptr element)
    {
        MetaParentElement::_signal_write_impl(type, prop, element);

        if (_vscrollbar == nullptr || _hscrollbar == nullptr)
            return;

        if (prop == initial_property || prop == data::EnableScrollVert ||
            prop == data::EnableScrollHoriz || prop == data::Width || prop == data::Height)
        {
            _update_layout_impl();
            _sync_scrollbars();
        }
    }

    bool MultiInput::_provides_input_impl() const
    {
        return true;
    }

    Unit MultiInput::_layout_impl(Element::Layout type) const
    {
        const auto& doc = _doc_model();
        switch (type)
        {
        case Element::Layout::X:
            return data::x;
        case Element::Layout::Y:
            return data::y;
        case Element::Layout::Width:
            if (data::width.getunits() == Unit::Auto)
                return HDUnit(int(_render_max_width(doc)));
            return data::width;
        case Element::Layout::Height:
            if (data::height.getunits() == Unit::Auto)
                return VDUnit(int(std::max<std::size_t>(1, _line_count(doc))));
            return data::height;
        }

        return data::width;
    }

    void MultiInput::_state_change_impl(State state, bool value)
    {
        if (state == State::Focused)
        {
            if (value)
            {
                _start_blink();
            }
            else
            {
                _stop_blink();
                _reset_highlight();
            }
        }
        else if (state == State::Clicked && value)
        {
            _move_to(_mouse_abs_pos());
            _begin_highlight_if();
        }
        else if (state == State::Swapped)
        {
            if (value)
                _freeze_blink();
            else
                _unfreeze_blink();
        }
        else if (state == State::Created && value)
        {
            _buffer.buf.locale<fb::loc::Byte>();
            _buffer.buf.setup_buffer("");
            _doc_dirty();

            if (_vscrollbar == nullptr)
            {
                _vscrollbar = Element::make<VerticalSlider>("", this->state());
                _insert_setstate(_vscrollbar);

                _vscrollbar->set<Slider::ZIndex>(5);
                _vscrollbar->set<Slider::Height>(1.0_pc);
                _vscrollbar->set<Slider::OnSubmit>(
                    [this](auto, auto, auto abs)
                    {
                        if (_syncing_vscroll_)
                            return;

                        _view_y = std::max<std::size_t>(0, abs);
                        _signal_write(WriteType::Style);
                    }
                );
                _vscrollbar->setstate(Display, false);
            }

            if (_hscrollbar == nullptr)
            {
                _hscrollbar = Element::make<Slider>("", this->state());
                _insert_setstate(_hscrollbar);

                _hscrollbar->set<Slider::ZIndex>(5);
                _hscrollbar->set<Slider::Width>(1.0_pc);
                _hscrollbar->set<Slider::OnSubmit>(
                    [this](auto, auto, auto abs)
                    {
                        if (_syncing_hscroll_)
                            return;

                        _view_x = std::max<std::size_t>(0, abs);
                        _signal_write(WriteType::Style);
                    }
                );
                _hscrollbar->setstate(Display, false);
            }

            _update_layout_impl();
            _sync_scrollbars();
        }
        else if (state == State::Created && !value)
        {
            if (data::input_options & InputOptions::Censor)
                clear();
        }
        MetaParentElement::_state_change_impl(state, value);
    }

    void MultiInput::_event_impl(in::InputFrame& frame)
    {
        write_flag flag = WriteType::Style;
        bool write = false;

        if ((frame.had_event(in::Event::MouseMovement) ||
             frame.had_event(in::Event::KeyMouseInput)) &&
            !(_vscrollbar->getstate(Clicked) || _hscrollbar->getstate(Clicked)))
        {
            if (frame.active(in::mouse::Left, in::mode::Hold))
            {
                const auto next = _mouse_abs_pos();

                if (_highlight_end != next || _ptr_abs != next)
                {
                    _highlight_end = next;
                    _ptr_abs = next;
                    _sync_ptr();
                    _ensure_ptr_visible();
                    _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
                    write = true;
                }
            }
        }
        if (frame.had_event(in::Event::ScrollWheelMovement) && getstate(RcHover) &&
            !_vscrollbar->getstate(Hovered) && !_hscrollbar->getstate(Hovered))
        {
            const auto& doc = _doc_model();
            const auto vp = _viewport(doc);

            const auto [dx, dy] = frame.scroll_delta();

            if (vp.show_vscroll && dy != 0)
            {
                const auto max_y = (_line_count(doc) > std::max<std::size_t>(1, vp.height))
                                       ? int(_line_count(doc) - std::max<std::size_t>(1, vp.height))
                                       : 0;

                _scroll_y_to(std::size_t(std::clamp(int(_view_y) + dy, 0, max_y)));
                write = true;
            }

            if (vp.show_hscroll && dx != 0)
            {
                const auto render_width = _render_max_width(doc);
                const auto max_x = (render_width > std::max<std::size_t>(1, vp.width))
                                       ? int(render_width - std::max<std::size_t>(1, vp.width))
                                       : 0;

                _scroll_x_to(std::size_t(std::clamp(int(_view_x) + dx, 0, max_x)));
                write = true;
            }
            else if (vp.show_hscroll && !vp.show_vscroll && dy != 0)
            {
                const auto render_width = _render_max_width(doc);
                const auto max_x = (render_width > std::max<std::size_t>(1, vp.width))
                                       ? int(render_width - std::max<std::size_t>(1, vp.width))
                                       : 0;

                _scroll_x_to(std::size_t(std::clamp(int(_view_x) + dy, 0, max_x)));
                write = true;
            }
        }
        if (frame.had_event(in::Event::KeyInput))
        {
            if (frame.active(in::special::Escape, in::mode::Hold))
            {
                screen()->focus(nullptr);
            }
            if (frame.active(in::special::Enter, in::mode::Hold) &&
                !(data::input_options & ReadOnly))
            {
                if (frame.active(in::special::LeftControl, in::mode::Hold))
                {
                    if (data::on_submit != nullptr)
                        data::on_submit(
                            std::static_pointer_cast<MultiInput>(shared_from_this()), value()
                        );
                }
                else
                {
                    _insert("\n");
                    flag |= WriteType::Offset;
                    write = true;
                }
            }
            if (frame.active(in::special::Backspace, in::mode::Hold) &&
                !(data::input_options & ReadOnly))
            {
                _erase_before();
                flag |= WriteType::Offset;
                write = true;
            }
            if (frame.active(in::special::Delete, in::mode::Hold) &&
                !(data::input_options & ReadOnly))
            {
                _erase_after();
                flag |= WriteType::Offset;
                write = true;
            }
            if (frame.active(in::special::Home, in::mode::Hold))
            {
                const auto& doc = _doc_model();
                const auto ctrl = frame.active(in::special::LeftControl, in::mode::Hold);
                const auto shift = frame.active(in::special::Shift, in::mode::Hold);

                if (shift)
                    _begin_highlight_if();
                else
                    _reset_highlight();

                const auto target = [&]() -> std::size_t
                {
                    if (ctrl)
                        return 0;

                    const auto [line, col] = _line_col_from_abs(doc, _ptr_abs);
                    return _abs_from_line_col(doc, line, 0);
                }();

                _ptr_abs = target;
                if (shift)
                    _highlight_end = _ptr_abs;

                _sync_ptr();
                _preferred_x = _line_col_from_abs(doc, _ptr_abs).second;
                _ensure_ptr_visible();
                write = true;
            }
            if (frame.active(in::special::End, in::mode::Hold))
            {
                const auto& doc = _doc_model();
                const auto ctrl = frame.active(in::special::LeftControl, in::mode::Hold);
                const auto shift = frame.active(in::special::Shift, in::mode::Hold);

                if (shift)
                    _begin_highlight_if();
                else
                    _reset_highlight();

                const auto target = [&]() -> std::size_t
                {
                    if (ctrl)
                        return doc.length;

                    const auto [line, col] = _line_col_from_abs(doc, _ptr_abs);
                    return _abs_from_line_col(doc, line, _line_length(doc, line));
                }();

                _ptr_abs = target;
                if (shift)
                    _highlight_end = _ptr_abs;

                _sync_ptr();
                _preferred_x = _line_col_from_abs(doc, _ptr_abs).second;
                _ensure_ptr_visible();
                write = true;
            }
            if (frame.active(in::special::ArrowLeft, in::mode::Hold))
            {
                if (frame.active(in::special::Shift, in::mode::Hold))
                {
                    _begin_highlight_if();
                    if (_ptr_abs > 0)
                    {
                        _ptr_abs--;
                        _highlight_end = _ptr_abs;
                        _sync_ptr();
                        _ensure_ptr_visible();
                        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
                        write = true;
                    }
                }
                else
                    _move_left();
            }
            if (frame.active(in::special::ArrowRight, in::mode::Hold))
            {
                const auto& doc = _doc_model();
                if (frame.active(in::special::Shift, in::mode::Hold))
                {
                    _begin_highlight_if();
                    if (_ptr_abs < doc.length)
                    {
                        _ptr_abs++;
                        _highlight_end = _ptr_abs;
                        _sync_ptr();
                        _ensure_ptr_visible();
                        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
                        write = true;
                    }
                }
                else
                    _move_right();
            }
            if (frame.active(in::special::ArrowUp, in::mode::Hold))
            {
                if (frame.active(in::special::Shift, in::mode::Hold))
                {
                    _begin_highlight_if();
                    const auto before = _ptr_abs;
                    _move_up();
                    _highlight_end = _ptr_abs;
                    write = (before != _ptr_abs);
                }
                else
                    _move_up();
            }
            if (frame.active(in::special::ArrowDown, in::mode::Hold))
            {
                if (frame.active(in::special::Shift, in::mode::Hold))
                {
                    _begin_highlight_if();
                    const auto before = _ptr_abs;
                    _move_down();
                    _highlight_end = _ptr_abs;
                    write = (before != _ptr_abs);
                }
                else
                    _move_down();
            }
            if (frame.active(in::special::LeftControl, in::mode::Hold))
            {
                auto* clipboard = context()->sys_if<sys::ext::SystemClipboard>();
                if (clipboard)
                {
                    if (frame.active(in::key('a'), in::mode::Press))
                    {
                        const auto& doc = _doc_model();
                        _highlight_start = 0;
                        _highlight_end = doc.length;
                        _ptr_abs = doc.length;
                        _sync_ptr();
                        _ensure_ptr_visible();
                        write = true;
                    }
                    else if (frame.active(in::key('c'), in::mode::Press) && _is_highlighted())
                    {
                        clipboard->copy(_highlighted_value());
                    }
                    else if (
                        frame.active(in::key('x'), in::mode::Press) && _is_highlighted() &&
                        !(data::input_options & ReadOnly)
                    )
                    {
                        clipboard->copy(_highlighted_value());
                        _remove_highlighted();
                        flag |= WriteType::Offset;
                        write = true;
                    }
                    else if (
                        frame.active(in::key('v'), in::mode::Press) &&
                        !(data::input_options & ReadOnly)
                    )
                    {
                        if (!clipboard->empty())
                        {
                            _insert(clipboard->paste());
                            flag |= WriteType::Offset;
                            write = true;
                        }
                    }
                    else if (
                        frame.active(in::key('z'), in::mode::Press) &&
                        !(data::input_options & ReadOnly)
                    )
                    {
                        undo();
                        flag |= WriteType::Offset;
                        write = true;
                    }
                    else if (
                        frame.active(in::key('y'), in::mode::Press) &&
                        !(data::input_options & ReadOnly)
                    )
                    {
                        redo();
                        flag |= WriteType::Offset;
                        write = true;
                    }
                }
            }
        }

        if (frame.had_event(in::Event::KeySequenceInput) && !(data::input_options & ReadOnly))
        {
            auto seq = std::string(frame.sequence());

            if (!seq.empty())
            {
                if (frame.active(in::special::Shift, in::mode::Hold))
                {
                    std::transform(
                        seq.cbegin(),
                        seq.cend(),
                        seq.begin(),
                        [](char ch) -> char { return std::toupper(static_cast<unsigned char>(ch)); }
                    );
                }

                _insert(seq);
                flag |= WriteType::Offset;
                write = true;
            }
        }

        if (write)
        {
            _sync_scrollbars();
            _signal_write(flag);
        }
    }

    void MultiInput::_frame_impl(PixelBuffer::View buffer)
    {
        const auto color = Pixel::combine(data::foreground_color, data::background_color);
        const auto& doc = _doc_model();

        buffer.fill(data::background_color);

        _clamp_ptr();

        const auto vp = _viewport(doc);
        const auto [ptr_line, ptr_col] = _line_col_from_abs(doc, _ptr_abs);
        const auto hlower = std::min(_highlight_start, _highlight_end);
        const auto hupper = std::max(_highlight_start, _highlight_end);
        const auto draw_ptr = _draw_ptr();
        const auto ptr_overlapped = _ptr_overlapped();

        bool has_visible_text = false;

        for (std::size_t vy = 0; vy < vp.height; vy++)
        {
            const auto line_index = _view_y + vy;
            if (line_index >= _line_count(doc))
                break;

            const auto line_start = doc.starts[line_index];
            const auto line_len = _line_length(doc, line_index);
            const auto line_has_ptr = draw_ptr && (line_index == ptr_line);

            auto seg_col = _view_x;
            if (line_has_ptr && !ptr_overlapped && seg_col > ptr_col)
                seg_col--;

            const auto segment = _line_segment(doc, line_index, seg_col, vp.width + 1);

            for (std::size_t vx = 0; vx < vp.width; vx++)
            {
                const auto screen_col = _view_x + vx;

                if (line_has_ptr && !ptr_overlapped && screen_col == ptr_col)
                {
                    auto& px = buffer.at(vx, vy);
                    _write_ptr(px);
                    has_visible_text = true;
                    continue;
                }

                auto src_col = screen_col;

                if (line_has_ptr && !ptr_overlapped && screen_col > ptr_col)
                    src_col--;

                if (src_col >= line_len)
                    continue;

                const auto seg_index = src_col - seg_col;
                if (seg_index >= segment.size())
                    continue;

                has_visible_text = true;

                auto& px = buffer.at(vx, vy);
                const auto abs = line_start + src_col;

                if (abs >= hlower && abs < hupper)
                    px.blend(color.mask(data::marked_mask));
                else
                    px.blend(color);

                px.v = (data::input_options & InputOptions::Censor) ? '*' : segment[seg_index];
            }
        }

        if (!has_visible_text && !data::hint.empty() && !draw_ptr && vp.width > 0 && vp.height > 0)
        {
            const auto hcolor = data::foreground_color.alpha(0.5f);
            for (std::size_t i = 0; i < std::min<std::size_t>(vp.width, data::hint.size()); i++)
            {
                buffer.at(i, 0).blend(
                    hcolor.stylize(d2::px::foreground::Style::Bold).extend(data::hint[i])
                );
            }
        }

        if (draw_ptr && ptr_overlapped && ptr_line >= _view_y && ptr_line < _view_y + vp.height &&
            ptr_col >= _view_x && ptr_col < _view_x + vp.width)
        {
            auto& px = buffer.at(ptr_col - _view_x, ptr_line - _view_y);
            _write_ptr(px);
        }

        if (vp.show_vscroll && _vscrollbar != nullptr)
        {
            const auto frame = _vscrollbar->frame();
            const auto [x, y] = _vscrollbar->position();
            buffer.inscribe(x, y, frame.buffer());
        }

        if (vp.show_hscroll && _hscrollbar != nullptr)
        {
            const auto frame = _hscrollbar->frame();
            const auto [x, y] = _hscrollbar->position();
            buffer.inscribe(x, y, frame.buffer());
        }
    }

    void MultiInput::_update_layout_impl()
    {
        if (_vscrollbar == nullptr || _hscrollbar == nullptr)
            return;

        const auto& doc = _doc_model();
        const auto vp = _viewport(doc);
        const auto [bw, bh] = box();

        const auto vbar_w = _vscroll_width();
        const auto hbar_h = _hscroll_height();

        _vscrollbar->set<Slider::X>(HDUnit(std::max(0, bw - vbar_w)));
        _vscrollbar->set<Slider::Y>(0.0_px);
        _vscrollbar->set<Slider::Height>(VDUnit(int(vp.height)));

        _hscrollbar->set<Slider::X>(0.0_px);
        _hscrollbar->set<Slider::Y>(VDUnit(std::max(0, bh - hbar_h)));
        _hscrollbar->set<Slider::Width>(HDUnit(int(vp.width)));

        _sync_scrollbars();
    }

    void MultiInput::foreach_internal(foreach_internal_callback callback) const
    {
        if (_vscrollbar != nullptr)
            callback(_vscrollbar);
        if (_hscrollbar != nullptr)
            callback(_hscrollbar);
    }

    string MultiInput::value() const
    {
        string out{};

        _buffer.buf.iterate(
            [&](fb::Locale::Value v)
            {
                const auto sv = v.value();
                out += string(sv.begin(), sv.end());
                return fb::FragmentedBuffer::It::Continue;
            }
        );

        return out;
    }

    void MultiInput::value(std::string text)
    {
        _buffer.buf.setup_buffer(std::move(text));
        _doc_dirty();

        _ptr_abs = 0;
        _preferred_x = 0;
        _view_x = 0;
        _view_y = 0;
        _highlight_start = 0;
        _highlight_end = 0;
        _sync_ptr();
        _update_layout_impl();
        _sync_scrollbars();
        _signal_write(WriteType::Offset | WriteType::Style);
    }

    void MultiInput::clear()
    {
        value("");
    }

    void MultiInput::undo()
    {
        _buffer.history.revert();
        _doc_dirty();

        _clamp_ptr();
        _sync_ptr();
        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
        _ensure_ptr_visible();
        _reset_highlight();
        _sync_scrollbars();
        _signal_write(WriteType::Offset | WriteType::Style);
    }

    void MultiInput::redo()
    {
        _buffer.history.reapply();
        _doc_dirty();

        _clamp_ptr();
        _sync_ptr();
        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
        _ensure_ptr_visible();
        _reset_highlight();
        _sync_scrollbars();
        _signal_write(WriteType::Offset | WriteType::Style);
    }

    void MultiInput::jump(std::size_t pos)
    {
        _move_to(pos);
    }

    bool MultiInput::is_synced() const noexcept
    {
        const auto& doc = _doc_model();
        return _ptr_abs == doc.length;
    }
    void MultiInput::sync()
    {
        const auto& doc = _doc_model();
        jump(doc.length);
    }

    void MultiInput::append(std::string_view text)
    {
        if (text.empty())
            return;

        const auto& old_doc = _doc_model();
        const auto old_length = old_doc.length;

        const auto old_ptr = _ptr_abs;
        const auto old_pref = _preferred_x;
        const auto old_view_x = _view_x;
        const auto old_view_y = _view_y;
        const auto old_hstart = _highlight_start;
        const auto old_hend = _highlight_end;

        _buffer.act.insert(old_length, text);
        _doc_dirty();

        _ptr_abs = std::min(old_ptr, old_length);
        _preferred_x = old_pref;
        _view_x = old_view_x;
        _view_y = old_view_y;
        _highlight_start = std::min(old_hstart, old_length);
        _highlight_end = std::min(old_hend, old_length);

        _sync_ptr();
        _sync_scrollbars();
        _signal_write(WriteType::Offset | WriteType::Style);
    }

    void MultiInput::insert(std::string_view text)
    {
        _insert(text);
    }

    void MultiInput::erase(std::size_t len)
    {
        _erase_after(len);
    }

    void MultiInput::erase_at(std::size_t pos, std::size_t len)
    {
        _buffer.act.erase(pos, len);
        _doc_dirty();

        const auto& doc = _doc_model();
        _ptr_abs = std::min(_ptr_abs, doc.length);

        _sync_ptr();
        _preferred_x = _line_col_from_abs(doc, _ptr_abs).second;
        _ensure_ptr_visible();
        _sync_scrollbars();

        _signal_write(WriteType::Offset | WriteType::Style);
    }

    void MultiInput::disable_history()
    {
        _buffer.history.disable();
    }

    void MultiInput::enable_history()
    {
        _buffer.history.enable();
    }

    std::size_t MultiInput::cursor() const
    {
        return _ptr_abs;
    }

    d2::TreeIter<VerticalSlider> MultiInput::vertical_scollbar() const noexcept
    {
        return _vscrollbar;
    }
    d2::TreeIter<Slider> MultiInput::horizontal_scrollbar() const noexcept
    {
        return _hscrollbar;
    }

    const fb::FragmentedBuffer& MultiInput::buffer() const noexcept
    {
        return _buffer;
    }
} // namespace d2::dx
