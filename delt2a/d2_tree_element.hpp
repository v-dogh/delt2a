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
    class TreeIter
    {
    public:
        using foreach_callback = std::function<void(TreeIter)>;
    private:
        std::weak_ptr<Element> _elem{};
    public:
        TreeIter() = default;
        TreeIter(std::nullptr_t) {}
        TreeIter(const TreeIter&) = default;
        TreeIter(TreeIter&&) = default;
        template<typename Type>
        TreeIter(std::shared_ptr<Type> p) :
            _elem(std::static_pointer_cast<Element>(p))
        {}

        template<typename Type>
        bool is_type() const
        {
            if (_elem.expired())
                return false;
            return dynamic_cast<const Type*>(_elem.lock().get()) != nullptr;
        }
        bool is_type_of(TreeIter other) const;

        template<typename Type>
        void set(std::shared_ptr<Type> ptr = nullptr) noexcept
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
            return const_cast<const TreeIter*>(this)->as<Type>();
        }

        std::shared_ptr<ParentElement> asp() const
        {
            return as<ParentElement>();
        }
        std::shared_ptr<ParentElement> asp()
        {
            return as<ParentElement>();
        }

        TreeIter up() const;
        TreeIter up(std::size_t cnt) const;
        TreeIter up(const std::string& name) const;

        void foreach(foreach_callback);

        std::shared_ptr<Element> operator->() const noexcept;
        std::shared_ptr<Element> operator->() noexcept;

        Element& operator*() const noexcept;
        Element& operator*() noexcept;

        operator std::shared_ptr<Element>();
        operator std::weak_ptr<Element>();

        bool operator==(std::nullptr_t) const noexcept
        {
            return _elem.expired();
        }
        bool operator==(const TreeIter& other) const noexcept
        {
            return (_elem.expired() && other._elem.expired()) || (_elem.lock() == other._elem.lock());
        }
        bool operator!=(const TreeIter& other) const noexcept
        {
            return !operator==(other);
        }

        TreeIter operator/(const std::string& path);
        TreeIter operator^(std::size_t cnt);
        TreeIter operator+();

        TreeIter& operator=(const TreeIter&) = default;
        TreeIter& operator=(TreeIter&&) = default;
    };

    template<typename Type>
    class TypedTreeIter : public TreeIter
    {
    public:
        using type = Type;
    public:
        TypedTreeIter() = default;
        TypedTreeIter(std::nullptr_t) {}
        TypedTreeIter(const TypedTreeIter&) = default;
        TypedTreeIter(TypedTreeIter&&) = default;
        TypedTreeIter(const TreeIter& copy) : TreeIter(copy) {}
        TypedTreeIter(std::shared_ptr<Type> p) :
            TreeIter(std::static_pointer_cast<Element>(p))
        {}

        TypedTreeIter operator/(const std::string& path)
        {
            return TreeIter::operator/(path);
        }
        TypedTreeIter operator^(std::size_t cnt)
        {
            return TreeIter::operator^(cnt);
        }
        TypedTreeIter operator+()
        {
            return TreeIter::operator+();
        }

        std::shared_ptr<Type> operator->() const noexcept
        {
            return as<Type>();
        }
        std::shared_ptr<Type> operator->() noexcept
        {
            return as<Type>();
        }

        Type& operator*() const noexcept
        {
            return *as<Type>();
        }
        Type& operator*() noexcept
        {
            return *as<Type>();
        }

        TypedTreeIter& operator=(const TypedTreeIter&) = default;
        TypedTreeIter& operator=(TypedTreeIter&&) = default;
    };

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
        using pptr = std::shared_ptr<ParentElement>;
        using cpptr = std::shared_ptr<const ParentElement>;
        using pwptr = std::weak_ptr<ParentElement>;
        using iptr = TreeIter;

        // Acquires a shared_lock for the buffer for it's lifetime
        // Ensures object lifetime
        // Provides access to the framebuffer
        // Returned from Element::frame
        class Frame
        {
        private:
            cptr _object{ nullptr };
        public:
            explicit Frame(cptr obj) :
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

        using event_callback = std::function<void(EventListener, TreeIter)>;
        using foreach_callback = TreeIter::foreach_callback;
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
            // Set if the element is being initialized (prevents write signal propagation etc.)
            IsBeingInitialized = 1 << 1,
            // Set if the style is changed (i.e. requires a redraw)
            WasWritten = 1 << 2,
            // Set if the layout of the object has changed (for parent elements)
            WasWrittenLayout = 1 << 3,
            // Set when the x-position of the object was updated
            PositionXUpdated = 1 << 4,
            // Set when the y-position of the object was updated
            PositionYUpdated = 1 << 5,
            // Set when the width of the bounding box of the object was updated
            DimensionsWidthUpdated = 1 << 6,
            // Set when the height of the bounding box of the object was updated
            DimensionsHeightUpdated = 1 << 7,
            // Indicates that the object's framebuffer should not be compressed
            CachePolicyDynamic = 1 << 8,
            // Indicates that the object should me drawn in immediate mode
            CachePolicyVolatile = 1 << 9,
            // Indicates that the object's framebuffer should be preemptively compressed
            CachePolicyStatic = 1 << 10,
        };
        enum WriteType : write_flag
        {
            None = 0x00,
            Style = WasWritten,
            LayoutXPos = PositionXUpdated | WasWrittenLayout,
            LayoutYPos = PositionYUpdated | WasWrittenLayout,
            LayoutWidth = DimensionsWidthUpdated | WasWrittenLayout | Style,
            LayoutHeight = DimensionsHeightUpdated | WasWrittenLayout | Style,
            Dimensions = LayoutWidth | LayoutHeight,
            Offset = LayoutXPos | LayoutYPos,
            Masked = Style | Dimensions | Offset,
            Strip = LayoutXPos | LayoutYPos | LayoutWidth | LayoutHeight,
            StripPerm = Strip & ~(Style | WasWrittenLayout)
        };
        enum UnitState : unit_meta_flag
        {
            RelativeXPos = 1 << 0,
            RelativeYPos = 1 << 1,
            RelativeWidth = 1 << 2,
            RelativeHeight = 1 << 3,

            // Contextual - dependent on the parent (context)

            ContextualXPos = 1 << 4,
            ContextualYPos = 1 << 5,
            ContextualWidth = 1 << 6,
            ContextualHeight = 1 << 7,
        };

        enum class Layout : unsigned int
        {
            X,
            Y,
            Width,
            Height
        };
    private:
        struct LayoutStorage
        {
        private:
            std::array<int, 4> _storage{};
        public:
            LayoutStorage() = default;
            LayoutStorage(const LayoutStorage&) = default;
            LayoutStorage(LayoutStorage&&) = default;

            int get(Layout comp) const noexcept;
            void set(Layout comp, int value) noexcept;

            LayoutStorage& operator=(const LayoutStorage&) = default;
            LayoutStorage& operator=(LayoutStorage&&) = default;
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

        const std::string _name{};
        const TreeState::ptr _state_ptr{ nullptr };
        pwptr _parent{};

        // Flags

        mutable std::atomic<state_flag> _state{ Display | Swapped };
        mutable std::atomic<internal_flag> _internal_state{ WasWritten | WasWrittenLayout | IsBeingInitialized | CachePolicyStatic };

        // Listeners

        std::vector<EventListenerState::ptr> _subscribers{};

        // Rendering

        mutable LayoutStorage _layout{};
        mutable PixelBuffer _buffer{};
    private:
        // Listeners

        EventListener _push_listener(State event, bool value, event_callback callback) noexcept;
        void _unmute_listener(EventListenerState::ptr listener) noexcept;
        void _mute_listener(EventListenerState::ptr listener) noexcept;
        void _destroy_listener(EventListenerState::ptr listener) noexcept;
        void _trigger(State event, bool value);

        // Other stuff

        bool _contextual_change(write_flag flags) const noexcept;

        void _signal_write_internal(write_flag type = WriteType::Masked, unsigned int prop = ~0u) const noexcept;
        void _signal_write_local(write_flag type, unsigned int prop, ptr element) noexcept;
        void _signal_write_local(write_flag type = WriteType::Masked, unsigned int prop = ~0u) noexcept;
        void _invalidate_state(write_flag type) const noexcept;
    protected:
        // Write handling
        // Context changes occur when a parent element changes it's layout and a child has relative layout
        // Then we notify it to recompute it's values for the layout
        // Layout writes signal that the bounding box and position of an object changed
        // Style writes signal that either the framebuffer itself, or it's contents changed (for an element)
        // For example a resize is both a Style and Layout write since it requires a resize of the buffer and changes the box

        bool _is_write_type(write_flag type) const noexcept;

        virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) noexcept {}
        virtual void _signal_context_change_impl(write_flag type, unsigned int prop, ptr element) noexcept {}

        void _signal_write(write_flag type, unsigned int prop, ptr element) noexcept;
    public:
        void _signal_context_change(write_flag type, unsigned int prop, ptr element) noexcept;
        void _signal_context_change(write_flag type = WriteType::Masked, unsigned int prop = ~0u) noexcept;
        void _signal_write(write_flag type = WriteType::Masked, unsigned int prop = ~0u) noexcept;
        void _signal_initialization(unsigned int prop) noexcept;
        void _signal_write_update(write_flag type) const noexcept;
        void _signal_update(internal_flag type) const noexcept;
        void _trigger_event(IOContext::Event ev);
    protected:
        // Layout

        virtual Unit _layout_impl(Layout) const = 0;

        // Units

        virtual bool _provides_input_impl() const noexcept { return false; }
        virtual unit_meta_flag _unit_report_impl() const noexcept = 0;
        virtual char _index_impl() const noexcept = 0;

        // Events

        virtual void _event_impl(IOContext::Event) {}
        virtual void _state_change_impl(State, bool) {}

        // Layout


        virtual BoundingBox _reserve_buffer_impl() const noexcept { return box(); }
        virtual PixelBuffer::View _fetch_pixel_buffer_impl() const noexcept;
        virtual bool _provides_buffer_impl() const noexcept { return false; }

        // Core

        virtual void _update_layout_impl() noexcept {}
        virtual void _update_style_impl() noexcept {}
        virtual void _frame_impl(PixelBuffer::View) noexcept = 0;
    public:
        template<typename Type>
        static auto make(const std::string& name, TreeState::ptr state, pptr parent)
        {
            auto ptr = std::make_shared<Type>(name, state, parent);
            ptr->setstate(State::Created);
            return ptr;
        }

        Element() = default;
        Element(
            const std::string& name,
            TreeState::ptr state,
            pptr parent
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
        pptr parent() const noexcept;
        const std::string& name() const noexcept;

        // State

        bool getistate(internal_flag flags) const noexcept;
        void setcache(InternalState flag) const noexcept;
        bool getstate(State state) const noexcept;
        void setstate(State state, bool value = true);
        void setparent(pptr parent) noexcept;

        // Units

        unit_meta_flag unit_report() const noexcept;

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
        Position mouse_object_space() const noexcept;

        int layout(Layout type) const noexcept;
        Unit internal_layout(Layout type) const noexcept;

        void accept_layout() const noexcept;
        void accept_layout(Layout type) const noexcept;
        void override_layout(Layout type, int value) const noexcept;
        void override_dimensions(int width, int height) const noexcept;
        void override_position(int x, int y) const noexcept;

        bool contextual_layout(Layout type) const noexcept;
        bool relative_layout(Layout type) const noexcept;

        char getzindex() const noexcept;

        int resolve_units(Unit unit) const noexcept;

        TreeIter traverse() noexcept;
        TreeIter operator+() noexcept;

        Element& operator=(const Element&) = delete;
        Element& operator=(Element&&) = delete;
    };
}

#endif // D2_TREE_ELEMENT_HPP
