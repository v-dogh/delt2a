#ifndef D2_DROPDOWN_HPP
#define D2_DROPDOWN_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
	namespace style
	{
		struct Dropdown
		{
			string default_value{ "" };

			PixelForeground selected_foreground_color{};
			PixelBackground selected_background_color{};

			PixelForeground option_foreground_color{};
			PixelBackground option_background_color{};

			bool show_choice{ true };

			std::vector<string> options{};
			std::function<void(Element::TraversalWrapper, const string&, std::size_t)> on_submit{ nullptr };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP(Style)
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, default_value)
				D2_UAI_GET_VAR_A(1, selected_foreground_color)
				D2_UAI_GET_VAR_A(2, selected_background_color)
				D2_UAI_GET_VAR_A(3, option_foreground_color)
				D2_UAI_GET_VAR_A(4, option_background_color)
				D2_UAI_GET_VAR_A(5, show_choice)
				D2_UAI_GET_VAR_A(6, options)
				D2_UAI_GET_VAR(7, on_submit, None)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct IDropdown : Dropdown, InterfaceHelper<IDropdown, PropBase, 8>
		{
			using data = Dropdown;
			enum Property : uai_property
			{
				DefaultValue = PropBase,
				SelectedForeground,
				SelectedBackground,
				OptionForeground,
				OptionBackground,
				ShowChoice,
				Options,
				OnSubmit
			};
		};
		using IZDropdown = IDropdown<0>;
	}
	namespace dx
	{
		class Dropdown :
			public Element,
			public style::UAI<
				Dropdown,
				style::ILayout,
				style::IColors,
				style::IContainer,
				style::IDropdown,
				style::IKeyboardNav
			>,
			public impl::UnitUpdateHelper<Dropdown>,
			public impl::TextHelper<Dropdown>,
			public impl::ContainerHelper<Dropdown>
		{
		public:
			friend class ContainerHelper;
			friend class TextHelper;
			friend class UnitUpdateHelper;
			using data = style::UAI<
				Dropdown,
				style::ILayout,
				style::IColors,
				style::IContainer,
				style::IDropdown,
				style::IKeyboardNav
			>;
			using opts = std::vector<string>;
		protected:
			mutable int cell_height_{ 0 };
			int soft_selected_{ -1 };
			int selected_{ -1 };
			bool opened_{ false };

			int _border_width()
			{
				if (!(data::container_options & data::EnableBorder))
					return 0;

				const auto bbox = box();
				auto bw = _resolve_units(data::border_width);
				[[ unlikely ]] if (bw > bbox.height || bw > bbox.width)
				{
					data::border_width = Unit(std::min(bbox.width, bbox.height), Unit::Px);
					bw = _resolve_units(data::border_width);
				}
				return bw;
			}

			virtual BoundingBox _dimensions_impl() const noexcept override
			{
				const auto bw = (data::container_options & data::EnableBorder) ?
					_resolve_units(data::border_width) : 0;
				if (data::width.getunits() == Unit::Auto || data::height.getunits() == Unit::Auto)
				{
					int perfect_width = 0;
					int perfect_height = 0;
					{
						const auto [ width, height ] = TextHelper::_text_bounding_box(data::default_value);
						perfect_width = std::max(width, perfect_width);
						perfect_height = std::max(height, perfect_height);
					}
					for (decltype(auto) it : data::options)
					{
						const auto [ width, height ] = TextHelper::_text_bounding_box(it);
						perfect_width = std::max(width, perfect_width);
						perfect_height = std::max(height, perfect_height);
					}

					BoundingBox res;
					const auto cnt = (data::options.size() * opened_) + 1;
					if (data::width.getunits() == Unit::Auto)
						res.width = perfect_width + (bw * 2) + data::width.raw();
					else res.width = _resolve_units(data::width);
					if (data::height.getunits() == Unit::Auto)
						res.height = (perfect_height + bw) * cnt + bw + data::height.raw();
					else res.height = _resolve_units(data::height) * cnt;
					cell_height_ = perfect_height;

					return res;
				}
				else
				{
					const auto cnt = (data::options.size() * opened_) + 1;
					const auto width = _resolve_units(data::width);
					const auto height = int(_resolve_units(data::height));
					cell_height_ = height - (bw * 2);
					return {
						width,
						int((height - bw) * cnt + bw)
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

			virtual void _state_change_impl(State state, bool value) override
			{
				if (state == State::Clicked && value)
				{
					const auto [ x, y ] = mouse_object_space();
					const auto bcw = _border_width();
					const auto yoff = (cell_height_ / 2) + bcw;
					const auto c = (y - yoff) / (cell_height_ + bcw);
					if (c >= 1)
					{
						choose(c - 1);
					}
					else
					{
						opened_ = !opened_;
					}
					_signal_write(WriteType::Masked);
				}
				else if (state == State::Focused && !value)
				{
					opened_ = false;
					_signal_write(WriteType::Masked);
				}
				else if (state == State::Keynavi)
				{
					opened_ = value;
					_signal_write(Masked);
				}
			}
			virtual void _event_impl(IOContext::Event ev) override
			{
				if (ev == IOContext::Event::KeyInput)
				{
					if (context()->input()->is_pressed(sys::SystemInput::ArrowLeft) ||
						context()->input()->is_pressed(sys::SystemInput::ArrowDown))
					{
						soft_selected_ = (soft_selected_ + 1) % data::options.size();
						_signal_write(Style);
					}
					else if (context()->input()->is_pressed(sys::SystemInput::ArrowRight) ||
							 context()->input()->is_pressed(sys::SystemInput::ArrowUp))
					{
						const auto s = int(data::options.size());
						soft_selected_ = ((soft_selected_ - 1) % s + s) % s;
						_signal_write(Style);
					}
					else if (context()->input()->is_pressed(sys::SystemInput::Enter))
					{
						if (opened_ && soft_selected_ != -1)
						{
							choose(soft_selected_);
						}
						else
						{
							opened_ = !opened_;
							_signal_write(Masked);
						}
						soft_selected_ = -1;
					}
				}
			}

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
			{
				// Also computes cell height
				const auto bbox = box();
				const auto color = Pixel::combine(data::foreground_color, data::background_color);
				const auto color_focused = Pixel::combine(data::foreground_color, data::focused_color);

				buffer.fill(color);

				// Display
				const auto bw = _border_width();
				const auto yoff = (cell_height_ / 2) + bw;
				{
					if (data::show_choice && selected_ != -1)
					{
						const auto dims = TextHelper::_text_bounding_box(
							data::options[selected_]
						);
						const auto xoff = ((bbox.width - dims.width - bw) / 2) + bw;
						TextHelper::_render_text(
							data::options[selected_], color,
							{ xoff, yoff },
							{ bbox.width, cell_height_ },
							buffer
						);
					}
					else
					{
						const auto dims = TextHelper::_text_bounding_box(data::default_value);
						const auto xoff = ((bbox.width - dims.width - bw) / 2) + bw;

						TextHelper::_render_text(
							data::default_value, color,
							{ xoff, yoff },
							{ bbox.width, cell_height_ },
							buffer
						);
					}
				}

				if (opened_)
				{
					for (std::size_t i = 0; i < data::options.size(); i++)
					{
						const auto dims = TextHelper::_text_bounding_box(
							data::options[i]
						);
						const auto xoff = ((bbox.width - dims.width - bw) / 2) + bw;
						const auto yoff2 = int(yoff + ((i + 1) * (cell_height_ + bw)));
						TextHelper::_render_text(
							data::options[i], (i == soft_selected_ ? color_focused : color),
							{ xoff, yoff2 },
							{ bbox.width, cell_height_ },
							buffer
						);
					}
				}

				const bool is_border = data::container_options & ContainerOptions::EnableBorder;
				const bool is_topfix = data::container_options & ContainerOptions::TopFix;
				if (is_border)
				{
					ContainerHelper::_render_border(buffer);

					const auto& bch = data::border_horizontal_color;
					const auto ch = cell_height_ + bw;
					const auto opt = opened_ * data::options.size();

					for (std::size_t c = 1; c < opt + 1; c++)
						for (std::size_t i = 0; i < bw; i++)
							for (std::size_t x = is_topfix; x < bbox.width - is_topfix; x++)
							{
								buffer.at(x, (ch * c) - i) = bch;
							}
				}
			}
		public:
			using Element::Element;

			void choose(int idx)
			{
				if (idx >= 0)
				{
					if (idx >= data::options.size())
						throw std::runtime_error{ "Invalid option selected" };
					if (data::on_submit != nullptr )
						data::on_submit(shared_from_this(), options[idx], idx);
				}
				opened_ = false;
				selected_ = idx;
				_signal_write(WriteType::Masked);
			}
			void reset()
			{
				choose(-1);
			}
			void reset(const string& value)
			{
				data::default_value = value;
				reset();
			}

			int index()
			{
				if (selected_ == -1)
					return 0;
				return selected_;
			}
			string value()
			{
				if (selected_ == -1)
					return data::options[0];
				return data::options[selected_];
			}
		};
	}
}

#endif // D2_DROPDOWN_HPP
