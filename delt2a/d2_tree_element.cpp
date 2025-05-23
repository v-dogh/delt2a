#include "d2_tree_element.hpp"
#include "d2_exceptions.hpp"
#include <typeinfo>
#include <vector>
#include <cmath>

namespace d2
{
	// Traversal Wrapper

	bool Element::TraversalWrapper::is_type_of(TraversalWrapper other) const
	{
		if (_elem.expired())
			return false;
		return typeid(other._elem.lock().get()) == typeid(_elem.lock().get());
	}

	Element::TraversalWrapper Element::TraversalWrapper::up() const
	{
		return TraversalWrapper(_elem.lock()->parent());
	}
	Element::TraversalWrapper Element::TraversalWrapper::up(std::size_t cnt) const
	{
		if (!cnt)
			return _elem.lock();
		return TraversalWrapper(_elem.lock()->parent()).up(cnt - 1);
	}
	Element::TraversalWrapper Element::TraversalWrapper::up(const std::string& name) const
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

	Element::TraversalWrapper Element::TraversalWrapper::operator/(const std::string& path)
	{
		return as<ParentElement>()->at(path);
	}
	void Element::TraversalWrapper::foreach(foreach_callback callback)
	{
		auto p = std::dynamic_pointer_cast<ParentElement>(as());
		if (p) p->foreach(std::move(callback));
	}


	// Event Listener

	Element::EventListenerState::Mode Element::EventListenerState::state() const noexcept
	{
		return _state;
	}
	Element::State Element::EventListenerState::event() const noexcept
	{
		return _event;
	}
	bool Element::EventListenerState::value() const noexcept
	{
		return _value;
	}
	std::size_t Element::EventListenerState::index() const noexcept
	{
		return _idx;
	}

	void Element::EventListenerState::setstate(Mode state) noexcept
	{
		_state = state;
	}

	void Element::EventListenerState::unmute() noexcept
	{
		auto ptr = _obj.lock();
		if (ptr)
		{
			ptr->_unmute_listener(shared_from_this());
		}
	}
	void Element::EventListenerState::mute() noexcept
	{
		auto ptr = _obj.lock();
		if (ptr)
		{
			ptr->_mute_listener(shared_from_this());
		}
	}
	void Element::EventListenerState::destroy() noexcept
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

	//
	// Element
	//

	// Listeners

