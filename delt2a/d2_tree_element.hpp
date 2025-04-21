#ifndef D2_TREE_ELEMENT_HPP
#define D2_TREE_ELEMENT_HPP

#include <functional>
#include <typeinfo>
#include <atomic>
#include <memory>
#include <cmath>
#include "d2_tree_element_frwd.hpp"
#include "d2_exceptions.hpp"
#include "d2_io_handler.hpp"
#include "d2_element_units.hpp"
#include "d2_pixel.hpp"

namespace d2
{
	class ParentElement;
	class Element : public std::enable_shared_from_this<Element>
	{
	public:
		template<typename Type>
		using eptr = std::shared_ptr<Type>;
		using ptr = std::shared_ptr<Element>;
		using cptr = std::shared_ptr<const Element>;
		using wptr = std::weak_ptr<Element>;

		// Provides a simpler interface for traversal across the tree
		class TraversalWrapper
		{
		public:
			using foreach_callback = std::function<void(TraversalWrapper)>;
		private:
			wptr _elem{};
		public:
			TraversalWrapper() = default;
			TraversalWrapper(std::nullptr_t) {}
			TraversalWrapper(const TraversalWrapper&) = default;
			TraversalWrapper(TraversalWrapper&&) = default;
			template<typename Type>
			TraversalWrapper(eptr<Type> p) :
				_elem(std::static_pointer_cast<Element>(p))
			{}

			template<typename Type>
			bool is_type() const
			{
				if (_elem.expired())
					return false;
				return dynamic_cast<const Type*>(_elem.lock().get()) != nullptr;
			}
			bool is_type_of(TraversalWrapper other) const
			{
				if (_elem.expired())
					return false;
				return typeid(other._elem.lock().get()) == typeid(_elem.lock().get());
			}

			template<typename Type>
			void set(eptr<Type> ptr = nullptr) noexcept
			{
				_elem = ptr;
			}

			template<typename Type = Element> std::shared_ptr<Type> as() const
			{
				auto p = std::dynamic_pointer_cast<Type>(_elem.lock());
				if (p == nullptr)
					throw std::runtime_error{ "Attempt to access an element through an invalid object" };
				return p;
			}
			template<typename Type = Element> auto as()
			{
				return const_cast<const TraversalWrapper*>(this)->as<Type>();
			}

			std::shared_ptr<ParentElement> asp() const
			{
				return as<ParentElement>();
			}
			std::shared_ptr<ParentElement> asp()
			{
				return as<ParentElement>();
			}

			TraversalWrapper up() const
			{
				return TraversalWrapper(_elem.lock()->parent());
			}
			TraversalWrapper up(std::size_t cnt) const
			{
				if (!cnt)
					return _elem.lock();
				return TraversalWrapper(_elem.lock()->parent()).up(cnt - 1);
			}

			void foreach(foreach_callback);

			auto operator->() const noexcept
			{
				return _elem.lock();
			}
			auto operator->() noexcept
			{
				return _elem.lock();
			}

			bool operator==(std::nullptr_t) const noexcept
			{
				return _elem.expired();
			}
			bool operator==(const TraversalWrapper& other) const noexcept
			{
				return (_elem.expired() && other._elem.expired()) || (_elem.lock() == other._elem.lock());
			}
			bool operator!=(const TraversalWrapper& other) const noexcept
			{
				return !operator==(other);
			}

			TraversalWrapper operator/(const std::string& path);

			TraversalWrapper& operator=(const TraversalWrapper&) = default;
			TraversalWrapper& operator=(TraversalWrapper&&) = default;
		};
		using tptr = TraversalWrapper;

		// Acquires a shared_lock for the buffer for it's lifetime
		// Ensures object lifetime
		// Provides access to the framebuffer
		// Returned from Element::frame
		class Frame
		{
		private:
			ptr _object{ nullptr };
		public:
			explicit Frame(ptr obj) :
				_object(obj)
			{}
			Frame(Frame&&) = default;
			Frame(const Frame&) = delete;

			PixelBuffer::View buffer() const noexcept
			{
				return _object->_fetch_pixel_buffer_impl();
			}
			auto data() const noexcept
			{
				return _object->_buffer.data();
			}
		};
		class EventListener;

