#ifndef D2_BOX_HPP
#define D2_BOX_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2::dx
{	
	class Box :
		public style::UAIC<VecParentElement, Box, style::ILayout, style::IContainer, style::IColors>,
		public impl::UnitUpdateHelper<Box>,
		public impl::ContainerHelper<Box>
	{
	public:
		static constexpr auto overlap = 6;
		friend class UnitUpdateHelper;
		friend class ContainerHelper;
		using data = style::UAIC<VecParentElement, Box, style::ILayout, style::IContainer, style::IColors>;
		using data::data;
	protected:
		void _recompute_layout(int basex = 0, int basey = 0) const noexcept;
		bool _is_automatic_layout() const noexcept;

		virtual Position _position_for(cptr elem) const override;
		virtual BoundingBox _dimensions_for(cptr elem) const override;

		virtual bool _provides_layout_impl(cptr elem) const noexcept override;
		virtual bool _provides_dimensions_impl(cptr elem) const noexcept override;

		virtual int _resolve_units_impl(Unit value, cptr elem) const noexcept override;
		virtual int _get_border_impl(BorderType type, cptr elem) const noexcept override;
		virtual char _index_impl() const noexcept override;
		virtual unit_meta_flag _unit_report_impl() const noexcept override;

		virtual BoundingBox _dimensions_impl() const noexcept override;
		virtual Position _position_impl() const noexcept override;

		virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;

		// Fleshbox protocol

		virtual void _fleshbox_render_start() {}
		virtual void _fleshbox_object_render(PixelBuffer::View buffer, ptr obj);
	};
}

#endif // D2_BOX_HPP