	Element::EventListener Element::_push_listener(State event, bool value, event_callback callback) noexcept
	{
		D2_ASSERT(callback != nullptr);
		auto& l = _subscribers.emplace_back(std::make_shared<EventListenerState>(
			shared_from_this(),
			_subscribers.size(),
			value,
			event,
			std::move(callback)
		));
		return EventListener(l);
	}
	void Element::_unmute_listener(EventListenerState::ptr listener) noexcept
	{
		D2_ASSERT(listener->index() < _subscribers.size());
		_subscribers[listener->index()]->setstate(EventListenerState::Mode::Active);
	}
	void Element::_mute_listener(EventListenerState::ptr listener) noexcept
	{
		D2_ASSERT(listener->index() < _subscribers.size());
		_subscribers[listener->index()]->setstate(EventListenerState::Mode::Muted);
	}
	void Element::_destroy_listener(EventListenerState::ptr listener) noexcept
	{
		D2_ASSERT(listener->index() < _subscribers.size());
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

	bool Element::_is_write_type(write_flag type) const noexcept
	{
		return (_internal_state & type) == type;
	}
	void Element::_invalidate_state(write_flag type) const noexcept
	{
		if (type & WriteType::Layout)
		{
			_internal_state &= ~(
				InternalState::DimensionsUpdated |
				InternalState::PositionUpdated
			);
		}
		_internal_state |= type;
	}

	void Element::_signal_write_internal(write_flag type, unsigned int prop) const noexcept
	{
		_internal_state |= type;
	}
	void Element::_signal_write_local(write_flag type, unsigned int prop, ptr element) noexcept
	{
		if (!_is_write_type(type))
		{
			_signal_write_impl(type, prop, element);
			_invalidate_state(type);
		}
	}
	void Element::_signal_write_local(write_flag type, unsigned int prop) noexcept
	{
		if (type & WriteType::Layout)
			_signal_context_change_impl(type, prop, shared_from_this());
		_signal_write_local(type, prop, shared_from_this());
	}
	void Element::_signal_write(write_flag type, unsigned int prop, ptr element) noexcept
	{
		_signal_write_local(type, prop, element);
		type |= (bool(type & WriteType::Layout) * WriteType::Style);
		if (
			parent() &&
			!parent()->_is_write_type(type)
			)
		{
			// Any layout write to a sub-object will result in a Style write to the parent
			parent()->_signal_write(
				type,
				prop, element
			);
		}
	}

	void Element::_signal_context_change(write_flag type, unsigned int prop, ptr element) noexcept
	{
		if (
			type & WriteType::Layout &&
			(contextual_dimensions() ||
			contextual_position() ||
			element->_provides_layout_impl(shared_from_this()))
			)
		{
			_signal_context_change_impl(type, prop, element);
			_signal_write_impl(type, prop, element);
			_invalidate_state(type);
		}
	}
	void Element::_signal_context_change(write_flag type, unsigned int prop) noexcept
	{
		// Propagate down
		_signal_context_change(type, prop, shared_from_this());
	}
	void Element::_signal_write(write_flag type, unsigned int prop) noexcept
	{
		// Propagate down
		if (type & WriteType::Layout)
		{
			// Propagate down for the parent if we could influence the layout
			if (parent() &&
				parent()->_provides_layout_impl(shared_from_this()))
			{
				parent()->_signal_context_change_impl(type, prop, parent());
			}
			_signal_context_change_impl(type, prop, shared_from_this());
		}
		// Propagate up
		_signal_write(type, prop, shared_from_this());
	}
	void Element::_signal_write_update(write_flag type) const noexcept
	{
		_internal_state |= type;
	}
	void Element::_signal_update(internal_flag type) const noexcept
	{
		_internal_state &= ~type;
	}
	void Element::_trigger_event(IOContext::Event ev)
	{
		_event_impl(ev);
	}

	// Rendering

	int Element::_resolve_units(Unit unit) const noexcept
	{
		if (parent())
			return parent()->_resolve_units_impl(unit, shared_from_this());
		D2_ASSERT(unit.getunits() == Unit::Px);
		return unit.raw();
	}

	PixelBuffer::View Element::_fetch_pixel_buffer_impl() const noexcept
	{
		if ((_internal_state & CachePolicyVolatile) && parent() != nullptr)
		{
			const auto [ width, height ] = box();
			const auto [ x, y ] = position();
			return PixelBuffer::View{
				parent()->_fetch_pixel_buffer_impl(),
				x, y, width, height
			};
		}
		else
		{
			return PixelBuffer::View{
				&_buffer, 0, 0,
				_buffer.width(), _buffer.height()
			};
		}
	}

	// Public interface

	// Metadata

	std::shared_ptr<Screen> Element::screen() const noexcept
	{
		return _state_ptr->screen();
	}
	IOContext::ptr Element::context() const noexcept
	{
		return _state_ptr->context();
	}
	TreeState::ptr Element::state() const noexcept
	{
		return _state_ptr;
	}
	Element::ptr Element::parent() const noexcept
	{
		return _parent.lock();
	}
	const std::string& Element::name() const noexcept
	{
		return _name;
	}

	// State

	bool Element::getistate(internal_flag flags) const noexcept
	{
		return (_internal_state & flags) == flags;
	}
	void Element::setcache(InternalState flag) const noexcept
	{
		D2_ASSERT(
			flag == CachePolicyStatic ||
			flag == CachePolicyVolatile ||
			flag == CachePolicyDynamic
		)
		_internal_state &= ~(CachePolicyStatic | CachePolicyVolatile | CachePolicyDynamic);
		_internal_state |= flag;
	}
	bool Element::getstate(State state) const noexcept
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
					_signal_write(WriteType::Masked);
					setstate(Swapped, true);
				}
			}
			_state_change_impl(state, value);
			_trigger(state, value);
		}
	}

	Element::unit_meta_flag Element::unit_report() const noexcept
	{
		return _unit_report_impl();
	}

	bool Element::relative_dimensions() const noexcept
	{
		const unit_meta_flag rep = _unit_report_impl();
		return
			(rep & RelativeWidth) ||
			(rep & RelativeHeight);
	}
	bool Element::relative_position() const noexcept
	{
		const unit_meta_flag rep = _unit_report_impl();
		return
			(rep & RelativeXPos) ||
			(rep & RelativeYPos);
	}

	bool Element::contextual_dimensions() const noexcept
	{
		const unit_meta_flag rep = _unit_report_impl();
		return rep & ContextualDimensions;
	}
	bool Element::contextual_position() const noexcept
	{
		const unit_meta_flag rep = _unit_report_impl();
		return rep & ContextualPosition;
	}

	bool Element::managed_position() const noexcept
	{
		return
			parent() &&
			parent()->_provides_layout_impl(shared_from_this());
	}
	bool Element::managed_dimensions() const noexcept
	{
		return
			parent() &&
			parent()->_provides_dimensions_impl(shared_from_this());
	}

	bool Element::provides_input() const noexcept
	{
		return getstate(Display) && _provides_input_impl();
	}
	bool Element::needs_update() const noexcept
	{
		return
			_internal_state & WasWritten ||
			_internal_state & CachePolicyVolatile;
	}

	// Listeners

	Element::EventListener Element::listen(State event, bool value, event_callback callback) noexcept
	{
		return _push_listener(event, value, callback);
	}

	// Rendering

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
			if (const auto [ width, height ] = box();
				width > 0 && height > 0)
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
			_signal_update(WasWritten);
		}
		return Frame(shared_from_this());
	}
	Element::BoundingBox Element::box() const noexcept
	{
		if (!(_internal_state & DimensionsUpdated))
		{
			if (managed_dimensions())
			{
				 _bounding_box = parent()
					->_dimensions_for(shared_from_this());
			}
			else
			{
				_bounding_box = _dimensions_impl();
			}
			_signal_write_update(DimensionsUpdated);
		}
		return _bounding_box;
	}
	Element::Position Element::position() const noexcept
	{
		if (!(_internal_state & PositionUpdated))
		{
			if (managed_position())
			{
				_position = parent()
					->_position_for(shared_from_this());
			}
			else
			{
				_position = _position_impl();
			}
			_signal_write_update(PositionUpdated);
		}
		return _position;
	}
	Element::BoundingBox Element::internal_box() const noexcept
	{
		return _dimensions_impl();
	}
	Element::Position Element::internal_position() const noexcept
	{
		return _position_impl();
	}
	Element::Position Element::position_screen_space() const noexcept
	{
		if (parent())
		{
			const Position bpos = position();
			const Position ppos = parent()->position_screen_space();
			return Position{
				.x = bpos.x + ppos.x,
				.y = bpos.y + ppos.y,
			};
		}
		return position();
	}
	Element::Position Element::mouse_object_space() noexcept
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

	void Element::accept_position() const noexcept
	{
		_internal_state |= PositionUpdated;
	}
	void Element::accept_dimensions() const noexcept
	{
		_internal_state |= DimensionsUpdated;
	}
	void Element::override_position(Position position) const noexcept
	{
		_position = position;
		_internal_state |= PositionUpdated;
	}
	void Element::override_dimensions(BoundingBox dims) const noexcept
	{
		_bounding_box = dims;
		_internal_state |= DimensionsUpdated;
	}

	char Element::getzindex() const noexcept
	{
		return _index_impl();
	}

	Element::TraversalWrapper Element::traverse() noexcept
	{
		return { shared_from_this() };
	}
	Element::TraversalWrapper Element::operator+() noexcept
	{
		return traverse();
	}

	// Dynamic Iterator

	void ParentElement::DynamicIterator::increment(int cnt) noexcept
	{
		if (!_ptr.expired())
			_adaptor->increment(cnt, _ptr.lock());
	}
	void ParentElement::DynamicIterator::decrement(int cnt) noexcept
	{
		if (!_ptr.expired())
			_adaptor->decrement(cnt, _ptr.lock());
	}

	bool ParentElement::DynamicIterator::is_begin() const noexcept
	{
		if (_ptr.expired() || _adaptor == nullptr)
			return false;
		return _adaptor->is_begin(_ptr.lock());
	}
	bool ParentElement::DynamicIterator::is_end() const noexcept
	{
		if (_ptr.expired() || _adaptor == nullptr)
			return false;
		return _adaptor->is_end(_ptr.lock());
	}
	bool ParentElement::DynamicIterator::is_null() const noexcept
	{
		return
			_ptr.expired() ||
			_adaptor == nullptr ||
			_adaptor->is_null(_ptr.lock());
	}
	bool ParentElement::DynamicIterator::is_equal(DynamicIterator it) const noexcept
	{
		if ((it._ptr.expired() && it._ptr.expired()) ||
			(it._adaptor == nullptr && _adaptor == nullptr))
			return true;
		if (!it._ptr.expired() && !_ptr.expired() &&
			it._ptr.lock() == _ptr.lock())
		{
			return _adaptor->is_equal(
				it._adaptor.get(), _ptr.lock()
			);
		}
		return false;
	}

	Element::ptr ParentElement::DynamicIterator::value() const noexcept
	{
		if (_ptr.expired() || _adaptor == nullptr)
			return nullptr;
		return _adaptor->value(_ptr.lock());
	}

	Element::ptr ParentElement::DynamicIterator::operator->() const noexcept
	{
		return _adaptor->value(_ptr.lock());
	}
	Element& ParentElement::DynamicIterator::operator*() const noexcept
	{
		return *_adaptor->value(_ptr.lock());
	}

	bool ParentElement::DynamicIterator::operator==(const DynamicIterator& other) const noexcept
	{
		return
			(_ptr.expired() && other._ptr.expired()) ||
			(_adaptor == nullptr && other._adaptor == nullptr) ||
			(_adaptor != nullptr && other._adaptor != nullptr &&
			!_ptr.expired() && !other._ptr.expired() &&
			_adaptor->is_equal(other._adaptor.get(), _ptr.lock()));
	}
	bool ParentElement::DynamicIterator::operator!=(const DynamicIterator& other) const noexcept
	{
		return !operator==(other);
	}

	ParentElement::DynamicIterator& ParentElement::DynamicIterator::operator=(const DynamicIterator& copy) noexcept
	{
		_ptr = copy._ptr;
		_adaptor = copy._adaptor->clone();
		return *this;
	}

	//
	// Parent Element
	//

	int ParentElement::_resolve_units_impl(Unit value, cptr elem) const noexcept
	{
		const bool dims = value.getmods() & UnitContext::Dimensions;
		const bool pos = !dims;
		const bool horiz = value.getmods() & UnitContext::Horizontal;
		const bool inv = value.getmods() & Unit::Mods::Inverted;
		const float val = value.raw();

		const auto border_top = _get_border_impl(BorderType::Top, elem);
		const auto border_bottom = _get_border_impl(BorderType::Bottom, elem);
		const auto border_left = _get_border_impl(BorderType::Left, elem);
		const auto border_right = _get_border_impl(BorderType::Right, elem);
		const auto border_horizontal = border_left + border_right;
		const auto border_vertical = border_top + border_bottom;
		const auto border_off = (horiz ?
			(inv ? -border_right : border_left) :
			(inv ? -border_bottom : border_top));

		switch (value.getunits())
		{
		case Unit::Auto:
		{
			Unit cpy = value;
			if (dims)
			{
				cpy.setunits(Unit::Pc);
				cpy = 1.f - val;
			}
			else 	if (cpy.getmods() & Unit::Relative)
			{
				cpy.setunits(Unit::Center);
			}
			else
			{
				cpy.setunits(Unit::Px);
			}
			return _resolve_units_impl(cpy, elem);
		}
		case Unit::Px:
		{
			if (inv)
			{
				if (dims) return (horiz ?
					box().width : box().height) - val - 1 + border_off;
				else return ((horiz ?
					(box().width - elem->box().width) :
					(box().height - elem->box().height)) -
					val) + border_off;
			}
			else
				return val + (pos * border_off);
		}
		case Unit::Pv:
		{
			const auto input = context()->input();
			const auto [ width, height ] = input->screen_size();
			const auto mval = (inv ? (1 - val) : val) - dims;
			return std::ceil((mval * (horiz ? width : height)) + border_off);
		}
		case Unit::Pc:
		{
			const auto mval = inv ? (1 - val) : val;
			return std::ceil((mval * (horiz ?
				(box().width - border_horizontal) :
				(box().height - border_vertical))) + (pos * border_off));
		}
		case Unit::Center:
		{
			if (dims)
				return 0;
			return ((horiz ?
				(box().width - elem->box().width - border_horizontal) :
				(box().height - elem->box().height - border_vertical)) / 2) +
				border_off + val;
		}
		}
	}
	void ParentElement::_signal_context_change_impl(write_flag type, unsigned int prop, ptr element) noexcept
	{
		foreach_internal([&](ptr elem) {
			elem->_signal_context_change(type, prop, element);
		});
	}
	void ParentElement::_state_change_impl(State state, bool value)
	{
		if (state == State::Swapped || state == State::Display)
		{
			foreach_internal([&](ptr elem) {
				elem->setstate(state, value);
			});
		}
	}

	//
	// VecParent
	//

	struct VecParentElement::LinearIteratorAdaptor :
		ParentElement::DynamicIterator::DynamicIteratorAdaptor
	{
		LinearIteratorAdaptor() = default;
		LinearIteratorAdaptor(const LinearIteratorAdaptor&) = default;
		LinearIteratorAdaptor(LinearIteratorAdaptor&&) = default;
		LinearIteratorAdaptor(std::vector<ptr>::iterator it) : current(it) {}

		std::vector<ParentElement::ptr>::iterator current{};

		auto ptr(std::shared_ptr<ParentElement> elem) const noexcept
		{
			return std::static_pointer_cast<VecParentElement>(elem);
		}

		Element::ptr value(std::shared_ptr<ParentElement> elem) const noexcept
		{
			return *current;
		}
		void increment(std::shared_ptr<ParentElement> elem) noexcept
		{
			if (current != ptr(elem)->_elements.end())
				current++;
		}
		void decrement(std::shared_ptr<ParentElement> elem) noexcept
		{
			if (current != ptr(elem)->_elements.begin())
				current--;
		}
		bool is_null(std::shared_ptr<ParentElement> elem) const noexcept
		{
			return current.base() == nullptr;
		}
		bool is_begin(std::shared_ptr<ParentElement> elem) const noexcept
		{
			return current == ptr(elem)->_elements.begin();
		}
		bool is_end(std::shared_ptr<ParentElement> elem) const noexcept
		{
			return current == ptr(elem)->_elements.end();
		}
		bool is_equal(DynamicIteratorAdaptor* adapter, std::shared_ptr<ParentElement> elem) const noexcept
		{
			return current == static_cast<LinearIteratorAdaptor*>(adapter)->current;
		}
		std::unique_ptr<DynamicIteratorAdaptor> clone() const noexcept
		{
			return std::make_unique<LinearIteratorAdaptor>(*this);
		}
	};

	// Implementation

	bool VecParentElement::_empty_impl() const noexcept
	{
		return _elements.empty();
	}
	bool VecParentElement::_exists_impl(const std::string& name) const noexcept
	{
		return _find(name) != ~0ull;
	}
	bool VecParentElement::_exists_impl(ptr ptr) const noexcept
	{
		return _find(ptr) != ~0ull;
	}

	const VecParentElement::TraversalWrapper VecParentElement::_at_impl(const std::string& name) const
	{
		const auto idx = _find(name);
		if (idx == ~0ull)
			throw std::runtime_error{ std::format("Attempt to reference invalid object: {}", name) };
		return { _elements[idx] };
	}
	VecParentElement::TraversalWrapper VecParentElement::_create_impl(ptr ptr)
	{
		if (_find(ptr->name()) != ~0ull)
			throw std::runtime_error{ "Attempt to create an object with a duplicate name" };
		auto result = _elements.emplace_back(ptr);
		_elements.back()->setstate(Swapped, false);
		_signal_write();
		return result;
	}
	VecParentElement::TraversalWrapper VecParentElement::_override_impl(ptr ptr) noexcept
	{
		const auto f = _find(ptr->name());
		if (f != ~0ull)
		{
			_elements[f] = ptr;
			ptr->setstate(Swapped, false);
			_signal_write();
			return ptr;
		}
		_elements.emplace_back(ptr);
		ptr->setstate(Swapped, false);
		_signal_write();
		return ptr;
	}
	void VecParentElement::_remove_impl(const std::string& name)
	{
		const auto idx = _find(name);
		if (idx >= _elements.size())
			throw std::runtime_error{ "Attempt to remove invalid object" };
		_elements[idx]->setstate(Swapped);
		_elements.erase(_elements.begin() + idx);
		_signal_write();
	}
	void VecParentElement::_remove_impl(ptr ptr)
	{
		const auto idx = _find(ptr);
		if (idx >= _elements.size())
			throw std::runtime_error{ "Attempt to remove invalid object" };
		_elements[idx]->setstate(Swapped);
		_elements.erase(_elements.begin() + idx);
		_signal_write();
	}
	void VecParentElement::_clear_impl() noexcept
	{
		for (decltype(auto) it : _elements)
		{
			it->setstate(Swapped);
		}
		_elements.clear();
		_elements.shrink_to_fit();
		_signal_write();
	}

	std::size_t VecParentElement::_find(const std::string& name) const noexcept
	{
		if (name.empty()) return ~0ull;
		auto f = std::find_if(_elements.begin(), _elements.end(), [&name](const auto& elem) {
			return elem != nullptr && elem->name() == name;
		});
		if (f == _elements.end())
			return ~0ull;
		return f - _elements.begin();
	}
	std::size_t VecParentElement::_find(ptr ptr) const noexcept
	{
		auto f = std::find(_elements.begin(), _elements.end(), ptr);
		if (f == _elements.end())
			return ~0ull;
		return f - _elements.begin();
	}

	// Public interface

	VecParentElement::DynamicIterator VecParentElement::begin() noexcept
	{
		return DynamicIterator::make<LinearIteratorAdaptor>(
			std::static_pointer_cast<ParentElement>(shared_from_this()), _elements.begin()
		);
	}
	VecParentElement::DynamicIterator VecParentElement::end() noexcept
	{
		return DynamicIterator::make<LinearIteratorAdaptor>(
			std::static_pointer_cast<ParentElement>(shared_from_this()), _elements.end()
		);
	}
	void VecParentElement::foreach_internal(foreach_internal_callback callback) const
	{
		for (decltype(auto) it : _elements)
			callback(it);
	}
	void VecParentElement::foreach(foreach_callback callback) const
	{
		for (decltype(auto) it : _elements)
			callback(it->traverse());
	}
}
