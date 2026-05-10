#ifndef D2_TREE_CONSTRUCT_HPP
#define D2_TREE_CONSTRUCT_HPP

#include <d2_io_handler.hpp>
#include <d2_meta.hpp>
#include <d2_theme.hpp>
#include <d2_tree.hpp>
#include <d2_tree_state.hpp>
#include <mods/d2_core.hpp>

#include <type_traits>

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
            [](std::remove_cvref_t<decltype(ctx.template var<_var_>())> value)                     \
            { return __VA_ARGS__; }                                                                \
        >()                                                                                        \
    )
#define dapply(_stylesheet_, ...) ctx.template apply<_stylesheet_>(__VA_ARGS__)
#define dmstyle(_elem_, _style_, ...)                                                              \
    _elem_->template set<decltype(_elem_)::type::_style_>(__VA_ARGS__)

    class TreeCtxBase
    {
    protected:
        TreeIter<> _ptr{};
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

        template<typename Type, typename Func>
        void subscribe(const std::string& name, Func&& callback) const
        {
            dep<Type>(name).subscribe(
                _ptr.weak(),
                [callback](d2::TreeIter<> ptr, Type value)
                {
                    if constexpr (
                        std::is_same_v<std::invoke_result_t<Func, decltype(ptr), Type>, void>
                    )
                    {
                        callback(ptr, std::move(value));
                        return true;
                    }
                    else
                        return callback(ptr, std::move(value));
                }
            );
        }
        template<typename Type, typename Func>
        void subscribe(const std::string& tree, const std::string& name, Func&& callback) const
        {
            dep<Type>(tree, name)
                .subscribe(
                    _ptr.weak(),
                    [callback](d2::TreeIter<> ptr, Type value)
                    {
                        if constexpr (
                            std::is_same_v<std::invoke_result_t<Func, decltype(ptr), Type>, void>
                        )
                        {
                            callback(ptr, std::move(value));
                            return true;
                        }
                        else
                            return callback(ptr, std::move(value));
                    }
                );
        }
        template<typename Type, typename Func>
        void subscribe(style::Theme::Dependency<Type>& dep, Func&& callback) const
        {
            dep.subscribe(
                _ptr.weak(),
                [callback](d2::TreeIter<> ptr, Type value)
                {
                    if constexpr (
                        std::is_same_v<std::invoke_result_t<Func, decltype(ptr), Type>, void>
                    )
                    {
                        callback(ptr, std::move(value));
                        return true;
                    }
                    else
                        return callback(ptr, std::move(value));
                }
            );
        }

        template<auto Var> auto& var() const
        {
            return screen()->template themev<Var>();
        }
        template<auto Var, auto Callback> auto& dynavar() const
        {
            return style::dynavar<Callback>(var<Var>());
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
            (ctx->is_synced()
                 ? (std::forward<Func>(callback)(std::forward<Argv>(args)...))
                 : ctx->sync(std::bind(std::forward<Func>(callback), std::forward<Argv>(args)...)));
        }
        template<typename Func, typename... Argv>
        auto sync_async(Func&& callback, Argv&&... args) const
        {
            const auto ctx = context();
            (ctx->is_synced()
                 ? (std::forward<Func>(callback)(std::forward<Argv>(args)...))
                 : ctx->sync_async(
                       std::bind(std::forward<Func>(callback), std::forward<Argv>(args)...)
                   ));
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

        // Children

        template<typename Type, typename Func, typename... Arge, typename... Args>
            requires std::invocable<Func, TreeCtx<Type, State>>
        void embed(
            const std::string& name,
            Func&& callback,
            std::tuple<Arge...> arge = {},
            std::tuple<Args...> args = {}
        ) const
        {
            auto nsrc = std::apply(
                [&](auto... args)
                { return Type::build_sub(name, state(), _ptr.asp(), std::move(args)...); },
                args
            );
            auto cptr = nsrc->core();
            std::apply(
                [&](auto... args)
                { Type::create_at(nsrc->core(), nsrc, std::forward<Args>(args)...); },
                arge
            );
            nsrc->swap_in();
        }
        template<typename Type, typename Func, typename... Arge, typename... Args>
            requires std::invocable<Func, TreeCtx<Type, State>>
        void
        embed(Func&& callback, std::tuple<Arge...> arge = {}, std::tuple<Args...> args = {}) const
        {
            embed<Type>("", std::forward<Func>(callback), std::move(arge), std::move(args));
        }
        template<typename Type, typename... Args, typename... Arge>
        void embed(
            const std::string& name, std::tuple<Arge...> arge = {}, std::tuple<Args...> args = {}
        ) const
        {
            embed<Type>(name, [](auto) {}, std::move(arge), std::move(args));
        }
        template<typename Type, typename... Arge, typename... Args>
        void embed(std::tuple<Arge...> arge = {}, std::tuple<Args...> args = {}) const
        {
            embed<Type>("", [](auto) {}, std::move(arge), std::move(args));
        }

        template<typename Type, typename Func, typename... Argv>
            requires std::invocable<Func, TreeCtx<Type, State>>
        void elem(Func&& callback, Argv&&... args) const
        {
            auto ptr = Element::make<Type>("", state(), std::forward<Argv>(args)...);
            internal::ElementView::from(ptr).setparent(_ptr.asp());
            callback(TreeCtx<Type, State>(ptr));
            _ptr.asp()->override(ptr);
        }
        template<typename Type, typename Func, typename... Argv>
            requires std::invocable<Func, TreeCtx<Type, State>>
        void elem(std::string name, Func&& callback, Argv&&... args) const
        {
            auto ptr = Element::make<Type>(std::move(name), state(), std::forward<Argv>(args)...);
            internal::ElementView::from(ptr).setparent(_ptr.asp());
            callback(TreeCtx<Type, State>(ptr));
            _ptr.asp()->override(ptr);
        }
        template<typename Type, typename Func>
            requires std::invocable<Func, TreeCtx<Type, State>>
        void elem(Element::eptr<Type> ptr, Func&& callback) const
        {
            internal::ElementView::from(ptr).setparent(_ptr.asp());
            callback(TreeCtx<Type, State>(ptr));
            _ptr.asp()->override(ptr);
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
            Base::construct(root, state, std::move(args)...);
        }
    };
} // namespace d2

#endif