		struct BoundingBox
		{
			int width{ 0 };
			int height{ 0 };
		};
		struct Position
		{
			int x{ 0 };
			int y{ 0 };
		};

		using unit_meta_flag = unsigned char;
		using internal_flag = unsigned short;
		using state_flag = unsigned char;
		using write_flag = unsigned char;

		using event_callback = std::function<void(EventListener, TraversalWrapper)>;
		using foreach_callback = TraversalWrapper::foreach_callback;
		using foreach_internal_callback = std::function<void(ptr)>;

		// Flags representing the meta state of the object
		// Event listeners notify when changes to them are made
		enum State : state_flag
		{
			Invalid = 0,
			Hovered = 1 << 0,
			Clicked = 1 << 1,
			Focused = 1 << 2,
			Display = 1 << 3,
			Swapped = 1 << 4,
			Keynavi = 1 << 5,
		};
		enum InternalState : internal_flag
		{
			IsBeingRendered = 1 << 0,
			// Set if the style is changed (i.e. requires a redraw)
			WasWritten = 1 << 1,
			// Set if the layout of the object has changed (for parent elements)
			WasWrittenLayout = 1 << 2,
			// Set when the bounding box of the object was updated
			DimensionsUpdated = 1 << 3,
			// Set when the position of the object was updated
			PositionUpdated = 1 << 4,
			// Indicates that the object's framebuffer should not be compressed
			CachePolicyDynamic = 1 << 5,
			// Indicates that the object's framebuffer should not be cached
			CachePolicyVolatile = 1 << 6
		};
		enum WriteType : write_flag
		{
			None = 0x00,
			// Modifies only the style of the object
			// Does not change it's dimensions or position
			Style = WasWritten,
			// May result in changes to dimensions or position
			Layout = WasWrittenLayout,
			Masked = Style | Layout,
		};
		enum UnitState : unit_meta_flag
		{
			// Relative - dependent on the other objects
			// Or on the parent's layout algorithm

			RelativeXPos = 1 << 0,
			RelativeYPos = 1 << 1,
			RelativeWidth = 1 << 2,
			RelativeHeight = 1 << 3,

			// Contextual - dependent on the parent (context)

			ContextualDimensions = 1 << 4,
			ContextualPosition = 1 << 5,
		};
	private:
		class EventListenerState : public std::enable_shared_from_this<EventListenerState>
		{
		public:
			using ptr = std::shared_ptr<EventListenerState>;
			using wptr = std::weak_ptr<EventListenerState>;
			enum Mode
			{
				Active,
				Muted,
				Destroyed
			};
		private:
			Element::wptr _obj{};
			std::size_t _idx{ ~0ull };
			State _event{};
			bool _value{ true };
			event_callback _callback{ nullptr };
			Mode _state{ Mode::Active };
		public:
			EventListenerState() = default;
			EventListenerState(EventListenerState&&) = delete;
			EventListenerState(const EventListenerState&) = default;
			template<typename Func>
			EventListenerState(
				Element::ptr ptr,
				std::size_t idx,
				bool value,
				State ev,
				Func callback
			) :
				_obj(ptr), _idx(idx),
				_value(value), _event(ev),
				_callback(std::forward<Func>(callback))
			{}

			Mode state() const noexcept
			{
				return _state;
			}
			State event() const noexcept
			{
				return _event;
			}
			bool value() const noexcept
			{
				return _value;
			}
			std::size_t index() const noexcept
			{
				return _idx;
			}

			void setstate(Mode state) noexcept
			{
				_state = state;
			}

			void unmute() noexcept
			{
				auto ptr = _obj.lock();
				if (ptr)
				{
					ptr->_unmute_listener(shared_from_this());
				}
			}
			void mute() noexcept
			{
				auto ptr = _obj.lock();
				if (ptr)
				{
					ptr->_mute_listener(shared_from_this());
				}
			}
			void destroy() noexcept
			{
				auto ptr = _obj.lock();
				if (ptr)
				{
					ptr->_destroy_listener(shared_from_this());
				}
			}

			template<typename... Argv>
			void invoke(Argv&&... args)
			{
				if (_callback != nullptr && _state == Mode::Active)
					_callback(EventListener(shared_from_this()), std::forward<Argv>(args)...);
			}

