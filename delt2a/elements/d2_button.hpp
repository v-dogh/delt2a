#ifndef D2_BUTTON_HPP
#define D2_BUTTON_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2::dx
{
	class Button :
		public style::UAI<
			Button,
			style::ILayout,
			style::IContainer,
			style::IColors,
			style::IResponsive,
			style::IText,
			style::IKeyboardNav
		>,
		public impl::UnitUpdateHelper<Button>,
		public impl::TextHelper<Button>,
		public impl::ContainerHelper<Button>
	{
	public:
		friend class ContainerHelper;
		friend class TextHelper;
		friend class UnitUpdateHelper;
		using data = style::UAI<
			Button,
			style::ILayout,
			style::IContainer,
			style::IColors,
			style::IResponsive,
			style::IText,
			style::IKeyboardNav
		>;
		using data::data;
	protected:
		virtual char _index_impl() const noexcept override
		{
			return data::zindex;
		}
		virtual unit_meta_flag _unit_report_impl() const noexcept override
		{
			return UnitUpdateHelper::_report_units();
		}
		virtual bool _provides_input_impl() const noexcept override
		{
			return true;
		}

		virtual BoundingBox _dimensions_impl() const noexcept override
		{
			if (data::width.getunits() == Unit::Auto || data::height.getunits() == Unit::Auto)
			{
				const auto bw = (data::container_options & ContainerOptions::EnableBorder) ?
					_resolve_units(data::border_width) : 0;
				auto tbox = TextHelper::_text_bounding_box(data::text);
				tbox.width += (bw * 2);
				tbox.height += (bw * 2);

				if (data::width.getunits() != Unit::Auto) tbox.width = _resolve_units(data::width);
				if (data::height.getunits() != Unit::Auto) tbox.height = _resolve_units(data::height);

				return tbox;
			}
			else
			{
				return {
					_resolve_units(data::width),
					_resolve_units(data::height)
				};
			}
		}
		virtual Position _position_impl() const noexcept override
		{
			return {
				_resolve_units(data::x),
				_resolve_units(data::y)
			};
		}

		virtual void _state_change_impl(State state, bool value) override
		{
			if (state == State::Clicked && value)
			{
				if (data::on_submit != nullptr)
					data::on_submit(shared_from_this());
			}
			else if (state == State::Keynavi)
			{
				_signal_write(Style);
			}
		}
		virtual void _event_impl(IOContext::Event ev) override
		{
			if (getstate(State::Keynavi) &&
				ev == IOContext::Event::KeyInput &&
				context()->input()->is_pressed(sys::SystemInput::Enter, sys::SystemInput::KeyMode::Press))
			{
				if (data::on_submit != nullptr)
					data::on_submit(shared_from_this());
			}
		}

		virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
		{
			buffer.fill(
				Pixel::combine(
					data::foreground_color,
					getstate(State::Keynavi) ? data::focused_color : data::background_color
				)
			);

			const auto bbox = box();
			if (data::container_options & data::EnableBorder)
				ContainerHelper::_render_border(buffer);
			TextHelper::_render_text(
				data::text,
				data::foreground_color,
				style::Text::Alignment::Center,
				{}, bbox,
				buffer
			);
		}
	};
} // d2::elem

#endif // D2_BUTTON_HPP
