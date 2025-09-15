#ifndef D2_DSL_HPP
#define D2_DSL_HPP

#include "d2_tree.hpp"
#include <list>

// DSL macros (oh yeah macaroles!!! so many macaroles!!!!)

#define D2_THEME(name, ...) struct name : ::d2::style::Theme __VA_OPT__(,) __VA_ARGS__ {
#define D2_THEME_END(...) }
#define D2_DEPENDENCY(type, name) Dependency<type> name;
#define D2_DEPENDENCY_LINK(dname, name) virtual decltype(name)& dname() override { return name; }

#define D2_EMBED_NAMED(_type_, _name_, ...) { \
    auto __uptr = __ptr; \
    auto __nsrc = _type_::build_sub(#_name_, __uptr->state(), __state->root() __VA_OPT__(,) __VA_ARGS__); \
    auto __nptr = _type_::create_at(__nsrc->core(), __nsrc); \
    __nsrc->swap_in(); }
#define D2_EMBED(_type_, ...) D2_EMBED_NAMED(_type_,, __VA_ARGS__)
#define D2_EMBED_ELEM_NAMED(_type_, _name_, ...) \
    __ptr.asp()->override(::d2::Element::make<_type_>( \
        #_name_, __state, __ptr.asp() __VA_OPT__(,) __VA_ARGS__ \
    ));
#define D2_EMBED_ELEM(_type_, ...) D2_EMBED_ELEM_NAMED(_type_,, __VA_ARGS__)

#define D2_STATEFUL_TREE_ROOT(_alias_, _state_, _root_, ...) \
struct _alias_ : ::d2::TreeTemplateInit<#_alias_, _state_, _alias_, _root_> { \
        using __state_type = _state_; \
        static ::d2::TreeIter create_at(::d2::TreeIter __ptr, std::shared_ptr<__state_type> __state __VA_OPT__(,) __VA_ARGS__) { \
            using __object_type = _root_; \
            using namespace d2::dx; \
            auto& state = __state; \
            auto& ptr   = __ptr;
#define D2_STATEFUL_TREE(_alias_, _state_, ...) D2_STATEFUL_TREE_ROOT(_alias_, _state_, ::d2::dx::Box, __VA_ARGS__)
#define D2_STATELESS_TREE_ROOT(_alias_, _root_, ...) D2_STATEFUL_TREE_ROOT(_alias_, ::d2::TreeState, _root_, __VA_ARGS__)
#define D2_STATELESS_TREE(_alias_, ...) D2_STATEFUL_TREE(_alias_, ::d2::TreeState, __VA_ARGS__)

#define D2_STATEFUL_TREE_ROOT_FORWARD(_alias_, _state_, _root_) \
    struct _alias_ : ::d2::TreeTemplateInit<#_alias_, _state_, _alias_, _root_> { \
        using __state_type = _state_; \
        static ::d2::TreeIter create_at(::d2::TreeIter __ptr, ::d2::TreeState::ptr __state); };
#define D2_STATEFUL_TREE_FORWARD(_alias_, _state_) D2_STATEFUL_TREE_ROOT_FORWARD(_alias_, _state_, ::d2::dx::Box)
#define D2_STATELESS_TREE_ROOT_FORWARD(_alias_, _root_) D2_STATEFUL_TREE_ROOT_FORWARD(_alias_, ::d2::TreeState, _root_)
#define D2_STATELESS_TREE_FORWARD(_alias_) D2_STATEFUL_TREE_FORWARD(_alias_, ::d2::TreeState)

