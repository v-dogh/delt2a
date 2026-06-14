#pragma once

#include "d2_exceptions.hpp"
#include "d2_interpolator.hpp"
#include "d2_tree_element.hpp"
#include "d2_tree_element_frwd.hpp"
#include <chrono>
#include <cstdint>
#include <d2_io_handler.hpp>
#include <d2_meta.hpp>
#include <d2_screen.hpp>
#include <d2_theme.hpp>
#include <d2_tree.hpp>
#include <d2_tree_state.hpp>
#include <mods/d2_core.hpp>
#include <type_traits>
#include <utility>

namespace d2
{
// These are just for convenience and to look nice
#define dstyle(_style_, ...)                                                                       \
    ctx.ptr()->template set<decltype(ctx)::element_type::_style_>(__VA_ARGS__)
#define dstylev(_style_, _var_) dstyle(_style_, ctx.template var<_var_>())
#define dstyledv(_style_, _var_, ...)                                                              \
    dstyle(                                                                                        \
        _style_,                                                                                   \
        ctx.template dynavar<                                                                      \
            _var_,                                                                                 \
            [](const std::remove_cvref_t<decltype(ctx.template var<_var_>())>::value_type& value)  \
            { return __VA_ARGS__; }                                                                \
        >()                                                                                        \
    )
#define dapply(_stylesheet_, ...) ctx.template apply<_stylesheet_>(__VA_ARGS__)
#define dmstyle(_elem_, _style_, ...)                                                              \
    _elem_->template set<decltype(_elem_)::type::_style_>(__VA_ARGS__)

    namespace impl
    {
        template<typename> struct lambda_arg;
        template<typename Type, typename Ret, typename Arg>
        struct lambda_arg<Ret (Type::*)(Arg) const>
        {
            using type = Arg;
        };
        template<typename Type, typename Ret, typename Arg> struct lambda_arg<Ret (Type::*)(Arg)>
        {
            using type = Arg;
        };

        template<typename Func>
        using lambda_arg_t =
            typename lambda_arg<decltype(&std::remove_reference_t<Func>::operator())>::type;

        template<typename Func> using deduced_type_t = typename lambda_arg_t<Func>::element_type;
        template<typename Func> using deduced_state_t = typename lambda_arg_t<Func>::state_type;
    } // namespace impl

    // For TreeCtx::branch
    template<typename First, typename Second> using Case = std::pair<First, Second>;

    class TreeCtxBase
    {
        D2_TAG_MODULE(tree)
    private:
        struct BranchState
        {
            std::vector<TreeIter<>> ptrs;
            std::uintptr_t origin;
        };
        static inline thread_local std::optional<BranchState> _bstate{std::nullopt};
    protected:
        struct BranchGuard
        {
            std::optional<BranchState> saved;
            BranchGuard()
            {
                saved = _bstate;
                _bstate = BranchState{.origin = std::uintptr_t(this)};
            }
            ~BranchGuard()
            {
                _bstate = saved;
            }
        };
    protected:
        TreeIter<> _ptr{};

        void _reg_elem(TreeIter<> ptr) const
        {
            if (_bstate.has_value() && _bstate->origin == std::uintptr_t(this))
                _bstate->ptrs.push_back(ptr);
        }
        static std::vector<TreeIter<>> _get_branch()
        {
            return _bstate->ptrs;
        }
    public:
        TreeCtxBase(d2::TreeIter<> ptr) : _ptr(ptr) {}
        TreeCtxBase(const TreeCtxBase&) = default;
        TreeCtxBase(TreeCtxBase&&) = default;

        IOContext::ptr context() const noexcept
        {
            return _ptr->context();
        }
        mt::ConcurrentPool::ptr scheduler() const noexcept
        {
            return context()->scheduler();
        }

        sys::module<sys::screen> screen() const noexcept
        {
            return context()->screen();
        }
        sys::module<sys::input> input() const noexcept
        {
            return context()->input();
        }
        sys::module<sys::output> output() const noexcept
        {
            return context()->output();
        }

        // Tags

