#ifndef D2_STYLES_BASE_HPP
#define D2_STYLES_BASE_HPP

#include <type_traits>
#include <functional>
#include <memory>
#include "d2_tree_element_frwd.hpp"

namespace d2::style
{
	using uai_property = unsigned int;

	// I <heart emoji> templates

	namespace impl
	{
#		define D2_UAI_INTERFACE_TEMPL template<std::size_t> typename
#		define D2_UAI_CHAIN(base) \
		using data::set; \
		using data::get; \
		using data::interface;

#		define D2_UAI_SETUP(type) unsigned char __type = Element::WriteType::type;
#		define D2_UAI_SETUP_EMPTY unsigned char __type = 0x00;
#		define D2_UAI_SETUP_MUL(type) unsigned char __type = type;

#		define D2_UAI_VAR_START if constexpr (false) {}
#		define D2_UAI_GET_VAR(prop, var, type) \
		else if constexpr (Property == prop) return std::make_pair(&var, ::d2::Element::WriteType::type);
#		define D2_UAI_GET_VAR_MUL(prop, var, type) \
		else if constexpr (Property == prop) return std::make_pair(&var, type);
#		define D2_UAI_GET_VAR_A(prop, var) \
		else if constexpr (Property == prop) return std::make_pair(&var, __type);
#		define D2_UAI_GET_VAR_COMPUTED(prop, type, ...) \
		else if constexpr (Property == prop) return std::make_pair(__VA_ARGS__, ::d2::Element::WriteType::type);
#		define D2_UAI_GET_VAR_COMPUTED_MUL(prop, type, ...) \
		else if constexpr (Property == prop) return std::make_pair(__VA_ARGS__, type);
#		define D2_UAI_GET_VAR_COMPUTED_A(prop, ...) \
		else if constexpr (Property == prop) return std::make_pair(__VA_ARGS__, __type);
#		define D2_UAI_VAR_END else static_assert(false, "Invalid Property");

		template<typename Chain>
		struct ResolveChain
		{
			static constexpr auto offset = Chain::last_offset_;
		};
		template<>
		struct ResolveChain<void>
		{
			static constexpr auto offset = 0;
		};

		template<D2_UAI_INTERFACE_TEMPL First, D2_UAI_INTERFACE_TEMPL... Rest>
		struct LastImpl
		{
			template<std::size_t Prop>
			using type = LastImpl<Rest...>::template type<Prop>;
		};
		template<D2_UAI_INTERFACE_TEMPL Last>
		struct LastImpl<Last>
		{
			template<std::size_t Prop>
			using type = Last<Prop>;
		};

		namespace
		{
			template<
				std::size_t Search,
				typename Current,
				D2_UAI_INTERFACE_TEMPL... Rest
			> struct SearchPropertyOwnerImpl { };
			template<
				std::size_t Search,
				typename Current,
				D2_UAI_INTERFACE_TEMPL Next,
				D2_UAI_INTERFACE_TEMPL... Rest
			>
			struct SearchPropertyOwnerImpl<Search, Current, Next, Rest...>
			{
				using type = std::conditional<
					Search >= Current::base && Search < Current::offset,
					Current,
					typename SearchPropertyOwnerImpl<
						Search,
						Next<Current::offset>,
						Rest...
					>::type
				>::type;
			};
			template<
				std::size_t Search,
				typename Current
			>
			struct SearchPropertyOwnerImpl<Search, Current>
			{
				using type = std::conditional<
					Search >= Current::base && Search < Current::offset,
					Current,
					void
				>::type;
			};
		}
		template<
			std::size_t BaseOffset,
			std::size_t Search,
			D2_UAI_INTERFACE_TEMPL First,
			D2_UAI_INTERFACE_TEMPL... Rest
		>
		struct SearchPropertyOwner
		{
			using type = SearchPropertyOwnerImpl<
				Search,
				First<BaseOffset>,
				Rest...
			>::type;
		};

		namespace
		{
			template<
				std::size_t Offset,
				D2_UAI_INTERFACE_TEMPL Search,
				D2_UAI_INTERFACE_TEMPL... Rest
			> struct SearchInterfaceImpl { };
			template<
				std::size_t Offset,
				D2_UAI_INTERFACE_TEMPL Search,
				D2_UAI_INTERFACE_TEMPL Current,
				D2_UAI_INTERFACE_TEMPL... Rest
			>
			struct SearchInterfaceImpl<Offset, Search, Current, Rest...>
			{
				using type = std::conditional<
					std::is_same_v<Search<0>, Current<0>>,
					Current<Offset>,
					typename SearchInterfaceImpl<
						Current<Offset>::offset,
						Search,
						Rest...
					>::type
				>::type;
			};
			template<
				std::size_t Offset,
				D2_UAI_INTERFACE_TEMPL Search,
				D2_UAI_INTERFACE_TEMPL Current
			>
			struct SearchInterfaceImpl<Offset, Search, Current>
			{
				using type = std::conditional<
					std::is_same_v<Search<0>, Current<0>>,
					Current<Offset>,
					void
				>::type;
			};
		}
		template<
			std::size_t BaseOffset,
			D2_UAI_INTERFACE_TEMPL Search,
			D2_UAI_INTERFACE_TEMPL... Rest
		>
		struct SearchInterface
		{
			using type = SearchInterfaceImpl<
				BaseOffset,
				Search,
				Rest...
			>;
		};

