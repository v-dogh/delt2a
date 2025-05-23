#include "d2_scrollbox.hpp"

namespace d2::dx
{
	void ScrollBox::_recompute_height() noexcept
	{
		const auto cbox = box();
		computed_height_ = cbox.height;
		foreach_internal([this](TraversalWrapper it) {
			if (it != scrollbar_ && it->getstate(Display))
			{
				const auto bbox = it->box();
				const auto pos = it->position();
				computed_height_ =
					std::max(computed_height_, (pos.y + offset_) + bbox.height);
			}
		});
	}

	int ScrollBox::_get_border_impl(BorderType type, Element::cptr elem) const noexcept
	{
		if (elem != scrollbar_ && type == BorderType::Right)
		{
			const auto sw = scrollbar_->box().width;
			return Box::_get_border_impl(type, elem) + sw;
		}
		return Box::_get_border_impl(type, elem);
	}
	bool ScrollBox::_provides_layout_impl(cptr elem) const noexcept
	{
		return elem != scrollbar_;
	}
	ScrollBox::Position ScrollBox::_position_for(cptr elem) const
	{
		if (Box::_provides_layout_impl(elem))
		{
			_recompute_layout(0, -offset_);
			return elem->position();
		}
		const auto [ x, y ] = elem->internal_position();
		return { x, y - offset_ };
	}

	void ScrollBox::_state_change_impl(State state, bool value)
	{
		if (state == State::Swapped && !value)
		{
			if (scrollbar_ == nullptr)
			{
				scrollbar_ = std::make_shared<VerticalSlider>(
					"", this->state(), shared_from_this()
				);
				scrollbar_->setstate(State::Swapped, false);

				scrollbar_->zindex = 5;
				scrollbar_->x = 0.0_pxi;
				scrollbar_->y = 0.0_center;
				scrollbar_->height = 1.0_pc;
				scrollbar_->slider_background_color = PixelForeground{ .v = '|' };
				scrollbar_->on_change = [this](auto, auto, auto abs) {
					offset_ = abs;
					_signal_write(WriteType::Masked);
				};

				scrollbar_->_signal_write(WriteType::Masked);
			}
		}
	}

	void ScrollBox::_update_layout_impl() noexcept
	{
		const auto bbox = box();
		const auto bw = (Box::container_options & Box::EnableBorder) ?
			_resolve_units(Box::border_width) : 0;

		_recompute_height();

		scrollbar_->slider_width =
			computed_height_ ? ((float(bbox.height) / computed_height_) * bbox.height) : 0;
		scrollbar_->min = 0;
		scrollbar_->max = (computed_height_ - bbox.height) + bw;
		scrollbar_->_signal_write(WriteType::Style);
	}

	ScrollBox::TraversalWrapper ScrollBox::scrollbar() const noexcept
	{
		return scrollbar_;
	}

	void ScrollBox::foreach_internal(foreach_internal_callback callback) const
	{
		for (decltype(auto) it : _elements)
			callback(it);
		callback(scrollbar_);
	}
}
