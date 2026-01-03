#include "d2_tree_element.hpp"
#include "d2_tree_parent.hpp"
#include "d2_exceptions.hpp"
#include <typeinfo>
#include <vector>

namespace d2
{
    int Element::LayoutStorage::get(enum Layout comp) const
    {
        return _storage[static_cast<std::size_t>(comp)];
    }
    void Element::LayoutStorage::set(enum Layout comp, int value)
    {
        _storage[static_cast<std::size_t>(comp)] = value;
    }

    // Traversal Wrapper

    bool TreeIter::is_type_of(TreeIter other) const
    {
        if (_elem.expired())
            return false;
        return typeid(other._elem.lock().get()) == typeid(_elem.lock().get());
    }

    TreeIter TreeIter::up() const
    {
        return TreeIter(_elem.lock()->parent());
    }
    TreeIter TreeIter::up(std::size_t cnt) const
    {
        if (!cnt)
            return _elem.lock();
        return TreeIter(_elem.lock()->parent()).up(cnt - 1);
    }
    TreeIter TreeIter::up(const std::string& name) const
    {
        auto current = _elem.lock();
        while (current->name() != name)
        {
            current = current->parent();
            if (current == nullptr)
                throw std::runtime_error{ std::format("Failed to locate parent with ID: {}", name) };
        }
        return current;
    }

    TreeIter TreeIter::operator/(const std::string& path)
    {
        return as<ParentElement>()->at(path);
    }
    TreeIter TreeIter::operator^(std::size_t cnt)
    {
        return up(cnt);
    }
    TreeIter TreeIter::operator+()
    {
        return up();
    }

    TreeIter::operator std::shared_ptr<Element>()
    {
        return _elem.lock();
    }
    TreeIter::operator std::weak_ptr<Element>()
    {
        return _elem;
    }

    std::shared_ptr<Element> TreeIter::operator->() const
    {
        return _elem.lock();
    }
    std::shared_ptr<Element> TreeIter::operator->()
    {
        return _elem.lock();
    }

    Element& TreeIter::operator*() const
    {
        return *_elem.lock();
    }
    Element& TreeIter::operator*()
    {
        return *_elem.lock();
    }

    void TreeIter::
    foreach (foreach_callback callback)
    {
        auto p = std::dynamic_pointer_cast<ParentElement>(as());
        if (p) p->foreach(std::move(callback));
    }


    // Event Listener

    Element::EventListenerState::Mode Element::EventListenerState::state() const
    {
        return _state;
    }
    Element::State Element::EventListenerState::event() const
    {
        return _event;
    }
    bool Element::EventListenerState::value() const
    {
        return _value;
    }
    std::size_t Element::EventListenerState::index() const
    {
        return _idx;
    }

    void Element::EventListenerState::setstate(Mode state)
    {
        _state = state;
    }

    void Element::EventListenerState::unmute()
    {
        auto ptr = _obj.lock();
        if (ptr)
        {
            ptr->_unmute_listener(shared_from_this());
        }
    }
    void Element::EventListenerState::mute()
    {
        auto ptr = _obj.lock();
        if (ptr)
        {
            ptr->_mute_listener(shared_from_this());
        }
    }
    void Element::EventListenerState::destroy()
    {
        auto ptr = _obj.lock();
        if (ptr)
        {
            ptr->_destroy_listener(shared_from_this());
        }
    }

    template<typename... Argv>
    void Element::EventListenerState::invoke(Argv&&... args)
    {
        if (_callback != nullptr && _state == Mode::Active)
            _callback(EventListener(shared_from_this()), std::forward<Argv>(args)...);
    }

    // Other stuff

    Element::write_flag Element::_contextual_change(write_flag flags) const
    {
        const unit_meta_flag rep = _unit_report_impl();
        if (flags & WriteType::Dimensions)
            return
                (bool(rep & ContextualXPos) * WriteType::LayoutXPos) |
                (bool(rep & ContextualYPos) * WriteType::LayoutYPos) |
                (bool(rep & (ContextualWidth | RelativeWidth)) * WriteType::LayoutWidth) |
                (bool(rep & ContextualHeight | RelativeHeight) * WriteType::LayoutHeight);
        return 0x00;
    }