        template<meta::ConstString Name, typename Type> void tag(Type&& value) const
        {
            screen()->tags().set(std::string(Name.view()), std::forward<Type&&>(value));
            using ss = std::string_view;
            constexpr auto name = Name.view();
            static_assert(
                (name == ss("SwapClean") && std::is_same_v<Type, bool>) ||
                    (name == ss("SwapOut") && std::is_same_v<Type, bool>) ||
                    (name == ss("OverrideRefresh") &&
                     std::is_same_v<Type, std::chrono::milliseconds>),
                "Invalid type or tag name (use ctag for non-standard tags to "
                "bypass the checks)"
            );
        }
        template<typename Type> void tag(const std::string& name, Type&& value) const
        {
            screen()->tags().set(name, std::forward<Type&&>(value));
        }

        // Deps

        template<typename Type> auto& dep(const std::string& name) const
        {
            return screen()->deps().template get<Type>(name);
        }
        template<typename Type> auto& dep(const std::string& tree, const std::string& name) const
        {
            return screen()->deps(tree).template get<Type>(name);
        }

        template<auto Var> auto& var() const
        {
            return screen()->template themev<Var>();
        }
        template<auto Var, auto Callback> auto dynavar() const
        {
            return style::dynavar<Callback>(var<Var>());
        }

        template<typename Type, typename Func>
        style::DependencyHandle subscribe(style::Dependency<Type>& dep, Func&& callback) const
        {
            return dep.subscribe(
                _ptr.weak(),
                [callback](d2::TreeIter<> ptr, const Type& value)
                {
                    if constexpr (
                        std::is_same_v<std::invoke_result_t<Func, decltype(ptr), Type>, void>
                    )
                    {
                        callback(ptr, value);
                        return true;
                    }
                    else
                        return callback(ptr, value);
                }
            );
        }
        template<typename Type, typename Func>
        style::DependencyHandle subscribe(const std::string& name, Func&& callback) const
        {
            return subscribe(dep<Type>(name), std::forward<Func>(callback));
        }
        template<typename Type, typename Func>
        style::DependencyHandle
        subscribe(const std::string& tree, const std::string& name, Func&& callback) const
        {
            return subscribe(dep<Type>(tree, name), std::forward<Func>(callback));
        }
        template<auto Var, typename Func>
        style::DependencyHandle
        subscribe(const std::string& tree, const std::string& name, Func&& callback) const
        {
            return subscribe(var<Var>(), std::forward<Func>(callback));
        }

        template<typename Type, typename... Argv> Type& declare(Argv&&... args) const
        {
            struct
            {
                std::shared_ptr<Type> value;
                void operator()(Element::EventListener, TreeIter<>) {}
            } cb{.value{std::make_shared<Type>(std::forward<Argv>(args)...)}};
            _ptr->listen(Element::State::Created, false, cb);
            return *cb.value;
        }

        // Listeners

        template<typename Func, typename... Argv>
        auto repeat(std::chrono::milliseconds timing, Func&& callback, Argv&&... args) const
        {
            return scheduler()->launch_cyclic(
                timing, std::forward<Func>(callback), std::forward<Argv>(args)...
            );
        }
        template<typename Func, typename... Argv>
        auto defer(std::chrono::milliseconds delay, Func&& callback, Argv&&... args) const
        {
            return scheduler()->launch_deferred(
                delay, std::forward<Func>(callback), std::forward<Argv>(args)...
            );
        }
        template<typename Func, typename... Argv> auto async(Func&& callback, Argv&&... args) const
        {
            return scheduler()->launch(std::forward<Func>(callback), std::forward<Argv>(args)...);
        }
        template<typename Func, typename... Argv> auto sync(Func&& callback, Argv&&... args) const
        {
            const auto ctx = context();
            ctx->sync(std::forward<Func>(callback), std::forward<Argv>(args)...);
        }
        template<typename Func, typename... Argv>
        auto sync_async(Func&& callback, Argv&&... args) const
        {
            const auto ctx = context();
            ctx->sync_async(std::forward<Func>(callback), std::forward<Argv>(args)...);
        }

