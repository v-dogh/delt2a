#ifndef D2_TREE_HPP
#define D2_TREE_HPP

#include "d2_tree_element.hpp"
#include "d2_screen.hpp"

namespace d2
{
	// Tree wrappers
	namespace tree
	{
		namespace
		{
			struct PaddingElem
			{
				static Element::TraversalWrapper create_at(Element::TraversalWrapper loc, TreeState::ptr src)
				{ return nullptr; }
			};

			struct SubTreeDefBuild
			{
				static constexpr auto build = []<typename Type, typename... Argv>(::d2::Screen::ptr src, Argv&&... args)
				{
					return Type::build(src, std::forward<Argv>(args)...);
				};
			};
		}

		template<typename Tree, util::cmp::str String>
		struct make
		{
			static constexpr std::string_view name = String.view();
			using tree = Tree;
		};

		template<typename Type, util::CmpString Name, auto Callback, typename... Argv>
		struct Elem { };
		template<typename Type, util::CmpString Name, auto Callback, typename Sub, typename... Argv>
		struct Elem<Type, Name, Callback, Sub, Argv...>
		{
			static Element::TraversalWrapper create_at(Element::TraversalWrapper loc, TreeState::ptr src)
			{
				auto cur = loc.asp()->override(std::make_shared<Type>(
					std::string(Name.view()),
					src,
					loc.as()
				));
				auto res = Sub::create_at(cur, src);
				(Argv::create_at(cur, src), ...);
				Callback(Element::TraversalWrapper(std::move(cur)), src);
				return res;
			}
		};
		template<typename Type, util::CmpString Name, auto Callback>
		struct Elem<Type, Name, Callback>
		{
			static Element::TraversalWrapper create_at(Element::TraversalWrapper loc, TreeState::ptr src)
			{
				auto cur = loc.asp()->override(std::make_shared<Type>(
					std::string(Name.view()),
					src,
					loc.as()
				));
				Callback(Element::TraversalWrapper(std::move(cur)), src);
				return cur;
			}
		};

		template<auto Make>
		struct ArgElem
		{
			static Element::TraversalWrapper create_at(Element::TraversalWrapper loc, TreeState::ptr src)
			{
				return Make(loc, src);
			}
		};

		template<typename Type, auto Build = SubTreeDefBuild::build>
		struct SubTree
		{
			template<typename... Argv>
			static Element::TraversalWrapper create_at(Element::TraversalWrapper loc, TreeState::ptr src, Argv&&... args)
			{
				auto nsrc = Build.template operator()<Type>(loc->screen(), std::forward<Argv>(args)...);
				nsrc->set_root(loc.asp());
				nsrc->set_core(nsrc->root());
				auto res = Type::create_at(loc, nsrc);
				nsrc->swap_in();
				return res;
			}
		};
	}

	template<typename State, typename Sub, typename... Argv>
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
			state->set_root(std::make_shared<dx::Box>(
				"",
				state,
				nullptr
			));
			state->set_core(state->root());
			return state;
		}
		static Element::TraversalWrapper create_at(Element::TraversalWrapper loc, TreeState::ptr src)
		{
			if constexpr (std::is_void_v<Sub>)
			{
				return nullptr;
			}
			else
			{
				auto res = Sub::create_at(loc, src);
				(Argv::create_at(loc, src), ...);
				return res;
			}
		}
	};

	template<typename Sub, typename... Argv>
	struct TreeTemplate : TreeTemplateInit<TreeState, Sub, Argv...>{ };
}

#endif // D2_TREE_HPP
