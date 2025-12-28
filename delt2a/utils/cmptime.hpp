// Sigma License (c) 2024 vdogh
// Commercial use requires approval. Contact <pvdogh@protonmail.com>.
// See LICENSE.md for details.

#ifndef CMPTIME_HPP
#define CMPTIME_HPP

#include <string_view>
#include <algorithm>
#include <type_traits>
#include <array>

namespace util
{
	namespace meta
	{
		namespace impl
		{
			template<template<typename...> typename Type, typename... Argv>
			struct is_instantiation_of : std::false_type { };

			template<template<typename...> typename Type, typename... Argv>
			struct is_instantiation_of<Type, Type<Argv...>> : std::true_type { };
		}

		template<template<typename...> typename Type, typename... Argv>
		concept is_instantiation_of = (impl::is_instantiation_of<Type, Argv...>::value);

		template<typename Arg, typename... Argv>
		concept uniform_pack = ((std::is_same_v<Arg, Argv>) && ...);

		template<template<typename...> typename Arg, typename... Argv>
		concept uniform_template_pack = ((is_instantiation_of<Arg, Argv>) && ...);

		template<typename Type>
		concept dereferenceable = requires(Type value)
		{
			(*value);
		};

		template<typename One, typename Two>
		concept assignable = requires(One a, Two b)
		{
			(a = b);
		};

		template<typename One, typename Two>
		concept mv_assignable = requires(One a, Two b)
		{
			(a = std::move(b));
		};

		template<typename One, typename Two>
		concept frwd_assignable = requires(One a, Two b)
		{
			(a = std::forward<Two>(b));
		};

		namespace impl
		{
			template<typename Ret, typename Arg, typename... Rest> Arg first_arg_of(Ret(*) (Arg, Rest...));
			template<typename Ret, typename Callback, typename Arg, typename... Rest> Arg first_arg_of(Ret(Callback::*) (Arg, Rest...));
			template<typename Ret, typename Callback, typename Arg, typename... Rest> Arg first_arg_of(Ret(Callback::*) (Arg, Rest...) const);
			template <typename Callback> decltype(first_arg_of(&Callback::operator())) first_arg_of(Callback);
		}
		template <typename Callback> using first_arg_of_t = decltype(impl::first_arg_of(std::declval<Callback>()));
	}

	template<typename... Argv>
	class PackInfo
	{
	public:
		template<typename Type>
		using map_t = std::array<Type, sizeof...(Argv)>;

		template<typename Type>
		struct type_t { using result = Type; };
	private:
		template<typename Arg, typename... Rest>
		static consteval std::size_t _largest_impl()
		{
			if constexpr (!sizeof...(Rest))
				return sizeof(Arg);
			else
				return std::max(sizeof(Arg), _largest_impl<Rest...>());
		}

		template<std::size_t Idx, typename Arg, typename... Rest>
		static consteval auto _at_impl()
		{
			if constexpr (Idx == 0)
				return type_t<Arg>();
			else
				return _at_impl<Idx - 1, Rest...>();
		}

		template<typename Search>
		static consteval std::size_t _map_impl() noexcept
		{
			std::size_t index{ 0 };

			auto search = [&index]<typename Arg>() consteval -> bool
			{
				if (std::is_same_v<Arg, Search>)
					return true;
				index++;
				return false;
			};
			((search.template operator()<Argv>()) || ...);

			return index;
		}
	public:
		PackInfo() = delete;

		static consteval std::size_t largest()
		{
			return _largest_impl<Argv...>();
		}
		template<std::size_t Idx>
		static consteval auto at()
		{
			return _at_impl<Idx, Argv...>();
		}
		static consteval auto size()
		{
			return sizeof...(Argv);
		}
		static consteval auto filter(auto f)
		{
			return (f.template operator()<Argv>() || ...);
		}
		static auto rfilter(auto f)
		{
			return (f.template operator()<Argv>() || ...);
		}

		template<typename Search, typename Type>
		static consteval const auto& map(const map_t<Type>& values)
		{
			return values[_map_impl<Search>()];
		}

		template<typename Search, typename Type>
		static consteval auto map(map_t<Type>&& values)
		{
			return values[_map_impl<Search>()];
		}

		template<auto Idx>
		using At = decltype(at<static_cast<std::size_t>(Idx)>())::result;
	};

	template<typename EnType, typename Pack, std::size_t Offset = 0>
	class EnumStateMap
	{
	private:
		template<std::size_t Idx>
		struct State
		{
			typename decltype(Pack::template at<Idx>())::result data{};
		};

