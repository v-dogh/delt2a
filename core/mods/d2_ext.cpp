#include "core/mods/d2_ext.hpp"
#include <core/screen/d2_screen.hpp>

namespace d2::sys
{
    SystemSelector::Status SystemSelector::_load_impl()
    {
        const auto ctx = context();
        _sig = ctx->connect<EventManifest>();
        auto weak = std::weak_ptr<SystemSelector>(std::static_pointer_cast<SystemSelector>(shared_from_this()));
        _events[0] = ctx->listen<screen::Event::MouseInput>([weak](in::InputFrame& frame)
        {
            if (auto self = weak.lock())
                self->_mouse(frame);
        });
        _events[1] = ctx->listen<screen::Event::MouseMoved>([weak](Position position)
        {
            if (auto self = weak.lock(); self && self->_selection.dragging())
            {
                self->_end = position;
                self->_selection.extend(position);
            }
        });
        _events[2] = ctx->listen<screen::Event::KeyInput>([weak](in::InputFrame& frame)
        {
            if (auto self = weak.lock())
                self->_key(frame);
        });
        _events[3] = ctx->listen<screen::Event::TreeSwap>([weak](std::string, std::string)
        {
            if (auto self = weak.lock())
                self->clear();
        });
        _events[4] = ctx->listen<screen::Event::Update>([weak]
        {
            if (auto self = weak.lock())
                self->_selection.refresh();
        });
        return Status::Ok;
    }
    void SystemSelector::_mouse(in::InputFrame& frame)
    {
        const auto map = frame.map();
        const auto left = std::size_t(in::mouse::Left);
        const auto pressed = map.current[left] && !map.previous[left];
        const auto released = !map.current[left] && map.previous[left];
        if (pressed)
        {
            _start = _end = frame.mouse_position();
            if (_selection.begin(context()->screen()->root().shared(), _start))
                frame.active(in::mouse::Left, in::mode::Press);
        }
        if (_selection.dragging() && released)
        {
            _end = frame.mouse_position();
            _selection.extend(_end);
            _selection.finish();
            frame.active(in::mouse::Left, in::mode::Release);
            if (!_selection.empty())
                context()->signal<Event::Selected>(_selection.value(), Position(_start), Position(_end));
        }
    }
    void SystemSelector::_key(in::InputFrame& frame)
    {
        if (_selection.empty())
            return;
        const auto focused = context()->screen()->focused();
        if (focused != nullptr && focused->provides_input())
            return;
        const auto map = frame.map();
        if (map.current[std::size_t(in::special::LeftControl)] ||
            map.current[std::size_t(in::special::RightControl)])
        {
            if (frame.active(in::key('c'), in::mode::Press))
                copy();
        }
        else if (frame.active(in::special::Escape, in::mode::Press))
            clear();
    }
    bool SystemSelector::copy()
    {
        const auto text = value();
        if (text.empty())
            return false;
        if (auto clipboard = context()->sys_if<SystemClipboard>())
        {
            clipboard->copy(text);
            return true;
        }
        return false;
    }
    SystemSelector::Status SystemSelector::_unload_impl()
    {
        for (auto& event : _events)
            event.close();
        _sig.disconnect();
        clear();
        return Status::Ok;
    }
} // namespace d2::sys
