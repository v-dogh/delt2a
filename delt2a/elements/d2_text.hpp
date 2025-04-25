#ifndef D2_TEXT_HPP
#define D2_TEXT_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2::dx
{
	class Text :
		public style::UAI<Text, style::ILayout, style::IText, style::IColors>,
		public impl::UnitUpdateHelper<Text>,
		public impl::TextHelper<Text>
	{
	public:
		friend class UnitUpdateHelper;
		friend class TextHelper;
		using data = style::UAI<Text, style::ILayout, style::IText, style::IColors>;
	protected:
		virtual BoundingBox _dimensions_impl() const noexcept override
		{
			int width = 0;
			int height = 0;
			if (data::width.getunits() == Unit::Auto ||
				data::height.getunits() == Unit::Auto)
			{
				if (data::text_options & HandleNewlines)
				{
					if (data::text_options & PreserveWordBoundaries)
					{
						const auto [ bwidth, bheight ] =
							TextHelper::_paragraph_bounding_box(data::text,
							(data::width.getunits() == Unit::Auto ? INT_MAX : _resolve_units(data::width)));
						width = bwidth;
						height = (data::height.getunits() == Unit::Auto) ?
							bheight : _resolve_units(data::height);
					}
					else
					{
						const auto [ bwidth, bheight ] =
							TextHelper::_text_bounding_box(data::text);
						width = (data::width.getunits() == Unit::Auto) ?
							bwidth : _resolve_units(data::width);
						height = (data::height.getunits() == Unit::Auto) ?
							bheight : _resolve_units(data::height);
					}
				}
				else
				{
					width = (data::width.getunits() == Unit::Auto) ?
						data::text.size() : _resolve_units(data::width);
					height = (data::height.getunits() == Unit::Auto) ?
						1 : _resolve_units(data::height);
				}
			}
			else
			{
				width = _resolve_units(data::width);
				height = _resolve_units(data::height);
			}
			return { width, height };
		}
		virtual Position _position_impl() const noexcept override
		{
			return {
				_resolve_units(data::x),
				_resolve_units(data::y)
			};
		}

		virtual unit_meta_flag _unit_report_impl() const noexcept override
		{
			return UnitUpdateHelper::_report_units();
		}
		virtual char _index_impl() const noexcept override
		{
			return data::zindex;
		}

		virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
		{
			buffer.fill(data::background_color);
			if (data::text_options & HandleNewlines)
			{
				if (data::text_options & PreserveWordBoundaries)
				{
					TextHelper::_render_paragraph(
						data::text,
						Pixel::combine(data::foreground_color, data::background_color),
						data::alignment,
						{}, box(),
						buffer
					);
				}
				else
				{
					TextHelper::_render_text(
						data::text,
						Pixel::combine(data::foreground_color, data::background_color),
						data::alignment,
						{}, box(),
						buffer
					);
				}
			}
			else
			{
				TextHelper::_render_text_simple(
					data::text,
					Pixel::combine(data::foreground_color, data::background_color),
					{}, box(),
					buffer
				);
			}
		}
	public:
		Text(
			const std::string& name,
			TreeState::ptr state,
			ptr parent
		) : data(name, state, parent)
		{
			data::background_color.a = 0;
		}
	};
} // d2::elem

#endif // D2_TEXT_HPP
