#ifndef D2_TREE_HPP
#define D2_TREE_HPP

#include <elements/d2_box.hpp>
#include <d2_tree_element.hpp>
#include <d2_tree_parent.hpp>
#include <d2_screen.hpp>
#include <d2_meta.hpp>

namespace d2
{
    template<meta::ConstString Name, typename State, typename Tree, typename Root = dx::Box>
    struct TreeTemplateInit
    {
    public:
        using root = Root;
        using state = State;
        using __state_type = state;

        static constexpr std::string_view name = Name.view();

        template<typename... Argv>
        static auto build_sub(const std::string& name, TreeState::ptr state, d2::Element::pptr root, Argv&&... args)
        {
            auto s = TreeState::make<State>(
                state->root(),
                nullptr,
                nullptr,
                std::forward<Argv>(args)...
            );
            auto core = d2::Element::make<Root>(
                name,
                s
            );
            internal::ElementView::from(core).setparent(root);
            s->set_core(core);
            s->construct();
            root->override(core);
            return s;
        }
        template<typename... Argv>
        static auto build(IOContext::ptr ctx, Argv&&... args)
        {
            auto state = TreeState::make<State>(
                nullptr,
                nullptr,
                ctx,
                std::forward<Argv>(args)...
            );
            state->set_root(d2::Element::make<Root>(
                "",
                state
            ));
            state->set_core(state->root());
            state->construct();
            state->root()->initialize();
            state->root()->setstate(Element::Created, true);
            return state;
        }
    };

    template<meta::ConstString Name, typename Tree, typename Root = dx::Box>
    struct TreeTemplate : TreeTemplateInit<Name, TreeState, Tree, Root> { };
}

#endif // D2_TREE_HPP