    //
    // Element
    //

    // Listeners

    Element::EventListener Element::_push_listener(State event, bool value, event_callback callback)
    {
        D2_ASSERT(callback != nullptr);
        auto& l = _subscribers.emplace_back(std::make_shared<EventListenerState>(
            shared_from_this(),
            _subscribers.size(),
            value,
            event,
            std::move(callback)
        ));
        if (event == State::Clicked || event == State::Focused)
            _cursor_sink_listener_cnt++;
        return EventListener(l);
    }
    void Element::_unmute_listener(EventListenerState::ptr listener)
    {
        D2_ASSERT(listener->index() < _subscribers.size());
        if (listener->event() == State::Clicked || listener->event() == State::Focused)
            _cursor_sink_listener_cnt--;
        _subscribers[listener->index()]->setstate(EventListenerState::Mode::Active);
    }
    void Element::_mute_listener(EventListenerState::ptr listener)
    {
        D2_ASSERT(listener->index() < _subscribers.size());
        if (listener->event() == State::Clicked || listener->event() == State::Focused)
            _cursor_sink_listener_cnt++;
        _subscribers[listener->index()]->setstate(EventListenerState::Mode::Muted);
    }
    void Element::_destroy_listener(EventListenerState::ptr listener)
    {
        D2_ASSERT(listener->index() < _subscribers.size());
        if (listener->event() == State::Clicked || listener->event() == State::Focused)
            _cursor_sink_listener_cnt--;
        _subscribers[listener->index()] = nullptr;
        for (auto it = _subscribers.begin(); it != _subscribers.end();)
        {
            const auto beg = it;
            for (; it != _subscribers.end(); ++it)
            {
                if (*it != nullptr)
                    break;
            }
            if (it == _subscribers.end())
            {
                _subscribers.erase(
                    beg, _subscribers.end()
                );
                return;
            }
            else ++it;
        }
    }
    void Element::_trigger(State event, bool value)
    {
        for (std::size_t i = 0; i < _subscribers.size(); i++)
        {
            auto& l = _subscribers[i];
            if (l != nullptr && l->event() == event && l->value() == value)
                l->invoke(traverse());
        }
    }

    // Internal state

    bool Element::_is_write_type(write_flag type) const
    {
        return (_internal_state & type) == type;
    }
    void Element::_invalidate_state(write_flag type) const
    {
        _internal_state &= ~(type & WriteType::Strip & (WriteType::StripPerm));
        _internal_state |= (type & ~(WriteType::Strip & WriteType::StripPerm));
    }

    void Element::_signal_write_internal(write_flag type, unsigned int prop) const
    {
        _internal_state |= type;
    }
    void Element::_signal_write_local(write_flag type, unsigned int prop, ptr element)
    {
        if ((type & ~(WriteType::Style | WriteType::InternalLayout)) & WriteType::Dimensions)
        {
            _signal_context_change_impl(type, prop, shared_from_this());
            // Check if after the change of the dimensions the coordinates
            // on the corresponding axis are not going to change due to the context
            const auto pos = (type >> 2) & (WriteType::LayoutXPos | WriteType::LayoutYPos);
            _invalidate_state(
                _contextual_change(pos)
            );
        }
        _signal_write_impl(type, prop, element);
        _invalidate_state(type);
    }
    void Element::_signal_write_local(write_flag type, unsigned int prop)
    {
        _signal_write_local(type, prop, shared_from_this());
    }
    void Element::_signal_write(write_flag type, unsigned int prop, ptr element)
    {
        _signal_write_local(type, prop, element);
        if (const auto uptype = WriteType::Style | (type & WriteType::InternalLayout);
            parent() && !parent()->_is_write_type(uptype))
        {
            // Any layout write to a sub-object will result in a Style write to the parent
            parent()->_signal_write(uptype, prop, element);
        }
    }

