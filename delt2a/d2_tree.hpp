#ifndef D2_TREE_HPP
#define D2_TREE_HPP

#include "elements/d2_box.hpp"
#include "d2_tree_element.hpp"
#include "d2_tree_parent.hpp"
#include "d2_screen.hpp"
#include "d2_meta.hpp"

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
            auto s = std::make_shared<State>(
                state->root(),
                nullptr,
                std::forward<Argv>(args)...
            );
            auto core = d2::Element::make<Root>(
                name,
                s,
                root
            );
            s->set_core(core);
            s->construct();
            root->override(core);
            return s;
        }
        template<typename... Argv>
        static auto build(IOContext::ptr ctx, Argv&&... args)
        {
            auto state = std::make_shared<State>(
                nullptr,
                nullptr,
                std::forward<Argv>(args)...
            );
            state->set_context(ctx);
            state->set_root(d2::Element::make<Root>(
                "",
                state,
                nullptr
            ));
            state->construct();
            state->root()->initialize();
            state->root()->setstate(Element::Created, true);
            state->set_core(state->root());
            return state;
        }
    };

    template<meta::ConstString Name, typename Tree, typename Root = dx::Box>
    struct TreeTemplate : TreeTemplateInit<Name, TreeState, Tree, Root> { };
}

#endif // D2_TREE_HPP
