#ifndef D2_STYLES_HPP
#define D2_STYLES_HPP

#include <type_traits>
#include <bitset>
#include "d2_styles_base.hpp"
#include "d2_tree_element.hpp"
#include "d2_tree_parent.hpp"
#include "d2_element_units.hpp"
#include "d2_pixel.hpp"
#include "d2_screen.hpp"
#include "d2_chardef.hpp"

namespace d2::style
{
    namespace impl
    {
        template<typename Type>
        concept chained = requires
        {
            (Type::last_offset_);
        };

        template<typename Chain>
        struct ResolveChain
        {
            static constexpr auto offset = 0;
        };
        template<chained Chain>
        struct ResolveChain<Chain>
        {
            static constexpr auto offset = Chain::last_offset_;
        };

        template<D2_UAI_INTERFACE_TEMPL First, D2_UAI_INTERFACE_TEMPL... Rest>
        struct LastImpl
        {
            template<std::size_t Prop>
            using type = LastImpl<Rest...>::template type<Prop>;
        };
        template<D2_UAI_INTERFACE_TEMPL Last>
        struct LastImpl<Last>
        {
            template<std::size_t Prop>
            using type = Last<Prop>;
        };

        template<
            D2_UAI_INTERFACE_TEMPL Search,
            typename Current,
            D2_UAI_INTERFACE_TEMPL CurrentType,
            D2_UAI_INTERFACE_TEMPL Next,
            D2_UAI_INTERFACE_TEMPL... Rest
        >
        struct OffsetHelperImpl
        {
            using type = std::conditional<
                std::is_same_v<Next<0>, Search<0>>,
                Current,
                typename OffsetHelperImpl<
                    Search,
                    Next<Current::offset>,
                    Next,
                    Rest...
                >::type
            >::type;
        };
        template<
            D2_UAI_INTERFACE_TEMPL Search,
            typename Current,
            D2_UAI_INTERFACE_TEMPL CurrentType,
            D2_UAI_INTERFACE_TEMPL Next
        >
        struct OffsetHelperImpl<Search, Current, CurrentType, Next>
        {
            using type = Current;
        };

        template<
            std::size_t BaseOffset,
            D2_UAI_INTERFACE_TEMPL Current,
            D2_UAI_INTERFACE_TEMPL First,
            D2_UAI_INTERFACE_TEMPL... Rest
        >
        struct OffsetHelper
        {
            using result = OffsetHelperImpl<
                Current,
                First<BaseOffset>,
                First,
                Rest...
            >;
            using type = result::type;
            static constexpr auto offset = result::type::offset;
        };
        template<
            std::size_t BaseOffset,
            D2_UAI_INTERFACE_TEMPL Current,
            D2_UAI_INTERFACE_TEMPL First
        >
        struct OffsetHelper<BaseOffset, Current, First>
        {
            static constexpr auto offset = BaseOffset;
        };
    }

    // UAI provides read/write access to styles (member variables) of objects that derive from it (elements)
    // It handles proper synchronization and signals, and manages the process of assigning unique ID's to properties for elements at compile time
    // It also facilitates the ability for objects to derive from other objects through chaining
    // I.e. the topmost UAI is aware of the lower UAI and delegates their property accesses to it
    // Access to variables through UAI is also automatically synchronized
    // I.e. accesses from separate threads will be queued on the main thread by default
    // Main thread reads/writes should mostly be zero overhead
    // The ILayout interface is special as every UAI needs to have one in the base element

    template<std::size_t PropBase>
    struct ILayout;