			operator bool() const noexcept
			{
				return !_obj.expired();
			}

			bool operator==(std::nullptr_t) const noexcept
			{
				return !bool();
			}
			bool operator!=(std::nullptr_t) const noexcept
			{
				return bool();
			}

			EventListenerState& operator=(EventListenerState&&) = default;
			EventListenerState& operator=(const EventListenerState&) = default;
		};
	public:
		class EventListener
		{
		private:
			EventListenerState::wptr _ptr{};
		public:
			EventListener() = default;
			EventListener(EventListener&&) = default;
			EventListener(const EventListener&) = default;
			EventListener(EventListenerState::ptr ptr)
				: _ptr(ptr) {}

			State event() const noexcept
			{
				if (_ptr.expired())
					return State::Invalid;
				return _ptr.lock()->event();
			}
			std::size_t index() const noexcept
			{
				if (_ptr.expired())
					return ~0ull;
				return _ptr.lock()->index();
			}

			void unmute() noexcept
			{
				if (!_ptr.expired())
				{
					_ptr.lock()->unmute();
				}
			}
			void mute() noexcept
			{
				if (!_ptr.expired())
				{
					_ptr.lock()->mute();
				}
			}
			void destroy() noexcept
			{
				if (!_ptr.expired())
				{
					_ptr.lock()->destroy();
				}
			}

			operator bool() const noexcept
			{
				return !_ptr.expired();
			}

			bool operator==(std::nullptr_t) const noexcept
			{
				return _ptr.expired();
			}
			bool operator!=(std::nullptr_t) const noexcept
			{
				return !_ptr.expired();
			}

			EventListener& operator=(EventListener&&) = default;
			EventListener& operator=(const EventListener&) = default;
		};
	private:
		// Metadata

		const wptr _parent{};
		const std::string _name{ nullptr };
		const TreeState::ptr _state_ptr{ nullptr };

		// Flags

		mutable std::atomic<state_flag> _state{ Display | Swapped };
		mutable std::atomic<internal_flag> _internal_state{ WasWritten | WasWrittenLayout };

		// Listeners

		std::vector<EventListenerState::ptr> _subscribers{};

		// Rendering

		mutable BoundingBox _bounding_box{};
		mutable Position _position{};
		mutable PixelBuffer _buffer{};
	private:
		// Listeners

		EventListener _push_listener(State event, bool value, event_callback callback) noexcept
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
		void _unmute_listener(EventListenerState::ptr listener) noexcept
		{
			D2_ASSERT(listener->index() < _subscribers.size());
			_subscribers[listener->index()]->setstate(EventListenerState::Mode::Active);
		}
		void _mute_listener(EventListenerState::ptr listener) noexcept
		{
			D2_ASSERT(listener->index() < _subscribers.size());
			_subscribers[listener->index()]->setstate(EventListenerState::Mode::Muted);
		}
		void _destroy_listener(EventListenerState::ptr listener) noexcept
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
		void _trigger(State event, bool value)
		{
			for (std::size_t i = 0; i < _subscribers.size(); i++)
			{
				auto& l = _subscribers[i];
				if (l != nullptr && l->event() == event && l->value() == value)
					l->invoke(traverse());
			}
		}
	protected:
		// Write handling
		// Context changes occur when a parent element changes it's layout and a child has relative layout
		// Then we notify it to recompute it's values for the layout
		// Layout writes signal that the bounding box and position of an object changed
		// Style writes signal that either the framebuffer itself, or it's contents changed (for an element)
		// For example a resize is both a Style and Layout write since it requires a resize of the buffer and changes the box

		bool _is_write_type(write_flag type) const noexcept
		{
			return (_internal_state & type) == type;
		}
		void _invalidate_state(write_flag type) const noexcept
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

		virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) noexcept {}
		virtual void _signal_context_change_impl(write_flag type, unsigned int prop, ptr element) noexcept {}

