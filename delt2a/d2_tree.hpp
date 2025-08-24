#ifndef D2_TREE_HPP
#define D2_TREE_HPP

#include "elements/d2_box.hpp"
#include "d2_tree_element.hpp"
#include "d2_tree_parent.hpp"
#include "d2_screen.hpp"

namespace d2
{
    template<util::cmp::str Name, typename State, typename Tree, typename Root = dx::Box>
    struct TreeTemplateInit
    {
        static constexpr std::string_view name = Name.view();

        template<typename... Argv>
        static auto build_sub(const std::string& name, TreeState::ptr state, d2::Element::pptr root, Argv&&... args)
        {
            auto s = std::make_shared<State>(
                state->screen(),
                state->context(),
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
            root->override(core);
            return s;
        }
        template<typename... Argv>
        static auto build(Screen::ptr src, Argv&&... args)
        {
            auto state = std::make_shared<State>(
                src,
                src->context(),
                nullptr,
                nullptr,
                std::forward<Argv>(args)...
                );
            state->set_root(d2::Element::make<Root>(
                "",
                state,
                nullptr
            ));
            state->set_core(state->root());
            return state;
        }
        static void create_at(TreeIter loc, TreeState::ptr src)
        {
            if constexpr(!std::is_void_v<Tree>)
                Tree::create_at(loc, src);
        }
    };

    template<util::cmp::str Name, typename Tree, typename Root = dx::Box>
    struct TreeTemplate : TreeTemplateInit<Name, TreeState, Tree, Root> { };
}

#endif // D2_TREE_HPP
