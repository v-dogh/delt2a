#ifndef D2_STYLES_BASE_HPP
#define D2_STYLES_BASE_HPP

#include <type_traits>
#include <functional>
#include <algorithm>
#include <memory>
#include "d2_tree_element_frwd.hpp"

namespace d2::style
{
	namespace impl
	{
#		define D2_UAI_INTERFACE_TEMPL template<std::size_t> typename
#		define D2_UAI_CHAIN(base) \
		using data::set; \
		using data::get; \
        using data::interface; \
        using data::data;
	}

	using uai_property = unsigned int;

	// I <heart emoji> templates

	class Theme : public std::enable_shared_from_this<Theme>
	{
	public:
		template<typename Type>
		class Dependency
		{
		public:
			using value_type = Type;
            using set_func = bool(*)(void*, const Type&, bool);
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
            Dependency(const Dependency&) = default;
			Dependency(Dependency&&) = default;

            template<typename Data, typename Elem, typename Func>
            void subscribe(std::weak_ptr<Elem> handle, Func&& callback)
            {
                struct Dummy {};
                struct State : std::conditional_t<
                    std::is_same_v<void, Data>,
                    Dummy,
                    Data
                >
                {
                    Func callback;
                    std::weak_ptr<Elem> ptr;
                };
                _dependencies.push_back(Deps{
                    handle, new State{ .callback = std::forward<Func>(callback), .ptr = handle },
                    +[](void* ptr, const Type& value, bool destroy) -> bool {
                        auto* state = static_cast<State*>(ptr);
                        if (destroy)
                        {
                            delete state;
                            return false;
                        }
                        else
                        {
                            if constexpr (std::is_same_v<void, Data>)
                            {
                                return state->callback(state->ptr.lock(), value);
                            }
                            else
                            {
                                return state->callback(
                                    state->ptr.lock(),
                                    static_cast<Data*>(state),
                                    value
                                );
                            }
                        }
                    }
                });
                _dependencies.back().func(
                    _dependencies.back().base,
                    _value,
                    false
                );
            }
			void subscribe(std::weak_ptr<void> handle, void* base, set_func func)
			{
				_dependencies.push_back(Deps{ handle, base, func });
				_dependencies.back().func(
					_dependencies.back().base,
                    _value,
                    false
				);
			}
			void apply()
			{
				bool has_expired = false;
				for (decltype(auto) it : _dependencies)
				{
					const auto exp = it.obj.expired();
					has_expired = exp || has_expired;
					if (!exp)
					{
                        const auto result = it.func(it.base, _value, false);
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
                            [&](const auto& v) {
                                if (v.obj.expired())
                                {
                                    v.func(v.base, _value, true);
                                    return true;
                                }
                                return false;
                            }
						)
					);
				}
			}

			template<typename Value>
			void set(Value&& value)
			{
				_value = std::forward<Value>(value);
				apply();
			}

			const auto& value() const
			{
				return _value;
			}

			template<typename Value>
            operator Value() const requires (!std::is_reference_v<Value> || std::is_const_v<Value>)
			{
				return static_cast<Value>(_value);
			}

            Dependency& operator=(const Dependency&) = default;
			Dependency& operator=(Dependency&&) = default;

            template<typename Value> requires std::is_assignable_v<Type&, Value>
			Dependency& operator=(Value&& value)
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
		Theme& as()
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

        template<typename Value>
        operator Value() const requires (!std::is_reference_v<Value> || std::is_const_v<Value>)
        {
            return Func(dependency.value());
        }
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
        static constexpr auto initial_property = ~0u - 1;
	private:
		template<D2_UAI_INTERFACE_TEMPL Interface>
		auto* _interface_for()
		{
			return static_cast<Interface<0>::data*>(
				_has_interface_impl(Interface<0>::xid)
			);
		}

		template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property, typename Type>
		auto& _int_set_for(Type&& value, bool temporary = false)
		{
			auto* interface = _interface_for<Interface>();
			if (interface == nullptr)
				throw std::logic_error{ "Type does not implement interface" };

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
				auto [ ptr, type ] = interface->template at_style<Property>();
				*ptr = std::forward<Type>(value);
				_signal_base_impl(type, Property);
			}
			return *this;
		}
		template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property>
		auto _int_get_for() const
		{
			return *getref_for<Interface, Property>();
		}
	protected:
        virtual void _initialize_impl(bool force) = 0;
		virtual void _signal_base_impl(element_write_flag, property) = 0;
		virtual void _set_dynamic_impl(property, bool) = 0;
		virtual bool _test_dynamic_impl(property) = 0;
		virtual void* _has_interface_impl(std::size_t) = 0;
		virtual void _sync_impl(std::function<void()>) const = 0;
		virtual std::weak_ptr<void> _handle_impl() = 0;
	public:
		template<D2_UAI_INTERFACE_TEMPL Interface>
		bool has_interface()
		{
			return _interface_for<Interface>() != nullptr;
		}

		template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property, typename Type>
		auto& set_for(Type&& value, bool temporary = false)
		{
			_sync_impl([value = std::forward<Type>(value), &temporary, this]() mutable {
                _int_set_for<Interface, Property>(std::forward<Type>(value), temporary);
			});
			return *this;
		}
		template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property>
		auto get_for() const
		{
			using type = std::remove_cvref_t<decltype(*getref_for<Interface, Property>())>;
			type result;
			_sync_impl([&result, this]() {
				result = _int_get_for<Interface, Property>();
			});
			return result;
		}
		template<D2_UAI_INTERFACE_TEMPL Interface, Interface<0>::Property Property>
		auto* getref_for()
		{
			auto* interface = _interface_for<Interface>();
			if (interface == nullptr)
				throw std::logic_error{ "Type does not implement interface" };
			auto [ ptr, _ ] = interface->template at_style<Property>();
			return ptr;
		}

        void initialize(bool force = false) { _initialize_impl(force); }
    };
}

#endif // D2_STYLES_BASE_HPP