		void _signal_write_internal(write_flag type = WriteType::Masked, unsigned int prop = ~0u) const noexcept
		{
			_internal_state |= type;
		}
		void _signal_write_local(write_flag type, unsigned int prop, ptr element) noexcept
		{
			if (!_is_write_type(type))
			{
				_signal_write_impl(type, prop, element);
				_invalidate_state(type);
			}
		}
		void _signal_write_local(write_flag type = WriteType::Masked, unsigned int prop = ~0u) noexcept
		{
			if (type & WriteType::Layout)
				_signal_context_change_impl(type, prop, shared_from_this());
			_signal_write_local(type, prop, shared_from_this());
		}
		void _signal_write(write_flag type, unsigned int prop, ptr element) noexcept
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
	public:
		void _signal_context_change(write_flag type, unsigned int prop, ptr element) noexcept
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
		void _signal_context_change(write_flag type = WriteType::Masked, unsigned int prop = ~0u) noexcept
		{
			// Propagate down
			_signal_context_change(type, prop, shared_from_this());
		}
		void _signal_write(write_flag type = WriteType::Masked, unsigned int prop = ~0u) noexcept
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
		void _signal_write_update(write_flag type) const noexcept
		{
			_internal_state |= type;
		}
		void _signal_update(internal_flag type) const noexcept
		{
			_internal_state &= ~type;
		}
		void _trigger_event(IOContext::Event ev)
		{
			_event_impl(ev);
		}
	protected:
		// Units, alignment and dimensions

		int _resolve_units(Unit unit) const noexcept
		{
			if (parent())
				return parent()->_resolve_units_impl(unit, shared_from_this());
			D2_ASSERT(unit.getunits() == Unit::Px);
			return unit.raw();
		}

		virtual Position _position_for(cptr) const { return {}; }
		virtual BoundingBox _dimensions_for(cptr) const { return {}; }
		virtual unit_meta_flag _unit_report_impl() const noexcept = 0;
		virtual bool _provides_input_impl() const noexcept { return false; }
		virtual bool _provides_layout_impl(cptr) const noexcept { return false; }
		virtual bool _provides_dimensions_impl(cptr) const noexcept { return false; }
		virtual int _resolve_units_impl(Unit, cptr) const noexcept { return 0; }
		virtual char _index_impl() const noexcept = 0;

		// Events

		virtual void _event_impl(IOContext::Event) {}
		virtual void _state_change_impl(State, bool) {}

		// Layout

		virtual Position _position_impl() const noexcept = 0;
		virtual BoundingBox _dimensions_impl() const noexcept = 0;
		virtual BoundingBox _reserve_buffer_impl() const noexcept { return box(); }
		virtual PixelBuffer::View _fetch_pixel_buffer_impl() const noexcept
		{
			return PixelBuffer::View{
				&_buffer, 0, 0,
				_buffer.width(), _buffer.height()
			};
		}
		virtual bool _provides_buffer_impl() const noexcept { return false; }

		// Core

		virtual void _update_layout_impl() noexcept {}
		virtual void _update_style_impl() noexcept {}
		virtual void _frame_impl(PixelBuffer::View) noexcept = 0;
	public:
		Element() = default;
		Element(
			const std::string& name,
			TreeState::ptr state,
			ptr parent
		) :
			_parent(parent),
			_name(name),
			_state_ptr(state)
		{}
		Element(Element&&) = delete;
		Element(const Element&) = delete;
		virtual ~Element() = default;

		// Metadata

		std::shared_ptr<Screen> screen() const noexcept
		{
			return _state_ptr->screen();
		}
		IOContext::ptr context() const noexcept
		{
			return _state_ptr->context();
		}
		TreeState::ptr state() const noexcept
		{
			return _state_ptr;
		}
		Element::ptr parent() const noexcept
		{
			return _parent.lock();
		}
		const std::string& name() const noexcept
		{
			return _name;
		}

		// State

		bool getistate(internal_flag flags) const noexcept
		{
			return (_internal_state & flags) == flags;
		}
		bool getstate(State state) const noexcept
		{
			return _state & state;
		}
		void setstate(State state, bool value = true)
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

		unit_meta_flag unit_report() const noexcept
		{
			return _unit_report_impl();
		}

