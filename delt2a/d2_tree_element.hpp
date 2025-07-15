#ifndef D2_TREE_ELEMENT_HPP
#define D2_TREE_ELEMENT_HPP

#include <functional>
#include <memory>
#include <string>
#include "d2_tree_element_frwd.hpp"
#include "d2_styles_base.hpp"
#include "d2_io_handler.hpp"
#include "d2_pixel.hpp"
#include "d2_element_units.hpp"

namespace d2
{
	class ParentElement;
	class Element :
		public std::enable_shared_from_this<Element>,
		public style::UniversalAccessInterfaceBase
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
			bool is_type_of(TraversalWrapper other) const;

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

			TraversalWrapper up() const;
			TraversalWrapper up(std::size_t cnt) const;
			TraversalWrapper up(const std::string& name) const;

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
		using write_flag = element_write_flag;

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
			Created = 1 << 6,
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
			// Indicates that the object should me drawn in immediate mode
			CachePolicyVolatile = 1 << 6,
			// Indicates that the object's framebuffer should be preemptively compressed
			CachePolicyStatic = 1 << 7,
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

			Mode state() const noexcept;
			State event() const noexcept;
			bool value() const noexcept;
			std::size_t index() const noexcept;

			void setstate(Mode state) noexcept;

			void unmute() noexcept;
			void mute() noexcept;
			void destroy() noexcept;

			template<typename... Argv>
			void invoke(Argv&&... args);

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
		mutable std::atomic<internal_flag> _internal_state{ WasWritten | WasWrittenLayout | CachePolicyStatic };

		// Listeners

		std::vector<EventListenerState::ptr> _subscribers{};

		// Rendering

		mutable BoundingBox _bounding_box{};
		mutable Position _position{};
		mutable PixelBuffer _buffer{};
	private:
		// Listeners

		EventListener _push_listener(State event, bool value, event_callback callback) noexcept;
		void _unmute_listener(EventListenerState::ptr listener) noexcept;
		void _mute_listener(EventListenerState::ptr listener) noexcept;
		void _destroy_listener(EventListenerState::ptr listener) noexcept;
		void _trigger(State event, bool value);
	protected:
		// Write handling
		// Context changes occur when a parent element changes it's layout and a child has relative layout
		// Then we notify it to recompute it's values for the layout
		// Layout writes signal that the bounding box and position of an object changed
		// Style writes signal that either the framebuffer itself, or it's contents changed (for an element)
		// For example a resize is both a Style and Layout write since it requires a resize of the buffer and changes the box

		bool _is_write_type(write_flag type) const noexcept;
		void _invalidate_state(write_flag type) const noexcept;

		virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) noexcept {}
		virtual void _signal_context_change_impl(write_flag type, unsigned int prop, ptr element) noexcept {}

		void _signal_write_internal(write_flag type = WriteType::Masked, unsigned int prop = ~0u) const noexcept;
		void _signal_write_local(write_flag type, unsigned int prop, ptr element) noexcept;
		void _signal_write_local(write_flag type = WriteType::Masked, unsigned int prop = ~0u) noexcept;
		void _signal_write(write_flag type, unsigned int prop, ptr element) noexcept;
	public:
		void _signal_context_change(write_flag type, unsigned int prop, ptr element) noexcept;
		void _signal_context_change(write_flag type = WriteType::Masked, unsigned int prop = ~0u) noexcept;
		void _signal_write(write_flag type = WriteType::Masked, unsigned int prop = ~0u) noexcept;
		void _signal_write_update(write_flag type) const noexcept;
		void _signal_update(internal_flag type) const noexcept;
		void _trigger_event(IOContext::Event ev);
	protected:
		// Units, alignment and dimensions

		int _resolve_units(Unit unit) const noexcept;

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
		virtual PixelBuffer::View _fetch_pixel_buffer_impl() const noexcept;
		virtual bool _provides_buffer_impl() const noexcept
		{
			return false;
		}

		// Core

		virtual void _update_layout_impl() noexcept {}
		virtual void _update_style_impl() noexcept {}
		virtual void _frame_impl(PixelBuffer::View) noexcept = 0;
	public:
		template<typename Type>
		static auto make(const std::string& name, TreeState::ptr state, ptr parent)
		{
			auto ptr = std::make_shared<Type>(name, state, parent);
			ptr->setstate(State::Created);
			return ptr;
		}

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
		virtual ~Element()
		{
			setstate(State::Created, false);
		}

		// Metadata

		std::shared_ptr<Screen> screen() const noexcept;
		IOContext::ptr context() const noexcept;
		TreeState::ptr state() const noexcept;
		Element::ptr parent() const noexcept;
		const std::string& name() const noexcept;

		// State

		bool getistate(internal_flag flags) const noexcept;
		void setcache(InternalState flag) const noexcept;
		bool getstate(State state) const noexcept;
		void setstate(State state, bool value = true);

		// Units

		unit_meta_flag unit_report() const noexcept;

		bool relative_dimensions() const noexcept;
		bool relative_position() const noexcept;

		bool contextual_dimensions() const noexcept;
		bool contextual_position() const noexcept;

		bool managed_position() const noexcept;
		bool managed_dimensions() const noexcept;

		bool provides_input() const noexcept;
		bool needs_update() const noexcept;

		EventListener listen(State event, bool value, event_callback callback) noexcept;

		// Rendering

		Frame frame();
		BoundingBox box() const noexcept;
		Position position() const noexcept;
		BoundingBox internal_box() const noexcept;
		Position internal_position() const noexcept;
		Position position_screen_space() const noexcept;
		Position mouse_object_space() noexcept;

		void accept_position() const noexcept;
		void accept_dimensions() const noexcept;
		void override_position(Position position) const noexcept;
		void override_dimensions(BoundingBox dims) const noexcept;

		char getzindex() const noexcept;

		TraversalWrapper traverse() noexcept;
		TraversalWrapper operator+() noexcept;

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

			void increment(int cnt = 1) noexcept;
			void decrement(int cnt = 1) noexcept;

			bool is_begin() const noexcept;
			bool is_end() const noexcept;
			bool is_null() const noexcept;
			bool is_equal(DynamicIterator it) const noexcept;

			Element::ptr value() const noexcept;

			Element::ptr operator->() const noexcept;
			Element& operator*() const noexcept;

			bool operator==(const DynamicIterator& other) const noexcept;
			bool operator!=(const DynamicIterator& other) const noexcept;

			DynamicIterator& operator=(const DynamicIterator& copy) noexcept;
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
		virtual int _resolve_units_impl(Unit value, cptr elem) const noexcept override;
		virtual void _signal_context_change_impl(write_flag type, unsigned int prop, ptr element) noexcept override;
		virtual void _state_change_impl(State state, bool value) override;

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
			return create(Element::make<Type>(
				name,
				state(),
				shared_from_this()
			));
		}
		template<typename Type> TraversalWrapper override(const std::string& name = "")
		{
			return override(Element::make<Type>(
				name,
				state(),
				shared_from_this()
			));
		}
		TraversalWrapper create(ptr ptr)
		{
			ptr->setstate(State::Created);
			return _create_impl(ptr);
		}
		TraversalWrapper override(ptr ptr) noexcept
		{
			ptr->setstate(State::Created);
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

		virtual DynamicIterator begin() noexcept { return nullptr; }
		virtual DynamicIterator end() noexcept { return nullptr; }
		virtual void foreach_internal(foreach_internal_callback callback) const = 0;
		virtual void foreach(foreach_callback callback) const = 0;
	};
	class VecParentElement : public ParentElement
	{
	private:
		class LinearIteratorAdaptor;
	protected:
		std::vector<ptr> _elements{};

		virtual bool _empty_impl() const noexcept override;
		virtual bool _exists_impl(const std::string& name) const noexcept override;
		virtual bool _exists_impl(ptr ptr) const noexcept override;

		virtual const TraversalWrapper _at_impl(const std::string& name) const override;
		virtual TraversalWrapper _create_impl(ptr ptr) override;
		virtual TraversalWrapper _override_impl(ptr ptr) noexcept override;
		virtual void _remove_impl(const std::string& name) override;
		virtual void _remove_impl(ptr ptr) override;
		virtual void _clear_impl() noexcept override;

		std::size_t _find(const std::string& name) const noexcept;
		std::size_t _find(ptr ptr) const noexcept;
	public:
		using ParentElement::ParentElement;

		virtual DynamicIterator begin() noexcept override;
		virtual DynamicIterator end() noexcept override;
		virtual void foreach_internal(foreach_internal_callback callback) const override;
		virtual void foreach(foreach_callback callback) const override;
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
}

#endif // D2_TREE_ELEMENT_HPP