        template<typename Event, typename Func> Signals::Handle on(Event ev, Func&& callback) const
        {
            return context()->listen(ev, std::forward<Func>(callback));
        }
        Element::EventListener
        onv(Element::State state, bool value, Element::event_callback callback) const
        {
            return _ptr->listen(state, value, std::move(callback));
        }
        Element::EventListener onv(Element::State state, Element::event_callback callback) const
        {
            return _ptr->listen(state, std::move(callback));
        }

        TreeCtxBase& operator=(const TreeCtxBase&) = default;
        TreeCtxBase& operator=(TreeCtxBase&&) = default;

        bool operator==(std::nullptr_t) const noexcept
        {
            return _ptr == nullptr;
        }
        bool operator!=(std::nullptr_t) const noexcept
        {
            return _ptr != nullptr;
        }
    };
    template<typename Parent, typename State = d2::TreeState> class TreeCtx : public TreeCtxBase
    {
    public:
        using element_type = Parent;
        using state_type = State;
    public:
        using TreeCtxBase::TreeCtxBase;

        TreeCtx(d2::TreeIter<Parent> ptr) : TreeCtxBase(ptr) {}

        TreeIter<Parent> ptr() const noexcept
        {
            return _ptr.as<Parent>();
        }
        tree_state_ptr<State> state() const noexcept
        {
            return _ptr->state()->as<State>();
        }

        template<typename Type> void clone(TreeIter<Type> ptr, auto&& callback) const
        {
            callback(TreeCtx<Type, State>(ptr));
        }

        template<typename Type, typename... Argv>
        auto anchor(std::string name = "", Argv&&... args) const
        {
            return Element::make<Type>(std::move(name), state(), std::forward<Argv>(args)...);
        }

        // Animations

        template<
            template<typename, Element::property> typename Interpolator,
            Element::property Prop,
            typename... Argv
        >
        Animation::ptr animate(std::chrono::milliseconds time, Argv&&... args)
        {
            return screen()->template animate<Interpolator<Parent, Prop>>(
                time, _ptr.as<Parent>(), std::forward<Argv>(args)...
            );
        }

        template<
            template<typename, Element::property> typename Interpolator,
            Element::property Prop,
            typename... Argv
        >
        Animation::ptr animate_twoway(std::chrono::milliseconds time, Argv&&... args)
        {
            return animate_twoway<Interpolator, Interpolator>(
                time, time, std::forward<Argv>(args)...
            );
        }
        template<
            template<typename, Element::property> typename InterpolatorTo,
            template<typename, Element::property> typename InterpolatorFrom,
            Element::property Prop,
            typename... Argv
        >
        Animation::ptr animate_twoway(
            std::chrono::milliseconds time_to, std::chrono::milliseconds time_from, Argv&&... args
        )
        {
            onv(Element::State::Clicked,
                [=,
                 ... args =
                     std::forward<Argv>(args)](Element::EventListener listener, TreeIter<> ptr)
                {
                    auto ctx = TreeCtx<Parent, State>(ptr);
                    if (ptr->getstate(Element::State::Clicked))
                    {
                        ctx.template animate<InterpolatorTo>(time_to, args...);
                    }
                    else
                    {
                        ctx.template animate<InterpolatorFrom>(time_from, args...);
                    }
                });
        }

        template<
            template<typename, Element::property> typename Interpolator,
            Element::property Prop,
            typename... Argv
        >
        Animation::ptr animate_toggle(std::chrono::milliseconds time, Argv&&... args)
        {
            return animate_toggle<Interpolator, anim::Linear>(
                time, std::chrono::milliseconds{0}, std::forward<Argv>(args)...
            );
        }
        template<
            template<typename, Element::property> typename InterpolatorTo,
            template<typename, Element::property> typename InterpolatorFrom,
            Element::property Prop,
            typename... Argv
        >
        Animation::ptr animate_toggle(
            std::chrono::milliseconds time_to, std::chrono::milliseconds time_from, Argv&&... args
        )
        {
            onv(Element::State::Clicked,
                true,
                [=,
                 saved = Parent::template type_of<Prop>(),
                 state = false,
                 ... args = std::forward<Argv>(args)](
                    Element::EventListener listener, TreeIter<> ptr
                ) mutable
                {
                    auto ctx = TreeCtx<Parent, State>(ptr);
                    if (state)
                    {
                        saved = ctx->template get<Prop>();
                        ctx.template animate<InterpolatorTo>(time_to, args...);
                    }
                    else
                    {
                        ctx.template animate<InterpolatorFrom>(time_from, saved);
                    }
                    state = !state;
                });
        }

        // Children

        template<typename Func, typename... Arge, typename... Args>
            requires std::invocable<Func&&, TreeCtx<impl::deduced_type_t<Func>, State>>
        auto embed(
            const std::string& name,
            Func&& callback,
            std::tuple<Arge...> arge = {},
            std::tuple<Args...> args = {}
        ) const
        {
            using type = impl::deduced_type_t<Func>;
            auto nsrc = std::apply(
                [&](auto... args)
                { return type::build_sub(name, state(), _ptr.asp(), std::move(args)...); },
                args
            );
            auto cptr = nsrc->core();
            std::apply(
                [&](auto... args)
                { type::create_at(nsrc->core(), nsrc, std::forward<Args>(args)...); },
                arge
            );
            nsrc->swap_in();
            _reg_elem(cptr);
            return cptr;
        }

        template<typename Func, typename... Arge, typename... Args>
            requires std::invocable<Func&&, TreeCtx<impl::deduced_type_t<Func>, State>>
        auto
        embed(Func&& callback, std::tuple<Arge...> arge = {}, std::tuple<Args...> args = {}) const
        {
            return embed("", std::forward<Func>(callback), std::move(arge), std::move(args));
        }
        template<typename Type, typename... Args, typename... Arge>
        auto embed(
            const std::string& name, std::tuple<Arge...> arge = {}, std::tuple<Args...> args = {}
        ) const
        {
            return embed<Type>(
                name, [](d2::TreeCtx<Type, State>) {}, std::move(arge), std::move(args)
            );
        }
        template<typename Type, typename... Arge, typename... Args>
        auto embed(std::tuple<Arge...> arge = {}, std::tuple<Args...> args = {}) const
        {
            return embed("", [](d2::TreeCtx<Type, State>) {}, std::move(arge), std::move(args));
        }

        // Deduced

        template<typename Func, typename... Argv>
            requires std::invocable<Func&, d2::TreeIter<>>
        auto elem(Func&& callback, Argv&&... args) const
        {
            using type = impl::deduced_type_t<Func>;
            using state_type = impl::deduced_state_t<Func>;
            auto ptr = Element::make<type>("", state(), std::forward<Argv>(args)...);
            internal::ElementView::from(ptr).setparent(_ptr.asp());
            callback(TreeCtx<type, state_type>(ptr));
            _ptr.asp()->override(ptr);
            _reg_elem(ptr);
            return TreeIter<type>(ptr);
        }
        template<typename Func, typename... Argv>
            requires std::invocable<Func&, d2::TreeIter<>>
        auto elem(std::string name, Func&& callback, Argv&&... args) const
        {
            using type = impl::deduced_type_t<Func>;
            using state_type = impl::deduced_state_t<Func>;
            auto ptr = Element::make<type>(std::move(name), state(), std::forward<Argv>(args)...);
            internal::ElementView::from(ptr).setparent(_ptr.asp());
            callback(TreeCtx<type, state_type>(ptr));
            _ptr.asp()->override(ptr);
            _reg_elem(ptr);
            return TreeIter<type>(ptr);
        }
        template<typename Type, typename Func>
            requires std::invocable<Func, TreeCtx<Type, State>>
        auto elem(Element::eptr<Type> ptr, Func&& callback) const
        {
            internal::ElementView::from(ptr).setparent(_ptr.asp());
            callback(TreeCtx<Type, State>(ptr));
            _ptr.asp()->override(ptr);
            _reg_elem(ptr);
            return TreeIter<Type>(ptr);
        }

        template<typename Var, typename... Branch>
            requires(std::invocable<Branch, TreeCtx<Parent, State>> && ...)
        void branch(Var& var, std::pair<typename Var::value_type, Branch>... branches)
        {
            auto handle = subscribe(
                var,
                [... branches = std::move(branches),
                 current = d2::TreeIter<>{},
                 ptrs = std::vector<TreeIter<>>{}](
                    d2::TreeIter<> ptr, const Var::value_type& value
                ) mutable
                {
                    for (decltype(auto) it : ptrs)
                        ptr.asp()->remove(it);
                    ptrs.clear();
                    (
                        [&]()
                        {
                            if (branches.first == value)
                            {
                                BranchGuard guard;
                                branches.second(TreeCtx<Parent, State>(ptr));
                                ptrs = TreeCtxBase::_get_branch();
                                return true;
                            }
                            return false;
                        }() ||
                        ...);
                }
            );
            declare<style::DependencyHandle>() = std::move(handle);
        }
        template<typename Type, typename... Branch>
            requires(std::invocable<Branch, TreeCtx<Parent, State>> && ...)
        void branch(
            const std::string& tree, const std::string& name, std::pair<Type, Branch>... branches
        )
        {
            branch(dep<Type>(tree, name), std::move(branches)...);
        }
        template<typename Type, typename... Value, typename... Branch>
            requires(std::invocable<Branch, TreeCtx<Parent, State>> && ...)
        void branch(const std::string& name, std::pair<Type, Branch>... branches)
        {
            branch(dep<Type>(name), std::move(branches)...);
        }
        template<auto Var, typename... Value, typename... Branch>
            requires(std::invocable<Branch, TreeCtx<Parent, State>> && ...)
        void
        branch(std::pair<typename style::ThemeRegistry<decltype(Var)>::type, Branch>... branches)
        {
            branch(var<Var>(), std::move(branches)...);
        }

        void persist()
        {
            if (_ptr->parent() != screen()->root())
                D2_THRW("Attempt to persist children below the root layer");
            auto handle = context()->listen(
                sys::screen::Event::TreeSwap,
                [ptr = _ptr](IOContext::ptr ctx)
                {
                    if (ptr != nullptr)
                    {
                        auto root = ctx->screen()->root();
                        if (root->exists(ptr->name()))
                            D2_TLOG(
                                Warning,
                                "Element already exists at: '",
                                ptr->name(),
                                "'; While attempting to persist (overriding)"
                            );
                        root->override(ptr->extract());
                    }
                }
            );
            declare<Signals::Handle>() = std::move(handle);
        }

        // Styles

        template<typename Style, typename... Argv> void apply(Argv&&... args) const
        {
            Style::apply(TreeCtx<Parent, State>(_ptr), std::forward<Argv>(args)...);
        }

        TreeIter<Parent> operator->() const noexcept
        {
            return _ptr;
        }

        template<typename OtherRoot, typename OtherState>
        operator TreeCtx<OtherRoot, OtherState>() const noexcept
        {
            return TreeCtx<OtherRoot, OtherState>(_ptr);
        }
    };