		bool relative_dimensions() const noexcept
		{
			const unit_meta_flag rep = _unit_report_impl();
			return
				(rep & RelativeWidth) ||
				(rep & RelativeHeight);
		}
		bool relative_position() const noexcept
		{
			const unit_meta_flag rep = _unit_report_impl();
			return
				(rep & RelativeXPos) ||
				(rep & RelativeYPos);
		}

		bool contextual_dimensions() const noexcept
		{
			const unit_meta_flag rep = _unit_report_impl();
			return rep & ContextualDimensions;
		}
		bool contextual_position() const noexcept
		{
			const unit_meta_flag rep = _unit_report_impl();
			return rep & ContextualPosition;
		}

		bool managed_position() const noexcept
		{
			return
				parent() &&
				parent()->_provides_layout_impl(shared_from_this());
		}
		bool managed_dimensions() const noexcept
		{
			return
				parent() &&
				parent()->_provides_dimensions_impl(shared_from_this());
		}

		bool provides_input() const noexcept
		{
			return getstate(Display) && _provides_input_impl();
		}
		bool needs_update() const noexcept
		{
			return _internal_state & WasWritten;
		}

		// Listeners

		EventListener listen(State event, bool value, event_callback callback) noexcept
		{
			return _push_listener(event, value, callback);
		}

		// Rendering

