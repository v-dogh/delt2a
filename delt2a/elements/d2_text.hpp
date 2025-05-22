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
		virtual BoundingBox _dimensions_impl() const noexcept override;
		virtual Position _position_impl() const noexcept override;

		virtual unit_meta_flag _unit_report_impl() const noexcept override;
		virtual char _index_impl() const noexcept override;

		virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
	public:
		Text(
			const std::string& name,
			TreeState::ptr state,
			ptr parent
		);
	};
} // d2::elem

#endif // D2_TEXT_HPP
