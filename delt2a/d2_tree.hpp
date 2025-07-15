#ifndef D2_TREE_HPP
#define D2_TREE_HPP

#include "elements/d2_box.hpp"
#include "d2_tree_element.hpp"
#include "d2_screen.hpp"

namespace d2
{	
	namespace tree
	{
		template<typename Tree, util::cmp::str String>
		struct make
		{
			static constexpr std::string_view name = String.view();
			using tree = Tree;
		};
	}

	template<typename State, typename Tree>
	struct TreeTemplateInit
	{
		template<typename... Argvv>
		static auto build(Screen::ptr src, Argvv&&... args)
		{
			auto state = std::make_shared<State>(
				src,
				src->context(),
				nullptr,
				nullptr,
				args...
			);
			state->set_root(d2::Element::make<dx::Box>(
				"",
				state,
				nullptr
			));
			state->set_core(state->root());
			return state;
		}
		static Element::TraversalWrapper create_at(Element::TraversalWrapper loc, TreeState::ptr src)
		{
			if constexpr (std::is_void_v<Tree>)
			{
				return nullptr;
			}
			else
			{
				return Tree::create_at(loc, src);
			}
		}
	};

	template<typename Tree>
	struct TreeTemplate : TreeTemplateInit<TreeState, Tree>{ };
}

#endif // D2_TREE_HPP