		namespace
		{
			template<
				D2_UAI_INTERFACE_TEMPL Search,
				typename Current,
				D2_UAI_INTERFACE_TEMPL CurrentType,
				D2_UAI_INTERFACE_TEMPL Next,
				D2_UAI_INTERFACE_TEMPL... Rest
			>
			struct OffsetHelperImpl
			{
				using type = std::conditional<
					std::is_same_v<Next<0>, Search<0>>,
					Current,
					typename OffsetHelperImpl<
						Search,
						Next<Current::offset>,
						Next,
						Rest...
					>::type
				>::type;
			};
			template<
				D2_UAI_INTERFACE_TEMPL Search,
				typename Current,
				D2_UAI_INTERFACE_TEMPL CurrentType,
				D2_UAI_INTERFACE_TEMPL Next
			>
			struct OffsetHelperImpl<Search, Current, CurrentType, Next>
			{
				using type = Current;
			};
		}
		template<
			std::size_t BaseOffset,
			D2_UAI_INTERFACE_TEMPL Current,
			D2_UAI_INTERFACE_TEMPL First,
			D2_UAI_INTERFACE_TEMPL... Rest
		>
		struct OffsetHelper
		{
			using result = OffsetHelperImpl<
				Current,
				First<BaseOffset>,
				First,
				Rest...
			>;
			using type = result::type;
			static constexpr auto offset = result::type::offset;
		};
		template<
			std::size_t BaseOffset,
			D2_UAI_INTERFACE_TEMPL Current,
			D2_UAI_INTERFACE_TEMPL First
		>
		struct OffsetHelper<BaseOffset, Current, First>
		{
			static constexpr auto offset = BaseOffset;
		};
	}

	class Theme : public std::enable_shared_from_this<Theme>
	{
	public:
		template<typename Type>
		class Dependency
		{
		public:
			using value_type = Type;
			using set_func = bool(*)(void*, const Type&);
		private:
			struct Deps
			{
				std::weak_ptr<void> obj;
				void* base;
				set_func func;
			};
		private:
			Type _value{};
			std::vector<Deps> _dependencies{};
		public:
			Dependency() = default;
			Dependency(const Dependency&) = delete;
			Dependency(Dependency&&) = default;

			void subscribe(std::weak_ptr<void> handle, void* base, set_func func) noexcept
			{
				_dependencies.push_back(Deps{ handle, base, func });
				_dependencies.back().func(
					_dependencies.back().base,
					_value
				);
			}
			void apply() noexcept
			{
				bool has_expired = false;
				for (decltype(auto) it : _dependencies)
				{
					const auto exp = it.obj.expired();
					has_expired = exp || has_expired;
					if (!exp)
					{
						const auto result = it.func(it.base, _value);
						if (!result)
						{
							it.obj = std::weak_ptr<void>();
							has_expired = true;
						}
					}
				}

				if (has_expired)
				{
					_dependencies.erase(
						std::remove_if(
							_dependencies.begin(), _dependencies.end(),
							[](const auto& v) { return v.obj.expired(); }
						)
					);
				}
			}

			template<typename Value>
			void set(Value&& value) noexcept
			{
				_value = std::forward<Value>(value);
				apply();
			}

			template<typename Value>
			operator Value() requires (!std::is_reference_v<Value> || std::is_const_v<Value>)
			{
				return static_cast<Value>(_value);
			}

			Dependency& operator=(const Dependency&) = delete;
			Dependency& operator=(Dependency&&) = default;

			template<typename Value> requires std::is_assignable_v<Type, Value>
			Dependency& operator=(Value&& value) noexcept
			{
				_value = std::forward<Value>(value);
				apply();
				return *this;
			}
		};
		using ptr = std::shared_ptr<Theme>;
	public:
		template<typename Type, typename Func, typename... Argv> requires std::is_invocable_v<Func, Type*, Argv...>
		static auto make(Func&& func, Argv&&... args)
		{
			auto theme = std::make_shared<Type>();
			func(theme.get(), std::forward<Argv>(args)...);
			return theme;
		}
		template<typename Type, typename Arg, typename... Argv>
		static auto make(Arg&& arg, Argv&&... args) requires (!std::is_invocable_v<Arg, Type*, Argv...>)
		{
			auto theme = std::make_shared<Type>();
			theme->accents(std::forward<Arg>(arg), std::forward<Argv>(args)...);
			return theme;
		}
		template<typename Type>
		static auto make()
		{
			return std::make_shared<Type>();
		}

		Theme() = default;
		Theme(const Theme&) = delete;
		Theme(Theme&&) = default;
		virtual ~Theme() = default;

