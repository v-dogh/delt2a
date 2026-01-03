#ifndef D2_SDSL_HPP
#define D2_SDSL_HPP

#include "d2_tree.hpp"
#include <list>
#include <string_view>

namespace d2::dx {}
namespace d2::ctm {}

  ////////////////
 /// FUN TIME ///
////////////////

// You can use this instead of IDE breakpoints since macros kind of break them
#ifdef NDEBUG
    #define _D2_IMPL_BREAKPOINT
#else
    #if defined(_MSC_VER)
        #define _D2_IMPL_BREAKPOINT __debugbreak()
    #elif defined(__GNUC__) || defined(__clang__)
        #if defined(__has_builtin)
            #if __has_builtin(__builtin_debugtrap)
                #define _D2_IMPL_BREAKPOINT __builtin_debugtrap();
            #elif __has_builtin(__builtin_trap)
                #define _D2_IMPL_BREAKPOINT __builtin_trap();
            #else
                #define _D2_IMPL_BREAKPOINT asm("int $3");
            #endif
        #else
            #if (defined(__i386__) || defined(__x86_64__))
                #define _D2_IMPL_BREAKPOINT asm("int $3");
            #else
                #define _D2_IMPL_BREAKPOINT __builtin_trap();
            #endif
        #endif
    #else
        #define _D2_IMPL_BREAKPOINT static_assert(false, "_D2_IMPL_BREAKPOINT is not defined for this platform");
    #endif
#endif

  ////////////////
 /// SUBTREES ///
////////////////

#define _D2_IMPL_TREE_STATE(...) auto __args_tree_state = std::tuple(__VA_ARGS__);
#define _D2_IMPL_TREE_ARGS(...)  auto __args_tree       = std::tuple(__VA_ARGS__);

#define _D2_IMPL_EMBED_NAMED_STR(_type_, _name_, ...) { \
    auto __args_tree = std::tuple(); \
    auto __args_tree_state = std::tuple(); \
    { __VA_ARGS__ \
    auto __uptr = __ptr; \
    auto __nsrc = std::apply([&]<typename... Argv>(Argv&&... args) { \
        return _type_::build_sub(_name_, __uptr->state(), __ptr.asp(), std::forward<Argv>(args)...); \
    }, __args_tree_state); \
    auto __nptr = __nsrc->core(); \
    std::apply([&]<typename... Argv>(Argv&&... args) { \
        _type_::create_at(__nsrc->core(), __nsrc, std::forward<Argv>(args)...); \
    }, __args_tree); \
    __nsrc->swap_in(); } }

#define _D2_IMPL_EMBED_NAMED(_type_, _name_, ...) _D2_IMPL_EMBED_NAMED_STR(_type_, #_name_, __VA_ARGS__)
#define _D2_IMPL_EMBED(_type_, ...) _D2_IMPL_EMBED_NAMED(_type_,, __VA_ARGS__)

#define _D2_IMPL_EMBED_ELEM_NAMED_STR(_type_, _name_, ...) \
    __ptr.asp()->override(::d2::Element::make<_type_>( \
        _name_, __state, __ptr.asp() __VA_OPT__(,) __VA_ARGS__ \
    ));

#define _D2_IMPL_EMBED_ELEM_NAMED(_type_, _name_, ...) _D2_IMPL_EMBED_ELEM_NAMED_STR(_type_, #_name_, __VA_ARGS__)
#define _D2_IMPL_EMBED_ELEM(_type_, ...) _D2_IMPL_EMBED_ELEM_NAMED(_type_,, __VA_ARGS__)

