#ifndef D2_STYLES_HPP
#define D2_STYLES_HPP

#include <type_traits>
#include <bitset>
#include "d2_styles_base.hpp"
#include "d2_tree_element.hpp"
#include "d2_element_units.hpp"
#include "d2_pixel.hpp"

namespace d2::style
{
	// UAI provides read/write access to styles (member variables) of objects that derive from it (elements)
	// It handles proper synchronization and signals, and manages the process of assigning unique ID's to properties for elements at compile time
	// It also facilitates the ability for objects to derive from other objects through chaining
	// I.e. the topmost UAI is aware of the lower UAI and delegates their property accesses to it
	// Access to variables through UAI is also automatically synchronized
	// I.e. accesses from separate threads will be queued on the main thread by default
	// Main thread reads/writes should mostly be zero overhead

	template<
		typename Chain,
		typename Base,
		D2_UAI_INTERFACE_TEMPL Interface,
		D2_UAI_INTERFACE_TEMPL... Interfaces
	>
	class UniversalAccessInterface :
		public Interface<impl::ResolveChain<Chain>::offset>,
		public Interfaces<
			impl::OffsetHelper<
				impl::ResolveChain<Chain>::offset,
				Interfaces, Interface, Interfaces...
			>::offset
		>...
	{
	public:
		using property = uai_property;
		template<property Property>
		using type_of = decltype(get<Property>());
	private:
		template<std::size_t Prop>
		using last_interface =
			impl::LastImpl<Interface, Interfaces...>::template type<Prop>;
		static constexpr std::size_t base_offset_ = impl::ResolveChain<Chain>::offset;
		static constexpr std::size_t last_offset_ =
			impl::OffsetHelper<
				base_offset_,
				last_interface,
				Interface, Interfaces...
			>::offset + last_interface<0>::count;
		static constexpr std::size_t chain_base_ =
			impl::ResolveChain<Chain>::offset;
		static constexpr bool chain_ =
			!std::is_same_v<Chain, void>;

		template<D2_UAI_INTERFACE_TEMPL Type>
		struct FindInterface
		{
			using type = impl::SearchInterface<
				base_offset_,
				Type,
				Interface, Interfaces...
			>::type;
		};
	private:
		// Bit is set if the variable is dynamic (i.e. dependent on Theme::Dependency<T>)
		std::bitset<last_offset_ - base_offset_> _var_flags{};

		const Base* _base() const noexcept
		{
			return static_cast<const Base*>(this);
		}
		Base* _base() noexcept
		{
			return static_cast<Base*>(this);
		}
		auto _screen() const noexcept
		{
			return _base()->state()->screen();
		}

		template<property Property, typename Type>
		auto _int_set(Type&& value, bool temporary = false)
		{
			static_assert(Property <= last_offset_, "Invalid Property");
			using interface = impl::SearchPropertyOwner<
				base_offset_, Property, Interface, Interfaces...
			>::type;
			constexpr auto off = Property - base_offset_;

			if constexpr (impl::is_var<std::remove_cvref_t<Type>>)
			{
				_var_flags.set(off);
				value.subscribe(_handle_impl(), this, [](void* base, const auto& value) -> bool {
					auto* base_ptr = static_cast<UniversalAccessInterface*>(base);
					if (!base_ptr->_var_flags.test(off))
						return false;
					base_ptr->template set<Property>(value);
					base_ptr->_var_flags.set(off);
					return true;
				});
			}
			else if constexpr (impl::is_dynavar<std::remove_cvref_t<Type>>)
			{
				_var_flags.set(off);
				value.dependency.subscribe(_handle_impl(), this, [](void* base, const auto& v) -> bool {
					auto* base_ptr = static_cast<UniversalAccessInterface*>(base);
					if (!base_ptr->_var_flags.test(off))
						return false;
					base_ptr->template set<Property>(std::remove_cvref_t<decltype(value)>::filter(v));
					base_ptr->_var_flags.set(off);
					return true;
				});
			}
			else
			{
				_var_flags.set(off, _var_flags.test(off) && temporary);
				auto [ ptr, type ] = interface::template get<Property>();
				*ptr = std::forward<Type>(value);
				_signal_base_impl(type, Property);
			}
		}
		template<property Property>
		auto _int_get_vals()
		{
			static_assert(Property <= last_offset_, "Invalid Property");
			using interface = impl::SearchPropertyOwner<
				base_offset_, Property, Interface, Interfaces...
			>::type;
			return interface::template get<Property>();
		}
		template<property Property>
		auto _int_get()
		{
			auto [ ptr, _ ] = _int_get_vals<Property>();
			return *ptr;
		}
		template<property Property>
		auto* _int_getref()
		{
			auto [ ptr, _ ] = _int_get_vals<Property>();
			return ptr;
		}

		template<property Property, typename Type>
		void _int_set_synced(Type&& value, bool temporary = false)
		{
			const auto src = _screen();
			if (src->is_synced())
			{
				_int_set<Property>(std::forward<Type>(value), temporary);
			}
			else
			{
				const auto _ = _base()->shared_from_this();
				if constexpr (impl::is_var<std::remove_cvref_t<Type>>)
				{
					static_assert(std::is_reference_v<Type>, "Dependency must be a reference");
					src->sync([this, &value, &temporary]() {
						_int_set<Property>(std::move(value), temporary);
					}).sync();
				}
				else
				{
					src->sync([this, value = std::forward<Type>(value), &temporary]() {
						_int_set<Property>(std::move(value), temporary);
					}).sync();
				}
			}
		}
		template<property Property>
		auto _int_get_synced()
		{
			const auto src = _screen();
			if (src->is_synced())
			{
				return _int_get<Property>();
			}
			else
			{
				const auto _ = _base()->shared_from_this();
				return src->sync([this]() {
					return _int_get<Property>();
				}).value();
			}
		}
	protected:
		void* _has_interface_own_impl(std::size_t id) noexcept
		{
			return UniversalAccessInterface::_has_interface_impl(id);
		}
		virtual void _signal_base_impl(Element::write_flag type, property prop) override
		{
			if (type != 0x00)
				_base()->_signal_write(type, prop);
		}
		virtual void _set_dynamic_impl(property prop, bool value) override
		{
			if constexpr (chain_)
			{
				if (prop < base_offset_)
				{
					Chain::_set_dynamic_impl(prop, value);
					return;
				}
			}
			_var_flags.set(prop - base_offset_, value);
		}
		virtual bool _test_dynamic_impl(property prop) override
		{
			if constexpr (chain_)
			{
				if (prop < base_offset_)
					return Chain::_test_dynamic_impl(prop);
			}
			return _var_flags.test(prop - base_offset_);
		}
		virtual void* _has_interface_impl(std::size_t id) noexcept override
		{
			if constexpr (chain_)
			{
				auto* ptr =
					static_cast<Chain*>(
					static_cast<Base*>(this)
					)->_has_interface_own_impl(id);
				if (ptr != nullptr)
					return ptr;
			}

			void* out = nullptr;
			auto check_xid = [&id, &out, this]<D2_UAI_INTERFACE_TEMPL Type>()
			{
				const auto pred = Type<0>::xid == id;
				if (pred)
					out = static_cast<Type<0>::data*>(this);
				return pred;
			};
			[[ maybe_unused ]] const auto result = ((check_xid.template operator()<Interfaces>()) || ...);
			return out;
		}
		virtual void _sync_impl(std::function<void()> func) const noexcept override
		{
			const auto src = _screen();
			if (src->is_synced())
			{
				func();
			}
			else
			{
				const auto _ = _base()->shared_from_this();
				return src->sync([&func]() {
					func();
				}).sync();
			}
		}
		virtual std::weak_ptr<void> _handle_impl() override
		{
			return _base()->shared_from_this();
		}
	public:
		template<typename>
		friend class impl::ResolveChain;
		template<typename, typename, D2_UAI_INTERFACE_TEMPL, D2_UAI_INTERFACE_TEMPL...>
		friend class UniversalAccessInterface;

		UniversalAccessInterface() = default;

		template<D2_UAI_INTERFACE_TEMPL Type>
		auto& interface() noexcept
		{
			if constexpr (chain_)
			{
				using resolved_chain = Chain::template FindInterface<Type>::type;
				if constexpr (std::is_same_v<void, typename resolved_chain::type>)
				{
					return static_cast<FindInterface<Type>::type&>(*this);
				}
				else
				{
					return static_cast<resolved_chain::type&>(*this);
				}
			}
			else
			{
				return static_cast<FindInterface<Type>::type&>(*this);
			}
		}

		template<property Property, typename Type>
		auto& set(Type&& value, bool temporary = false)
		{
			if constexpr (chain_)
			{
				if constexpr (Property < chain_base_)
				{
					static_cast<Chain&>(
						static_cast<Base&>(*this)
					).template set<Property>(
						std::forward<Type>(value),
						temporary
					);
				}
				else
				{			
					_int_set_synced<Property>(std::forward<Type>(value), temporary);
				}
			}
			else
			{
				_int_set_synced<Property>(std::forward<Type>(value), temporary);
			}
			return static_cast<Base&>(*this);
		}
		template<property Property>
		auto get()
		{
			if constexpr (chain_)
			{
				if constexpr (Property < chain_base_)
				{
					return static_cast<Chain&>(
						static_cast<Base&>(*this)
					).template get<Property>();
				}
				else
				{
					return _int_get_synced<Property>();
				}
			}
			else
			{
				return _int_get_synced<Property>();
			}
		}
		template<property Property>
		auto* getref()
		{
			if constexpr (chain_)
			{
				if constexpr (Property < chain_base_)
				{
					return static_cast<Chain&>(
						static_cast<Base&>(*this)
					).template getref<Property>();
				}
				else
				{
					return _int_getref<Property>();
				}
			}
			else
			{
				return _int_getref<Property>();
			}
		}

		template<property Property, typename Func>
		auto apply_get(Func&& func)
		{
			const auto _ = _base()->shared_from_this();
			const auto src = _screen();
			if (src->is_synced())
				return func(*getref<Property>());
			return src->sync([		 
				this,
				func = std::forward<Func>(func)
			] {
				return func(*getref<Property>());
			}).value();
		}
		template<property Property, typename Func>
		auto apply_set(Func&& func)
		{
			const auto _ = _base()->shared_from_this();
			const auto src = _screen();
			if (src->is_synced())
			{
				_var_flags.set(Property - base_offset_, false);
				auto [ var, type ] = _int_get_vals<Property>();
				if constexpr (std::is_same_v<decltype(func(*var)), void>)
				{
					func(*var);
					_signal_base_impl(type, Property);
				}
				else
				{
					auto result = func(*var);
					_signal_base_impl(type, Property);
					return result;
				}
			}
			else
			{
				return src->sync([
					this,
					func = std::forward<Func>(func)
				] {
					_var_flags.set(Property - base_offset_, false);
					auto [ var, type ] = _int_get_vals<Property>();
					if constexpr (std::is_same_v<decltype(func(*var)), void>)
					{
						func(*var);
						_signal_base_impl(type, Property);
					}
					else
					{
						auto result = func(*var);
						_signal_base_impl(type, Property);
						return result;
					}
				}).value();
			}
		}
	};
	template<
		typename Base,
		D2_UAI_INTERFACE_TEMPL Interface,
		D2_UAI_INTERFACE_TEMPL... Interfaces
	>
	using UAI = UniversalAccessInterface<void, Base, Interface, Interfaces...>;
	template<
		typename Chain,
		typename Base,
		D2_UAI_INTERFACE_TEMPL Interface,
		D2_UAI_INTERFACE_TEMPL... Interfaces
	>
	using UAIC = UniversalAccessInterface<Chain, Base, Interface, Interfaces...>;
	using UAIB = UniversalAccessInterfaceBase;

