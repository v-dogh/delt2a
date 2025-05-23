#ifndef D2_SCROLLBOX_HPP
#define D2_SCROLLBOX_HPP

#include "d2_box.hpp"
#include "d2_slider.hpp"

namespace d2::dx
{
	class ScrollBox : public Box
	{
	protected:
		std::shared_ptr<VerticalSlider> scrollbar_{ nullptr };
		int offset_{ 0 };
		int computed_height_{ 0 };

		void _recompute_height() noexcept;

		virtual int _get_border_impl(BorderType type, Element::cptr elem) const noexcept override;
		virtual bool _provides_layout_impl(cptr elem) const noexcept override;
		virtual Position _position_for(cptr elem) const override;

		virtual void _state_change_impl(State state, bool value) override;

		virtual void _update_layout_impl() noexcept override;
	public:
		using Box::Box;

		TraversalWrapper scrollbar() const noexcept;

		virtual void foreach_internal(foreach_internal_callback callback) const override;
	};
}

#endif // D2_SCROLLBOX_HPP