#define _D2_IMPL_EMBED_NAMED_STR_BEGIN(_type_, _name_, ...) { \
    auto __args_tree = std::tuple(); \
    auto __args_tree_state = std::tuple(); \
    { __VA_ARGS__ \
    auto __uptr = __ptr; \
    auto __nsrc = std::apply([&]<typename... Argv>(Argv&&... args) { \
        return _type_::build_sub(_name_, __uptr->state(), __ptr.asp(), std::forward<Argv>(args)...); \
    }, __args_tree_state); \
    auto __nptr = __nsrc->core(); \
    std::apply([&]<typename... Argv>(Argv&&... args) { \
        _type_::create_at(__nsrc->core(), __nsrc, std::forward<Argv>(args)...); \
    }, __args_tree); \
    __nsrc->swap_in(); \
    _D2_IMPL_CONTEXT_EXT(__nptr, _type_::root, __nsrc, _type_::state)

#define _D2_IMPL_EMBED_NAMED_BEGIN(_type_, _name_, ...) _D2_IMPL_EMBED_NAMED_STR_BEGIN(_type_, #_name_, __VA_ARGS__)
#define _D2_IMPL_EMBED_BEGIN(_type_, ...)              _D2_IMPL_EMBED_NAMED_BEGIN(_type_,, __VA_ARGS__)

#define _D2_IMPL_EMBED_ELEM_NAMED_STR_BEGIN(_type_, _name_, ...) {{ \
    auto __nptr = __ptr.asp()->override(::d2::Element::make<_type_>( \
        _name_, __state, __ptr.asp() __VA_OPT__(,) __VA_ARGS__ \
    )); \
    _D2_IMPL_CONTEXT(__nptr, _type_)

#define _D2_IMPL_EMBED_ELEM_NAMED_BEGIN(_type_, _name_, ...) _D2_IMPL_EMBED_ELEM_NAMED_STR_BEGIN(_type_, #_name_, __VA_ARGS__)
#define _D2_IMPL_EMBED_ELEM_BEGIN(_type_, ...)              _D2_IMPL_EMBED_ELEM_NAMED_BEGIN(_type_,, __VA_ARGS__)

#define _D2_IMPL_EMBED_END _D2_IMPL_CONTEXT_END }}

  /////////////
 /// TREES ///
/////////////

#define _D2_IMPL_STATEFUL_TREE_ROOT(_alias_, _state_, _root_, ...) \
struct _alias_ : ::d2::TreeTemplateInit<#_alias_, _state_, _alias_, _root_> { \
        using __state_type = _state_; \
        static ::d2::TreeIter create_at(::d2::TreeIter __ptr, std::shared_ptr<__state_type> __state __VA_OPT__(,) __VA_ARGS__) { \
            using __object_type = _root_; \
            using namespace d2::dx; \
            using namespace d2::ctm; \
            auto& state = __state; \
            auto& ptr   = __ptr;

#define _D2_IMPL_STATEFUL_TREE(_alias_, _state_, ...)      _D2_IMPL_STATEFUL_TREE_ROOT(_alias_, _state_, ::d2::dx::Box, __VA_ARGS__)
#define _D2_IMPL_STATELESS_TREE_ROOT(_alias_, _root_, ...) _D2_IMPL_STATEFUL_TREE_ROOT(_alias_, ::d2::TreeState, _root_, __VA_ARGS__)
#define _D2_IMPL_STATELESS_TREE(_alias_, ...)              _D2_IMPL_STATEFUL_TREE(_alias_, ::d2::TreeState, __VA_ARGS__)

#define _D2_IMPL_STATEFUL_TREE_ROOT_FORWARD(_alias_, _state_, _root_, ...) \
    struct _alias_ : ::d2::TreeTemplateInit<#_alias_, _state_, _alias_, _root_> { \
        using __state_type = _state_; \
        static ::d2::TreeIter create_at(::d2::TreeIter __ptr, ::d2::TreeState::ptr __state __VA_OPT__(,) __VA_ARGS__); };

#define _D2_IMPL_STATEFUL_TREE_FORWARD(_alias_, _state_, ...)      _D2_IMPL_STATEFUL_TREE_ROOT_FORWARD(_alias_, _state_, ::d2::dx::Box, __VA_ARGS__)
#define _D2_IMPL_STATELESS_TREE_ROOT_FORWARD(_alias_, _root_, ...) _D2_IMPL_STATEFUL_TREE_ROOT_FORWARD(_alias_, ::d2::TreeState, _root_, __VA_ARGS__)
#define _D2_IMPL_STATELESS_TREE_FORWARD(_alias_, ...)              _D2_IMPL_STATEFUL_TREE_FORWARD(_alias_, ::d2::TreeState, __VA_ARGS__)

#define _D2_IMPL_TREE_DEFINE(_alias_, ...) \
    ::d2::TreeIter _alias_::create_at(::d2::TreeIter __uptr, ::d2::TreeState::ptr __ustate __VA_OPT__(,) __VA_ARGS__) { \
        { using __object_type = ::d2::dx::Box; \
        using namespace ::d2::dx; \
        using namespace d2::ctm; \
        auto __ptr = ::d2::TypedTreeIter<_alias_::root>(__uptr); \
        auto __state = std::static_pointer_cast<_alias_::state>(__ustate); \
        auto ptr = __ptr; \
        auto state = __state;

#define _D2_IMPL_TREE_DEFINITION_END } return __uptr; }
#define _D2_IMPL_TREE_END return __ptr; }};

#define _D2_IMPL_TREE_TAG(_name_, _value_) \
    __state->screen()->tags().set(std::string(#_name_), (_value_)); \
    { \
        using type = std::remove_cvref_t<decltype(_value_)>; \
        using ss = std::string_view; \
        constexpr auto name = std::string_view(#_name_); \
        static_assert( \
            (name == ss("SwapClean") && std::is_same_v<type, bool>) || \
            (name == ss("SwapOut") && std::is_same_v<type, bool>) || \
            (name == ss("OverrideRefresh") && std::is_same_v<type, std::chrono::milliseconds>), \
            "Invalid type or tag name (use _D2_IMPL_TREE_CTAG for non-standard tags to bypass the checks)" \
        ); \
    }

#define _D2_IMPL_TREE_CTAG(_name_, _value_) __state->screen()->tags().set(std::string(#_name_), (_value_));

  ////////////////////////
 /// ELEMENT CREATION ///
////////////////////////

#define _D2_IMPL_CREATE_OBJECT(_type_, _name_) { \
    using __object_type = _type_; \
    auto& __uptr = __ptr; \
    auto  __nptr = ::d2::Element::make<_type_>(_name_, __state, __uptr.asp()); \
    auto  __ptr  = ::d2::TypedTreeIter<_type_>(__nptr); \
    auto& state  = __state; \
    auto  ptr    = ::d2::TypedTreeIter<_type_>(__nptr);

#define _D2_IMPL_STYLE(_prop_, ...) __ptr.template as<__object_type>()->template set<__object_type::_prop_>(__VA_ARGS__);
#define _D2_IMPL_REQUIRE(...) __ptr.template as<__object_type>()->constraint(__VA_ARGS__);

#define _D2_IMPL_ELEM(_type_, ...) _D2_IMPL_CREATE_OBJECT(_type_, #__VA_ARGS__)
#define _D2_IMPL_ELEM_STR(_type_, _name_) _D2_IMPL_CREATE_OBJECT(_type_, _name_)

#define _D2_IMPL_ELEM_END __uptr.asp()->override(__nptr); }

#define _D2_IMPL_ANCHOR(_type_, _name_) \
    auto __##_name_ = ::d2::Element::make<_type_>(#_name_, __state, nullptr); \
    auto _name_     = ::d2::TypedTreeIter<_type_>(__##_name_);
#define _D2_IMPL_ANCHOR_STATE(_type_, _name_) \
    __state->_name_ = ::d2::Element::make<_type_>(#_name_, __state, nullptr); \
    auto __##_name_ = state->_name_; \
    auto _name_     = ::d2::TypedTreeIter<_type_>(__state->_name_);
#define _D2_IMPL_ELEM_ANCHOR(_name_) { \
    _name_->setparent(__ptr.asp()); \
    using __object_type = decltype(_name_)::type; \
    auto& __uptr = __ptr; \
    auto  __nptr = __##_name_; \
    auto  __ptr  = ::d2::TypedTreeIter<__object_type>(__nptr); \
    auto& state  = __state; \
    auto  ptr    = _name_;

  ///////////////////
 /// STYLESHEETS ///
///////////////////

#define _D2_IMPL_STYLESHEET_BEGIN(_name_) \
    struct _name_ { \
        template<typename Type> \
        static void apply(::d2::TypedTreeIter<Type> __ptr, ::d2::TreeState::ptr __state) { \
            using __object_type = Type; \
            auto  __uptr = ::d2::TypedTreeIter<::d2::ParentElement>(__ptr->parent()); \
            auto& state  = __state; \
            auto& ptr    = __ptr;

#define _D2_IMPL_STYLESHEET_END }};

#define _D2_IMPL_STYLES_APPLY_MANUAL(_name_, _ptr_, _state_) _name_::apply(::d2::TypedTreeIter(_ptr_), _state_);
#define _D2_IMPL_STYLES_APPLY(_name_) _D2_IMPL_STYLES_APPLY_MANUAL(_name_, __ptr, __state)

  ///////////////
 /// THEMING ///
///////////////

#define _D2_IMPL_THEME(_name_, ...) struct _name_ : ::d2::style::Theme __VA_OPT__(,) __VA_ARGS__ {
#define _D2_IMPL_THEME_END(...) }

#define _D2_IMPL_DEPENDENCY(_type_, _name_) Dependency<_type_> _name_;
#define _D2_IMPL_DEPENDENCY_LINK(_dname_, _name_) virtual decltype(_name_)& _dname_() override { return _name_; }

#define _D2_IMPL_DEP(_name_, _type_) __state->screen()->deps().get<_type_>(#_name_)
#define _D2_IMPL_TDEP(_tree_, _name_, _type_) __state->screen()->deps(#_tree_).get<_type_>(#_name_)

#define _D2_IMPL_VAR(_type_, _var_) __state->screen()->theme<_type_>()._var_
#define _D2_IMPL_VAR_TYPE(_type_, _var_) typename std::remove_cvref_t<decltype(_D2_IMPL_VAR(_type_, _var_))>::value_type

#define _D2_IMPL_DYNAVAR(_type_, _var_, ...) ::d2::style::dynavar< \
    [](const _D2_IMPL_VAR_TYPE(_type_, _var_)& value) { return __VA_ARGS__; }>(_D2_IMPL_VAR(_type_, _var_))

// Safe per-element storage (no duplicated data across multiple instances)
#define _D2_IMPL_STATIC(_name_, ...) \
    static thread_local std::list<std::shared_ptr<__VA_ARGS__>> __values##_name_; \
    __values##_name_.emplace_front(std::make_shared<__VA_ARGS__>()); \
    __ptr->listen(::d2::Element::State::Created, false, [&, __iterator = __values##_name_.begin()](auto&&...) { \
        __values##_name_.erase(__iterator); \
    }); \
    std::shared_ptr<__VA_ARGS__> _name_ = __values##_name_.front();

  ///////////////
 /// CONTEXT ///
///////////////

#define _D2_IMPL_CONTEXT(_ptrv_, _typev_) \
    { ::d2::TreeIter __ptr = _ptrv_; \
        ::d2::TreeState::ptr __state = __ptr->state(); \
        using __object_type = _typev_; \
        auto ptr = ::d2::TypedTreeIter<_typev_>(__ptr);

#define _D2_IMPL_CONTEXT_EXT(_ptrv_, _typev_, _statev_, _stypev_) \
    _D2_IMPL_CONTEXT(_ptrv_, _typev_) \
    auto state = std::static_pointer_cast<_stypev_>(_statev_);

#define _D2_IMPL_CONTEXT_RED(_statev_) { ::d2::TreeState::ptr __state = _statev_;

#define _D2_IMPL_CONTEXT_AUTO(_ptrv_) { \
    using __object_type = decltype(_ptrv_)::type; \
    ::d2::TypedTreeIter __ptr = _ptrv_; \
    ::d2::TreeState::ptr __state = __ptr->state(); \
    auto ptr = __ptr;

#define _D2_IMPL_CONTEXT_END }

  ////////////////////////////
 /// HELPERS/UNIFORMATORS ///
////////////////////////////

#define _D2_IMPL_VOID_EXPR , void()

// EEXPR variants do not include line breaks
#define _D2_IMPL_SYNC_EEXPR(...) (state->context()->is_synced() ? \
(__VA_ARGS__) : state->context()->sync([=]() -> decltype(auto) { return __VA_ARGS__; }).value())

#define _D2_IMPL_SYNC_EXPR(...) _D2_IMPL_SYNC_EEXPR(__VA_ARGS__);
#define _D2_IMPL_SYNC_BLOCK     state->context()->sync_if([=]{
#define _D2_IMPL_SYNC_BLOCK_END });

#define _D2_IMPL_LISTEN_EEXPR(_event_, _value_, ...) [=]() -> void { _D2_IMPL_LISTEN_EXPR(_event_, _value_, __VA_ARGS__) }()
#define _D2_IMPL_LISTEN_EXPR(_event_, _value_, ...)  _D2_IMPL_LISTEN(_event_, _value_) (__VA_ARGS__); _D2_IMPL_LISTEN_END

#define _D2_IMPL_LISTEN(_event_, _value_) \
    __ptr->listen(::d2::Element::_event_, _value_, \
    [=](::d2::Element::EventListener listener, ::d2::TreeIter __lptr) { _D2_IMPL_CONTEXT(__lptr, __object_type)

#define _D2_IMPL_LISTEN_END _D2_IMPL_CONTEXT_END });

#define _D2_IMPL_LISTEN_GLOBAL_EEXPR(_event_, ...) [=]() -> void { _D2_IMPL_LISTEN_GLOBAL_EXPR(_event_, __VA_ARGS__) }()
#define _D2_IMPL_LISTEN_GLOBAL_EXPR(_event_, ...)  _D2_IMPL_LISTEN_GLOBAL(_event_) (__VA_ARGS__); _D2_IMPL_LISTEN_GLOBAL_END

#define _D2_IMPL_LISTEN_GLOBAL(_event_) \
    __state->context()->listen<::d2::IOContext::Event::_event_>( \
    [=](::d2::IOContext::EventListener listener, ::d2::IOContext::ptr ctx, auto&&...) {

#define _D2_IMPL_LISTEN_GLOBAL_END });

#define _D2_IMPL_ASYNC_EEXPR(...) [=]() -> void { _D2_IMPL_ASYNC_EXPR(__VA_ARGS__) }()
#define _D2_IMPL_ASYNC_EXPR(...)  _D2_IMPL_ASYNC_TASK (__VA_ARGS__); _D2_IMPL_ASYNC_END
#define _D2_IMPL_ASYNC_TASK       __state->context()->scheduler()->launch_task([=]() {
#define _D2_IMPL_ASYNC_END        });

#define _D2_IMPL_CYCLIC_EEXPR(_time_, ...) [=]() -> void { _D2_IMPL_CYCLIC_EXPR(_time_, __VA_ARGS__) }()
#define _D2_IMPL_CYCLIC_EXPR(_time_, ...)  _D2_IMPL_CYCLIC_TASK(_time_) (__VA_ARGS__); _D2_IMPL_CYCLIC_END

#define _D2_IMPL_CYCLIC_TASK(_time_) \
    { constexpr auto __cyclic_time = std::chrono::milliseconds(_time_); \
        __state->context()->scheduler()->launch_cyclic_task([=](auto task) {

#define _D2_IMPL_CYCLIC_END }, __cyclic_time); }

#define _D2_IMPL_DEFER_EEXPR(_time_, ...) [=]() -> void { _D2_IMPL_DEFER_EXPR(_time_, __VA_ARGS__) }()
#define _D2_IMPL_DEFER_EXPR(_time_, ...)  _D2_IMPL_DEFER_TASK(_time_) (__VA_ARGS__); _D2_IMPL_DEFERRED_TASK_END

#define _D2_IMPL_DEFER_TASK(_time_) \
    { constexpr auto __defer_time = std::chrono::milliseconds(_time_); \
        __state->context()->scheduler()->launch_deferred_task([=]() -> void {

#define _D2_IMPL_DEFERRED_TASK_END }, __defer_time); }

  /////////////////////
 /// INTERPOLATION ///
/////////////////////

#define _D2_IMPL_INTERPOLATE_IMPL(_time_, _interpolator_, _ptr_, _style_, _dest_, ...) \
    [&]() { \
        using __object_type = decltype(::d2::TypedTreeIter(_ptr_))::type; \
        using __type = decltype(std::declval<__object_type>().template get<__object_type::_style_>()); \
        return _ptr_->state()->screen()->template interpolate<::d2::interp::_interpolator_<__object_type, __object_type::_style_>>( \
                std::chrono::milliseconds(_time_), \
                _ptr_, \
                _dest_ \
                __VA_OPT__(,) __VA_ARGS__ \
        ); \
    }()
#define _D2_IMPL_INTERPOLATE_EVENT_IMPL(_event_, _sstate_, _time_, _interpolator_, _ptr_, _style_, _dest_, ...) \
    { \
        using __object_type = decltype(::d2::TypedTreeIter(_ptr_))::type; \
        using __type = decltype(std::declval<__object_type>().template get<__object_type::_style_>()); \
        _ptr_->listen(::d2::Element::State::_event_, _sstate_, [=](auto, auto ptr) { \
            const auto tptr = ptr.template as<__object_type>(); \
            tptr->state()->screen()->template interpolate<::d2::interp::_interpolator_<__object_type, __object_type::_style_>>( \
                    std::chrono::milliseconds(_time_), \
                    tptr, \
                    _dest_ \
                    __VA_OPT__(,) __VA_ARGS__ \
            ); \
        }); \
    }
#define _D2_IMPL_INTERPOLATE_EVENT_TWOWAY_IMPL(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, ...) \
    { \
        using __object_type = decltype(::d2::TypedTreeIter(_ptr_))::type; \
        using __type = decltype(std::declval<__object_type>().template get<__object_type::_style_>()); \
        static bool __is_on{ false }; \
        static __type __saved_initial{}; \
        static std::chrono::steady_clock::time_point __rev_timestamp{}; \
        _ptr_->listen(::d2::Element::State::_event_, true, [=](auto, auto ptr) { \
            const auto tptr = ptr.template as<__object_type>(); \
            const bool __finished = tptr->template get<__object_type::_style_>() == static_cast<__type>(_dest_); \
            if (!__is_on && !__finished) \
            { \
                if ((std::chrono::steady_clock::now() - __rev_timestamp) >= std::chrono::milliseconds(_time_)) \
                    __saved_initial = tptr->template get<__object_type::_style_>(); \
                __rev_timestamp = std::chrono::steady_clock::time_point(); \
                tptr->state()->screen()->template interpolate<::d2::interp::_interpolator_<__object_type, __object_type::_style_>>( \
                        std::chrono::milliseconds(_time_), \
                        tptr, \
                        _dest_ \
                        __VA_OPT__(,) __VA_ARGS__ \
                ); \
            } \
            else if (__finished) \
            { \
                __is_on = false; \
            } \
        }); \
        _ptr_->listen(::d2::Element::State::_event_, false, [=](auto, auto ptr) { \
            const auto tptr = ptr.template as<__object_type>(); \
            __rev_timestamp = std::chrono::steady_clock::now(); \
            tptr->state()->screen()->template interpolate<::d2::interp::_interpolator_<__object_type, __object_type::_style_>>( \
                    std::chrono::milliseconds(_time_), \
                    tptr, \
                    __saved_initial \
                    __VA_OPT__(,) __VA_ARGS__ \
            ); \
        }); \
    }
#define _D2_IMPL_INTERPOLATE_TOGGLE_IMPL(_time_, _interpolator_, _ptr_, _style_, _dest_, ...) \
    { \
        using __object_type = decltype(::d2::TypedTreeIter(_ptr_))::type; \
        using __type = decltype(std::declval<__object_type>().template get<__object_type::_style_>()); \
        static bool __is_on{ false }; \
        static __type __saved_initial{}; \
        static std::chrono::steady_clock::time_point __rev_timestamp{}; \
        _ptr_->listen(::d2::Element::State::Clicked, false, [=](auto, auto ptr) { \
            const auto tptr = ptr.template as<__object_type>(); \
            const bool __finished = tptr->template get<__object_type::_style_>() == static_cast<__type>(_dest_); \
            if (!__is_on) \
            { \
                if ((std::chrono::steady_clock::now() - __rev_timestamp) >= std::chrono::milliseconds(_time_)) \
                    __saved_initial = tptr->template get<__object_type::_style_>(); \
                __rev_timestamp = std::chrono::steady_clock::time_point(); \
                tptr->state()->screen()->template interpolate<::d2::interp::_interpolator_<__object_type, __object_type::_style_>>( \
                    std::chrono::milliseconds(_time_), \
                    tptr, \
                    _dest_ \
                    __VA_OPT__(,) __VA_ARGS__ \
                ); \
            } \
            else \
            { \
                __rev_timestamp = std::chrono::steady_clock::now(); \
                tptr->state()->screen()->template interpolate<::d2::interp::_interpolator_<__object_type, __object_type::_style_>>( \
                    std::chrono::milliseconds(_time_), \
                    tptr, \
                    __saved_initial \
                    __VA_OPT__(,) __VA_ARGS__ \
                ); \
            } \
            __is_on = !__is_on; \
        }); \
    }

#define _D2_IMPL_INTERPOLATE(_time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE_IMPL(_time_, _interpolator_, _ptr_, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_GEN(_time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_IMPL(_time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)
#define _D2_IMPL_INTERPOLATE_TOGGLE(_time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE_TOGGLE_IMPL(_time_, _interpolator_, _ptr_, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_TOGGLE_GEN(_time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_TOGGLE_IMPL(_time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)
#define _D2_IMPL_INTERPOLATE_ACQ(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE_EVENT_IMPL(_event_, true, _time_, _interpolator_, _ptr_, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_ACQ_GEN(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_EVENT_IMPL(_event_, true, _time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)
#define _D2_IMPL_INTERPOLATE_REL(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE_EVENT_IMPL(_event_, false, _time_, _interpolator_, _ptr_, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_REL_GEN(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_EVENT_IMPL(_event_, false, _time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)
#define _D2_IMPL_INTERPOLATE_TWOWAY(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE_EVENT_TWOWAY_IMPL(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_TWOWAY_GEN(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_EVENT_TWOWAY_IMPL(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)

// Automatic (i.e. gets all of the variables from the _D2_IMPL_CONTEXT parameters)
#define _D2_IMPL_INTERPOLATE_AUTO(_time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_IMPL(_time_, _interpolator_, __ptr, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_GEN_AUTO(_time_, _interpolator_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_IMPL(_time_, _interpolator_, __ptr, _style_, _dest_, __VA_ARGS__)
#define _D2_IMPL_INTERPOLATE_TOGGLE_AUTO(_time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_TOGGLE_IMPL(_time_, _interpolator_, __ptr, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_TOGGLE_GEN_AUTO(_time_, _interpolator_, _style_, _dest_, ...)_D2_IMPL_INTERPOLATE_TOGGLE_IMPL(_time_, _interpolator_, __ptr, _style_, _dest_, __VA_ARGS__)
#define _D2_IMPL_INTERPOLATE_ACQ_AUTO(_event_, _time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_EVENT_IMPL(_event_, true, _time_, _interpolator_, __ptr, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_ACQ_GEN_AUTO(_event_, _time_, _interpolator_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_EVENT_IMPL(_event_, true, _time_, _interpolator_, __ptr, _style_, _dest_, __VA_ARGS__)
#define _D2_IMPL_INTERPOLATE_REL_AUTO(_event_, _time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_EVENT_IMPL(_event_, false, _time_, _interpolator_, __ptr, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_REL_GEN_AUTO(_event_, _time_, _interpolator_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_EVENT_IMPL(_event_, false, _time_, _interpolator_, __ptr, _style_, _dest_, __VA_ARGS__)
#define _D2_IMPL_INTERPOLATE_TWOWAY_AUTO(_event_, _time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_EVENT_TWOWAY_IMPL(_event_, _time_, _interpolator_, __ptr, _style_, _dest_)
#define _D2_IMPL_INTERPOLATE_TWOWAY_GEN_AUTO(_event_, _time_, _interpolator_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_EVENT_TWOWAY_IMPL(_event_, _time_, _interpolator_, __ptr, _style_, _dest_, __VA_ARGS__)

#endif // D2_SDSL_HPP
