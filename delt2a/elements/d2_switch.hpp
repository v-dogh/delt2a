#ifndef D2_SWITCH_HPP
#define D2_SWITCH_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
	namespace style
	{
		struct Switch
		{
			std::function<void(Element::TraversalWrapper, int)> on_change{ nullptr };
			std::vector<string> options{};

			PixelBackground enabled_background_color{};
			PixelForeground enabled_foreground_color{};
			PixelBackground disabled_background_color{
				.r = 255, .g = 255, .b = 255
			};
			PixelForeground disabled_foreground_color{
				.r = 0, .g = 0, .b = 0
			};
			Pixel separator_color{ .a = 0, .v = '|' };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP_EMPTY
				D2_UAI_VAR_START
				D2_UAI_GET_VAR(0, options, Masked)
				D2_UAI_GET_VAR(1, enabled_foreground_color, Style)
				D2_UAI_GET_VAR(2, enabled_background_color, Style)
				D2_UAI_GET_VAR(3, disabled_foreground_color, Style)
				D2_UAI_GET_VAR(4, disabled_background_color, Style)
				D2_UAI_GET_VAR_A(5, on_change)
				D2_UAI_GET_VAR(6, separator_color, Style)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct ISwitch : Switch, InterfaceHelper<ISwitch, PropBase, 7>
		{
			using data = Switch;
			enum Property : uai_property
			{
				Options = PropBase,
				EnabledForegroundColor,
				EnabledBackgroundColor,
				DisabledForegroundColor,
				DisabledBackgroundColor,
				OnChange,
				SeparatorColor
			};
		};
		using IZSwitch = ISwitch<0>;
	}
	namespace dx
	{
		class Switch :
			public style::UAI<
				Switch,
				style::ISwitch,
				style::ILayout,
				style::IContainer,
				style::IColors,
				style::IKeyboardNav
			>,
			public impl::UnitUpdateHelper<Switch>,
			public impl::ContainerHelper<Switch>,
			public impl::TextHelper<Switch>
		{
		public:
			friend class UnitUpdateHelper;
			friend class ContainerHelper;
			friend class TextHelper;
			using opts = std::vector<string>;
			using data = style::UAI<
				Switch,
				style::ISwitch,
				style::ILayout,
				style::IContainer,
				style::IColors,
				style::IKeyboardNav
			>;
			using data::data;
		protected:
			int _idx{ 0 };

			virtual char _index_impl() const noexcept override;
			virtual unit_meta_flag _unit_report_impl() const noexcept override;
			virtual bool _provides_input_impl() const noexcept override;

			virtual BoundingBox _dimensions_impl() const noexcept override;
			virtual Position _position_impl() const noexcept override;

			virtual void _state_change_impl(State state, bool value) override;
			virtual void _event_impl(IOContext::Event ev) override;

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;

			void _submit();
		public:
			void reset(int idx) noexcept;
		};
	}
}

#endif // D2_SWITCH_HPP
