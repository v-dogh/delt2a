#ifndef D2_DRAGGABLE_BOX_HPP
#define D2_DRAGGABLE_BOX_HPP

#include "d2_box.hpp"

namespace d2
{
	namespace style
	{
		struct DraggableBox
		{
			enum VBoxOptions : unsigned char
			{
				Resizable = 1 << 0,
				Draggable = 1 << 1,
				Minimizable = 1 << 2,

				Auto = Resizable | Draggable | Minimizable,
				Freezed = 0x00,
				Static = Draggable,
			};

			string title{ "" };
			PixelForeground bar_color{ .v = '.' };
			Pixel::value_type resize_point_value{ ',' };
			VDUnit bar_height{ 2.0_px };
			unsigned char vbox_options{ Auto };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP(Style)
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, title)
				D2_UAI_GET_VAR_A(1, bar_color)
				D2_UAI_GET_VAR_MUL(2, bar_height, Element::WriteType::Masked)
				D2_UAI_GET_VAR_A(3, bar_height)
				D2_UAI_GET_VAR_A(4, vbox_options)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct IDraggableBox : DraggableBox, InterfaceHelper<IDraggableBox, PropBase, 5>
		{
			using data = DraggableBox;
			enum Property : uai_property
			{
				Title = PropBase,
				BarColor,
				BarHeight,
				ResizePointValue,
				VBoxOptions
			};
		};
	}
	namespace dx
	{
		class VirtualBox : public style::UAIC<Box, VirtualBox, style::IDraggableBox, style::IKeyboardNav>
		{
		public:
			using data = style::UAIC<Box, VirtualBox, style::IDraggableBox, style::IKeyboardNav>;
			using data::data;
			D2_UAI_CHAIN(Box)
		protected:
			std::pair<int, int> offset_{ 0, 0 };
			bool minimized_{ false };

			bool _is_being_resized() const noexcept
			{
				const auto [ width, height ] = box();
				return
					(data::vbox_options & VBoxOptions::Resizable) &&
					offset_.first == width - 1 && offset_.second == 0;
			}

			virtual int _get_border_impl(BorderType type, Element::cptr elem) const noexcept override
			{
				if (type == BorderType::Top)
					return _resolve_units(data::bar_height);
				return Box::_get_border_impl(type, elem);
			}
			virtual int _resolve_units_impl(Unit value, Element::cptr elem) const noexcept override
			{
				if (minimized_)
					return INT_MAX;
				return Box::_resolve_units_impl(value, elem);
			}
			virtual BoundingBox _dimensions_impl() const noexcept override
			{
				if (minimized_)
				{
					return {
						Box::_dimensions_impl().width,
						_resolve_units(data::bar_height)
					};
				}
				return Box::_dimensions_impl();
			}
			virtual bool _provides_input_impl() const noexcept override
			{
				return true;
			}

			virtual void _state_change_impl(State state, bool value) override
			{
				Box::_state_change_impl(state, value);
				if (state == State::Clicked)
				{
					if (value)
					{
						const auto [ x, y ] = mouse_object_space();
						offset_.first = x;
						offset_.second = y;
					}
					else if (!_is_being_resized())
					{
						offset_ = { 0, 0 };
					}
				}
				else if (state == State::Keynavi)
				{
					_signal_write(Style);
				}
			}
			virtual void _event_impl(IOContext::Event ev) override
			{
				Box::_event_impl(ev);
				if (ev == IOContext::Event::MouseInput)
				{
					const auto rlc = context()->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Left, sys::SystemInput::KeyMode::Release);
					const auto rc = context()->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Right, sys::SystemInput::KeyMode::Press);
					const auto lc = context()->input()->is_pressed_mouse(sys::SystemInput::MouseKey::Left);

					if (getstate(Hovered) && !_is_being_resized())
					{
						if (getstate(Clicked) && lc && (data::vbox_options & VBoxOptions::Draggable))
						{
							const auto [ x, y ] = context()->input()->mouse_position();
							const auto nx = std::max(x - offset_.first, 0);
							const auto ny = std::max(y - offset_.second, 0);
							if (nx != _resolve_units(Box::x) ||
								ny != _resolve_units(Box::y))
							{
								Box::x = nx;
								Box::y = ny;
								_signal_write(WriteType::Layout);
							}
						}
						if (rc && (data::vbox_options & VBoxOptions::Minimizable))
						{
							minimized_ = !minimized_;
							_signal_write(WriteType::Masked);
						}
					}
					else if (rlc && _is_being_resized())
					{
						const auto [ x, y ] = mouse_object_space();
						const auto yoff = _resolve_units(Box::height) - y;
						const auto bh = _resolve_units(data::bar_height);
						if (x > 0 && (yoff - bh) > 0 &&
							x != _resolve_units(Box::width) &&
							yoff != _resolve_units(Box::height))
						{
							Box::width = x;
							Box::height = yoff;
							Box::y = y + _resolve_units(Box::y);
							_signal_write(WriteType::Masked);
						}
						offset_ = { 0, 0 };
					}
				}
				else if (ev == IOContext::Event::KeyInput)
				{
					const auto left = context()->input()->is_pressed(sys::SystemInput::key('h'));
					const auto right = context()->input()->is_pressed(sys::SystemInput::key('l'));
					const auto down = context()->input()->is_pressed(sys::SystemInput::key('j'));
					const auto up = context()->input()->is_pressed(sys::SystemInput::key('k'));

					if ((data::vbox_options & data::Resizable) &&
						context()->input()->is_pressed(sys::SystemInput::Shift))
					{
						if (left || right)
						{
							const auto width = _resolve_units(Box::width);
							if (width > 1)
							{
								Box::width = width + (left ? -1 : 1);
								_signal_write(WriteType::Masked);
							}
						}
						if (down || up)
						{
							const auto height = _resolve_units(Box::height);
							if (height > 1)
							{
								Box::height = height + (up ? -1 : 1);
								_signal_write(WriteType::Masked);
							}
						}
					}
					else if (data::vbox_options & data::Draggable)
					{
						if (left || right)
						{
							const auto x = _resolve_units(Box::x);
							if (x > 1)
							{
								Box::x = x + (left ? -1 : 1);
								_signal_write(WriteType::Layout);
							}
						}
						if (down || up)
						{
							const auto y = _resolve_units(Box::y);
							if (y > 1)
							{
								Box::y = y + (up ? -1 : 1);
								_signal_write(WriteType::Layout);
							}
						}
					}
				}
			}

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
			{
				if (!minimized_)
					Box::_frame_impl(buffer);

				const auto bh = _resolve_units(data::bar_height);
				buffer.fill(
					getstate(Keynavi) ?
						data::focused_color.extend(data::bar_color.v) :
						data::bar_color,
					0, 0, buffer.width(), bh
				);

				const auto s = (buffer.width() < data::title.size()) ?
					0 : (buffer.width() - data::title.size()) / 2;
				const auto w = std::min(int(data::title.size()), buffer.width());
				const auto p = bh / 2;
				for (std::size_t i = 0; i < w; i++)
				{
					auto& px = buffer.at(i + s, p);
					px.v = data::title[i];
				}

				if (data::vbox_options & VBoxOptions::Resizable)
				{
					buffer.at(buffer.width() - 1).v =
						data::resize_point_value;
				}
			}
		public:
			void close() noexcept
			{
				parent()
					->traverse().asp()
					->remove(shared_from_this());
			}
		};
	}
}

#endif // D2_DRAGGABLE_BOX_HPP
