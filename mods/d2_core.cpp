#include "d2_core.hpp"

namespace d2::sys
{
    SystemInput::FrameLock::FrameLock(SystemInput* ptr, bool mask) : _ptr(ptr), _mask(mask)
    {
        if (ptr)
        {
            ptr->_lock_frame_impl();
            if (mask)
            {
                _consume_state =
                    in::internal::InputFrameView::from(ptr->_frame_impl().get()).get_consume();
                in::internal::InputFrameView::from(ptr->_frame_impl().get()).reset_consume();
            }
        }
    }
    SystemInput::FrameLock::FrameLock(FrameLock&& copy) :
        _ptr(copy._ptr), _consume_state(copy._consume_state), _mask(copy._mask)
    {
        copy._ptr = nullptr;
        copy._mask = false;
    }
    SystemInput::FrameLock::~FrameLock()
    {
        if (_ptr)
        {
            if (_mask)
                in::internal::InputFrameView::from(_ptr->_frame_impl().get())
                    .restore_consume(_consume_state);
            _ptr->_unlock_frame_impl();
        }
    }

    SystemInput::FrameLock& SystemInput::FrameLock::operator=(FrameLock&& copy)
    {
        if (_ptr != nullptr)
            _ptr->_unlock_frame_impl();
        _ptr = copy._ptr;
        _consume_state = copy._consume_state;
        _mask = copy._mask;
        copy._ptr = nullptr;
        copy._mask = false;
        return *this;
    }

    SystemInput::FramePtr::FramePtr(in::InputFrame::ptr ptr, FrameLock lock) :
        _ptr(ptr), _lock(std::move(lock))
    {
    }

    in::InputFrame* SystemInput::FramePtr::get() const
    {
        return _ptr.get();
    }

    in::InputFrame& SystemInput::FramePtr::operator*() const
    {
        return *_ptr;
    }
    in::InputFrame* SystemInput::FramePtr::operator->() const
    {
        return _ptr.get();
    }

    SystemInput::FramePtr SystemInput::frame()
    {
        return FramePtr(_frame_impl(), this);
    }
    MainWorker::ptr SystemInput::worker()
    {
        return _worker_impl();
    }

    void SystemInput::begincycle()
    {
        _begincycle_impl();
    }
    void SystemInput::endcycle()
    {
        _endcycle_impl();
    }

    void SystemInput::enable()
    {
        _enabled = true;
    }
    void SystemInput::disable()
    {
        _enabled = false;
    }

    bool SystemInput::had_event(in::Event ev)
    {
        FrameLock lock(this, true);
        if (!_enabled && (ev == in::Event::KeyInput || ev == in::Event::KeyMouseInput ||
                          ev == in::Event::KeySequenceInput || ev == in::Event::MouseMovement))
            return false;
        return _frame_impl()->had_event(ev);
    }

    in::mouse_position SystemInput::mouse_position()
    {
        FrameLock lock(this, true);
        return _enabled ? _frame_impl()->mouse_position() : in::mouse_position{-1, -1};
    }
    in::screen_size SystemInput::screen_capacity()
    {
        FrameLock lock(this, true);
        return _frame_impl()->screen_capacity();
    }
    in::screen_size SystemInput::screen_size()
    {
        FrameLock lock(this, true);
        return _frame_impl()->screen_size();
    }

    bool SystemInput::active(in::Mouse mouse, in::Mode mode)
    {
        FrameLock lock(this, true);
        return _enabled && _frame_impl()->active(mouse, mode);
    }
    bool SystemInput::active(in::Special mod, in::Mode mode)
    {
        FrameLock lock(this, true);
        return _enabled && _frame_impl()->active(mod, mode);
    }
    bool SystemInput::active(in::keytype key, in::Mode mode)
    {
        FrameLock lock(this, true);
        return _enabled && _frame_impl()->active(key, mode);
    }

    std::string_view SystemInput::sequence()
    {
        FrameLock lock(this, true);
        if (_enabled)
            return _frame_impl()->sequence();
        return "";
    }
} // namespace d2::sys