	// Common styles
	// Each interface (beginning with an 'I') has to
	// 1. derive from InterfaceHelper
	// 2. derive from a separate POD that stores the actual data
	// 3. alias the data type
	// 4. define get_style in the POD which maps each style value (abolute, i.e. not aligned to PropBase) to a variable in the struct
	// Refer to practical examples below for more details

	namespace impl
	{
		template<typename Type>
		struct BitFlagPtrWrapper
		{
			Type* ptr{ nullptr };
			Type mask{ 0x00 };

			BitFlagPtrWrapper& operator*() noexcept
			{
				return *this;
			}
			BitFlagPtrWrapper& operator=(bool value) noexcept
			{
				*ptr &= ~mask;
				*ptr |= (mask * value);
				return *this;
			}
		};
		using uchar_bit_ptr = BitFlagPtrWrapper<unsigned char>;

		struct XAllocInterfaceTracker
		{
			static inline std::size_t id{ 0 };
		};
		template<template<std::size_t> typename>
		struct XAllocInterface
		{
			static inline const std::size_t xid{ XAllocInterfaceTracker::id++ };
		};
	}
	namespace
	{
		template<template<std::size_t> typename Type, std::size_t PropBase, std::size_t Count>
		struct InterfaceHelper : impl::XAllocInterface<Type>
		{
			static constexpr std::size_t base = PropBase;
			static constexpr std::size_t offset = PropBase + Count;
			static constexpr std::size_t count = Count;

