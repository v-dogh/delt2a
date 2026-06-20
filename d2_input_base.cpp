#include "d2_input_base.hpp"

#include <utility>

namespace d2::in
{
    std::size_t InputFrame::_cur_poll_idx() const
    {
        return std::size_t(_poll_index);
    }
    std::size_t InputFrame::_prev_poll_idx() const
    {
        return std::size_t(!_poll_index);
    }

    bool InputFrame::had_event(Event ev) const
    {
        return _event_state & static_cast<unsigned char>(ev);
    }
    bool InputFrame::had_pulse() const
    {
        const auto& cur = _keys_poll[_cur_poll_idx()];
        return (_pulse & cur).any();
    }

    mouse_position InputFrame::mouse_position()
    {
        return _mouse_pos;
    }
    mouse_position InputFrame::scroll_delta()
    {
        if (_consume)
        {
            if (_active_consume.consumed_scroll && _consume_ctx == std::this_thread::get_id())
                return {0, 0};
            _consume_swap.consumed_scroll = true;
        }
        return _scroll_delta;
    }
    screen_size InputFrame::screen_size()
    {
        return _screen_size;
    }
    screen_size InputFrame::screen_capacity()
    {
        return _screen_capacity;
    }

    absl::InlinedVector<std::pair<keytype, mode>, 8> InputFrame::active_list() const
    {
        const auto c = _cur_poll_idx();
        const auto p = _prev_poll_idx();
        absl::InlinedVector<std::pair<keytype, mode>, 8> keys;
        const auto& cb = _keys_poll[c];
        const auto& pb = _keys_poll[p];
        for (std::size_t i = 0; i < cb.size(); i++)
        {
            if (!(_consume && _active_consume.keys_consumed.test(i)))
            {
                const auto ck = cb[i];
                const auto pk = pb[i];
                if (ck)
                    keys.push_back({keytype(i), mode::Hold});
                if (ck && !pk)
                    keys.push_back({keytype(i), mode::Press});
                else if (!ck && pk)
                    keys.push_back({keytype(i), mode::Release});
            }
        }
        return keys;
    }

    bool InputFrame::active(Mouse mouse, Mode mode)
    {
        return active(keytype(mouse), mode);
    }
    bool InputFrame::active(Special mod, Mode mode)
    {
        return active(keytype(mod), mode);
    }
    bool InputFrame::active(keytype key, Mode mode)
    {
        const auto idx = key;
        const auto c = _cur_poll_idx();
        const auto p = _prev_poll_idx();
        bool result = false;
        switch (mode)
        {
        case Mode::Press:
        {
            result = _keys_poll[c].test(idx) && !_keys_poll[p].test(idx) &&
                     !(_consume && _active_consume.keys_consumed.test(idx));
            break;
        }
        case Mode::Release:
        {
            result = (!_keys_poll[c].test(idx) && _keys_poll[p].test(idx)) &&
                     !(_consume && _active_consume.keys_consumed.test(idx));
            break;
        }
        case Mode::Hold:
        {
            result =
                _keys_poll[c].test(idx) && !(_consume && _active_consume.keys_consumed.test(idx));
            break;
        }
        }
        if (result && _consume && std::this_thread::get_id() == _consume_ctx)
            _consume_swap.keys_consumed.set(idx);
        return result;
    }

    InputFrame::ExportKeymap<InputFrame::keymap> InputFrame::map() const noexcept
    {
        return {.current = _keys_poll[_cur_poll_idx()], .previous = _keys_poll[_prev_poll_idx()]};
    }

    std::string_view InputFrame::sequence()
    {
        if (_consume)
        {
            if (_active_consume.consumed_sequence && _consume_ctx == std::this_thread::get_id())
                return "";
            _consume_swap.consumed_sequence = true;
        }
        return _sequence;
    }

    void InputFrame::mask()
    {
        _active_consume.consumed_scroll = true;
        _active_consume.consumed_sequence = true;
        _active_consume.keys_consumed.set();
    }

