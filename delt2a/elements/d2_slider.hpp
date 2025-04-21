#ifndef D2_SLIDER_HPP
#define D2_SLIDER_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
	namespace style
	{
		struct Slider
		{
			float min{ 0 };
			float max{ 0 };
			PixelForeground slider_background_color{ .v = '-' };
			Pixel slider_color{ .r = 255, .g = 255, .b = 255, .v = ' ' };
			HDUnit slider_width{ 1 };
			std::function<void(Element::TraversalWrapper, float, float)> on_change{};

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP(Style)
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, max)
				D2_UAI_GET_VAR_A(1, min)
				D2_UAI_GET_VAR_A(2, slider_background_color)
				D2_UAI_GET_VAR_A(3, slider_color)
				D2_UAI_GET_VAR_A(4, slider_width)
				D2_UAI_GET_VAR(5, on_change, None)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct ISlider : Slider, InterfaceHelper<ISlider, PropBase, 6>
		{
			using data = Slider;
			enum Property : uai_property
			{
				Max = PropBase,
				Min,
				BackgroundColor,
				SliderColor,
				SliderWidth,
				OnChange,
			};
		};
		using IZSlider = ISlider<0>;
	}
	namespace dx
	{
		class Slider :
			public Element,
			public style::UAI<Slider, style::ILayout, style::ISlider, style::IKeyboardNav>,
			public impl::UnitUpdateHelper<Slider>
		{
		public:
			friend class UnitUpdateHelper;
			using data = style::UAI<Slider, style::ILayout, style::ISlider, style::IKeyboardNav>;
		protected:
			int slider_spos_{ 0 };
			int slider_pos_{ 0 };

			void _submit()
			{
				const auto rel = _relative_value();
				const auto abs = _absolute_value();
				if (data::on_change != nullptr)
					data::on_change(shared_from_this(), rel, abs);
			}

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
				BoundingBox bbox;
				if (data::width.getunits() == Unit::Auto) bbox.width = 1;
				else bbox.width = _resolve_units(data::width);
				if (data::height.getunits() == Unit::Auto) bbox.height = 1;
				else bbox.height = _resolve_units(data::height);
				return bbox;
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
				if (state == State::Clicked)
				{
					if (value)
					{
						const auto [ x, y ] = mouse_object_space();
						const auto sw = _resolve_units(data::slider_width);
						if (x >= slider_pos_ && x < slider_pos_ + sw)
						{
							slider_spos_ = x - slider_pos_;
						}
					}
					else
					{
						slider_spos_ = 0;
					}
				}
				else if (state == State::Keynavi)
				{
					_signal_write(Style);
				}
			}
			virtual void _event_impl(IOContext::Event ev) override
			{
				if (ev == IOContext::Event::MouseInput)
				{
					bool write = false;
					int npos = 0;
					if (getstate(Clicked))
					{
						const auto bbox = box();
						const auto [ x, y ] = mouse_object_space();
						const auto sw = _resolve_units(data::slider_width);
						npos = std::max(0, std::min(
							int(bbox.width - sw),
							int(x + slider_spos_)
						));
						write = true;
					}
					else if (getstate(Hovered))
					{
						const auto bbox = box();
						if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollUp))
						{
							const auto sw = _resolve_units(data::slider_width);
							if (sw < bbox.width && slider_pos_ < bbox.width - sw)
							{
								npos = slider_pos_ + 1;
								write = true;
							}
						}
						else if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollDown))
						{
							if (slider_pos_)
							{
								npos = slider_pos_ - 1;
								write = true;
							}
						}
					}

					if (write && npos != slider_pos_)
					{
						slider_pos_ = npos;
						_signal_write(WriteType::Style);
						_submit();
					}
				}
				else if (ev == IOContext::Event::KeyInput)
				{
					if (context()->input()->is_pressed(sys::SystemInput::ArrowLeft) ||
						context()->input()->is_pressed(sys::SystemInput::ArrowDown))
					{
						if (slider_pos_ > 0)
						{
							slider_pos_--;
							_signal_write(Style);
							_submit();
						}
					}
					else if (context()->input()->is_pressed(sys::SystemInput::ArrowRight) ||
							 context()->input()->is_pressed(sys::SystemInput::ArrowUp))
					{
						if (slider_pos_ < box().width - 1)
						{
							slider_pos_++;
							_signal_write(Style);
							_submit();
						}
					}
				}
			}

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
			{
				buffer.fill(data::slider_background_color);

				const auto sw = _resolve_units(data::slider_width);
				auto sc = getstate(State::Keynavi) ?
					px::combined(data::focused_color) : data::slider_color;
				sc.v = data::slider_color.v;
				for (std::size_t i = 0; i < sw; i++)
				{
					if (slider_pos_ + i >= buffer.width()) break;
					buffer.at(
						slider_pos_ + i
					) = sc;
				}
			}

			virtual float _absolute_value()
			{
				return std::clamp(
					data::min + (_relative_value() * (data::max - data::min)),
					data::min,
					data::max
				);
			}
			virtual float _relative_value()
			{
				const auto bbox = box();
				const auto sw = _resolve_units(data::slider_width);
				return std::clamp(float(slider_pos_) / (bbox.width - sw - 1), 0.f, 1.f);
			}
		public:
			using Element::Element;

			virtual void reset_absolute(int value = 0) noexcept
			{
				slider_pos_ = (value / (data::max - data::min)) * box().width;
				_submit();
				_signal_write(WriteType::Style);
			}
			virtual void reset_relative(float value = 0) noexcept
			{
				slider_pos_ = box().width * value;
				_submit();
				_signal_write(WriteType::Style);
			}

			float absolute_value() noexcept
			{
				return _absolute_value();
			}
			float relative_value() noexcept
			{
				return std::clamp(
					data::min + (_absolute_value() * (data::max - data::min)),
					data::min,
					data::max
				);
			}
		};

		class VerticalSlider : public Slider
		{
		protected:
			virtual void _state_change_impl(State state, bool value) override
			{
				if (state == State::Clicked)
				{
					if (value)
					{
						const auto [ x, y ] = mouse_object_space();
						const auto sw = _resolve_units(data::slider_width);
						if (y >= slider_pos_ && y < slider_pos_ + sw)
						{
							slider_spos_ = y - slider_pos_;
						}
					}
					else
					{
						slider_spos_ = 0;
					}
				}
			}
			virtual void _event_impl(IOContext::Event ev) override
			{
				bool write = false;
				int npos = 0;
				if (getstate(Clicked))
				{
					const auto bbox = box();
					const auto [ x, y ] = mouse_object_space();
					const auto sw = _resolve_units(data::slider_width);
					npos = std::max(0, std::min(
						int(bbox.height - sw),
						int(y + slider_spos_)
					));
					write = true;
				}
				else if (getstate(Hovered))
				{
					const auto bbox = box();
					if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollUp))
					{
						const auto sw = _resolve_units(data::slider_width);
						if (sw < bbox.height && slider_pos_ < bbox.height - sw)
						{
							npos = slider_pos_ + 1;
							write = true;
						}
					}
					else if (context()->input()->is_pressed_mouse(sys::SystemInput::ScrollDown))
					{
						if (slider_pos_)
						{
							npos = slider_pos_ - 1;
							write = true;
						}
					}
				}

				if (write && npos != slider_pos_)
				{
					slider_pos_ = npos;
					_signal_write(WriteType::Style);
					_submit();
				}
			}
			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
			{
				buffer.fill(data::slider_background_color);

				const auto sw = _resolve_units(data::slider_width);
				for (std::size_t i = 0; i < sw; i++)
				{
					if (slider_pos_ + i >= buffer.height()) break;
					buffer.at(
						0,
						slider_pos_ + i
					) = data::slider_color;
				}
			}
			virtual float _relative_value() override
			{
				const auto bbox = box();
				const auto sw = _resolve_units(data::slider_width);
				return std::clamp(float(slider_pos_) / (bbox.height - sw), 0.f, 1.f);
			}
		public:
			using Slider::Slider;

			virtual void reset_absolute(int value = 0) noexcept override
			{
				slider_pos_ = (value / (data::max - data::min)) * box().height;
				_submit();
				_signal_write(WriteType::Style);
			}
			virtual void reset_relative(float value = 0) noexcept override
			{
				slider_pos_ = box().width * value;
				_submit();
				_signal_write(WriteType::Style);
			}
		};
	}
}

#endif // D2_SLIDER_HPP