    template<
        typename Chain,
        typename Base,
        D2_UAI_INTERFACE_TEMPL Interface,
        D2_UAI_INTERFACE_TEMPL... Interfaces
    >
    class UniversalAccessInterface :
        public Chain,
        public Interface<impl::ResolveChain<Chain>::offset>,
        public Interfaces<
        impl::OffsetHelper<
            impl::ResolveChain<Chain>::offset,
                Interfaces, Interface, Interfaces...
            >::offset
        >...
    {
    public:
        using property = uai_property;
        template<property Property>
        using type_of = decltype(get<Property>());
    private:
        template<std::size_t Prop>
        using last_interface =
            impl::LastImpl<Interface, Interfaces...>::template type<Prop>;
        static constexpr std::size_t base_offset_ = impl::ResolveChain<Chain>::offset;
        static constexpr std::size_t last_offset_ =
            impl::OffsetHelper<
                base_offset_,
                last_interface,
                Interface, Interfaces...
            >::offset + last_interface<0>::count;
        static constexpr std::size_t chain_base_ = impl::ResolveChain<Chain>::offset;
        static constexpr bool chain_ = impl::chained<Chain>;

        // Templates

        template<
            std::size_t Offset,
            D2_UAI_INTERFACE_TEMPL Search,
            D2_UAI_INTERFACE_TEMPL... Rest
        > struct SearchInterfaceImpl { };
        template<
            std::size_t Offset,
            D2_UAI_INTERFACE_TEMPL Search,
            D2_UAI_INTERFACE_TEMPL Current,
            D2_UAI_INTERFACE_TEMPL... Rest
        >
        struct SearchInterfaceImpl<Offset, Search, Current, Rest...>
        {
            using type = std::conditional<
                std::is_same_v<Search<0>, Current<0>>,
                Current<Offset>,
                typename SearchInterfaceImpl<
                    Current<Offset>::offset,
                    Search,
                    Rest...
                >::type
            >::type;
        };
        template<
            std::size_t Offset,
            D2_UAI_INTERFACE_TEMPL Search,
            D2_UAI_INTERFACE_TEMPL Current
        >
        struct SearchInterfaceImpl<Offset, Search, Current>
        {
            using type = std::conditional<
                std::is_same_v<Search<0>, Current<0>>,
                Current<Offset>,
                void
            >::type;
        };
        template<D2_UAI_INTERFACE_TEMPL Type>
        struct FindInterface
        {
            using type = SearchInterfaceImpl<
                base_offset_,
                Type,
                Interface, Interfaces...
            >;
        };

        template<
            std::size_t Search,
            typename Current,
            D2_UAI_INTERFACE_TEMPL... Rest
        > struct SearchPropertyOwnerImpl { };
        template<
            std::size_t Search,
            typename Current,
            D2_UAI_INTERFACE_TEMPL Next,
            D2_UAI_INTERFACE_TEMPL... Rest
        >
        struct SearchPropertyOwnerImpl<Search, Current, Next, Rest...>
        {
            using type = std::conditional<
                Search >= Current::base && Search < Current::offset,
                Current,
                typename SearchPropertyOwnerImpl<
                    Search,
                    Next<Current::offset>,
                    Rest...
                >::type
            >::type;
        };
        template<
            std::size_t Search,
            typename Current
        >
        struct SearchPropertyOwnerImpl<Search, Current>
        {
            using type = std::conditional<
                Search >= Current::base && Search < Current::offset,
                Current,
                void
            >::type;
        };
        template<
            std::size_t BaseOffset,
            std::size_t Search
        >
        struct SearchPropertyOwner
        {
            using type = SearchPropertyOwnerImpl<
                Search,
                Interface<BaseOffset>,
                Interfaces...
            >::type;
        };
    private:
        // Bit is set if the variable is dynamic (i.e. dependent on Theme::Dependency<T>)
        std::bitset<last_offset_ - base_offset_> _var_flags{};

        const Base* _base() const
        {
            return static_cast<const Base*>(this);
        }
        Base* _base()
        {
            return static_cast<Base*>(this);
        }
        auto _screen() const
        {
            return _base()->state()->screen();
        }
        auto _context() const
        {
            return _base()->state()->context();
        }

        template<property Property, typename Type>
        auto _int_set(Type&& value, bool temporary = false)
        {
            static_assert(Property <= last_offset_, "Invalid Property");
            using interface = SearchPropertyOwner<base_offset_, Property>::type;
            constexpr auto off = Property - base_offset_;

            if constexpr(impl::is_var<std::remove_cvref_t<Type>>)
            {
                _var_flags.set(off);
                value.subscribe(_handle_impl(), this, [](void* base, const auto& value, bool destroy) -> bool
                {
                    if (destroy)
                        return true;
                    auto* base_ptr = static_cast<UniversalAccessInterface*>(base);
                    if (!base_ptr->_var_flags.test(off))
                        return false;
                    base_ptr->template set<Property>(value);
                    base_ptr->_var_flags.set(off);
                    return true;
                });
            }
            else if constexpr(impl::is_dynavar<std::remove_cvref_t<Type>>)
            {
                _var_flags.set(off);
                value.dependency.subscribe(_handle_impl(), this, [](void* base, const auto& v, bool destroy) -> bool
                {
                    if (destroy)
                        return true;
                    auto* base_ptr = static_cast<UniversalAccessInterface*>(base);
                    if (!base_ptr->_var_flags.test(off))
                        return false;
                    base_ptr->template set<Property>(std::remove_cvref_t<decltype(value)>::filter(v));
                    base_ptr->_var_flags.set(off);
                    return true;
                });
            }
            else
            {
                _var_flags.set(off, _var_flags.test(off) && temporary);
                auto [ ptr, type ] = interface::template get<Property - interface::base>();
                *ptr = std::forward<Type>(value);
                _signal_base_impl(type, Property);
            }
        }
        template<property Property>
        auto _int_get_vals()
        {
            static_assert(Property <= last_offset_, "Invalid Property");
            using interface = SearchPropertyOwner<base_offset_, Property>::type;
            return interface::template get<Property - interface::base>();
        }
        template<property Property>
        auto _int_get()
        {
            auto [ ptr, _ ] = _int_get_vals<Property>();
            return *ptr;
        }
        template<property Property>
        auto* _int_getref()
        {
            auto [ ptr, _ ] = _int_get_vals<Property>();
            return ptr;
        }

        template<property Property>
        auto _int_get_vals_chained()
        {
            if constexpr(chain_)
            {
                if constexpr(Property < chain_base_)
                {
                    return static_cast<Chain&>(
                        static_cast<Base&>(*this)
                    ).template _int_get_vals<Property>();
                }
                else
                {
                    return _int_get_vals<Property>();
                }
            }
            else
            {
                return _int_get_vals<Property>();
            }
        }

        template<property Property, typename Type>
        void _int_set_synced(Type&& value, bool temporary = false)
        {
            const auto ctx = _context();
            if (ctx->is_synced())
            {
                _int_set<Property>(std::forward<Type>(value), temporary);
            }
            else
            {
                const auto _ = _base()->shared_from_this();
                if constexpr(impl::is_var<std::remove_cvref_t<Type>>)
                {
                    static_assert(std::is_reference_v<Type>, "Dependency must be a reference");
                    ctx->sync([this, &value, &temporary]()
                    {
                        _int_set<Property>(std::move(value), temporary);
                    }).sync();
                }
                else
                {
                    ctx->sync([this, value = std::forward<Type>(value), &temporary]()
                    {
                        _int_set<Property>(std::move(value), temporary);
                    }).sync();
                }
            }
        }
        template<property Property>
        auto _int_get_synced()
        {
            const auto ctx = _context();
            if (ctx->is_synced())
            {
                return _int_get<Property>();
            }
            else
            {
                const auto _ = _base()->shared_from_this();
                return ctx->sync([this]()
                {
                    return _int_get<Property>();
                }).value();
            }
        }
    protected:
        void* _has_interface_own_impl(std::size_t id)
        {
            return UniversalAccessInterface::_has_interface_impl(id);
        }
        virtual void _initialize_impl(bool force) override
        {
            if ((this->parent() && !this->parent()->getistate(Element::InternalState::IsBeingInitialized)) || force)
            {
                _base()->_signal_initialization(Element::initial_property);
                if constexpr (std::is_base_of_v<ParentElement, UniversalAccessInterface>)
                {
                    this->foreach_internal([](Element::ptr ptr) {
                        ptr->initialize();
                        return true;
                    });
                }
            }
        }
        virtual void _signal_base_impl(Element::write_flag type, property prop) override
        {
            if (type != 0x00 && !this->getistate(Element::InternalState::IsBeingInitialized))
                internal::ElementView::from(_base()->shared_from_this()).signal_write(type, prop);
        }
        virtual void _set_dynamic_impl(property prop, bool value) override
        {
            if constexpr(chain_)
            {
                if (prop < base_offset_)
                {
                    Chain::_set_dynamic_impl(prop, value);
                    return;
                }
            }
            _var_flags.set(prop - base_offset_, value);
        }
        virtual bool _test_dynamic_impl(property prop) override
        {
            if constexpr(chain_)
            {
                if (prop < base_offset_)
                    return Chain::_test_dynamic_impl(prop);
            }
            return _var_flags.test(prop - base_offset_);
        }
        virtual void* _has_interface_impl(std::size_t id) override
        {
            if constexpr(chain_)
            {
                auto* ptr =
                    static_cast<Chain::data*>(
                        static_cast<Base*>(this)
                    )->_has_interface_own_impl(id);
                if (ptr != nullptr)
                    return ptr;
            }

            void* out = nullptr;
            auto check_xid = [&id, &out, this]<D2_UAI_INTERFACE_TEMPL Type>()
            {
                const auto pred = Type<0>::xid == id;
                if (pred)
                    out = static_cast<Type<0>::data*>(this);
                return pred;
            };
            [[ maybe_unused ]] const auto result =
                check_xid.template operator()<Interface>() ||
                ((check_xid.template operator()<Interfaces>()) || ...);
            return out;
        }
        virtual void _sync_impl(std::function<void()> func) const override
        {
            const auto ctx = _context();
            if (ctx->is_synced())
            {
                func();
            }
            else
            {
                const auto _ = _base()->shared_from_this();
                return ctx->sync([&func]()
                {
                    func();
                }).sync();
            }
        }
        virtual std::weak_ptr<void> _handle_impl() override
        {
            return _base()->shared_from_this();
        }
    public:
        template<typename>
        friend class impl::ResolveChain;
        template<typename, typename, D2_UAI_INTERFACE_TEMPL, D2_UAI_INTERFACE_TEMPL...>
        friend class UniversalAccessInterface;

        using Chain::Chain;

        template<D2_UAI_INTERFACE_TEMPL Type>
        auto& interface()
        {
            if constexpr(chain_)
            {
                using resolved_chain = Chain::template FindInterface<Type>::type;
                if constexpr(std::is_same_v<void, typename resolved_chain::type>)
                {
                    return static_cast<FindInterface<Type>::type&>(*this);
                }
                else
                {
                    return static_cast<resolved_chain::type&>(*this);
                }
            }
            else
            {
                return static_cast<FindInterface<Type>::type&>(*this);
            }
        }

        template<property Property, typename Type>
        auto& set(Type&& value, bool temporary = false)
        {
            if constexpr(chain_)
            {
                if constexpr(Property < chain_base_)
                {
                    static_cast<Chain&>(
                        static_cast<Base&>(*this)
                    ).template set<Property>(
                        std::forward<Type>(value),
                        temporary
                    );
                }
                else
                {
                    _int_set_synced<Property>(std::forward<Type>(value), temporary);
                }
            }
            else
            {
                _int_set_synced<Property>(std::forward<Type>(value), temporary);
            }
            return static_cast<Base&>(*this);
        }
        template<property Property>
        auto get()
        {
            if constexpr(chain_)
            {
                if constexpr(Property < chain_base_)
                {
                    return static_cast<Chain&>(
                               static_cast<Base&>(*this)
                           ).template get<Property>();
                }
                else
                {
                    return _int_get_synced<Property>();
                }
            }
            else
            {
                return _int_get_synced<Property>();
            }
        }
        template<property Property>
        auto* getref()
        {
            if constexpr(chain_)
            {
                if constexpr(Property < chain_base_)
                {
                    return static_cast<Chain&>(
                               static_cast<Base&>(*this)
                           ).template getref<Property>();
                }
                else
                {
                    return _int_getref<Property>();
                }
            }
            else
            {
                return _int_getref<Property>();
            }
        }

        template<property Property, typename Func>
        auto apply_get(Func&& func)
        {
            const auto _ = _base()->shared_from_this();
            const auto ctx = _context();
            if (ctx->is_synced())
                return func(*getref<Property>());
            return ctx->sync([
                                 this,
                                 func = std::forward<Func>(func)
                             ]
            {
                return func(*getref<Property>());
            }).value();
        }
        template<property Property, typename Func>
        auto apply_set(Func&& func)
        {
            const auto _ = _base()->shared_from_this();
            const auto ctx = _context();
            if (ctx->is_synced())
            {
                _set_dynamic_impl(Property, false);
                auto [ var, type ] = _int_get_vals_chained<Property>();
                if constexpr(std::is_same_v<decltype(func(*var)), void>)
                {
                    func(*var);
                    _signal_base_impl(type, Property);
                }
                else
                {
                    auto result = func(*var);
                    _signal_base_impl(type, Property);
                    return result;
                }
            }
            else
            {
                return ctx->sync([
                                     this,
                                     func = std::forward<Func>(func)
                                 ]
                {
                    _set_dynamic_impl(Property, false);
                    auto [ var, type ] = _int_get_vals_chained<Property>();
                    if constexpr(std::is_same_v<decltype(func(*var)), void>)
                    {
                        func(*var);
                        _signal_base_impl(type, Property);
                    }
                    else
                    {
                        auto result = func(*var);
                        _signal_base_impl(type, Property);
                        return result;
                    }
                }).value();
            }
        }
    };
    template<
        typename Elem,
        typename Base,
        D2_UAI_INTERFACE_TEMPL Interface,
        D2_UAI_INTERFACE_TEMPL... Interfaces
        >
    class UniversalAccessInterfaceProto :
        public UniversalAccessInterface<
            Elem, Base, Interface, Interfaces...
        >
    {
    private:
        using base = UniversalAccessInterface<
            Elem, Base, Interface, Interfaces...
        >;
        static_assert(
            std::is_same_v<Interface<0>, ILayout<0>> || (std::is_same_v<Interfaces<0>, ILayout<0>> || ...),
            "Base Universal Access Interface needs a Layout interface"
        );
    protected:
        virtual Unit _layout_impl(enum Element::Layout type) const override
        {
            switch (type)
            {
                case Element::Layout::X: return this->x;
                case Element::Layout::Y: return this->y;
                case Element::Layout::Width: return this->width;
                case Element::Layout::Height: return this->height;
            }
        }
        virtual char _index_impl() const override final
        {
            return this->zindex;
        }
        virtual Element::unit_meta_flag _unit_report_impl() const override final
        {
            return
                (Element::RelativeXPos * this->x.relative()) |
                (Element::RelativeYPos * this->y.relative()) |
                (Element::RelativeWidth * this->width.relative()) |
                (Element::RelativeHeight * this->height.relative()) |
                (Element::ContextualXPos * this->x.contextual()) |
                (Element::ContextualYPos * this->y.contextual()) |
                (Element::ContextualWidth * this->width.contextual()) |
                (Element::ContextualHeight * this->height.contextual()) |
                (Element::InvertedXPos * this->x.inverted()) |
                (Element::InvertedYPos * this->y.inverted()) |
                (Element::InvertedWidth * this->width.inverted()) |
                (Element::InvertedHeight * this->height.inverted());
        }
    public:
        using base::base;
    };