		template<typename>
		struct Storage;

		template<std::size_t... Idx>
		struct Storage<std::index_sequence<Idx...>> :
			State<Offset + Idx>...
		{
			template<EnType Ev>
			auto& at()
			{
				return State<static_cast<std::size_t>(Ev)>::data;
			}
			void apply(EnType ev, auto&& callback)
			{
				auto check = [this, &callback, &ev]<std::size_t Off>() -> bool {
					if (static_cast<std::size_t>(ev) != Off)
						return false;
					callback(at<EnType(Offset + Off)>());
					return true;
				};
				((check.template operator()<Idx>()) || ...);
			}
		};
		Storage<std::make_index_sequence<Pack::size()>> storage{};
	public:
		EnumStateMap() = default;

		template<EnType Ev>
		auto& state()
		{
			return storage.template at<Ev>();
		}
		void apply(EnType ev, auto&& callback)
		{
			storage.apply(ev, callback);
		}
	};

	template <std::size_t Count>
	struct CmpString
	{
		constexpr CmpString(const char (&str)[Count])
		{
			std::copy(str, str + Count, data.begin());
		}
		constexpr CmpString(std::string_view str)
		{
			std::copy(str.begin(), str.begin() + Count, data.begin());
			data[Count - 1] = '\0';
		}
		std::array<char, Count> data{};

		constexpr auto view() const
		{
			return std::string_view(data.data());
		}
		constexpr auto view()
		{
			return std::string_view(data.data());
		}
	};

	namespace cmp
	{
		template<typename Type>
		struct type
		{
			using value = Type;
		};

		template<std::size_t Count>
		struct str : CmpString<Count>
		{
			using base = CmpString<Count>;
			constexpr str(const char (&str)[Count]) : base(str) { }
			constexpr str(std::string_view str) : base(str) { }
		};

		template<str... Strings>
		struct Concat
		{
			static consteval auto _size()
			{
				return (Strings.view().size() + ...);
			}
			static consteval auto _concat()
			{
				std::array<char, _size() + 1> data;
				std::size_t ctr = 0;

				auto push = [&data, &ctr](auto str)
				{
					auto v = str.view();
					for (std::size_t i = 0; i < v.size(); i++)
					{
						data[ctr + i] = v[i];
					}
					ctr += v.size();
				};

				(push(Strings), ...);
				data[ctr] = '\0';

				return data;
			}
			static consteval auto _view()
			{
				return std::string_view(data.begin(), data.end());
			}

			static constexpr auto data = _concat();
			static constexpr auto str = _view();
		};

		template<str Separator, str... Args>
		struct VariadicFormat
		{
			template<str... Rest>
			static consteval std::size_t _csize_v()
			{
				return (Rest.view().size() + ...) + ((Separator.view().size() + 2) * sizeof...(Rest)) + 1;
			}
			static consteval std::size_t _csize()
			{
				return (Args.view().size() + ...) + ((Separator.view().size() + 2) * sizeof...(Args)) + 1;
			}

			template<str Arg, str... Rest>
			static consteval auto _generate_impl()
			{
				std::array<char, _csize_v<Arg, Rest...>()> data{};
				std::size_t offset = 0;
				auto map_to_data = [&data, &offset](const auto& arg)
				{
					for (std::size_t i = 0; i < arg.size(); i++)
					{
						data[offset + i] = arg[i];
					}
					offset += arg.size();
				};
				auto map_to_data_str = [&data, &offset](const auto& arg)
				{
					auto v = arg.view();
					for (std::size_t i = 0; i < v.size(); i++)
					{
						data[offset + i] = v[i];
					}
					offset += v.size();
				};

				if constexpr (sizeof...(Rest) == 0)
				{
					map_to_data_str(Arg);
					data[offset] = '\0';
					return data;
				}
				else
				{
					map_to_data_str(Arg);

					data[offset] = ' ';
					offset++;
					map_to_data_str(Separator);
					data[offset] = ' ';
					offset++;

					auto result = _generate_impl<Rest...>();
					map_to_data(result);
					return data;
				}
			}
			static consteval auto _generate()
			{
				return _generate_impl<Args...>();
			}
			static consteval const char* _from_data()
			{
				return data.data();
			}

			static constexpr auto data = _generate();
			static constexpr auto str = _from_data();
		};
	}
}

#endif
