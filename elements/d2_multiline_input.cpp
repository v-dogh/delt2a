#include "elements/d2_multiline_input.hpp"

#include <core/mods/d2_ext.hpp>
#include <core/screen/d2_screen.hpp>
#include <core/tree/d2_tree_element.hpp>

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

        if (_draw_ptr())
        {
            const auto [ptr_line, ptr_col] = _line_col_from_abs(doc, _ptr_abs);
            if (ptr_line < _line_count(doc))
                width = std::max({width, _line_length(doc, ptr_line) + !_ptr_overlapped(), ptr_col + 1});
        }

        return std::max<std::size_t>(1, width);
    }

    void MultiInput::_write_ptr(pixel& px)
    {
        if ((data::input_options & InputOptions::Blink) && !_ptr_blink)
        {
            px = pixel::combine(data::foreground_color.extend(' '), data::background_color);
        }
        else
        {
            px = pixel::combine(
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
        if (_dirty_dm || _dm_cache.text.censor != bool(data::input_options & Censor))
        {
            _dm_cache.text.reset(value(), data::input_options & Censor, true);
            _dm_cache.length = _dm_cache.text.source.size();
            _dm_cache.max_width = std::max(1, _dm_cache.text.display().bounds().width);
            _dirty_dm = false;
        }
        return _dm_cache;
    }
    std::size_t MultiInput::_line_count(const DocModel& doc) const
    {
        return doc.text.display().lines().size();
    }
    std::size_t MultiInput::_line_length(const DocModel& doc, std::size_t line) const
    {
        return line < _line_count(doc) ? doc.text.display().lines()[line].columns : 0;
    }
    std::pair<std::size_t, std::size_t>
    MultiInput::_line_col_from_abs(const DocModel& doc, std::size_t abs) const
    {
        const auto position = doc.text.display().caret(std::min(abs, doc.length));
        return {std::size_t(position.y), std::size_t(position.x)};
    }
    std::size_t
    MultiInput::_abs_from_line_col(const DocModel& doc, std::size_t line, std::size_t col) const
    {
        line = std::min(line, _line_count(doc) - 1);
        return doc.text.display().hit_test({int(std::min(col, _line_length(doc, line))), int(line)});
    }
    string MultiInput::_range_value(std::size_t pos, std::size_t len) const
    {
        const auto& doc = _doc_model();
        pos = std::min(pos, doc.length);
        return std::string(doc.text.source.extract({pos, pos + std::min(len, doc.length - pos)}));
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
        _buffer.ptr.jump(_doc_model().text.source.byte_offset(_ptr_abs));
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

    void MultiInput::_replace(std::size_t begin, std::size_t end, std::string_view value,
                              std::optional<std::size_t> cursor_after)
    {
        const auto& doc = _doc_model();
        begin = std::min(begin, doc.length);
        end = std::clamp(end, begin, doc.length);
        const auto byte = doc.text.source.byte_offset(begin);
        _sync_ptr();
        _buffer.act.replace(byte, doc.text.source.byte_offset(end) - byte, value, cursor_after);
        _doc_dirty();
        _ptr_abs = _doc_model().text.source.grapheme_at(_buffer.ptr.absolute_offset(), text::Affinity::Downstream);
        _sync_ptr();
        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
        _reset_highlight();
        _ensure_ptr_visible();
        _signal_write(WriteType::Masked);
    }
    void MultiInput::_remove_highlighted()
    {
        if (_is_highlighted())
            _replace(std::min(_highlight_start, _highlight_end), std::max(_highlight_start, _highlight_end), "");
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
                [weak = weak_from_this()](mt::future<void> task)
                {
                    if (auto element = weak.lock())
                    {
                        auto& self = static_cast<MultiInput&>(*element);
                        self._ptr_blink = !self._ptr_blink;
                        self._signal_write(WriteType::Style);
                    }
                    else
                        task.discard();
                }
            );
        }
    }

    MultiInput::~MultiInput()
    {
        _btask.discard();
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
        _clamp_ptr();
        _move_to(_ptr_abs + std::min(cnt, doc.length - _ptr_abs));
    }

    void MultiInput::_move_left(std::size_t cnt)
    {
        _move_to(_ptr_abs >= cnt ? _ptr_abs - cnt : 0);
    }

    void MultiInput::_move_up(std::size_t cnt, bool select)
    {
        const auto& doc = _doc_model();
        const auto line = _line_col_from_abs(doc, _ptr_abs).first;
        const auto preferred = _preferred_x;
        if (select)
            _begin_highlight_if();
        _move_to(_abs_from_line_col(doc, line - std::min(cnt, line), preferred), select);
        if (select)
            _highlight_end = _ptr_abs;
        _preferred_x = preferred;
    }
    void MultiInput::_move_down(std::size_t cnt, bool select)
    {
        const auto& doc = _doc_model();
        const auto line = _line_col_from_abs(doc, _ptr_abs).first;
        const auto preferred = _preferred_x;
        if (select)
            _begin_highlight_if();
        _move_to(_abs_from_line_col(doc, line + std::min(cnt, _line_count(doc) - 1 - line), preferred), select);
        if (select)
            _highlight_end = _ptr_abs;
        _preferred_x = preferred;
    }
    void MultiInput::_insert(std::string_view value)
    {
        if (value.empty())
            return;
        _replace(_is_highlighted() ? std::min(_highlight_start, _highlight_end) : _ptr_abs,
                 _is_highlighted() ? std::max(_highlight_start, _highlight_end) : _ptr_abs, value);
    }
    void MultiInput::_erase_before(std::size_t len)
    {
        if (_is_highlighted())
            _remove_highlighted();
        else if (len && _ptr_abs)
            _replace(_ptr_abs - std::min(len, _ptr_abs), _ptr_abs, "");
    }
    void MultiInput::_erase_after(std::size_t len)
    {
        if (_is_highlighted())
            _remove_highlighted();
        else if (len && _ptr_abs < _doc_model().length)
            _replace(_ptr_abs, _ptr_abs + std::min(len, _doc_model().length - _ptr_abs), "");
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
            _signal_write(WriteType::Dimensions);
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
        _clamp_ptr();
        const auto keys = frame.map().current;
        const auto shift = keys.test(std::size_t(in::special::Shift));
        const auto ctrl = keys.test(std::size_t(in::special::LeftControl));
        write_flag flag = WriteType::Style;
        bool write = false;

        if ((frame.had_event(in::Event::MouseMovement) ||
             frame.had_event(in::Event::KeyMouseInput)) &&
            !((_vscrollbar && _vscrollbar->getstate(Clicked)) || (_hscrollbar && _hscrollbar->getstate(Clicked))))
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
            !(_vscrollbar && _vscrollbar->getstate(Hovered)) && !(_hscrollbar && _hscrollbar->getstate(Hovered)))
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
                if (ctrl)
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
                if (shift)
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
                if (shift)
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
                _move_up(1, shift);
            if (frame.active(in::special::ArrowDown, in::mode::Hold))
                _move_down(1, shift);
            if (ctrl)
            {
                if (frame.active(in::key('a'), in::mode::Press))
                {
                    _highlight_start = 0;
                    _highlight_end = _ptr_abs = _doc_model().length;
                    _sync_ptr();
                    _ensure_ptr_visible();
                    write = true;
                }
                else if (frame.active(in::key('z'), in::mode::Press) && !(data::input_options & ReadOnly))
                    undo();
                else if (frame.active(in::key('y'), in::mode::Press) && !(data::input_options & ReadOnly))
                    redo();
                else if (auto ctx = state() ? context() : nullptr; ctx)
                {
                    if (auto clipboard = ctx->sys_if<sys::SystemClipboard>(); clipboard)
                    {
                        if (frame.active(in::key('c'), in::mode::Press) && _is_highlighted())
                            clipboard->copy(_highlighted_value());
                        else if (frame.active(in::key('x'), in::mode::Press) && _is_highlighted() &&
                                 !(data::input_options & ReadOnly))
                        {
                            clipboard->copy(_highlighted_value());
                            _remove_highlighted();
                        }
                        else if (frame.active(in::key('v'), in::mode::Press) && !(data::input_options & ReadOnly))
                            _insert(clipboard->paste());
                    }
                }
            }
        }
        if (frame.had_event(in::Event::KeySequenceInput) && !(data::input_options & ReadOnly) && !ctrl)
            _insert(frame.sequence());

        if (write)
        {
            _sync_scrollbars();
            _signal_write(flag);
        }
    }

    void MultiInput::_frame_impl(PixelBuffer::View buffer)
    {
        buffer.fill(data::background_color);
        if (buffer.empty())
            return;
        _clamp_ptr();
        const auto color = pixel::combine(data::foreground_color, data::background_color);
        const auto& doc = _doc_model();
        const auto vp = _viewport(doc);
        auto content = PixelBuffer::View(buffer, 0, 0, int(vp.width), int(vp.height));
        const auto& layout = doc.text.display();
        const auto gap = _draw_ptr() && !_ptr_overlapped();
        layout.draw(content, color, {-int(_view_x), -int(_view_y)},
                    text::Range{_highlight_start, _highlight_end}, data::highlight_mask,
                    gap ? std::optional(_ptr_abs) : std::nullopt);
        if (!doc.length && !_draw_ptr())
            text::Layout(data::hint).draw(content,
                pixel::combine(data::foreground_color.alpha(0.5f).stylize(px::style::Bold), data::background_color));
        const auto position = layout.caret(_ptr_abs) - Position{int(_view_x), int(_view_y)};
        if (_draw_ptr() && position.x >= 0 && position.x < int(vp.width) &&
            position.y >= 0 && position.y < int(vp.height))
        {
            if (!gap && _ptr_abs < layout.size())
                for (int i = 0; i < layout.glyphs()[_ptr_abs].columns && position.x + i < int(vp.width); ++i)
                    content.at(position.x + i, position.y) = pixel::combine(data::foreground_color.extend(' '), data::background_color);
            _write_ptr(content.at(position.x, position.y));
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
        _buffer.buf.iterate_raw([&](std::string_view part) { out += part; return true; });
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
        _signal_write(WriteType::Masked);
    }

    void MultiInput::clear()
    {
        value("");
    }

    void MultiInput::undo()
    {
        _buffer.history.revert();
        _doc_dirty();

        const auto& layout = _doc_model().text.source;
        _ptr_abs = layout.grapheme_at(std::min(_buffer.ptr.absolute_offset(), layout.source().size()), text::Affinity::Downstream);
        _sync_ptr();
        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
        _ensure_ptr_visible();
        _reset_highlight();
        _sync_scrollbars();
        _signal_write(WriteType::Masked);
    }

    void MultiInput::redo()
    {
        _buffer.history.reapply();
        _doc_dirty();

        const auto& layout = _doc_model().text.source;
        _ptr_abs = layout.grapheme_at(std::min(_buffer.ptr.absolute_offset(), layout.source().size()), text::Affinity::Downstream);
        _sync_ptr();
        _preferred_x = _line_col_from_abs(_doc_model(), _ptr_abs).second;
        _ensure_ptr_visible();
        _reset_highlight();
        _sync_scrollbars();
        _signal_write(WriteType::Masked);
    }

    void MultiInput::jump(std::size_t pos)
    {
        _move_to(pos);
    }

    bool MultiInput::is_synced() const
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

        _buffer.act.replace(old_doc.text.source.byte_offset(old_length), 0, text,
                            old_doc.text.source.byte_offset(old_ptr));
        _doc_dirty();

        _ptr_abs = std::min(old_ptr, old_length);
        _preferred_x = old_pref;
        _view_x = old_view_x;
        _view_y = old_view_y;
        _highlight_start = std::min(old_hstart, old_length);
        _highlight_end = std::min(old_hend, old_length);

        _sync_ptr();
        _sync_scrollbars();
        _signal_write(WriteType::Masked);
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
        const auto& doc = _doc_model();
        pos = std::min(pos, doc.length);
        len = std::min(len, doc.length - pos);
        if (!len)
            return;
        const auto old_byte = doc.text.source.byte_offset(_ptr_abs);
        const auto begin = doc.text.source.byte_offset(pos);
        const auto end = doc.text.source.byte_offset(pos + len);
        const auto next = old_byte <= begin ? old_byte : (old_byte < end ? begin : old_byte - (end - begin));
        _replace(pos, pos + len, "", next);
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

    d2::TreeIter<VerticalSlider> MultiInput::vertical_scrollbar() const noexcept
    {
        return _vscrollbar;
    }
    d2::TreeIter<VerticalSlider> MultiInput::vertical_scollbar() const noexcept
    {
        return vertical_scrollbar();
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