    template<
        typename Base,
        D2_UAI_INTERFACE_TEMPL Interface,
        D2_UAI_INTERFACE_TEMPL... Interfaces
    >
    using UAI = UniversalAccessInterfaceProto<Element, Base, Interface, Interfaces...>;
    template<
        typename Elem,
        typename Base,
        D2_UAI_INTERFACE_TEMPL Interface,
        D2_UAI_INTERFACE_TEMPL... Interfaces
    >
    using UAIE = UniversalAccessInterfaceProto<Elem, Base, Interface, Interfaces...>;
    template<
        typename Chain,
        typename Base,
        D2_UAI_INTERFACE_TEMPL Interface,
        D2_UAI_INTERFACE_TEMPL... Interfaces
    >
    using UAIC = UniversalAccessInterface<Chain, Base, Interface, Interfaces...>;
    using UAIB = UniversalAccessInterfaceBase;

    namespace impl
    {
        template<typename Type>
        struct BitFlagPtrWrapper
        {
            Type* ptr{ nullptr };
            Type mask{ 0x00 };

            BitFlagPtrWrapper& operator*()
            {
                return *this;
            }
            BitFlagPtrWrapper& operator=(bool value)
            {
                *ptr &= ~mask;
                *ptr |= (mask * value);
                return *this;
            }

