#ifndef D2_DSL_HPP
#define D2_DSL_HPP

#include "d2_sdsl.hpp"

#define D2_BREAKPOINT _D2_IMPL_BREAKPOINT

#define D2_TREE_STATE(...) _D2_IMPL_TREE_STATE(__VA_ARGS__)
#define D2_TREE_ARGS(...) _D2_IMPL_TREE_ARGS(__VA_ARGS__)

#define D2_EMBED_NAMED_STR(_type_, _name_, ...) _D2_IMPL_EMBED_NAMED_STR(_type_, _name_, __VA_ARGS__)
#define D2_EMBED_NAMED(_type_, _name_, ...) _D2_IMPL_EMBED_NAMED(_type_, _name_, __VA_ARGS__)
#define D2_EMBED(_type_, ...) _D2_IMPL_EMBED(_type_, __VA_ARGS__)

#define D2_EMBED_ELEM_NAMED_STR(_type_, _name_, ...) _D2_IMPL_EMBED_ELEM_NAMED_STR(_type_, _name_, __VA_ARGS__)
#define D2_EMBED_ELEM_NAMED(_type_, _name_, ...) _D2_IMPL_EMBED_ELEM_NAMED(_type_, _name_, __VA_ARGS__)
#define D2_EMBED_ELEM(_type_, ...) _D2_IMPL_EMBED_ELEM(_type_, __VA_ARGS__)

#define D2_EMBED_NAMED_STR_BEGIN(_type_, _name_, ...) _D2_IMPL_EMBED_NAMED_STR_BEGIN(_type_, _name_, __VA_ARGS__)
#define D2_EMBED_NAMED_BEGIN(_type_, _name_, ...) _D2_IMPL_EMBED_NAMED_BEGIN(_type_, _name_, __VA_ARGS__)
#define D2_EMBED_BEGIN(_type_, ...) _D2_IMPL_EMBED_BEGIN(_type_, __VA_ARGS__)

#define D2_EMBED_ELEM_NAMED_STR_BEGIN(_type_, _name_, ...) _D2_IMPL_EMBED_ELEM_NAMED_STR_BEGIN(_type_, _name_, __VA_ARGS__)
#define D2_EMBED_ELEM_NAMED_BEGIN(_type_, _name_, ...) _D2_IMPL_EMBED_ELEM_NAMED_BEGIN(_type_, _name_, __VA_ARGS__)
#define D2_EMBED_ELEM_BEGIN(_type_, ...) _D2_IMPL_EMBED_ELEM_BEGIN(_type_, __VA_ARGS__)

#define D2_EMBED_END _D2_IMPL_EMBED_END

#define D2_STATEFUL_TREE_ROOT(_alias_, _state_, _root_, ...) _D2_IMPL_STATEFUL_TREE_ROOT(_alias_, _state_, _root_, __VA_ARGS__)
#define D2_STATEFUL_TREE(_alias_, _state_, ...) _D2_IMPL_STATEFUL_TREE(_alias_, _state_, __VA_ARGS__)
#define D2_STATELESS_TREE_ROOT(_alias_, _root_, ...) _D2_IMPL_STATELESS_TREE_ROOT(_alias_, _root_, __VA_ARGS__)
#define D2_STATELESS_TREE(_alias_, ...) _D2_IMPL_STATELESS_TREE(_alias_, __VA_ARGS__)

#define D2_STATEFUL_TREE_ROOT_FORWARD(_alias_, _state_, _root_, ...) _D2_IMPL_STATEFUL_TREE_ROOT_FORWARD(_alias_, _state_, _root_, __VA_ARGS__)
#define D2_STATEFUL_TREE_FORWARD(_alias_, _state_, ...) _D2_IMPL_STATEFUL_TREE_FORWARD(_alias_, _state_, __VA_ARGS__)
#define D2_STATELESS_TREE_ROOT_FORWARD(_alias_, _root_, ...) _D2_IMPL_STATELESS_TREE_ROOT_FORWARD(_alias_, _root_, __VA_ARGS__)
#define D2_STATELESS_TREE_FORWARD(_alias_, ...) _D2_IMPL_STATELESS_TREE_FORWARD(_alias_, __VA_ARGS__)

#define D2_TREE_DEFINE(_alias_, ...) _D2_IMPL_TREE_DEFINE(_alias_, __VA_ARGS__)
#define D2_TREE_DEFINITION_END _D2_IMPL_TREE_DEFINITION_END
#define D2_TREE_END _D2_IMPL_TREE_END
#define D2_TREE_TAG(_name_, _value_) _D2_IMPL_TREE_TAG(_name_, _value_)
#define D2_TREE_CTAG(_name_, _value_) _D2_IMPL_TREE_CTAG(_name_, _value_)