    // Callback takes TreeCtx<Type> where Type is the stylesheet target's type
    template<auto Callback, typename... Argv> struct Stylesheet
    {
        template<typename Parent, typename State>
        static void apply(TreeCtx<Parent, State> ctx, Argv... args)
        {
            Callback(TreeCtx<Parent>(ctx), args...);
        }
        template<typename Type> static void apply(TreeIter<Type> ptr, Argv... args)
        {
            Callback(TreeCtx<Type>(ptr), args...);
        }
    };

    template<meta::ConstString Name, typename State, typename Root, auto Build, typename... Argv>
    struct Tree : d2::TreeTemplateInit<Name, State, void, Root>
    {
        static_assert(
            std::is_invocable_v<decltype(Build), TreeCtx<Root, State>, Argv...>,
            "Build argument must have a signature: TreeCtx<Root, State>, Argv..."
        );
        static void create_at(TreeIter<> root, TreeState::ptr state, Argv... args)
        {
            Build(TreeCtx<Root, State>(root.as<Root>()), std::move(args)...);
        }
    };
    template<meta::ConstString Name, typename Base, typename State, typename Root, typename... Argv>
    struct TreeForward : d2::TreeTemplateInit<Name, State, void, Root>
    {
        static void create_at(TreeIter<> root, TreeState::ptr state, Argv... args)
        {
            Base::construct(d2::TreeCtx<Root, State>(root), std::move(args)...);
        }
    };
} // namespace d2