    void Element::_signal_context_change_sub(write_flag type, unsigned int prop, ptr element)
    {
        if (const auto flags = _contextual_change(type);
            flags)
        {
            _signal_context_change_impl(type, prop, element);
            _signal_write_impl(flags, prop, element);
            _invalidate_state(flags);
        }
    }
    void Element::_signal_context_change_sub(write_flag type, unsigned int prop)
    {
        // Propagate down
        _signal_context_change_sub(type, prop, shared_from_this());
    }
    void Element::_signal_context_change(write_flag type, unsigned int prop, ptr element)
    {
        _signal_context_change_impl(type, prop, element);
    }
    void Element::_signal_context_change(write_flag type, unsigned int prop)
    {
        // Propagate down
        _signal_context_change(type, prop, shared_from_this());
    }
    void Element::_signal_write(write_flag type, unsigned int prop)
    {
        // Propagate up
        _signal_write(type, prop, shared_from_this());
    }
    void Element::_signal_initialization(unsigned int prop)
    {
        _signal_write_impl(0x00, prop, shared_from_this());
        _internal_state &= ~IsBeingInitialized;
    }
    void Element::_signal_write_update(write_flag type) const
    {
        _internal_state |= type;
    }
    void Element::_signal_update(internal_flag type) const
    {
        _internal_state &= ~type;
    }
    void Element::_trigger_event(IOContext::Event ev)
    {
        _event_impl(ev);
        _trigger(State::Event, true);
    }

    // Rendering

    PixelBuffer::View Element::_fetch_pixel_buffer_impl() const
    {
        if ((_internal_state & CachePolicyVolatile) && parent() != nullptr)
        {
            const auto [ width, height ] = box();
            const auto [ x, y ] = position();
            return PixelBuffer::View
            {
                parent()->_fetch_pixel_buffer_impl(),
                x, y, width, height
            };
        }
        else
        {
            return PixelBuffer::View
            {
                &_buffer, 0, 0,
                _buffer.width(), _buffer.height()
            };
        }
    }

    // Public interface

    // Metadata

    std::shared_ptr<Screen> Element::screen() const
    {
        return _state_ptr->screen();
    }
    IOContext::ptr Element::context() const
    {
        return _state_ptr->context();
    }
    TreeState::ptr Element::state() const
    {
        return _state_ptr;
    }
    Element::pptr Element::parent() const
    {
        return _parent.lock();
    }
    const std::string& Element::name() const
    {
        return _name;
    }

    // State

    bool Element::getistate(internal_flag flags) const
    {
        return (_internal_state & flags) == flags;
    }
    void Element::setcache(CachePolicy flag) const
    {
        _internal_state |= InternalState(flag);
    }
    bool Element::getstate(State state) const
    {
        return _state & state;
    }
    void Element::setstate(State state, bool value)
    {
        if (getstate(state) != value)
        {
            if (value)
            {
                _state |= state;
                if (state == Display)
                {
                    _signal_write(WriteType::Masked);
                    setstate(Swapped, false);
                }
                else if (state == Swapped)
                {
                    _buffer.clear();
                    _signal_write_local(WriteType::Style);
                }
            }
            else
            {
                _state &= ~state;
                if (state == Display)
                {
                    setstate(Swapped, true);
                    if (parent() != nullptr && parent()->getstate(Display))
                        parent()->_signal_write(WriteType::Masked, ~0u, shared_from_this());
                }
            }
            _state_change_impl(state, value);
            _trigger(state, value);
        }
    }
    void Element::setparent(pptr parent)
    {
        _parent = parent;
    }

    Element::unit_meta_flag Element::unit_report() const
    {
        return _unit_report_impl();
    }

    bool Element::provides_cursor_sink() const
    {
        return _cursor_sink_listener_cnt || _provides_input_impl();
    }
    bool Element::provides_input() const
    {
        return getstate(Display) && _provides_input_impl();
    }
    bool Element::needs_update() const
    {
        return
            _internal_state & WasWritten ||
            _internal_state & CachePolicyVolatile;
    }