#define D2_CREATE_OBJECT(_type_, _name_) _D2_IMPL_CREATE_OBJECT(_type_, _name_)
#define D2_STYLE(_prop_, ...) _D2_IMPL_STYLE(_prop_, __VA_ARGS__)
#define D2_REQUIRE(...) _D2_IMPL_REQUIRE(__VA_ARGS__)
#define D2_ELEM(_type_, ...) _D2_IMPL_ELEM(_type_, __VA_ARGS__)
#define D2_ELEM_STR(_type_, _name_) _D2_IMPL_ELEM_STR(_type_, _name_)
#define D2_ELEM_END _D2_IMPL_ELEM_END

#define D2_ANCHOR(_type_, _name_) _D2_IMPL_ANCHOR(_type_, _name_)
#define D2_ANCHOR_STATE(_type_, _name_) _D2_IMPL_ANCHOR_STATE(_type_, _name_)
#define D2_ELEM_ANCHOR(_name_) _D2_IMPL_ELEM_ANCHOR(_name_)

#define D2_STYLESHEET_BEGIN(_name_) _D2_IMPL_STYLESHEET_BEGIN(_name_)
#define D2_STYLESHEET_END _D2_IMPL_STYLESHEET_END
#define D2_STYLES_APPLY_MANUAL(_name_, _ptr_, _state_) _D2_IMPL_STYLES_APPLY_MANUAL(_name_, _ptr_, _state_)
#define D2_STYLES_APPLY(_name_) _D2_IMPL_STYLES_APPLY(_name_)

#define D2_THEME(_name_, ...) _D2_IMPL_THEME(_name_, __VA_ARGS__)
#define D2_THEME_END(...) _D2_IMPL_THEME_END(__VA_ARGS__)
#define D2_DEPENDENCY(_type_, _name_) _D2_IMPL_DEPENDENCY(_type_, _name_)
#define D2_DEPENDENCY_LINK(_dname_, _name_) _D2_IMPL_DEPENDENCY_LINK(_dname_, _name_)

#define D2_DEP(_name_, _type_) _D2_IMPL_DEP(_name_, _type_)
#define D2_TDEP(_tree_, _name_, _type_) _D2_IMPL_TDEP(_tree_, _name_, _type_)

#define D2_VAR(_type_, _var_) _D2_IMPL_VAR(_type_, _var_)
#define D2_VAR_TYPE(_type_, _var_) _D2_IMPL_VAR_TYPE(_type_, _var_)
#define D2_DYNAVAR(_type_, _var_, ...) _D2_IMPL_DYNAVAR(_type_, _var_, __VA_ARGS__)

#define D2_STATIC(_name_, ...) _D2_IMPL_STATIC(_name_, __VA_ARGS__)

#define D2_CONTEXT(_ptrv_, _typev_) _D2_IMPL_CONTEXT(_ptrv_, _typev_)
#define D2_CONTEXT_EXT(_ptrv_, _typev_, _statev_, _stypev_) _D2_IMPL_CONTEXT_EXT(_ptrv_, _typev_, _statev_, _stypev_)
#define D2_CONTEXT_RED(_statev_) _D2_IMPL_CONTEXT_RED(_statev_)
#define D2_CONTEXT_AUTO(_ptrv_) _D2_IMPL_CONTEXT_AUTO(_ptrv_)
#define D2_CONTEXT_END _D2_IMPL_CONTEXT_END

#define D2_VOID_EXPR _D2_IMPL_VOID_EXPR
#define D2_SYNC_EEXPR(...) _D2_IMPL_SYNC_EEXPR(__VA_ARGS__)
#define D2_SYNC_EXPR(...) _D2_IMPL_SYNC_EXPR(__VA_ARGS__)
#define D2_SYNC_BLOCK _D2_IMPL_SYNC_BLOCK
#define D2_SYNC_BLOCK_END _D2_IMPL_SYNC_BLOCK_END

#define D2_ON_EEXPR(_event_, ...) _D2_IMPL_LISTEN_EEXPR(_event_, true, __VA_ARGS__)
#define D2_ON_EXPR(_event_, ...) _D2_IMPL_LISTEN_EXPR(_event_, true, __VA_ARGS__)
#define D2_ON(_event_, ...) _D2_IMPL_LISTEN(_event_, true, __VA_ARGS__)
#define D2_ON_END _D2_IMPL_LISTEN_END

#define D2_OFF_EEXPR(_event_, ...) _D2_IMPL_LISTEN_EEXPR(_event_, false, __VA_ARGS__)
#define D2_OFF_EXPR(_event_, ...) _D2_IMPL_LISTEN_EXPR(_event_, false, __VA_ARGS__)
#define D2_OFF(_event_, ...) _D2_IMPL_LISTEN(_event_, false, __VA_ARGS__)
#define D2_OFF_END _D2_IMPL_LISTEN_END

#define D2_ON_EVENT_EEXPR(_event_, ...) _D2_IMPL_LISTEN_EVENT_EEXPR(_event_, __VA_ARGS__)
#define D2_ON_EVENT_EXPR(_event_, ...) _D2_IMPL_LISTEN_EVENT_EXPR(_event_, __VA_ARGS__)
#define D2_ON_EVENT(_event_, ...) _D2_IMPL_LISTEN_EVENT(_event_, __VA_ARGS__)
#define D2_ON_EVENT_END _D2_IMPL_LISTEN_EVENT_END