			template<uai_property Property>
			auto get()
			{
				return static_cast<Type<PropBase>*>(this)
					->template at_style<Property - PropBase>();
			}
		};
	}

	struct Layout
	{
		HPUnit x{ Unit::Auto };
		VPUnit y{ Unit::Auto };
		HDUnit width{ Unit::Auto };
		VDUnit height{ Unit::Auto };
		char zindex{ 0 };

		template<uai_property Property>
		auto at_style()
		{
			D2_UAI_SETUP(Layout)
			D2_UAI_VAR_START
			D2_UAI_GET_VAR_A(0, x)
			D2_UAI_GET_VAR_A(1, y)
			D2_UAI_GET_VAR_MUL(2, width, Element::WriteType::Masked)
			D2_UAI_GET_VAR_MUL(3, height, Element::WriteType::Masked)
			D2_UAI_GET_VAR_A(4, zindex)
			D2_UAI_VAR_END;
		}
	};

	template<std::size_t PropBase>
	struct ILayout : Layout, InterfaceHelper<ILayout, PropBase, 5>
	{
		using data = Layout;
		enum Property : uai_property
		{
			X = PropBase,
			Y,
			Width,
			Height,
			ZIndex
		};
	};
	using IZLayout = ILayout<0>;

	struct Container
	{
		enum ContainerOptions : unsigned char
		{
			EnableBorder = 1 << 0,
			OverrideCorners = 1 << 1,