            BitFlagPtrWrapper& operator&()
            {
                return *this;
            }
        };
        using uchar_bit_ptr = BitFlagPtrWrapper<unsigned char>;

        struct XAllocInterfaceTracker
        {
            static inline std::size_t id{ 0 };
        };
        template<template<std::size_t> typename>
        struct XAllocInterface
        {
        protected:
            static inline const std::size_t xid{ XAllocInterfaceTracker::id++ };
        };

        template<template<std::size_t> typename Type, std::size_t PropBase, std::size_t Count>
        struct InterfaceHelper : impl::XAllocInterface<Type>
        {
            static constexpr std::size_t base = PropBase;
            static constexpr std::size_t offset = PropBase + Count;
            static constexpr std::size_t count = Count;
        };
    }
    template<template<std::size_t> typename Type, std::size_t PropBase, std::size_t Count>
    using InterfaceHelper = impl::InterfaceHelper<Type, PropBase, Count>;
    // DSL
    namespace
    {
#       define D2_UAI_INTERFACE(_name_, _opts_, _fields_, _props_, _links_) \
            namespace uai { \
                template<std::size_t PropBase> \
                struct Props_##_name_ { \
                    static constexpr std::size_t base = 0; \
                    _props_ \
                    public: \
                        static constexpr auto _prop_count = _prot_prop_count; \
                }; \
                struct Data_##_name_ { \
                    private: \
                    static constexpr std::size_t base = 0; \
                    _props_; \
                    public: \
                    _opts_; \
                    protected: \
                    _fields_; \
                    _links_; \
                }; \
            } \
            template<std::size_t PropBase> \
            struct I##_name_ : \
                uai::Data_##_name_, \
                ::d2::style::impl::XAllocInterface<I##_name_> \
            { \
            private: \
                static constexpr std::size_t count = uai::Props_##_name_<PropBase>::_prop_count; \
                static constexpr std::size_t base = PropBase; \
                static constexpr std::size_t offset = PropBase + count; \
                using XAllocInterface<I##_name_>::xid; \
            protected: \
                using Data_##_name_::Data_##_name_; \
                using uai::Data_##_name_::get; \
                using data = uai::Data_##_name_; \
            public: \
                template< \
                    typename Chain, \
                    typename Base, \
                    D2_UAI_INTERFACE_TEMPL Interface, \
                    D2_UAI_INTERFACE_TEMPL... Interfaces \
                > \
                friend class UniversalAccessInterface; \
                template< \
                    D2_UAI_INTERFACE_TEMPL Search, \
                    typename Current, \
                    D2_UAI_INTERFACE_TEMPL CurrentType, \
                    D2_UAI_INTERFACE_TEMPL Next, \
                    D2_UAI_INTERFACE_TEMPL... Rest \
                > \
                friend class impl::OffsetHelperImpl; \
                template< \
                    std::size_t BaseOffset, \
                    D2_UAI_INTERFACE_TEMPL Current, \
                    D2_UAI_INTERFACE_TEMPL First, \
                    D2_UAI_INTERFACE_TEMPL... Rest \
                > \
                friend class impl::OffsetHelper; \
                _props_; \
            }; \
            using IZ##_name_ = I##_name_<0>;