#define D2_ON_SIG(_event_, ...) _D2_IMPL_LISTEN_SIG(_event_, __VA_ARGS__)
#define D2_ON_SIG_END _D2_IMPL_LISTEN_SIG_END
#define D2_SIG_ARGS(...) _D2_IMPL_SIG_ARGS(__VA_ARGS__)

#define D2_ASYNC_EEXPR(...) _D2_IMPL_ASYNC_EEXPR(__VA_ARGS__)
#define D2_ASYNC_EXPR(...) _D2_IMPL_ASYNC_EXPR(__VA_ARGS__)
#define D2_ASYNC_TASK _D2_IMPL_ASYNC_TASK
#define D2_ASYNC_END _D2_IMPL_ASYNC_END

#define D2_CYCLIC_EEXPR(_time_, ...) _D2_IMPL_CYCLIC_EEXPR(_time_, __VA_ARGS__)
#define D2_CYCLIC_EXPR(_time_, ...) _D2_IMPL_CYCLIC_EXPR(_time_, __VA_ARGS__)
#define D2_CYCLIC_TASK(_time_) _D2_IMPL_CYCLIC_TASK(_time_)
#define D2_CYCLIC_END _D2_IMPL_CYCLIC_END

#define D2_DEFER_EEXPR(_time_, ...) _D2_IMPL_DEFER_EEXPR(_time_, __VA_ARGS__)
#define D2_DEFER_EXPR(_time_, ...) _D2_IMPL_DEFER_EXPR(_time_, __VA_ARGS__)
#define D2_DEFER_TASK(_time_) _D2_IMPL_DEFER_TASK(_time_)
#define D2_DEFERRED_TASK_END _D2_IMPL_DEFERRED_TASK_END

#define D2_INTERP(_time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE(_time_, _interpolator_, _ptr_, _style_, _dest_)
#define D2_INTERP_GEN(_time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_GEN(_time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)
#define D2_INTERP_TOGGLE(_time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE_TOGGLE(_time_, _interpolator_, _ptr_, _style_, _dest_)
#define D2_INTERP_TOGGLE_GEN(_time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_TOGGLE_GEN(_time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)
#define D2_INTERP_ACQ(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE_ACQ(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_)
#define D2_INTERP_ACQ_GEN(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_ACQ_GEN(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)
#define D2_INTERP_REL(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE_REL(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_)
#define D2_INTERP_REL_GEN(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_REL_GEN(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)
#define D2_INTERP_TWOWAY(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_) _D2_IMPL_INTERPOLATE_TWOWAY(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_)
#define D2_INTERP_TWOWAY_GEN(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_TWOWAY_GEN(_event_, _time_, _interpolator_, _ptr_, _style_, _dest_, __VA_ARGS__)

#define D2_INTERP_AUTO(_time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_AUTO(_time_, _interpolator_, _style_, _dest_)
#define D2_INTERP_GEN_AUTO(_time_, _interpolator_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_GEN_AUTO(_time_, _interpolator_, _style_, _dest_, __VA_ARGS__)
#define D2_INTERP_TOGGLE_AUTO(_time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_TOGGLE_AUTO(_time_, _interpolator_, _style_, _dest_)
#define D2_INTERP_TOGGLE_GEN_AUTO(_time_, _interpolator_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_TOGGLE_GEN_AUTO(_time_, _interpolator_, _style_, _dest_, __VA_ARGS__)
#define D2_INTERP_ACQ_AUTO(_event_, _time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_ACQ_AUTO(_event_, _time_, _interpolator_, _style_, _dest_)
#define D2_INTERP_ACQ_GEN_AUTO(_event_, _time_, _interpolator_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_ACQ_GEN_AUTO(_event_, _time_, _interpolator_, _style_, _dest_, __VA_ARGS__)
#define D2_INTERP_REL_AUTO(_event_, _time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_REL_AUTO(_event_, _time_, _interpolator_, _style_, _dest_)
#define D2_INTERP_REL_GEN_AUTO(_event_, _time_, _interpolator_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_REL_GEN_AUTO(_event_, _time_, _interpolator_, _style_, _dest_, __VA_ARGS__)
#define D2_INTERP_TWOWAY_AUTO(_event_, _time_, _interpolator_, _style_, _dest_) _D2_IMPL_INTERPOLATE_TWOWAY_AUTO(_event_, _time_, _interpolator_, _style_, _dest_)
#define D2_INTERP_TWOWAY_GEN_AUTO(_event_, _time_, _interpolator_, _style_, _dest_, ...) _D2_IMPL_INTERPOLATE_TWOWAY_GEN_AUTO(_event_, _time_, _interpolator_, _style_, _dest_, __VA_ARGS__)

#endif // D2_DSL_HPP
