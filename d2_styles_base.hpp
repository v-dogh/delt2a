#ifndef D2_STYLES_BASE_HPP
#define D2_STYLES_BASE_HPP

#include <algorithm>
#include <d2_io_handler.hpp>
#include <d2_theme.hpp>
#include <d2_tree_element_frwd.hpp>
#include <functional>
#include <memory>
#include <type_traits>

namespace d2::style
{
    namespace impl
    {
#define D2_UAI_INTERFACE_TEMPL template<std::size_t> typename
#define D2_UAI_CHAIN(base)                                                                         \
    using data::set;                                                                               \
    using data::get;                                                                               \
    using data::interface;                                                                         \
    using data::data;
    } // namespace impl

    using uai_property = unsigned int;

    // I <heart emoji> templates

    namespace impl
    {
        template<typename Type> struct is_var_impl : std::false_type
        {
        };

        template<typename Arg> struct is_var_impl<Theme::Dependency<Arg>> : std::true_type
        {
        };

        template<typename Type>
        concept is_var = (is_var_impl<Type>::value);

        template<typename Type> struct is_dynavar_impl : std::false_type
        {
        };

        template<is_var Dep, auto Func> struct is_dynavar_impl<Dynavar<Func, Dep>> : std::true_type
        {
        };

        template<typename Type>
        concept is_dynavar = (is_dynavar_impl<Type>::value);
    } // namespace impl

    class UniversalAccessInterfaceBase
    {
    public:
        using property = unsigned int;
        static constexpr auto initial_property = std::numeric_limits<property>::max() - 1;
        static constexpr auto invalid_property = std::numeric_limits<property>::max();
    private:
        template<D2_UAI_INTERFACE_TEMPL Interface> auto* _interface_for()
        {
            return static_cast<Interface<0>*>(_has_interface_impl(Interface<0>::xid));
        }

        template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property, typename Type>
        auto& _int_set_for(Type&& value, bool temporary = false)
        {
            auto* interface = _interface_for<Interface>();
            if (interface == nullptr)
                throw std::logic_error{"Type does not implement interface"};

            if constexpr (impl::is_var<Type>)
            {
                _set_dynamic_impl(Property, true);
                value.subscribe(
                    _handle_impl(),
                    this,
                    [](void* base, const auto& value) -> bool
                    {
                        auto* base_ptr = static_cast<UniversalAccessInterfaceBase*>(base);
                        if (!base_ptr->_test_dynamic_impl(Property))
                            return false;
                        base_ptr->set_for<Interface, Property>(value);
                        base_ptr->_set_dynamic_impl(Property, true);
                        return true;
                    }
                );
            }
            else if constexpr (impl::is_dynavar<std::remove_cvref_t<Type>>)
            {
                _set_dynamic_impl(Property, true);
                value.dependency.subscribe(
                    _handle_impl(),
                    this,
                    [](void* base, const auto& v) -> bool
                    {
                        auto* base_ptr = static_cast<UniversalAccessInterfaceBase*>(base);
                        if (!base_ptr->_test_dynamic_impl(Property))
                            return false;
                        base_ptr->set_for<Interface, Property>(
                            std::remove_cvref_t<decltype(value)>::filter(v)
                        );
                        base_ptr->_set_dynamic_impl(Property, true);
                        return true;
                    }
                );
            }
            else
            {
                _set_dynamic_impl(Property, _test_dynamic_impl(Property) && temporary);
                auto [ptr, type] = interface->template get<Property>();
                *ptr = std::forward<Type>(value);
                _signal_base_impl(type, Property);
            }
            return *this;
        }
        template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property>
        auto _int_get_for() const
        {
            return *getref_for<Interface, Property>();
        }
    protected:
        virtual void _initialize_impl(bool force) = 0;
        virtual void _signal_base_impl(element_write_flag, property) = 0;
        virtual void _set_dynamic_impl(property, bool) = 0;
        virtual bool _test_dynamic_impl(property) = 0;
        virtual void* _has_interface_impl(std::size_t) = 0;
        virtual std::shared_ptr<IOContext> _context_impl() const = 0;
        virtual std::weak_ptr<void> _handle_impl() = 0;
    public:
        template<D2_UAI_INTERFACE_TEMPL Interface> bool has_interface()
        {
            return _interface_for<Interface>() != nullptr;
        }

        template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property, typename Type>
        auto& set_for(Type&& value, bool temporary = false)
        {
            auto ctx = _context_impl();
            if (ctx->is_synced())
            {
                _int_set_for<Interface, Property>(std::forward<Type>(value), temporary);
            }
            else
            {
                ctx->sync(
                       [value = std::forward<Type>(value), &temporary, this]() mutable
                       { _int_set_for<Interface, Property>(std::forward<Type>(value), temporary); }
                ).value();
            }
            return *this;
        }
        template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property>
        auto get_for() const
        {
            using type = std::remove_cvref_t<decltype(*getref_for<Interface, Property>())>;
            type result;
            auto ctx = _context_impl();
            if (ctx->is_synced())
            {
                result = _int_get_for<Interface, Property>();
            }
            else
            {
                result = ctx->sync(
                                [&result, this]() { result = _int_get_for<Interface, Property>(); }
                ).value();
            }
            return result;
        }
        template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property>
        auto* getref_for()
        {
            auto* interface = _interface_for<Interface>();
            if (interface == nullptr)
                throw std::logic_error{"Type does not implement interface"};
            auto [ptr, _] = interface->template get<Property>();
            return ptr;
        }

        template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property, typename Func>
        auto apply_get_for(Func&& func)
        {
            auto ctx = _context_impl();
            if (ctx->is_synced())
            {
                return func(*getref_for<Interface, Property>());
            }
            else
            {
                return ctx
                    ->sync([this, func = std::forward<Func>(func)]
                           { return func(*getref_for<Interface, Property>()); })
                    .value();
            }
        }
        template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property, typename Func>
        auto apply_set_for(Func&& func)
        {
            return _context_impl()
                ->sync(
                    [this, func = std::forward<Func>(func)]
                    {
                        _set_dynamic_impl(Property, false);
                        auto* interface = _interface_for<Interface>();
                        auto [var, type] = interface->template get<Property>();
                        if constexpr (std::is_same_v<decltype(func(*var)), void>)
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
                )
                .value();
        }

        void initialize(bool force = false)
        {
            _initialize_impl(force);
        }
    };
} // namespace d2::style

#endif // D2_STYLES_BASE_HPP
