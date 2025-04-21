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

		void _recompute_height() noexcept
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

		virtual int _get_border_impl(BorderType type, Element::cptr elem) const noexcept override
		{
			if (elem != scrollbar_ && type == BorderType::Right)
			{
				const auto sw = scrollbar_->box().width;
				return Box::_get_border_impl(type, elem) + sw;
			}
			return Box::_get_border_impl(type, elem);
		}
		virtual bool _provides_layout_impl(cptr elem) const noexcept override
		{
			return elem != scrollbar_;
		}
		virtual Position _position_for(cptr elem) const override
		{
			if (Box::_provides_layout_impl(elem))
			{
				_recompute_layout(0, -offset_);
				return elem->position();
			}
			const auto [ x, y ] = elem->internal_position();
			return { x, y - offset_ };
		}

		virtual void _state_change_impl(State state, bool value) override
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

		virtual void _update_layout_impl() noexcept override
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
	public:
		using Box::Box;

		TraversalWrapper scrollbar() const noexcept
		{
			return scrollbar_;
		}

		virtual void foreach_internal(foreach_internal_callback callback) const override
		{
			for (decltype(auto) it : _elements)
				callback(it);
			callback(scrollbar_);
		}
	};
}

#endif // D2_SCROLLBOX_HPP