			DisableBorderLeft = 1 << 2,
			DisableBorderRight = 1 << 3,
			DisableBorderTop = 1 << 4,
			DisableBorderBottom = 1 << 5,

			TopFix = 1 << 6,
			DisableAutoLayout = 1 << 7,
		};

		HDUnit border_width{ 1 };
		Pixel border_horizontal_color{ .a = 0, .v = '_' };
		Pixel border_vertical_color{ .a = 0, .v = '|'};
		struct
		{
			Pixel::value_type top_left{};
			Pixel::value_type top_right{};
			Pixel::value_type bottom_left{};
			Pixel::value_type bottom_right{};
		} corners;
		unsigned char container_options{ 0x00 };

		void _setopt(ContainerOptions opt, bool value)
		{
			container_options &= ~opt;
			container_options |= (opt * value);
		}

		template<uai_property Property>
		auto at_style()
		{
			constexpr auto snl = Element::WriteType::Masked;
			D2_UAI_SETUP_EMPTY
			D2_UAI_VAR_START
			D2_UAI_GET_VAR(0, border_width, Style)
			D2_UAI_GET_VAR(1, border_horizontal_color, Style)
			D2_UAI_GET_VAR(2, border_vertical_color, Style)
			D2_UAI_GET_VAR(3, corners.top_left, Style)
			D2_UAI_GET_VAR(4, corners.top_right, Style)
			D2_UAI_GET_VAR(5, corners.bottom_left, Style)
			D2_UAI_GET_VAR(6, corners.bottom_right, Style)
			D2_UAI_GET_VAR_MUL(7, container_options, snl)
			D2_UAI_GET_VAR_COMPUTED_MUL(8, snl, impl::uchar_bit_ptr(&container_options, EnableBorder))
			D2_UAI_GET_VAR_COMPUTED_MUL(9, snl, impl::uchar_bit_ptr(&container_options, DisableBorderLeft))
			D2_UAI_GET_VAR_COMPUTED_MUL(10, snl, impl::uchar_bit_ptr(&container_options, DisableBorderRight))
			D2_UAI_GET_VAR_COMPUTED_MUL(11, snl, impl::uchar_bit_ptr(&container_options, DisableBorderTop))
			D2_UAI_GET_VAR_COMPUTED_MUL(12, snl, impl::uchar_bit_ptr(&container_options, DisableBorderBottom))
			D2_UAI_GET_VAR_COMPUTED_MUL(13, snl, impl::uchar_bit_ptr(&container_options, DisableAutoLayout))
			D2_UAI_GET_VAR_COMPUTED(14, Style, impl::uchar_bit_ptr(&container_options, TopFix))
			D2_UAI_GET_VAR_COMPUTED(15, Style, impl::uchar_bit_ptr(&container_options, OverrideCorners))
			D2_UAI_VAR_END;
		}
	};