		Frame frame()
		{

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

					if (!_provides_buffer_impl())
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
				}
				_signal_update(WasWritten);
			}
			return Frame(shared_from_this());
		}
		BoundingBox box() const noexcept
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
		Position position() const noexcept
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
		BoundingBox internal_box() const noexcept
		{
			return _dimensions_impl();
		}
		Position internal_position() const noexcept
		{
			return _position_impl();
		}
		Position position_screen_space() const noexcept
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
		Position mouse_object_space() noexcept
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

		void accept_position() const noexcept
		{
			_internal_state |= PositionUpdated;
		}
		void accept_dimensions() const noexcept
		{
			_internal_state |= DimensionsUpdated;
		}
		void override_position(Position position) const noexcept
		{
			_position = position;
			_internal_state |= PositionUpdated;
		}
		void override_dimensions(BoundingBox dims) const noexcept
		{
			_bounding_box = dims;
			_internal_state |= DimensionsUpdated;
		}

		char getzindex() const noexcept
		{
			return _index_impl();
		}

		TraversalWrapper traverse() noexcept
		{
			return { shared_from_this() };
		}
		TraversalWrapper operator+() noexcept
		{
			return traverse();
		}

		Element& operator=(const Element&) = delete;
		Element& operator=(Element&&) = delete;
	};
	class ParentElement : public Element
	{
	public:
		class DynamicIterator
		{
		public:
			struct DynamicIteratorAdaptor
			{
				DynamicIteratorAdaptor() = default;
				DynamicIteratorAdaptor(const DynamicIteratorAdaptor&) = default;
				DynamicIteratorAdaptor(DynamicIteratorAdaptor&&) = default;
				virtual ~DynamicIteratorAdaptor() = default;

				void increment(int cnt, std::shared_ptr<ParentElement> elem) noexcept
				{
					for (std::size_t i = 0; i < cnt; i++)
						increment(elem);
				}
				void decrement(int cnt, std::shared_ptr<ParentElement> elem) noexcept
				{
					for (std::size_t i = 0; i < cnt; i++)
						decrement(elem);
				}

				virtual ptr value(std::shared_ptr<ParentElement> elem) const noexcept = 0;
				virtual void increment(std::shared_ptr<ParentElement> elem) noexcept = 0;
				virtual void decrement(std::shared_ptr<ParentElement> elem) noexcept = 0;
				virtual bool is_null(std::shared_ptr<ParentElement> elem) const noexcept = 0;
				virtual bool is_begin(std::shared_ptr<ParentElement> elem) const noexcept = 0;
				virtual bool is_end(std::shared_ptr<ParentElement> elem) const noexcept = 0;
				virtual bool is_equal(DynamicIteratorAdaptor* adapter, std::shared_ptr<ParentElement> elem) const noexcept = 0;
				virtual std::unique_ptr<DynamicIteratorAdaptor> clone() const noexcept = 0;
			};
		private:
			std::weak_ptr<ParentElement> _ptr{};
			std::unique_ptr<DynamicIteratorAdaptor> _adaptor{ nullptr };
		public:
			template<typename Adaptor, typename... Argv>
			static auto make(std::weak_ptr<ParentElement> ptr, Argv&&... args)
			{
				return DynamicIterator(
					ptr, std::make_unique<Adaptor>(std::forward<Argv>(args)...)
				);
			}

			DynamicIterator() = default;
			DynamicIterator(std::nullptr_t) {}
			DynamicIterator(const DynamicIterator& copy) :
				_ptr(copy._ptr), _adaptor(copy._adaptor->clone()) {}
			DynamicIterator(DynamicIterator&&) = default;
			DynamicIterator(std::weak_ptr<ParentElement> ptr, std::unique_ptr<DynamicIteratorAdaptor> adaptor) :
				_ptr(ptr), _adaptor(std::move(adaptor)) {}

			void increment(int cnt = 1) noexcept
			{
				if (!_ptr.expired())
					_adaptor->increment(cnt, _ptr.lock());
			}
			void decrement(int cnt = 1) noexcept
			{
				if (!_ptr.expired())
					_adaptor->decrement(cnt, _ptr.lock());
			}

			bool is_begin() const noexcept
			{
				if (_ptr.expired() || _adaptor == nullptr)
					return false;
				return _adaptor->is_begin(_ptr.lock());
			}
			bool is_end() const noexcept
			{
				if (_ptr.expired() || _adaptor == nullptr)
					return false;
				return _adaptor->is_end(_ptr.lock());
			}
			bool is_null() const noexcept
			{
				return
					_ptr.expired() ||
					_adaptor == nullptr ||
					_adaptor->is_null(_ptr.lock());
			}
			bool is_equal(DynamicIterator it) const noexcept
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

			Element::ptr value() const noexcept
			{
				if (_ptr.expired() || _adaptor == nullptr)
					return nullptr;
				return _adaptor->value(_ptr.lock());
			}

			Element::ptr operator->() const noexcept
			{
				return _adaptor->value(_ptr.lock());
			}
			Element& operator*() const noexcept
			{
				return *_adaptor->value(_ptr.lock());
			}

			bool operator==(const DynamicIterator& other) const noexcept
			{
				return
					(_ptr.expired() && other._ptr.expired()) ||
					(_adaptor == nullptr && other._adaptor == nullptr) ||
					(_adaptor != nullptr && other._adaptor != nullptr &&
					!_ptr.expired() && !other._ptr.expired() &&
					_adaptor->is_equal(other._adaptor.get(), _ptr.lock()));
			}
			bool operator!=(const DynamicIterator& other) const noexcept
			{
				return !operator==(other);
			}

			DynamicIterator& operator=(const DynamicIterator& copy) noexcept
			{
				_ptr = copy._ptr;
				_adaptor = copy._adaptor->clone();
				return *this;
			}
			DynamicIterator& operator=(DynamicIterator&&) noexcept = default;
		};
	protected:
		enum class BorderType
		{
			Top,
			Bottom,
			Left,
			Right
		};

		virtual int _get_border_impl(BorderType, cptr) const noexcept { return 0; }
		virtual int _resolve_units_impl(Unit value, cptr elem) const noexcept override
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
		virtual void _signal_context_change_impl(write_flag type, unsigned int prop, ptr element) noexcept override
		{
			foreach_internal([&](ptr elem) {
				elem->_signal_context_change(type, prop, element);
			});
		}
		virtual void _state_change_impl(State state, bool value) override
		{
			if (state == State::Swapped || state == State::Display)
			{
				foreach_internal([&](ptr elem) {
					elem->setstate(state, value);
				});
			}
		}

		virtual bool _empty_impl() const noexcept = 0;
		virtual bool _exists_impl(const std::string&) const noexcept = 0;
		virtual bool _exists_impl(ptr) const noexcept = 0;

		virtual const TraversalWrapper _at_impl(const std::string&) const = 0;
		virtual TraversalWrapper _create_impl(ptr) = 0;
		virtual TraversalWrapper _override_impl(ptr) noexcept = 0;
		virtual void _remove_impl(const std::string&) = 0;
		virtual void _remove_impl(ptr) = 0;
		virtual void _clear_impl() noexcept = 0;
	public:
		using Element::Element;

		bool empty() const noexcept
		{
			return _empty_impl();
		}
		bool exists(const std::string& name) const noexcept
		{
			return _exists_impl(name);
		}
		bool exists(ptr ptr) const noexcept
		{
			return _exists_impl(ptr);
		}

		TraversalWrapper at(const std::string& name) const
		{
			return _at_impl(name);
		}

		template<typename Type> TraversalWrapper create(const std::string& name = "")
		{
			return create(std::make_shared<Type>(
				name,
				state(),
				shared_from_this()
			));
		}
		template<typename Type> TraversalWrapper override(const std::string& name = "")
		{
			return override(std::make_shared<Type>(
				name,
				state(),
				shared_from_this()
			));
		}
		TraversalWrapper create(ptr ptr)
		{
			return _create_impl(ptr);
		}
		TraversalWrapper override(ptr ptr) noexcept
		{
			return _override_impl(ptr);
		}

		void clear() noexcept
		{
			_clear_impl();
		}

		void remove(const std::string& name)
		{
			_remove_impl(name);
		}
		void remove(ptr ptr)
		{
			_remove_impl(ptr);
		}

		virtual DynamicIterator begin() noexcept
		{
			return nullptr;
		}
		virtual DynamicIterator end() noexcept
		{
			return nullptr;
		}
		virtual void foreach_internal(foreach_internal_callback callback) const = 0;
		virtual void foreach(foreach_callback callback) const = 0;
	};
	class VecParentElement : public ParentElement
	{
	private:
		struct LinearIteratorAdaptor :
			DynamicIterator::DynamicIteratorAdaptor
		{
			LinearIteratorAdaptor() = default;
			LinearIteratorAdaptor(const LinearIteratorAdaptor&) = default;
			LinearIteratorAdaptor(LinearIteratorAdaptor&&) = default;
			LinearIteratorAdaptor(std::vector<ptr>::iterator it) : current(it) {}

			std::vector<ptr>::iterator current{};

			auto ptr(std::shared_ptr<ParentElement> elem) const noexcept
			{
				return std::static_pointer_cast<VecParentElement>(elem);
			}

			virtual Element::ptr value(std::shared_ptr<ParentElement> elem) const noexcept
			{
				return *current;
			}
			virtual void increment(std::shared_ptr<ParentElement> elem) noexcept
			{
				if (current != ptr(elem)->_elements.end())
					current++;
			}
			virtual void decrement(std::shared_ptr<ParentElement> elem) noexcept
			{
				if (current != ptr(elem)->_elements.begin())
					current--;
			}
			virtual bool is_null(std::shared_ptr<ParentElement> elem) const noexcept
			{
				return current.base() == nullptr;
			}
			virtual bool is_begin(std::shared_ptr<ParentElement> elem) const noexcept
			{
				return current == ptr(elem)->_elements.begin();
			}
			virtual bool is_end(std::shared_ptr<ParentElement> elem) const noexcept
			{
				return current == ptr(elem)->_elements.end();
			}
			virtual bool is_equal(DynamicIteratorAdaptor* adapter, std::shared_ptr<ParentElement> elem) const noexcept
			{
				return current == static_cast<LinearIteratorAdaptor*>(adapter)->current;
			}
			virtual std::unique_ptr<DynamicIteratorAdaptor> clone() const noexcept
			{
				return std::make_unique<LinearIteratorAdaptor>(*this);
			}
		};
	protected:
		std::vector<ptr> _elements{};

		virtual bool _empty_impl() const noexcept override
		{
			return _elements.empty();
		}
		virtual bool _exists_impl(const std::string& name) const noexcept override
		{
			return _find(name) != ~0ull;
		}
		virtual bool _exists_impl(ptr ptr) const noexcept override
		{
			return _find(ptr) != ~0ull;
		}

		virtual const TraversalWrapper _at_impl(const std::string& name) const override
		{
			const auto idx = _find(name);
			if (idx == ~0ull)
				throw std::runtime_error{ std::format("Attempt to reference invalid object: {}", name) };
			return { _elements[idx] };
		}
		virtual TraversalWrapper _create_impl(ptr ptr) override
		{
			if (_find(ptr->name()) != ~0ull)
				throw std::runtime_error{ "Attempt to create an object with a duplicate name" };
			auto result = _elements.emplace_back(ptr);
			_elements.back()->setstate(Swapped, false);
			_signal_write();
			return result;
		}
		virtual TraversalWrapper _override_impl(ptr ptr) noexcept override
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
		virtual void _remove_impl(const std::string& name) override
		{
			const auto idx = _find(name);
			if (idx >= _elements.size())
				throw std::runtime_error{ "Attempt to remove invalid object" };
			_elements[idx]->setstate(Swapped);
			_elements.erase(_elements.begin() + idx);
			_signal_write();
		}
		virtual void _remove_impl(ptr ptr) override
		{
			const auto idx = _find(ptr);
			if (idx >= _elements.size())
				throw std::runtime_error{ "Attempt to remove invalid object" };
			_elements[idx]->setstate(Swapped);
			_elements.erase(_elements.begin() + idx);
			_signal_write();
		}
		virtual void _clear_impl() noexcept override
		{
			for (decltype(auto) it : _elements)
			{
				it->setstate(Swapped);
			}
			_elements.clear();
			_elements.shrink_to_fit();
			_signal_write();
		}

		std::size_t _find(const std::string& name) const noexcept
		{
			if (name.empty()) return ~0ull;
			auto f = std::find_if(_elements.begin(), _elements.end(), [&name](const auto& elem) {
				return elem != nullptr && elem->name() == name;
			});
			if (f == _elements.end())
				return ~0ull;
			return f - _elements.begin();
		}
		std::size_t _find(ptr ptr) const noexcept
		{
			auto f = std::find(_elements.begin(), _elements.end(), ptr);
			if (f == _elements.end())
				return ~0ull;
			return f - _elements.begin();
		}
	public:
		using ParentElement::ParentElement;

		virtual DynamicIterator begin() noexcept override
		{
			return DynamicIterator::make<LinearIteratorAdaptor>(
				std::static_pointer_cast<ParentElement>(shared_from_this()), _elements.begin()
			);
		}
		virtual DynamicIterator end() noexcept override
		{
			return DynamicIterator::make<LinearIteratorAdaptor>(
				std::static_pointer_cast<ParentElement>(shared_from_this()), _elements.end()
			);
		}
		virtual void foreach_internal(foreach_internal_callback callback) const override
		{
			for (decltype(auto) it : _elements)
				callback(it);
		}
		virtual void foreach(foreach_callback callback) const override
		{
			for (decltype(auto) it : _elements)
				callback(it->traverse());
		}
	};
	class MetaParentElement : public ParentElement
	{
	protected:
		virtual bool _empty_impl() const noexcept override
		{
			return true;
		}
		virtual bool _exists_impl(const std::string& name) const noexcept override
		{
			return false;
		}
		virtual bool _exists_impl(ptr ptr) const noexcept override
		{
			return false;
		}

		virtual const TraversalWrapper _at_impl(const std::string& name) const override
		{
			throw std::logic_error{ "Not implemented" };
		}
		virtual TraversalWrapper _create_impl(ptr ptr) override
		{
			throw std::logic_error{ "Not implemented" };
		}
		virtual TraversalWrapper _override_impl(ptr ptr) noexcept override
		{
			return nullptr;
		}
		virtual void _remove_impl(const std::string& name) override
		{
			throw std::logic_error{ "Not implemented" };
		}
		virtual void _remove_impl(ptr ptr) override
		{
			throw std::logic_error{ "Not implemented" };
		}
		virtual void _clear_impl() noexcept override {}
	public:
		using ParentElement::ParentElement;

		virtual void foreach_internal(foreach_internal_callback callback) const override {}
		virtual void foreach(foreach_callback callback) const override {}
	};

	Element::TraversalWrapper Element::TraversalWrapper::operator/(const std::string& path)
	{
		return as<ParentElement>()->at(path);
	}
	void Element::TraversalWrapper::foreach(foreach_callback callback)
	{
		auto p = std::dynamic_pointer_cast<ParentElement>(as());
		if (p) p->foreach(std::move(callback));
	}
}

#endif // D2_TREE_ELEMENT_HPP