		template<typename Theme>
		Theme& as() noexcept
		{
			return *static_cast<Theme*>(this);
		}

		Theme& operator=(const Theme&) = delete;
		Theme& operator=(Theme&&) = default;
	};

	template<auto Func, typename Type>
	struct Dynavar
	{
		static constexpr auto filter = Func;
		Type& dependency;

		explicit Dynavar(Type& dep) : dependency(dep) {}
	};

	template<auto Func, typename Type>
	auto dynavar(Type& dep)
	{
		return Dynavar<Func, Type>(dep);
	}

	namespace impl
	{
		template<typename Type>
		struct is_var_impl : std::false_type { };

		template<typename Arg>
		struct is_var_impl<Theme::Dependency<Arg>> : std::true_type { };

		template<typename Type>
		concept is_var = (is_var_impl<Type>::value);

		template<typename Type>
		struct is_dynavar_impl : std::false_type { };

		template<is_var Dep, auto Func>
		struct is_dynavar_impl<Dynavar<Func, Dep>> : std::true_type { };

		template<typename Type>
		concept is_dynavar = (is_dynavar_impl<Type>::value);
	}

	class UniversalAccessInterfaceBase
	{
	public:
		using property = unsigned int;
	private:
		template<D2_UAI_INTERFACE_TEMPL Interface>
		auto* _interface_for()
		{
			return static_cast<Interface<0>::data*>(
				_has_interface_impl(Interface<0>::xid)
			);
		}

		template<D2_UAI_INTERFACE_TEMPL Interface, property Property, typename Type>
		auto _int_set_for(Type&& value, bool temporary = false)
		{
			if constexpr (impl::is_var<Type>)
			{
				_set_dynamic_impl(Property, true);
				value.subscribe(_handle_impl(), this, [](void* base, const auto& value) -> bool {
					auto* base_ptr = static_cast<UniversalAccessInterfaceBase*>(base);
					if (!base_ptr->_test_dynamic_impl(Property))
						return false;
					base_ptr->set_for<Interface, Property>(value);
					base_ptr->_set_dynamic_impl(Property, true);
					return true;
				});
			}
			else if constexpr (impl::is_dynavar<std::remove_cvref_t<Type>>)
			{
				_set_dynamic_impl(Property, true);
				value.dependency.subscribe(_handle_impl(), this, [](void* base, const auto& v) -> bool {
					auto* base_ptr = static_cast<UniversalAccessInterfaceBase*>(base);
					if (!base_ptr->_test_dynamic_impl(Property))
						return false;
					base_ptr->set_for<Interface, Property>(std::remove_cvref_t<decltype(value)>::filter(v));
					base_ptr->_set_dynamic_impl(Property, true);
					return true;
				});
			}
			else
			{
				_set_dynamic_impl(Property, _test_dynamic_impl(Property) && temporary);
				auto* interface = _interface_for<Interface>();
				if (interface == nullptr)
					throw std::logic_error{ "Type does not implement interface" };
				auto [ ptr, type ] = interface->template at_style<Property>();
				*ptr = std::forward<Type>(value);
				_signal_base_impl(type, Property);
			}
			return *this;
		}
		template<D2_UAI_INTERFACE_TEMPL Interface, property Property>
		auto _int_get_for() const
		{
			return *getref_for<Interface, Property>();
		}
	protected:
		virtual void _signal_base_impl(element_write_flag, property) = 0;
		virtual void _set_dynamic_impl(property, bool) = 0;
		virtual bool _test_dynamic_impl(property) = 0;
		virtual void* _has_interface_impl(std::size_t) = 0;
		virtual void _sync_impl(std::function<void()>) const noexcept = 0;
		virtual std::weak_ptr<void> _handle_impl() = 0;
	public:
		template<D2_UAI_INTERFACE_TEMPL Interface>
		bool has_interface() noexcept
		{
			return _interface_for<Interface>() != nullptr;
		}

		template<D2_UAI_INTERFACE_TEMPL Interface, property Property, typename Type>
		auto& set_for(Type&& value, bool temporary = false)
		{
			_sync_impl([value = std::forward<Type>(value), &temporary, this]() {
				_int_set_for<Interface, Property>(std::forward<Type>(value), temporary);
			});
			return *this;
		}
		template<D2_UAI_INTERFACE_TEMPL Interface, property Property>
		auto get_for() const
		{
			using type = std::remove_cvref_t<decltype(*getref_for<Interface, Property>())>;
			type result;
			_sync_impl([&result, this]() {
				result = _int_get_for<Interface, Property>();
			});
			return result;
		}
		template<D2_UAI_INTERFACE_TEMPL Interface, property Property>
		auto* getref_for()
		{
			auto* interface = _interface_for<Interface>();
			if (interface == nullptr)
				throw std::logic_error{ "Type does not implement interface" };
			auto [ ptr, _ ] = interface->template at_style<Property>();
			return ptr;
		}
	};
}

#endif // D2_STYLES_BASE_HPP