	template<std::size_t PropBase>
	struct IContainer : Container, InterfaceHelper<IContainer, PropBase, 16>
	{
		using data = Container;
		enum Property : uai_property
		{
			BorderWidth = PropBase,
			BorderHorizontalColor,
			BorderVerticalColor,
			// Corner options
			CornerTopLeft,
			CornerTopRight,
			CornerBottomLeft,
			CornerBottomRight,
			ContainerOptions,
			// Expanded options
			ContainerBorder,
			ContainerDisableLeft,
			ContainerDisableRight,
			ContainerDisableTop,
			ContainerDisableBottom,
			// Other flags
			ContainerAutoLayout,
			ContainerTopFix,
			ContainerCorners
		};
	};
	using IZContainer = IContainer<0>;

	struct Colors
	{
		PixelBackground background_color{ .a = 0 };
		PixelForeground foreground_color{};

		template<uai_property Property>
		auto at_style()
		{
			D2_UAI_SETUP_EMPTY
			D2_UAI_VAR_START
			D2_UAI_GET_VAR(0, background_color, Style)
			D2_UAI_GET_VAR(1, foreground_color, Style)
			D2_UAI_VAR_END;
		}
	};

	template<std::size_t PropBase>
	struct IColors : Colors, InterfaceHelper<IColors, PropBase, 2>
	{
		using data = Colors;
		enum Property : uai_property
		{
			BackgroundColor = PropBase,
			ForegroundColor,
		};
	};
	using IZColors = IColors<0>;

	struct KeyboardNav
	{
		PixelBackground focused_color{
			.r = 150, .g = 150, .b = 150
		};

		template<uai_property Property>
		auto at_style()
		{
			D2_UAI_SETUP(Style)
			D2_UAI_VAR_START
			D2_UAI_GET_VAR_A(0, focused_color)
			D2_UAI_VAR_END;
		}
	};

	template<std::size_t PropBase>
	struct IKeyboardNav : KeyboardNav, InterfaceHelper<IKeyboardNav, PropBase, 1>
	{
		using data = KeyboardNav;
		enum Property : uai_property
		{
			FocusedColor = PropBase,
		};
	};
	using IZKeyboardNav = IKeyboardNav<0>;

	struct Text
	{
		enum class Alignment
		{
			Left,
			Center,
			Right,
		};
		enum TextOption
		{
			PreserveWordBoundaries = 1 << 0,
			HandleNewlines = 1 << 1,
			Paragraph = PreserveWordBoundaries | HandleNewlines
		};

		string text{ "" };
		Alignment alignment{};
		unsigned char text_options{ 0x00 };

		template<uai_property Property>
		auto at_style()
		{
			D2_UAI_SETUP_MUL(Element::WriteType::Layout | Element::WriteType::Style)
			D2_UAI_VAR_START
			D2_UAI_GET_VAR_A(0, text)
			D2_UAI_GET_VAR_A(1, alignment)
			D2_UAI_GET_VAR_A(2, text_options)
			D2_UAI_GET_VAR_COMPUTED_A(3, impl::uchar_bit_ptr(&text_options, PreserveWordBoundaries))
			D2_UAI_GET_VAR_COMPUTED_A(4, impl::uchar_bit_ptr(&text_options, HandleNewlines))
			D2_UAI_VAR_END;
		}
	};

	template<std::size_t PropBase>
	struct IText : Text, InterfaceHelper<IText, PropBase, 5>
	{
		using data = Layout;
		enum Property : uai_property
		{
			Value = PropBase,
			TextAlignment,
			TextOptions,
			TextPreserveWordBoundaries,
			TextHandleNewlines
		};
	};
	using IZText = IText<0>;

	struct Responsive
	{
		std::function<void(Element::TraversalWrapper)> on_submit{};

		template<uai_property Property>
		auto at_style()
		{
			D2_UAI_SETUP_EMPTY
			D2_UAI_VAR_START
			D2_UAI_GET_VAR_A(0, on_submit)
			D2_UAI_VAR_END;
		}
	};

	template<std::size_t PropBase>
	struct IResponsive : Responsive, InterfaceHelper<IResponsive, PropBase, 1>
	{
		using data = Responsive;
		enum Property : uai_property
		{
			OnSubmit = PropBase,
		};
	};
	using IZResponsive = IResponsive<0>;
}

#endif // D2_STYLES_HPP