#define D2_TREE_DEFINE(_alias_) \
    ::d2::TreeIter _alias_::create_at(::d2::TreeIter __ptr, ::d2::TreeState::ptr __state) { \
        using __object_type = ::d2::dx::Box; \
        using namespace d2::dx;
#define D2_TREE_DEFINITION_END return __ptr; }
#define D2_TREE_END return __ptr; }};

#define D2_CREATE_OBJECT(_type_, _name_) { \
    using __object_type = _type_; \
    auto& __uptr = __ptr; \
    auto  __nptr = ::d2::Element::make<_type_>(_name_, __state, __uptr.asp()); \
    auto  __ptr  = ::d2::TypedTreeIter<_type_>(__nptr); \
    auto& state  = __state; \
    auto  ptr    = ::d2::TypedTreeIter<_type_>(__nptr);
#define D2_STYLE(_prop_, ...) __ptr.template as<__object_type>()->template set<__object_type::_prop_>(__VA_ARGS__);
#define D2_REQUIRE(...) __ptr.template as<__object_type>()->constraint(__VA_ARGS__);
#define D2_ELEM(_type_, ...) D2_CREATE_OBJECT(_type_, #__VA_ARGS__)
#define D2_ELEM_STR(_type_, _name_) D2_CREATE_OBJECT(_type_, _name_)
#define D2_ELEM_END __uptr.asp()->override(__nptr); }
#define D2_ANCHOR(_type_, __VA_ARGS__) \
    auto __##__VA_ARGS__ = ::d2::Element::make<_type_>(#__VA_ARGS__, __state, nullptr); \
    auto __VA_ARGS__    = ::d2::TypedTreeIter<_type_>(__##__VA_ARGS__);
#define D2_ANCHOR_STATE(_type_, __VA_ARGS__) \
    __state->__VA_ARGS__ = ::d2::Element::make<_type_>(#__VA_ARGS__, __state, nullptr); \
    auto __##__VA_ARGS__ = state->__VA_ARGS__; \
    auto __VA_ARGS__    = ::d2::TypedTreeIter<_type_>(__state->__VA_ARGS__);
#define D2_ELEM_ANCHOR(...) { \
    __VA_ARGS__->setparent(__ptr.asp()); \
    using __object_type = decltype(__VA_ARGS__)::type; \
    auto& __uptr = __ptr; \
    auto  __nptr = __##__VA_ARGS__; \
    auto  __ptr  = ::d2::TypedTreeIter<__object_type>(__nptr); \
    auto& state  = __state; \
    auto  ptr    = __VA_ARGS__;

#define D2_STYLESHEET_BEGIN(name) \
    struct name { \
        template<typename Type> \
        static void apply(::d2::TypedTreeIter<Type> __ptr, ::d2::TreeState::ptr __state) { \
            using __object_type = Type; \
            auto  __uptr = ::d2::TypedTreeIter<::d2::ParentElement>(__ptr->parent()); \
            auto& state  = __state; \
            auto& ptr    = __ptr;
#define D2_STYLESHEET_END }};
#define D2_STYLES_APPLY_MANUAL(name, ptr, state) name::apply(::d2::TypedTreeIter(ptr), state);
#define D2_STYLES_APPLY(name) D2_STYLES_APPLY_MANUAL(name, __ptr, __state)

#define D2_VAR(type, var) __state->screen()->theme<type>().var
#define D2_VAR_TYPE(type, var) typename std::remove_cvref_t<decltype(D2_VAR(type, var))>::value_type
#define D2_DYNAVAR(type, var, ...) ::d2::style::dynavar< \
    [](const D2_VAR_TYPE(type, var)& value) { return __VA_ARGS__; }>(D2_VAR(type, var))

#define D2_STATIC(_name_, ...) \
    static thread_local std::list<__VA_ARGS__> __values##_name_; \
    static thread_local __VA_ARGS__* _name_; \
    __values##_name_.emplace_front(); \
    __ptr->listen(::d2::Element::State::Created, false, [&, __iterator = __values##_name_.begin()](auto&&...) { \
        __values##_name_.erase(__iterator); \
    }); \
    _name_ = &__values##_name_.front();

// Helpers
// EEXPR variants do not include line breaks
// EXPR and EEXPR expect expressions as arguments
// For future you: initialize a D2_CONTEXT for those task wrappers (listen, listen_global etc.)

#define D2_CONTEXT(ptrv, typev) \
    { ::d2::TreeIter __ptr = ptrv; \
        ::d2::TreeState::ptr __state = ptrv->state(); \
        using __object_type = typev;
#define D2_CONTEXT_EXT(ptrv, typev) \
    D2_CONTEXT(ptrv, typev) \
    auto ptr = ::d2::TypedTreeIter<typev>(__ptr);
#define D2_CONTEXT_RED(statev) \
    { ::d2::TreeState::ptr __state = statev;
#define D2_CONTEXT_END }
#define D2_VOID_EXPR , void()

#define D2_SYNC_EEXPR(...) (state->screen()->is_synced() ? \
(__VA_ARGS__) : state->screen()->sync([=]() -> decltype(auto) { return __VA_ARGS__; }).value())
#define D2_SYNC_EXPR(...) D2_SYNC_EEXPR(__VA_ARGS__);
#define D2_SYNC_BLOCK state->screen()->sync_if([=]{
#define D2_SYNC_BLOCK_END });

#define D2_LISTEN_EEXPR(event, value, ...) [=]() -> void { D2_LISTEN_EXPR(event, value, __VA_ARGS__) }()
#define D2_LISTEN_EXPR(event, value, ...) D2_LISTEN(event, value) (__VA_ARGS__); D2_LISTEN_END
#define D2_LISTEN(event, value) \
    __ptr->listen(::d2::Element::event, value, \
    [=](::d2::Element::EventListener listener, ::d2::TreeIter __lptr) { D2_CONTEXT_EXT(__lptr, __object_type)
#define D2_LISTEN_END D2_CONTEXT_END });

#define D2_LISTEN_GLOBAL_EEXPR(event, ...) [=]() -> void { D2_LISTEN_GLOBAL_EXPR(event, __VA_ARGS__) }()
#define D2_LISTEN_GLOBAL_EXPR(event, ...) D2_LISTEN_GLOBAL(event) (__VA_ARGS__); D2_LISTEN_GLOBAL_END
#define D2_LISTEN_GLOBAL(event) \
    __state->context()->listen<::d2::IOContext::Event::event>( \
    [=](::d2::IOContext::EventListener listener, ::d2::IOContext::ptr ctx, auto&&...) {
#define D2_LISTEN_GLOBAL_END });

#define D2_ASYNC_EEXPR(...) [=]() -> void { D2_ASYNC_EXPR(__VA_ARGS__) }()
#define D2_ASYNC_EXPR(...) D2_ASYNC_TASK (__VA_ARGS__); D2_ASYNC_END
#define D2_ASYNC_TASK __state->context()->scheduler()->launch_task([=]() {
#define D2_ASYNC_END });

#define D2_CYCLIC_EEXPR(time, ...) [=]() -> void { D2_CYCLIC_EXPR(time, __VA_ARGS__) }()
#define D2_CYCLIC_EXPR(time, ...) D2_CYCLIC_TASK(time) (__VA_ARGS__); D2_CYCLIC_END
#define D2_CYCLIC_TASK(time) \
    { constexpr auto __cyclic_time = std::chrono::milliseconds(time); \
        __state->context()->scheduler()->launch_cyclic_task([=](auto task) {
#define D2_CYCLIC_END }, __cyclic_time); }

#define D2_DEFER_EEXPR(time, ...) [=]() -> void { D2_DEFER_EXPR(time, __VA_ARGS__) }()
#define D2_DEFER_EXPR(time, ...) D2_DEFER_TASK(time) (__VA_ARGS__); D2_DEFERRED_TASK_END
#define D2_DEFER_TASK(time) \
    { constexpr auto __defer_time = std::chrono::milliseconds(time); \
        __state->context()->scheduler()->launch_deferred_task([=]() -> void {
#define D2_DEFERRED_TASK_END }, __defer_time); }

// Interpolation

#define D2_INTERPOLATE_IMPL(time, interpolator, ptr, style, dest, ...) \
    [&]() { \
        using __object_type = decltype(::d2::TypedTreeIter(ptr))::type; \
        using __type = decltype(std::declval<__object_type>().template get<__object_type::style>());\
        return ptr->state()->screen()->template interpolate<::d2::interp::interpolator<__object_type, __object_type::style>>( \
                std::chrono::milliseconds(time), \
                ptr, \
                dest \
                __VA_OPT__(,) __VA_ARGS__ \
        ); \
    }()
#define D2_INTERPOLATE_EVENT_IMPL(event, sstate, time, interpolator, ptr, style, dest, ...) \
    { \
        using __object_type = decltype(::d2::TypedTreeIter(ptr))::type; \
        ptr->listen(::d2::Element::State::event, sstate, [=](auto, auto ptr) { \
            const auto tptr = ptr.template as<__object_type>(); \
            using __type = decltype(std::declval<__object_type>().template get<object::style>());\
            tptr->state()->screen()->template interpolate<::d2::interp::interpolator<__object_type, object::style>>( \
                    std::chrono::milliseconds(time), \
                    tptr, \
                    dest \
                    __VA_OPT__(,) __VA_ARGS__ \
            ); \
        }); \
    }
#define D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, ptr, style, dest, ...) \
    { \
        using __object_type = decltype(::d2::TypedTreeIter(ptr))::type; \
        using __type = decltype(std::declval<__object_type>().template get<__object_type::style>()); \
        static bool __is_on{ false }; \
        static __type __saved_initial{}; \
        static std::chrono::steady_clock::time_point __rev_timestamp{}; \
        ptr->listen(::d2::Element::State::event, true, [=](auto, auto ptr) { \
            const auto tptr = ptr.template as<__object_type>(); \
            const bool __finished = tptr->template get<__object_type::style>() == static_cast<__type>(dest); \
            if (!__is_on && !__finished) \
            { \
                if ((std::chrono::steady_clock::now() - __rev_timestamp) >= std::chrono::milliseconds(time)) \
                    __saved_initial = tptr->template get<__object_type::style>(); \
                __rev_timestamp = std::chrono::steady_clock::time_point(); \
                tptr->state()->screen()->template interpolate<::d2::interp::interpolator<__object_type, __object_type::style>>( \
                        std::chrono::milliseconds(time), \
                        tptr, \
                        dest \
                        __VA_OPT__(,) __VA_ARGS__ \
                ); \
            } \
            else if (__finished) \
            { \
                __is_on = false; \
            } \
        }); \
        ptr->listen(::d2::Element::State::event, false, [=](auto, auto ptr) { \
            const auto tptr = ptr.template as<__object_type>(); \
            __rev_timestamp = std::chrono::steady_clock::now(); \
            tptr->state()->screen()->template interpolate<::d2::interp::interpolator<__object_type, __object_type::style>>( \
                    std::chrono::milliseconds(time), \
                    tptr, \
                    __saved_initial \
                    __VA_OPT__(,) __VA_ARGS__ \
            ); \
        }); \
    }
#define D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, ptr, style, dest, ...) \
    { \
        using __object_type = decltype(::d2::TypedTreeIter(ptr))::type; \
        using __type = decltype(std::declval<__object_type>().template get<__object_type::style>()); \
        static bool __is_on{ false }; \
        static __type __saved_initial{}; \
        static std::chrono::steady_clock::time_point __rev_timestamp{}; \
        ptr->listen(::d2::Element::State::Clicked, false, [=](auto, auto ptr) { \
            const auto tptr = ptr.template as<__object_type>(); \
            const bool __finished = tptr->template get<__object_type::style>() == static_cast<__type>(dest); \
            if (!__is_on) \
            { \
                if ((std::chrono::steady_clock::now() - __rev_timestamp) >= std::chrono::milliseconds(time)) \
                    __saved_initial = tptr->template get<__object_type::style>(); \
                __rev_timestamp = std::chrono::steady_clock::time_point(); \
                tptr->state()->screen()->template interpolate<::d2::interp::interpolator<__object_type, __object_type::style>>( \
                    std::chrono::milliseconds(time), \
                    tptr, \
                    dest \
                    __VA_OPT__(,) __VA_ARGS__ \
                ); \
            } \
            else \
            { \
                __rev_timestamp = std::chrono::steady_clock::now(); \
                tptr->state()->screen()->template interpolate<::d2::interp::interpolator<__object_type, __object_type::style>>( \
                    std::chrono::milliseconds(time), \
                    tptr, \
                    __saved_initial \
                    __VA_OPT__(,) __VA_ARGS__ \
                ); \
            } \
            __is_on = !__is_on; \
        }); \
    }

// Manual

#define D2_INTERPOLATE(time, interpolator, ptr, style, dest) \
    D2_INTERPOLATE_IMPL(time, interpolator, ptr, style, dest)
#define D2_INTERPOLATE_GEN(time, interpolator, ptr, style, dest, ...) \
    D2_INTERPOLATE_IMPL(time, interpolator, ptr, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_TOGGLE(time, interpolator, ptr, style, dest) \
    D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, ptr, style, dest)
#define D2_INTERPOLATE_TOGGLE_GEN(time, interpolator, ptr, style, dest, ...) \
    D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, ptr, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_ACQ(event, time, interpolator, ptr, style, dest) \
    D2_INTERPOLATE_EVENT_IMPL(event, true, time, interpolator, ptr, style, dest)
#define D2_INTERPOLATE_ACQ_GEN(event, time, interpolator, ptr, style, dest, ...) \
    D2_INTERPOLATE_EVENT_IMPL(event, true, time, interpolator, ptr, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_REL(event, time, interpolator, ptr, style, dest) \
    D2_INTERPOLATE_EVENT_IMPL(event, false, time, interpolator, ptr, object, style, dest)
#define D2_INTERPOLATE_REL_GEN(event, time, interpolator, ptr, style, dest, ...) \
    D2_INTERPOLATE_EVENT_IMPL(event, false, time, interpolator, ptr, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_TWOWAY(event, time, interpolator, ptr, style, dest) \
    D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, ptr, style, dest)
#define D2_INTERPOLATE_TWOWAY_GEN(event, time, interpolator, ptr, style, dest, ...) \
    D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, ptr, style, dest,  __VA_ARGS__)

// Automatic (i.e. gets all of the variables from the D2_CONTEXT parameters)

#define D2_INTERPOLATE_AUTO(time, interpolator, style, dest) \
    D2_INTERPOLATE_IMPL(time, interpolator, __ptr, style, dest)
#define D2_INTERPOLATE_GEN_AUTO(time, interpolator, style, dest, ...) \
    D2_INTERPOLATE_IMPL(time, interpolator, __ptr, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_TOGGLE_AUTO(time, interpolator, style, dest) \
    D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, __ptr, style, dest)
#define D2_INTERPOLATE_TOGGLE_GEN_AUTO(time, interpolator, style, dest, ...) \
    D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, __ptr, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_ACQ_AUTO(event, time, interpolator, style, dest) \
    D2_INTERPOLATE_EVENT_IMPL(event, true, time, interpolator, __ptr, style, dest)
#define D2_INTERPOLATE_ACQ_GEN_AUTO(event, time, interpolator, style, dest, ...) \
    D2_INTERPOLATE_EVENT_IMPL(event, true, time, interpolator, __ptr, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_REL_AUTO(event, time, interpolator, style, dest) \
    D2_INTERPOLATE_EVENT_IMPL(event, false, time, interpolator, __ptr, style, dest)
#define D2_INTERPOLATE_REL_GEN_AUTO(event, time, interpolator, style, dest, ...) \
    D2_INTERPOLATE_EVENT_IMPL(event, false, time, interpolator, __ptr, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_TWOWAY_AUTO(event, time, interpolator, style, dest) \
    D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, __ptr, style, dest)
#define D2_INTERPOLATE_TWOWAY_GEN_AUTO(event, time, interpolator, style, dest, ...) \
    D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, __ptr, style, dest,  __VA_ARGS__)

#endif // D2_DSL_HPP
