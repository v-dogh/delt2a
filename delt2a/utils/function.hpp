#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <type_traits>
#include <stdexcept>
#include <memory>

namespace util
{
	namespace meta
	{
		template<typename Type, typename RetVal, typename... Argv>
		concept Lambda = requires (Type t)
		{
			{ t(std::declval<Argv>()...) } -> std::same_as<RetVal>;
		};

		template<typename Type>
		concept CopyConstruct = requires (Type& from, Type& to)
		{
			{ Type(from) };
		};

		template<typename Type>
		concept MoveConstruct = requires (Type& from, Type& to)
		{
			{ Type(std::move(from)) };
		};
	}

	template<typename>
	class FunctionWrapper;

	// No functional :,< since cant pass move-only lambdas to std::function :<<<<
	template<typename FRet, typename... FArgv>
	class FunctionWrapper<FRet(FArgv...)>
	{
	private:
		class BaseErased
		{
		public:
			virtual ~BaseErased() = default;
			virtual FRet invoke(FArgv...) const = 0;
			FRet invoke(FArgv... args)
			{
				return const_cast<const BaseErased*>(this)->invoke(args...);
			}
			virtual BaseErased* copy() = 0;
			virtual void copy_to(void*) const = 0;
			virtual void move_to(void*) = 0;
			void copy_to(void* ptr)
			{
				return const_cast<const BaseErased*>(this)->copy_to(ptr);
			}
		};
		template<typename Func>
		class ImplErased : public BaseErased
		{
		private:
			mutable Func m_function{};
		public:
			ImplErased(const Func& func) requires meta::CopyConstruct<Func>
				: m_function(func) { }
			ImplErased(Func&& func)
				: m_function(std::move(func)) { }

			virtual BaseErased* copy() override final
			{
				if constexpr (meta::CopyConstruct<Func>)
				{
					return new ImplErased{ m_function };
				}
				else
					throw std::logic_error{ std::string("attempt to copy construct a move-only lambda: ") + typeid(Func).name() };
			}
			virtual void copy_to(void* buffer) const override final
			{
				if constexpr (meta::CopyConstruct<Func>)
					new (buffer) ImplErased(m_function);
				else
					throw std::logic_error{ std::string("attempt to copy construct a move-only lambda: ") + typeid(Func).name() };
			}
			virtual void move_to(void* buffer) override final
			{
				if constexpr (meta::MoveConstruct<Func>)
					new (buffer) ImplErased(std::move(m_function));
				else
					throw std::logic_error{ std::string("attempt to move construct a move-only lambda: ") + typeid(Func).name() };

			}
			virtual FRet invoke(FArgv... args) const override final
			{
				return m_function(std::forward<FArgv>(args)...);
			}

			ImplErased& operator=(const ImplErased& copy) = delete;
			ImplErased& operator=(ImplErased&& copy) = delete;
		};
		template<typename Type>
		struct TargetDeleter
		{
			TargetDeleter() noexcept = default;

			template<class Up, typename = std::enable_if<std::is_convertible_v<Up*, Type*>>>
			TargetDeleter(const TargetDeleter<Up>&) noexcept { }

			void operator()(Type* ptr)
			{
				if (ptr)
				{
					ptr->~Type();
				}
			}
		};

		//using target_ptr = std::unique_ptr<BaseErased, TargetDeleter<BaseErased>>;
		using target_ptr = std::unique_ptr<BaseErased>;
	private:
		target_ptr m_value{};

		const auto& _target() const noexcept
		{
			return *m_value;
		}
		auto& _target() noexcept
		{
			return *m_value;
		}

		void _set_null() noexcept
		{
			m_value = nullptr;
		}
		bool _is_null() const noexcept
		{
			return m_value == nullptr;
		}
		bool _is_null() noexcept
		{
			return m_value == nullptr;
		}

		template<typename Func>
		void _allocate_target(Func&& target)
		{
			_allocate_dynamic_target(std::forward<Func>(target));
		}
		void _destroy_target()
		{

		}

		void _copy_target(const FunctionWrapper& copy) noexcept
		{
			if (!copy._is_null())
			{
				m_value = target_ptr(copy.m_value->copy());
			}
			else
				_set_null();
		}
		void _move_target(FunctionWrapper&& copy) noexcept
		{
			m_value = std::move(copy.m_value);
		}

		template<typename Func>
		void _allocate_dynamic_target(Func&& target) noexcept
		{
			using decayed = std::decay_t<Func>;
			m_value = std::make_unique<ImplErased<decayed>>(std::forward<Func>(target));
		}
	public:
		FunctionWrapper()
		{
			_set_null();
		}
		FunctionWrapper(const FunctionWrapper& copy)
		{
			_copy_target(copy);
		}
		FunctionWrapper(FunctionWrapper&& copy)
		{
			_move_target(std::move(copy));
		}
		FunctionWrapper(std::nullptr_t) { _set_null(); }
		template<typename Func>
		FunctionWrapper(Func&& lambda)
			requires (
					meta::Lambda<std::decay_t<Func>, FRet, FArgv...> &&
					!std::is_same_v<std::decay_t<Func>, FunctionWrapper>
					)
		{
			_set_null();
			_allocate_target<Func>(std::forward<Func>(lambda));
		}
		/*virtual*/ ~FunctionWrapper()
		{
			_destroy_target();
		}

		const void* target() const noexcept
		{
			return &_target();
		}
		void* target() noexcept
		{
			return &_target();
		}

		template<typename Func>
		void reset(Func&& lambda)
				requires (
						meta::Lambda<std::decay_t<Func>, FRet, FArgv...> &&
						!std::is_same_v<std::decay_t<Func>, FunctionWrapper>
						)
		{
			_allocate_target<Func>(std::forward<Func>(lambda));
		}

		template<typename Func>
		auto& operator=(Func&& lambda)
				requires (
						meta::Lambda<std::decay_t<Func>, FRet, FArgv...> &&
						!std::is_same_v<std::decay_t<Func>, FunctionWrapper>
						)
		{
			_allocate_target<Func>(std::forward<Func>(lambda));
			return *this;
		}

		auto& operator=(const FunctionWrapper& copy)
		{
			_copy_target(copy);
			return *this;
		}
		auto& operator=(FunctionWrapper&& copy)
		{
			_move_target(std::move(copy));
			return *this;
		}

		bool operator==(std::nullptr_t) const noexcept
		{
			return _is_null();
		}
		bool operator==(std::nullptr_t) noexcept
		{
			return _is_null();
		}
		bool operator!=(std::nullptr_t) const noexcept
		{
			return !_is_null();
		}
		bool operator!=(std::nullptr_t) noexcept
		{
			return !_is_null();
		}

		template<typename... Argv>
		FRet operator()(Argv&&... args) const
		{
			if (_is_null())
				throw std::runtime_error("Attempt to invoke null target");
			return _target().invoke(std::forward<Argv>(args)...);
		}

		template<typename... Argv>
		FRet operator()(Argv&&... args)
		{
			return const_cast<const FunctionWrapper*>(this)->operator()(std::forward<Argv>(args)...);
		}
	};
}

#endif // FUNCTION_HPP