    // Listeners

    Element::EventListener Element::listen(State event, bool value, event_callback callback)
    {
        return _push_listener(event, value, callback);
    }

    // Rendering

    int Element::resolve_units(Unit unit) const
    {
        [[ unlikely ]] if (!parent())
            return unit.raw();
        return parent()->resolve_units(unit, shared_from_this());
    }

    int Element::layout(enum Layout type) const
    {
        switch (type)
        {
            case Layout::X:
                if (!(_internal_state & PositionXUpdated))
                    parent()->layout_for(Layout::X, shared_from_this());
                break;
            case Layout::Y:
                if (!(_internal_state & PositionYUpdated))
                    parent()->layout_for(Layout::Y, shared_from_this());
                break;
            case Layout::Width:
                if (!(_internal_state & DimensionsWidthUpdated))
                    parent()->layout_for(Layout::Width, shared_from_this());
                break;
            case Layout::Height:
                if (!(_internal_state & DimensionsHeightUpdated))
                    parent()->layout_for(Layout::Height, shared_from_this());
                break;
        }
        return _layout.get(type);
    }
    Unit Element::internal_layout(enum Layout type) const
    {
        return _layout_impl(type);
    }

    void Element::accept_layout() const
    {
        _internal_state |= (PositionXUpdated | PositionYUpdated | DimensionsWidthUpdated | DimensionsHeightUpdated);
    }
    void Element::accept_layout(enum Layout type) const
    {
        switch (type)
        {
            case Layout::X: _internal_state |= PositionXUpdated; break;
            case Layout::Y: _internal_state |= PositionYUpdated; break;
            case Layout::Width: _internal_state |= DimensionsWidthUpdated; break;
            case Layout::Height: _internal_state |= DimensionsHeightUpdated; break;
        }
    }
    void Element::override_layout(enum Layout type, int value) const
    {
        _layout.set(type, value);
        accept_layout(type);
    }
    void Element::override_dimensions(int width, int height) const
    {
        _layout.set(Layout::Width, width);
        _layout.set(Layout::Height, height);
        _internal_state |= (DimensionsWidthUpdated | DimensionsHeightUpdated);

    }
    void Element::override_position(int x, int y) const
    {
        _layout.set(Layout::X, x);
        _layout.set(Layout::Y, y);
        _internal_state |= (PositionXUpdated | PositionYUpdated);
    }

    bool Element::contextual_layout(enum Layout type) const
    {
        const unit_meta_flag rep = _unit_report_impl();
        switch (type)
        {
            case Layout::X: return rep & ContextualXPos;
            case Layout::Y: return rep & ContextualYPos;
            case Layout::Width: return rep & ContextualWidth;
            case Layout::Height: return rep & ContextualHeight;
        }
    }
    bool Element::relative_layout(Layout type) const
    {
        const unit_meta_flag rep = _unit_report_impl();
        switch (type)
        {
            case Layout::X: return rep & RelativeXPos;
            case Layout::Y: return rep & RelativeYPos;
            case Layout::Width: return rep & RelativeWidth;
            case Layout::Height: return rep & RelativeHeight;
        }
    }

