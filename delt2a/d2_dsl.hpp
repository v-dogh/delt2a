#ifndef D2_DSL_HPP
#define D2_DSL_HPP

#include "d2_tree.hpp"

// DSL macros (oh yeah macaroles!!! so many macaroles!!!!)

#define D2_THEME(name, ...) struct name : ::d2::style::Theme __VA_OPT__(,) __VA_ARGS__ {
#define D2_THEME_END(...) }
#define D2_DEPENDENCY(type, name) Dependency<type> name;
#define D2_DEPENDENCY_LINK(dname, name) virtual decltype(name)& dname() override { return name; }

#define D2_EMBED(type, ...) ::d2::tree::SubTree<type, \
	[]<typename Type>(::d2::Screen::ptr src) \
	{ return Type::build(src __VA_OPT__(,) __VA_ARGS__); }>,
#define D2_EMBED_ELEM(type, name, ...) ::d2::tree::ArgElem<[](auto loc, auto src) { \
		return loc.asp()->override(std::make_shared<type>( \
			std::string(name), src, loc.as() __VA_OPT__(,) __VA_ARGS__ \
		)); \
	}>,
#define D2_EMBED_ELEM_UNNAMED(type, ...) D2_EMBED_ELEM(type, "", __VA_ARGS__)
#define D2_STATELESS_TREE(alias) using alias = ::d2::TreeTemplate<
#define D2_STATEFUL_TREE(alias, state) using alias = ::d2::TreeTemplateInit<state,
#define D2_TREE_END(...) ::d2::tree::PaddingElem>;

