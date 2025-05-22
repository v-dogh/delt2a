#ifndef D2_COLOR_PICKER_HPP
#define D2_COLOR_PICKER_HPP

#include "../elements/d2_std.hpp"

namespace d2
{
	namespace style
	{
		struct ColorPicker
		{
			std::function<void(Element::TraversalWrapper, px::background)> on_submit{ nullptr };

			template<uai_property Property>
			auto at_style();
		};

		template<std::size_t PropBase>
		struct IColorPicker : ColorPicker, InterfaceHelper<IColorPicker, PropBase, 1>
		{
			using data = ColorPicker;
			enum Property : uai_property
			{
				OnSubmit = PropBase,
			};
		};
		using IZColorPicker = IColorPicker<0>;
	}
	namespace ctm
	{
		class ColorPicker : public style::UAIC<dx::VirtualBox, ColorPicker, style::IColorPicker>
		{
		public:
			using data = style::UAIC<VirtualBox, ColorPicker, style::IColorPicker>;
			using data::data;
			D2_UAI_CHAIN(VirtualBox)
		protected:
			static eptr<ColorPicker> _core(TreeState::ptr state);

			virtual void _state_change_impl(State state, bool value) override;

			px::background _get_color() const noexcept;
		public:
			void submit();
		};
	}
}

#endif // D2_COLOR_PICKER_HPP