    Element::Frame Element::frame()
    {
        D2_ASSERT((_internal_state & (CachePolicyStatic | CachePolicyDynamic | CachePolicyVolatile)))
        if (_internal_state & WasWrittenLayout)
        {
            _update_layout_impl();
            _signal_update(WasWrittenLayout);
        }
        if (needs_update())
        {
            if (const auto [ width, height ] = box(); width > 0 && height > 0)
            {
                _update_style_impl();
                if (!_provides_buffer_impl() &&
                        (_internal_state & CachePolicyStatic) || parent() == nullptr)
                {
                    const auto [ bwidth, bheight ] = _reserve_buffer_impl();
                    _buffer.set_size(bwidth, bheight);
                }
                else
                {
                    _buffer.clear();
                }

                _internal_state |= IsBeingRendered;
                _frame_impl(_fetch_pixel_buffer_impl());
                _internal_state &= ~IsBeingRendered;

                if ((_internal_state & CachePolicyStatic) && parent() != nullptr)
                {
                    _buffer.compress();
                }
            }
            else
                _buffer.clear();
            _signal_update(WasWritten);
        }
        return Frame(shared_from_this());
    }
    Element::BoundingBox Element::box() const
    {
        if (!(_internal_state & DimensionsWidthUpdated)) parent()->layout_for(Layout::Width, shared_from_this());
        if (!(_internal_state & DimensionsHeightUpdated)) parent()->layout_for(Layout::Height, shared_from_this());
        return Element::BoundingBox{ _layout.get(Layout::Width), _layout.get(Layout::Height) };
    }
    Element::Position Element::position() const
    {
        if (!(_internal_state & PositionXUpdated)) parent()->layout_for(Layout::X, shared_from_this());
        if (!(_internal_state & PositionYUpdated)) parent()->layout_for(Layout::Y, shared_from_this());
        return Element::Position{ _layout.get(Layout::X), _layout.get(Layout::Y) };
    }
    Element::BoundingBox Element::internal_box() const
    {
        return
        {
            resolve_units(_layout_impl(Layout::Width)),
            resolve_units(_layout_impl(Layout::Height))
        };
    }
    Element::Position Element::internal_position() const
    {
        return
        {
            resolve_units(_layout_impl(Layout::X)),
            resolve_units(_layout_impl(Layout::Y))
        };
    }
    Element::Position Element::position_screen_space() const
    {
        if (parent())
        {
            const Position bpos = position();
            const Position ppos = parent()->position_screen_space();
            return Position
            {
                .x = bpos.x + ppos.x,
                .y = bpos.y + ppos.y,
            };
        }
        return position();
    }
    Element::Position Element::mouse_object_space() const
    {
        if (parent())
        {
            const auto [ xpos, ypos ] = position();
            const auto [ x, y ] = parent()->mouse_object_space();
            return { x - xpos, y - ypos };
        }
        const auto [ x, y ] = _state_ptr
            ->context()
            ->input()
            ->mouse_position();
        return { x, y };
    }

    char Element::getzindex() const
    {
        return _index_impl();
    }

    TreeIter Element::traverse()
    {
        return { shared_from_this() };
    }
    TreeIter Element::operator+()
    {
        return traverse();
    }

    namespace internal
    {
        ElementView ElementView::from(Element::ptr ptr)
        {
            return ElementView(ptr);
        }

        void ElementView::signal_context_change_sub(Element::write_flag type, unsigned int prop, Element::ptr element)
        {
            return _ptr->_signal_context_change_sub(type, prop, element);
        }
        void ElementView::signal_context_change_sub(Element::write_flag type, unsigned int prop)
        {
            return _ptr->_signal_context_change_sub(type, prop);
        }
        void ElementView::signal_context_change(Element::write_flag type, unsigned int prop, Element::ptr element)
        {
            return _ptr->_signal_context_change(type, prop, element);
        }
        void ElementView::signal_context_change(Element::write_flag type, unsigned int prop)
        {
            return _ptr->_signal_context_change(type, prop);
        }
        void ElementView::signal_write(Element::write_flag type, unsigned int prop)
        {
            return _ptr->_signal_write(type, prop);
        }
        void ElementView::signal_initialization(unsigned int prop)
        {
            return _ptr->_signal_initialization(prop);
        }
        void ElementView::signal_write_update(Element::write_flag type) const
        {
            return _ptr->_signal_write_update(type);
        }
        void ElementView::signal_update(Element::internal_flag type) const
        {
            return _ptr->_signal_update(type);
        }
        void ElementView::trigger_event(IOContext::Event ev)
        {
            return _ptr->_trigger_event(ev);
        }
    }
}