#define D2_STATELESS_TREE_FORWARD(alias) \
struct alias : TreeTemplateInit<::d2::TreeState, void>  { \
static ::d2::Element::TraversalWrapper create_at(::d2::Element::TraversalWrapper loc, TreeState::ptr src); };
#define D2_STATEFUL_TREE_FORWARD(alias, state) \
struct alias : TreeTemplateInit<state>  { \
static ::d2::Element::TraversalWrapper create_at(::d2::Element::TraversalWrapper loc, TreeState::ptr src); };
#define D2_TREE_DEFINE(alias) \
::d2::Element::TraversalWrapper alias::create_at(::d2::Element::TraversalWrapper loc, TreeState::ptr src) { \
D2_STATELESS_TREE(_tree_)
#define D2_TREE_DEFINITION_END(...) D2_TREE_END() return _tree_::create_at(loc, src); }

#define D2_INVOCATION(type) [](::d2::Element::TraversalWrapper ptr, ::d2::TreeState::ptr state) \
{ using __object_type = type; \
::d2::Element::TraversalWrapper& __ptr = ptr; \
::d2::TreeState::ptr& __state = state;
#define D2_STYLE(prop, ...) __ptr.template as<__object_type>()->template set<__object_type::prop>(__VA_ARGS__);
#define D2_ELEM_NESTED(type, name) ::d2::tree::Elem<type, #name, D2_INVOCATION(type)
#define D2_ELEM_NESTED_UNNAMED(type) ::d2::tree::Elem<type, "", D2_INVOCATION(type)
#define D2_ELEM(type, name) D2_ELEM_NESTED(type, name)
#define D2_ELEM_UNNAMED(type) D2_ELEM_NESTED_UNNAMED(type)
#define D2_ELEM_NESTED_BODY(...) },
#define D2_UELEM_NESTED_BODY D2_ELEM_NESTED_BODY()
#define D2_ELEM_END(...) }>,
#define D2_ELEM_NESTED_END(...) ::d2::tree::PaddingElem>,
#define D2_UELEM_END D2_ELEM_END()
#define D2_UELEM_NESTED_END D2_ELEM_NESTED_END()

#define D2_STYLESHEET_BEGIN(name) \
	struct name { \
	template<typename __object_type> \
	static void apply(::d2::Element::TraversalWrapper ptr, ::d2::TreeState::ptr state) { \
		::d2::Element::TraversalWrapper& __ptr = ptr; \
		::d2::TreeState::ptr& __state = state;
#define D2_STYLESHEET_END(...) }};
#define D2_STYLES_APPLY_MANUAL(name, object, ptr, state) name::template apply<object>(ptr, state);
#define D2_STYLES_APPLY(name) D2_STYLES_APPLY_MANUAL(name, __object_type, __ptr, __state)

#define D2_VAR(type, var) __state->screen()->theme<type>().var
#define D2_VAR_TYPE(type, var) typename std::remove_cvref_t<decltype(D2_VAR(type, var))>::value_type
#define D2_DYNAVAR(type, var, ...) ::d2::style::dynavar< \
[](const D2_VAR_TYPE(type, var)& value) { return __VA_ARGS__; }>(D2_VAR(type, var))

// Helpers
// EEXPR variants do not include line breaks
// EXPR and EEXPR expect expressions as arguments
// For future you: initialize a D2_CONTEXT for those task wrappers (listen, listen_global etc.)

#define D2_CONTEXT(ptrv, typev) \
	{ ::d2::Element::TraversalWrapper __ptr = ptrv; \
	::d2::TreeState::ptr __state = ptrv->state(); \
using __object_type = typev;
#define D2_CONTEXT_RED(statev) \
	{ ::d2::TreeState::ptr __state = statev;
#define D2_CONTEXT_END }
#define D2_VOID_EXPR , void()

#define D2_SYNCED_EEXPR(...) (state->screen()->is_synced() ? \
	(__VA_ARGS__) : state->screen()->sync([=]() -> decltype(auto) { return __VA_ARGS__; }).value())
#define D2_SYNCED_EXPR(...) D2_SYNCED_EEXPR(__VA_ARGS__);
#define D2_SYNCED_BLOCK state->screen()->sync_if([=]{
#define D2_SYNCED_BLOCK_END });

#define D2_LISTEN_EEXPR(event, value, ...) [=]() -> void { D2_LISTEN_EXPR(event, value, __VA_ARGS__) }()
#define D2_LISTEN_EXPR(event, value, ...) D2_LISTEN(event, value) (__VA_ARGS__); D2_LISTEN_END
#define D2_LISTEN(event, value) \
	__ptr->listen(::d2::Element::event, value, \
	[=](::d2::Element::EventListener listener, ::d2::Element::TraversalWrapper ptr) { D2_CONTEXT(ptr, __object_type)
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

#define D2_INTERPOLATE_IMPL(time, interpolator, ptr, object, style, dest, ...) \
{ \
	using __type = decltype(std::declval<object>().template get<object::style>());\
	ptr->state()->screen()->template interpolate<::d2::interp::interpolator<object, object::style>>( \
		std::chrono::milliseconds(time), \
		ptr.as(), \
		dest \
		__VA_OPT__(,) __VA_ARGS__ \
	); \
}
#define D2_INTERPOLATE_EVENT_IMPL(event, sstate, time, interpolator, ptr, object, style, dest, ...) \
{ \
	ptr->listen(::d2::Element::State::event, sstate, [=](auto, auto ptr) { \
		using __type = decltype(std::declval<object>().template get<object::style>());\
		ptr->state()->screen()->template interpolate<::d2::interp::interpolator<object, object::style>>( \
			std::chrono::milliseconds(time), \
			ptr.as(), \
			dest \
			__VA_OPT__(,) __VA_ARGS__ \
		); \
	}); \
}
#define D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, ptr, object, style, dest, ...) \
{ \
	using __type = decltype(std::declval<object>().template get<object::style>()); \
	static bool __is_on{ false }; \
	static __type __saved_initial{}; \
	static std::chrono::steady_clock::time_point __rev_timestamp{}; \
	ptr->listen(::d2::Element::State::event, true, [=](auto, auto ptr) { \
		const bool __finished = ptr.template as<object>()->template get<object::style>() == static_cast<__type>(dest); \
		if (!__is_on && !__finished) \
		{ \
			if ((std::chrono::steady_clock::now() - __rev_timestamp) >= std::chrono::milliseconds(time)) \
				__saved_initial = ptr.template as<object>()->template get<object::style>(); \
			__rev_timestamp = std::chrono::steady_clock::time_point(); \
			ptr->state()->screen()->template interpolate<::d2::interp::interpolator<object, object::style>>( \
				std::chrono::milliseconds(time), \
				ptr.as(), \
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
		__rev_timestamp = std::chrono::steady_clock::now(); \
		ptr->state()->screen()->template interpolate<::d2::interp::interpolator<object, object::style>>( \
			std::chrono::milliseconds(time), \
			ptr.as(), \
			__saved_initial \
			__VA_OPT__(,) __VA_ARGS__ \
		); \
	}); \
}
#define D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, ptr, object, style, dest, ...)

// Manual

#define D2_INTERPOLATE(time, interpolator, ptr, object, style, dest) \
D2_INTERPOLATE_IMPL(time, interpolator, ptr, object, style, dest)
#define D2_INTERPOLATE_GEN(time, interpolator, ptr, object, style, dest, ...) \
D2_INTERPOLATE_IMPL(time, interpolator, ptr, object, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_TOGGLE(time, interpolator, ptr, object, style, dest) \
D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, ptr, object, style, dest)
#define D2_INTERPOLATE_TOGGLE_GEN(time, interpolator, ptr, object, style, dest, ...) \
D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, ptr, object, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_ACQ(event, time, interpolator, ptr, object, style, dest) \
D2_INTERPOLATE_EVENT_IMPL(event, true, time, interpolator, ptr, object, style, dest)
#define D2_INTERPOLATE_ACQ_GEN(event, time, interpolator, ptr, object, style, dest, ...) \
D2_INTERPOLATE_EVENT_IMPL(event, true, time, interpolator, ptr, object, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_REL(event, time, interpolator, ptr, object, style, dest) \
D2_INTERPOLATE_EVENT_IMPL(event, false, time, interpolator, ptr, object, style, dest)
#define D2_INTERPOLATE_REL_GEN(event, time, interpolator, ptr, object, style, dest, ...) \
D2_INTERPOLATE_EVENT_IMPL(event, false, time, interpolator, ptr, object, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_TWOWAY(event, time, interpolator, ptr, object, style, dest) \
D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, ptr, object, style, dest)
#define D2_INTERPOLATE_TWOWAY_GEN(event, time, interpolator, ptr, object, style, dest, ...) \
D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, ptr, object, style, dest,  __VA_ARGS__)

// Automatic (i.e. gets all of the variables from the D2_CONTEXT parameters)

#define D2_INTERPOLATE_AUTO(time, interpolator, style, dest) \
D2_INTERPOLATE_IMPL(time, interpolator, __ptr, __object_type, style, dest)
#define D2_INTERPOLATE_GEN_AUTO(time, interpolator, style, dest, ...) \
D2_INTERPOLATE_IMPL(time, interpolator, __ptr, __object_type, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_TOGGLE_AUTO(time, interpolator, style, dest) \
D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, __ptr, __object_type, style, dest)
#define D2_INTERPOLATE_TOGGLE_GEN_AUTO(time, interpolator, style, dest, ...) \
D2_INTERPOLATE_TOGGLE_IMPL(time, interpolator, __ptr, __object_type, style, dest, __VA_ARGS__)


#define D2_INTERPOLATE_ACQ_AUTO(event, time, interpolator, style, dest) \
D2_INTERPOLATE_EVENT_IMPL(event, true, time, interpolator, __ptr, __object_type, style, dest)
#define D2_INTERPOLATE_ACQ_GEN_AUTO(event, time, interpolator, style, dest, ...) \
D2_INTERPOLATE_EVENT_IMPL(event, true, time, interpolator, __ptr, __object_type, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_REL_AUTO(event, time, interpolator, style, dest) \
D2_INTERPOLATE_EVENT_IMPL(event, false, time, interpolator, __ptr, __object_type, style, dest)
#define D2_INTERPOLATE_REL_GEN_AUTO(event, time, interpolator, style, dest, ...) \
D2_INTERPOLATE_EVENT_IMPL(event, false, time, interpolator, __ptr, __object_type, style, dest, __VA_ARGS__)

#define D2_INTERPOLATE_TWOWAY_AUTO(event, time, interpolator, style, dest) \
D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, __ptr, __object_type, style, dest)
#define D2_INTERPOLATE_TWOWAY_GEN_AUTO(event, time, interpolator, style, dest, ...) \
D2_INTERPOLATE_EVENT_TWOWAY_IMPL(event, time, interpolator, __ptr, __object_type, style, dest,  __VA_ARGS__)

#endif // D2_DSL_HPP
