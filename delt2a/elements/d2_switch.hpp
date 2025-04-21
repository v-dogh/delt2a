#ifndef D2_SWITCH_HPP
#define D2_SWITCH_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
	namespace style
	{
		struct Switch
		{
			std::function<void(Element::TraversalWrapper, int)> on_change{ nullptr };
			std::vector<string> options{};

			PixelBackground enabled_background_color{};
			PixelForeground enabled_foreground_color{};
			PixelBackground disabled_background_color{
				.r = 255, .g = 255, .b = 255
			};
			PixelForeground disabled_foreground_color{
				.r = 0, .g = 0, .b = 0
			};
			Pixel separator_color{ .a = 0, .v = '|' };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP_EMPTY
				D2_UAI_VAR_START
				D2_UAI_GET_VAR(0, options, Masked)
				D2_UAI_GET_VAR(1, enabled_foreground_color, Style)
				D2_UAI_GET_VAR(2, enabled_background_color, Style)
				D2_UAI_GET_VAR(3, disabled_foreground_color, Style)
				D2_UAI_GET_VAR(4, disabled_background_color, Style)
				D2_UAI_GET_VAR_A(5, on_change)
				D2_UAI_GET_VAR(6, separator_color, Style)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct ISwitch : Switch, InterfaceHelper<ISwitch, PropBase, 7>
		{
			using data = Switch;
			enum Property : uai_property
			{
				Options = PropBase,
				EnabledForegroundColor,
				EnabledBackgroundColor,
				DisabledForegroundColor,
				DisabledBackgroundColor,
				OnChange,
				SeparatorColor
			};
		};
		using IZSwitch = ISwitch<0>;
	}
	namespace dx
	{
		class Switch :
			public style::UAI<
				Switch,
				style::ISwitch,
				style::ILayout,
				style::IContainer,
				style::IColors,
				style::IKeyboardNav
			>,
			public impl::UnitUpdateHelper<Switch>,
			public impl::ContainerHelper<Switch>,
			public impl::TextHelper<Switch>
		{
		public:
			friend class UnitUpdateHelper;
			friend class ContainerHelper;
			friend class TextHelper;
			using opts = std::vector<string>;
			using data = style::UAI<
				Switch,
				style::ISwitch,
				style::ILayout,
				style::IContainer,
				style::IColors,
				style::IKeyboardNav
			>;
			using data::data;
		protected:
			int _idx{ 0 };

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
					const auto [ xbasis, ybasis ] = ContainerHelper::_border_base();
					const auto [ ixbasis, iybasis ] = ContainerHelper::_border_base_inv();

					int perfect_width = 0;
					for (decltype(auto) it : data::options)
					{
						perfect_width = std::max(
							perfect_width,
							int(it.size())
						);
					}

					BoundingBox res;
					if (data::width.getunits() == Unit::Auto)
					{
						res.width =
							(perfect_width * data::options.size()) +
							(data::options.size() - 1) + (xbasis + ixbasis) +
							data::width.raw();
					}
					else
						res.width = _resolve_units(data::width);
					if (data::height.getunits() == Unit::Auto)
					{
						res.height = 1 + (ybasis + iybasis);
					}
					else
						res.height = _resolve_units(data::height);

					return res;
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
					const auto [ width, height ] = box();
					const auto [ x, y ] = mouse_object_space();
					const auto [ basisx, basisy ] = ContainerHelper::_border_base();
					const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();
					const auto w = width - basisx - ibasisx;
					const auto cw =
						((w - (data::options.size() - 1)) /
						 (data::options.size())) + 1;

					if (cw > 0)
					{
						const auto idx = x / cw;
						const auto midx = x % cw;

						if (midx != (cw - 1) &&
							idx != _idx)
						{
							_idx = idx;
							_signal_write(Style);
							_submit();
						}
					}
				}
				else if (state == State::Keynavi)
				{
					_signal_write(Style);
				}
			}
			virtual void _event_impl(IOContext::Event ev) override
			{
				if (ev == IOContext::Event::KeyInput)
				{
					if (context()->input()->is_pressed(sys::SystemInput::Tab))
					{
						_idx = ++_idx % data::options.size();
						_signal_write(Style);
						_submit();
					}
				}
			}

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
			{
				buffer.fill(
					Pixel::combine(
						data::foreground_color,
						data::background_color
					)
				);

				const auto [ width, height ] = box();
				const auto [ basisx, basisy ] = ContainerHelper::_border_base();
				const auto [ ibasisx, ibasisy ] = ContainerHelper::_border_base_inv();
				const int width_slice =
					(width - (data::options.size() - 1) - (basisx + ibasisx)) /
					data::options.size();

				const auto disabled_color = Pixel::combine(
					data::disabled_foreground_color,
					data::disabled_background_color
				);
				const auto enabled_color = Pixel::combine(
					data::enabled_foreground_color,
					(getstate(Keynavi) ? data::focused_color : data::enabled_background_color)
				);
				for (std::size_t i = 0; i < data::options.size(); i++)
				{
					const auto xoff = int(basisx + ((width_slice + 1) * i));
					TextHelper::_render_text_simple(
						data::options[i],
						(i == _idx) ? enabled_color : disabled_color,
						style::Text::Alignment::Center,
						{ xoff, basisy }, { width_slice, 1 },
						buffer
					);
					if (i < data::options.size() - 1)
						buffer.at(xoff + width_slice, basisy)
							.blend(data::separator_color);
				}

				ContainerHelper::_render_border(buffer);
			}

			void _submit()
			{
				if (data::on_change != nullptr)
					data::on_change(shared_from_this(), _idx);
			}
		public:
			void reset(int idx) noexcept
			{
				if (_idx != idx)
				{
					_idx = idx;
					_signal_write(Style);
					_submit();
				}
			}
		};
	}
}

#endif // D2_SWITCH_HPP
