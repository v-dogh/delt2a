#include "d2_input_base.hpp"
#include <utility>

namespace d2::in
{
    std::size_t InputFrame::_cur_poll_idx() const
    {
        return std::size_t(_event_state & static_cast<unsigned char>(Event::Reserved));
    }
    std::size_t InputFrame::_prev_poll_idx() const
    {
        return std::size_t(!(_event_state & static_cast<unsigned char>(Event::Reserved)));
    }

    bool InputFrame::had_event(Event ev)
    {
        return _event_state & static_cast<unsigned char>(ev);
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

    bool InputFrame::active(Mouse mouse, Mode mode)
    {
        const auto idx = std::size_t(mouse);
        const auto c = _cur_poll_idx();
        const auto p = _prev_poll_idx();
        bool result = false;
        switch (mode)
        {
        case Mode::Press:
        {
            result = _mouse_poll[c].test(idx) && !_mouse_poll[p].test(idx) &&
                     !(_consume && _active_consume.mouse_consumed.test(idx));
            break;
        }
        case Mode::Release:
        {
            result = !_mouse_poll[c].test(idx) && _mouse_poll[p].test(idx) &&
                     !(_consume && _active_consume.mouse_consumed.test(idx));
            break;
        }
        case Mode::Hold:
        {
            result =
                _mouse_poll[c].test(idx) && !(_consume && _active_consume.mouse_consumed.test(idx));
            break;
        }
        }
        if (result && _consume && std::this_thread::get_id() == _consume_ctx)
            _consume_swap.mouse_consumed.set(idx);
        return result;
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
            result = !_keys_poll[c].test(idx) && _keys_poll[p].test(idx) &&
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

    InputFrame::ExportKeymap<InputFrame::keyboard_keymap> InputFrame::keyboard_map() const noexcept
    {
        return {.current = _keys_poll[_cur_poll_idx()], .previous = _keys_poll[_prev_poll_idx()]};
    }
    InputFrame::ExportKeymap<InputFrame::mouse_keymap> InputFrame::mouse_map() const noexcept
    {
        return {.current = _mouse_poll[_cur_poll_idx()], .previous = _mouse_poll[_prev_poll_idx()]};
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

    namespace internal
    {
        void InputFrameView::swap()
        {
            const auto ref = _ptr->_event_state & static_cast<unsigned char>(Event::Reserved);
            _ptr->_event_state = 0x00;
            if (!ref)
                _ptr->_event_state |= static_cast<unsigned char>(Event::Reserved);
            _ptr->_scroll_delta = {0, 0};
            _ptr->_sequence.clear();
            reset_consume();
            // Reset key state
            {
                auto& mpoll = _ptr->_mouse_poll[_ptr->_cur_poll_idx()];
                _ptr->_keys_poll[_ptr->_cur_poll_idx()].reset();
                mpoll = _ptr->_mouse_poll[_ptr->_prev_poll_idx()];
            }
        }
        void InputFrameView::swap(const InputFrame* ptr)
        {
            *_ptr = *ptr;
            swap();
        }

        void InputFrameView::set(Mouse mouse, bool value)
        {
            _ptr->_event_state |= static_cast<unsigned char>(Event::KeyMouseInput);
            _ptr->_mouse_poll[_ptr->_cur_poll_idx()].set(std::size_t(mouse), value);
        }
        void InputFrameView::set(Special mod, bool value)
        {
            _ptr->_event_state |= static_cast<unsigned char>(Event::KeyInput);
            _ptr->_keys_poll[_ptr->_cur_poll_idx()].set(std::size_t(mod), value);
        }
        void InputFrameView::set(keytype key, bool value)
        {
            _ptr->_event_state |= static_cast<unsigned char>(Event::KeyInput);
            _ptr->_keys_poll[_ptr->_cur_poll_idx()].set(key, value);
        }

        void InputFrameView::set_scroll_delta(mouse_position pos)
        {
            if (_ptr->scroll_delta() != pos)
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
            _ptr->_event_state |= static_cast<unsigned char>(Event::KeySequenceInput);
            _ptr->_sequence = std::move(in);
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
            _ptr->_active_consume.mouse_consumed.reset();
            _ptr->_active_consume.consumed_scroll = false;
            _ptr->_active_consume.consumed_sequence = false;
            _ptr->_consume_swap = _ptr->_active_consume;
        }
        void InputFrameView::apply_consume()
        {
            _ptr->_active_consume = _ptr->_consume_swap;
        }

        InputFrame::keyboard_keymap
        InputFrameView::mask_keyboard_consume(InputFrame::keyboard_keymap mask)
        {
            return std::exchange(_ptr->_active_consume.keys_consumed, mask);
        }
        InputFrame::mouse_keymap InputFrameView::mask_mouse_consume(InputFrame::mouse_keymap mask)
        {
            return std::exchange(_ptr->_active_consume.mouse_consumed, mask);
        }
    } // namespace internal
} // namespace d2::in