    namespace internal
    {
        void InputFrameView::swap()
        {
            _ptr->_poll_index = !_ptr->_poll_index;

            const auto old_idx = _ptr->_prev_poll_idx();
            const auto new_idx = _ptr->_cur_poll_idx();
            const auto& old_map = _ptr->_keys_poll[old_idx];
            auto& new_map = _ptr->_keys_poll[new_idx];
            const auto released = (old_map & ~new_map).any();

            new_map = old_map;
            new_map &= ~_ptr->_pulse;

            _ptr->_event_state = 0x00;
            if (released)
                _ptr->_event_state |= static_cast<unsigned char>(Event::KeyInput);
            _ptr->_scroll_delta = {0, 0};
            _ptr->_sequence.clear();
            _ptr->_pulse.reset();
        }

        void InputFrameView::set(Mouse mouse, bool value)
        {
            const auto idx = std::size_t(mouse);
            auto& cur = _ptr->_keys_poll[_ptr->_cur_poll_idx()];
            const auto old = cur.test(idx);
            if (old != value)
            {
                cur.set(idx, value);
                _ptr->_event_state |= static_cast<unsigned char>(Event::KeyMouseInput);
            }
        }
        void InputFrameView::set(Special mod, bool value)
        {
            set(keytype(mod), value);
        }
        void InputFrameView::set(keytype key, bool value)
        {
            const auto idx = std::size_t(key);
            auto& cur_keys = _ptr->_keys_poll[_ptr->_cur_poll_idx()];
            const auto old = cur_keys.test(idx);
            _ptr->_pulse.reset(idx);
            if (old != value)
            {
                cur_keys.set(idx, value);
                _ptr->_event_state |= static_cast<unsigned char>(Event::KeyInput);
            }
        }

        void InputFrameView::pulse(Mouse mouse)
        {
            const auto idx = std::size_t(mouse);
            set(mouse);
            _ptr->_pulse.set(idx);
        }
        void InputFrameView::pulse(Special mod)
        {
            pulse(keytype(mod));
        }
        void InputFrameView::pulse(keytype key)
        {
            const auto idx = std::size_t(key);
            set(key);
            _ptr->_pulse.set(idx);
        }

        void InputFrameView::set_scroll_delta(mouse_position pos)
        {
            if (_ptr->_scroll_delta != pos)
            {
                _ptr->_event_state |= static_cast<unsigned char>(Event::ScrollWheelMovement);
                _ptr->_scroll_delta = pos;
            }
        }
        void InputFrameView::set_mouse_position(mouse_position pos)
        {
            if (_ptr->mouse_position() != pos)
            {
                _ptr->_event_state |= static_cast<unsigned char>(Event::MouseMovement);
                _ptr->_mouse_pos = pos;
            }
        }
        void InputFrameView::set_screen_size(screen_size size)
        {
            if (_ptr->screen_size() != size)
            {
                _ptr->_event_state |= static_cast<unsigned char>(Event::ScreenResize);
                _ptr->_screen_size = size;
            }
        }
        void InputFrameView::set_screen_capacity(screen_size size)
        {
            if (_ptr->screen_capacity() != size)
            {
                _ptr->_event_state |= static_cast<unsigned char>(Event::ScreenCapacityChange);
                _ptr->_screen_capacity = size;
            }
        }
        void InputFrameView::set_text(std::string in)
        {
            _ptr->_sequence = std::move(in);

            if (!_ptr->_sequence.empty())
                _ptr->_event_state |= static_cast<unsigned char>(Event::KeySequenceInput);
        }

        std::pair<bool, std::thread::id> InputFrameView::get_consume()
        {
            return {_ptr->_consume, _ptr->_consume_ctx};
        }
        void InputFrameView::restore_consume(std::pair<bool, std::thread::id> data)
        {
            set_consume(data.first, data.second);
        }

        void InputFrameView::set_consume(bool value, std::thread::id ctx)
        {
            _ptr->_consume = value;
            _ptr->_consume_ctx = ctx;
        }
        void InputFrameView::reset_consume()
        {
            _ptr->_active_consume.keys_consumed.reset();
            _ptr->_active_consume.consumed_scroll = false;
            _ptr->_active_consume.consumed_sequence = false;
            _ptr->_consume_swap = _ptr->_active_consume;
        }
        void InputFrameView::apply_consume()
        {
            _ptr->_active_consume = _ptr->_consume_swap;
        }

        InputFrame::keymap InputFrameView::mask_key_consume(InputFrame::keymap mask)
        {
            return std::exchange(_ptr->_active_consume.keys_consumed, mask);
        }
    } // namespace internal
} // namespace d2::in