#       define D2_UAI_OPTS(...) __VA_ARGS__;
#       define D2_UAI_FIELDS(...) __VA_ARGS__;
#       define D2_UAI_PROPS(_first_, ...) \
            protected: \
                static constexpr auto _prot_prop_count = []() { \
                    enum class PropCnt \
                    { \
                        _first_, \
                        __VA_ARGS__ __VA_OPT__(,) \
                        Count \
                    }; \
                    return std::size_t(PropCnt::Count); \
                }(); \
            public: \
                enum Property : uai_property \
                { \
                    _first_ = base, \
                    __VA_ARGS__ \
                };
#       define D2_UAI_LINK(...) \
            template<uai_property Property> \
            auto get() \
            { \
                using enum Element::WriteType;\
                if constexpr (false) {} \
                __VA_ARGS__ \
                else \
                { \
                    static_assert(false, "Invalid Property"); \
                    return std::make_pair(nullptr, static_cast<unsigned char>(0x00)); \
                } \
            }
#       define D2_UAI_PROP(_prop_, _var_, ...) \
            else if constexpr (Property == unsigned(_prop_)) \
                return std::make_pair(&_var_, static_cast<unsigned char>(__VA_ARGS__));
#       define D2_UAI_PROP_VAR(...) __VA_ARGS__
    }

    D2_UAI_INTERFACE(Layout,
        D2_UAI_OPTS(),
        D2_UAI_FIELDS(
            HPUnit x{ Unit::Auto };
            VPUnit y{ Unit::Auto };
            HDUnit width{ Unit::Auto };
            VDUnit height{ Unit::Auto };
            char zindex{ 0 };
        ),
        D2_UAI_PROPS(X, Y, Width, Height, ZIndex),
        D2_UAI_LINK(
            D2_UAI_PROP(X, x, LayoutXPos)
            D2_UAI_PROP(Y, y, LayoutYPos)
            D2_UAI_PROP(Width, width, Style | LayoutWidth)
            D2_UAI_PROP(Height, height, Style | LayoutHeight)
            D2_UAI_PROP(ZIndex, zindex, LayoutXPos | LayoutYPos)
        )
    )

    D2_UAI_INTERFACE(Container,
        D2_UAI_OPTS(
            enum ContainerOptions : unsigned char
            {
                EnableBorder = 1 << 0,
                OverrideCorners = 1 << 1,

                DisableBorderLeft = 1 << 2,
                DisableBorderRight = 1 << 3,
                DisableBorderTop = 1 << 4,
                DisableBorderBottom = 1 << 5,

                TopFix = 1 << 6,
            }
        ),
        D2_UAI_FIELDS(
            HDUnit border_width{ 1 };
            Pixel border_horizontal_color{ .a = 0, .v = charset::box_border_horizontal };
            Pixel border_vertical_color{ .a = 0, .v = charset::box_border_vertical };

            struct
            {
                Pixel::value_type top_left{ charset::box_tl_corner };
                Pixel::value_type top_right{ charset::box_tr_corner };
                Pixel::value_type bottom_left{ charset::box_bl_corner };
                Pixel::value_type bottom_right{ charset::box_br_corner };
            } corners;

            unsigned char container_options {
                (charset::top_fix * TopFix) |
                (charset::corners * OverrideCorners)
            };
        ),
        D2_UAI_PROPS(
            BorderWidth,
            BorderHorizontalColor,
            BorderVerticalColor,

            CornerTopLeft,
            CornerTopRight,
            CornerBottomLeft,
            CornerBottomRight,

            ContainerOptions,

            ContainerBorder,
            ContainerDisableLeft,
            ContainerDisableRight,
            ContainerDisableTop,
            ContainerDisableBottom,

            ContainerTopFix,
            ContainerOverrideCorners
        ),
        D2_UAI_LINK(
            D2_UAI_PROP(BorderWidth, border_width, Style)
            D2_UAI_PROP(BorderHorizontalColor, border_horizontal_color, Style)
            D2_UAI_PROP(BorderVerticalColor, border_vertical_color, Style)
            D2_UAI_PROP(CornerTopLeft, corners.top_left, Style)
            D2_UAI_PROP(CornerTopRight, corners.top_right, Style)
            D2_UAI_PROP(CornerBottomLeft, corners.bottom_left, Style)
            D2_UAI_PROP(CornerBottomRight, corners.bottom_right, Style)
            D2_UAI_PROP(ContainerOptions, container_options, Element::WriteType::Masked)
            D2_UAI_PROP(ContainerBorder, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, EnableBorder)), Masked)
            D2_UAI_PROP(ContainerDisableLeft, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderLeft)), Masked)
            D2_UAI_PROP(ContainerDisableRight, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderRight)), Masked)
            D2_UAI_PROP(ContainerDisableTop, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderTop)), Masked)
            D2_UAI_PROP(ContainerDisableBottom, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, DisableBorderBottom)), Masked)
            D2_UAI_PROP(ContainerTopFix, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, TopFix)), Style)
            D2_UAI_PROP(ContainerOverrideCorners, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&container_options, OverrideCorners)), Style)
        )
    )

    D2_UAI_INTERFACE(Colors,
        D2_UAI_OPTS(),
        D2_UAI_FIELDS(
            PixelBackground background_color{ .a = 0 };
            PixelForeground foreground_color{ .a = 0 };
        ),
        D2_UAI_PROPS(BackgroundColor, ForegroundColor),
        D2_UAI_LINK(
            D2_UAI_PROP(BackgroundColor, background_color, Style)
            D2_UAI_PROP(ForegroundColor, foreground_color, Style)
        )
    )

    D2_UAI_INTERFACE(KeyboardNav,
        D2_UAI_OPTS(),
        D2_UAI_FIELDS(
            PixelBackground focused_color { .r = 150, .g = 150, .b = 150 };
        ),
        D2_UAI_PROPS(FocusedColor),
        D2_UAI_LINK(
            D2_UAI_PROP(FocusedColor, focused_color, Style)
        )
    )

    D2_UAI_INTERFACE(Text,
        D2_UAI_OPTS(
            enum class Alignment
            {
                Left,
                Center,
                Right,
            };
            enum TextOption
            {
                PreserveWordBoundaries = 1 << 0,
                HandleNewlines = 1 << 1,
                Paragraph = PreserveWordBoundaries | HandleNewlines
            };
        ),
        D2_UAI_FIELDS(
            string text{};
            Alignment alignment{ Alignment::Center };
            unsigned char text_options{ Paragraph };
        ),
        D2_UAI_PROPS(
            Value,
            TextAlignment,
            TextOptions,
            TextPreserveWordBoundaries,
            TextHandleNewlines
        ),
        D2_UAI_LINK(
            D2_UAI_PROP(Value, text, Style)
            D2_UAI_PROP(TextAlignment, alignment, Style)
            D2_UAI_PROP(TextOptions, text_options, Style)
            D2_UAI_PROP(TextPreserveWordBoundaries, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&text_options, PreserveWordBoundaries)), Style)
            D2_UAI_PROP(TextHandleNewlines, D2_UAI_PROP_VAR(impl::uchar_bit_ptr(&text_options, HandleNewlines)), Style)
        )
    )

    template<typename Type, typename... Argv>
    struct Responsive
    {
        std::function<void(d2::TreeIter<Type>, Argv...)> on_submit{};

        template<uai_property Property>
        auto get()
        {
            if constexpr (Property == 0) return std::make_pair(&on_submit, static_cast<unsigned char>(0x00));
            else
            {
                static_assert(false, "Invalid Property"); \
                return std::make_pair(nullptr, static_cast<unsigned char>(0x00)); \
            }
        }
    };

    template<typename Type, typename... Argv>
    struct IResponsive
    {
        template<std::size_t PropBase>
        struct type :
            Responsive<Type, Argv...>,
            impl::InterfaceHelper<type, PropBase, 1>
        {
            using data = Responsive<Type, Argv...>;
            enum Property : uai_property
            {
                OnSubmit = PropBase,
            };
        };
    };
    using IZResponsive = IResponsive<d2::Element>::type<0>;
}

#endif // D2_STYLES_HPP
