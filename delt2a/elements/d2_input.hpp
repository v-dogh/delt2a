#ifndef D2_INPUT_HPP
#define D2_INPUT_HPP

#include "../d2_tree_element.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
	namespace style
	{
		struct Input
		{
			enum InputOptions : unsigned char
			{
				DrawPtr = 1 << 0,
				Censor = 1 << 1,
				Blink = 1 << 2,
				AutoPtrColor = 1 << 3,

				Auto = DrawPtr | Blink | AutoPtrColor,
				Protected = Censor | AutoPtrColor | Auto,
			};

			string pre{};
			PixelBackground marked_mask{ .r = 170, .g = 170, .b = 170 };
			PixelForeground ptr_color{ .a = 255, .v = '_' };
			unsigned char input_options{ Auto };
			std::function<void(Element::TraversalWrapper, string)> on_submit{ nullptr };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP(Masked)
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, pre)
				D2_UAI_GET_VAR_A(1, marked_mask)
				D2_UAI_GET_VAR_A(2, ptr_color)
				D2_UAI_GET_VAR_A(3, input_options)
				D2_UAI_GET_VAR_A(4, on_submit)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct IInput : Input, InterfaceHelper<IInput, PropBase, 5>
		{
			using data = Input;
			enum Property : uai_property
			{
				Pre = PropBase,
				MarkedMask,
				PtrColor,
				InputOptions,
				OnSubmit
			};
		};
		using IZInput = IInput<0>;
	}
	namespace dx
	{
		class Input :
			public style::UAI<
				Input,
				style::ILayout,
				style::IColors,
				style::IText,
				style::IInput
			>,
			impl::UnitUpdateHelper<Input>
		{
		public:
			friend class UnitUpdateHelper;
			using data = style::UAI<
				Input,
				style::ILayout,
				style::IColors,
				style::IText,
				style::IInput
			>;
			using data::data;
		protected:
			bool ptr_blink_{ false };
			int ptr_{ 0 };
			int bound_lower_{ 0 };
			int highlight_start_{ 0 };
			int highlight_end_{ 0 };
			util::mt::future<void> btask_{};

			void _write_ptr(Pixel& px) noexcept
			{
				if ((data::input_options & InputOptions::Blink) && !ptr_blink_)
				{
					px.v = ' ';
				}
				else
				{
					px = Pixel::combine(
						(data::input_options & data::AutoPtrColor) ?
							data::foreground_color.extend(data::ptr_color.v) : data::ptr_color,
						data::background_color
					);
				}
			}

			bool _is_highlighted() noexcept
			{
				return highlight_start_ != highlight_end_;
			}
			void _remove_highlighted() noexcept
			{
				if (_is_highlighted())
				{
					const auto lower = std::min(highlight_start_, highlight_end_);
					const auto upper = std::max(highlight_start_, highlight_end_);
					const auto diff = upper - lower;
					data::text.erase(
						lower,
						diff
					);
					if (ptr_ >= upper)
						_move_left(diff);
					_reset_highlight();
				}
			}
			void _begin_highlight_if() noexcept
			{
				if (highlight_start_ == highlight_end_)
				{
					highlight_start_ = ptr_;
					highlight_end_ = ptr_;
					_signal_write(WriteType::Style);
				}
			}
			void _reset_highlight() noexcept
			{
				if (_is_highlighted())
				{
					highlight_start_ = 0;
					highlight_end_ = 0;
					_signal_write(WriteType::Style);
				}
			}
			void _reset_ptrs() noexcept
			{
				ptr_ = 0;
				bound_lower_ = 0;
				highlight_start_ = 0;
				highlight_end_ = 0;
				_reset_highlight();
			}
			string _highlighted_value() noexcept
			{
				if (_is_highlighted())
				{
					const auto lower = std::min(highlight_start_, highlight_end_);
					const auto upper = std::max(highlight_start_, highlight_end_);
					return data::text.substr(
						lower,
						upper - lower
					);
				}
				return "";
			}

			void _start_blink() noexcept
			{
				if (
					(data::input_options & InputOptions::Blink) &&
					getstate(State::Focused) &&
					btask_ == nullptr
					)
				{
					btask_ = context()->scheduler()->launch_cyclic_task([this](auto) {
						ptr_blink_ = !ptr_blink_;
						_signal_write(WriteType::Style);
					}, std::chrono::milliseconds(500));
				}
			}
			void _stop_blink() noexcept
			{
				if (btask_ != nullptr)
				{
					btask_.stop(false);
					btask_ = nullptr;
				}
				ptr_blink_ = false;
				_signal_write(WriteType::Style);
			}
			void _freeze_blink() noexcept
			{
				if (btask_ != nullptr)
				{
					btask_.stop(false);
					btask_ = nullptr;
				}
			}
			void _unfreeze_blink() noexcept
			{
				if (ptr_blink_ && btask_ == nullptr)
					_start_blink();
			}

			void _move_right(std::size_t cnt = 1)
			{
				_reset_highlight();

				ptr_ += cnt;
				if (ptr_ > data::text.size())
				{
					ptr_ = data::text.size();
				}

				const auto upper = bound_lower_ + box().width;
				if (ptr_ >= upper && bound_lower_ < data::text.size())
				{
					const auto diff = ptr_ - upper;
					bound_lower_ += diff;
				}

				_signal_write(WriteType::Style);
			}
			void _move_left(std::size_t cnt = 1)
			{
				_reset_highlight();

				if (ptr_ >= cnt)
				{
					ptr_ -= cnt;
				}
				else
				{
					ptr_ = 0;
				}

				if (ptr_ < bound_lower_ && bound_lower_ > 0)
				{
					const auto diff = bound_lower_ - ptr_;
					bound_lower_ -= diff;
				}

				_signal_write(WriteType::Style);
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
				BoundingBox bbox{};
				bbox.width = ((data::width.getunits() == Unit::Auto) ?
					int(data::pre.size() + data::text.size()) :
					data::pre.size() +
					_resolve_units(data::width)) +
					(data::input_options & InputOptions::DrawPtr);
				bbox.height = 1;
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
				if (state == State::Focused)
				{
					if (value)
					{
						_start_blink();
					}
					else
					{
						_stop_blink();
						_reset_highlight();
					}
				}
				else if (state == State::Clicked && value)
				{
					const auto [ x, y ] = mouse_object_space();
					if (x >= data::pre.size())
						ptr_ = bound_lower_ + (x - data::pre.size());
					_reset_highlight();
					_begin_highlight_if();
				}
				else if (state == State::Swapped)
				{
					if (value)
					{
						_freeze_blink();
					}
					else
					{
						_unfreeze_blink();
					}
				}
			}
			virtual void _event_impl(IOContext::Event ev) override
			{
				write_flag flag = WriteType::Style;
				bool write = false;

				const auto input = context()->input();
				if (ev == IOContext::Event::MouseInput)
				{
					if (input->is_pressed_mouse(sys::SystemInput::Left))
					{
						const auto [ x, y ] = mouse_object_space();
						if (x >= data::pre.size())
						{
							highlight_end_ = bound_lower_ + (x - data::pre.size());
							write = true;
						}
					}
				}
				else if (ev == IOContext::Event::KeyInput)
				{
					if (input->is_pressed(sys::SystemInput::Enter))
					{
						if (!data::text.empty())
						{
							if (data::on_submit != nullptr)
								data::on_submit(shared_from_this(), std::move(data::text));
							data::text.clear();
							_reset_ptrs();
							flag |= WriteType::Layout;
							write = true;
						}
					}
					else if (input->is_pressed(sys::SystemInput::Backspace))
					{
						if (_is_highlighted())
						{
							_remove_highlighted();
						}
						else if (!data::text.empty() && ptr_)
						{
							data::text.erase(ptr_ - 1, 1);
							if (ptr_ == bound_lower_)
								bound_lower_--;
							ptr_--;
							write = true;
						}
						flag |= WriteType::Layout;
					}
					else if (input->is_pressed(sys::SystemInput::Delete))
					{
						if (_is_highlighted())
						{
							_remove_highlighted();
						}
						else if (ptr_ < data::text.size())
						{
							data::text.erase(ptr_, 1);
							write = true;
						}
						flag |= WriteType::Layout;
					}
					else if (input->is_pressed(sys::SystemInput::ArrowLeft))
					{
						if (input->is_pressed(sys::SystemInput::Shift))
						{
							_begin_highlight_if();
							if (highlight_end_ > 0)
							{
								highlight_end_--;
								write = true;
							}
						}
						else
							_move_left();
					}
					else if (input->is_pressed(sys::SystemInput::ArrowRight))
					{
						if (input->is_pressed(sys::SystemInput::Shift))
						{
							_begin_highlight_if();
							if (highlight_end_ < data::text.size())
							{
								highlight_end_++;
								if (highlight_end_ > bound_lower_ + box().width)
									bound_lower_++;
								write = true;
							}
						}
						else
							_move_right();
					}
					else if (input->is_pressed(sys::SystemInput::Escape))
					{
						screen()->focus(nullptr);
					}
					else if (input->is_pressed(sys::SystemInput::LeftControl))
					{
						auto* clipboard = context()->sys_if<sys::ext::SystemClipboard>();
						if (clipboard)
						{
							if (input->is_pressed(sys::SystemInput::key('a')))
							{
								highlight_start_ = 0;
								highlight_end_ = data::text.size();
								write = true;
							}
							else if (input->is_pressed(sys::SystemInput::key('v')))
							{
								if (!clipboard->empty())
								{
									_remove_highlighted();
									const auto v = clipboard->paste();
									data::text.insert(ptr_, v);
									_move_right(v.size());
									flag |= WriteType::Layout;
									write = true;
								}
							}
							else if (
								input->is_pressed(sys::SystemInput::key('x')) &&
								highlight_start_ != highlight_end_
								)
							{
								clipboard->copy(_highlighted_value());
								_remove_highlighted();
							}
							else if (
								input->is_pressed(sys::SystemInput::key('c')) &&
								_is_highlighted()
								)
							{
								clipboard->copy(_highlighted_value());
							}
						}
					}
				}
				else if (ev == IOContext::Event::KeySequenceInput)
				{
					if (_is_highlighted())
						_remove_highlighted();
					if (ptr_ > data::text.size())
						ptr_ = data::text.size();

					auto seq = input->key_sequence();
					if (input->is_pressed(sys::SystemInput::Shift))
					{
						std::transform(seq.cbegin(), seq.cend(), seq.begin(), [](char ch) -> char {
							return std::toupper(ch);
						});
					}

					data::text.insert(data::text.begin() + ptr_, seq.begin(), seq.end());
					_move_right(seq.size());
					flag |= WriteType::Layout;
					write = true;
				}

				if (write)
				{
					_signal_write(flag);
				}
			}

			virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
			{
				const auto color = Pixel::combine(
					data::foreground_color,
					data::background_color
				);

				// Draw the pre-value
				{
					buffer.fill(data::background_color);
					for (std::size_t i = 0; i < data::pre.size(); i++)
					{
						auto& p = buffer.at(i);
						p = color;
						p.v = data::pre[i];
					}
				}

				// Correct bounds
				{
					if (ptr_ > data::text.size())
						ptr_ = data::text.size();
					if (bound_lower_ > data::text.size())
						bound_lower_ = data::text.size();
				}

				// Draw the input
				{
					if (!data::text.empty())
					{
						const auto hlower = std::min(highlight_start_, highlight_end_);
						const auto hupper = std::max(highlight_start_, highlight_end_);
						const auto m =
							std::min(bound_lower_ + box().width, int(data::text.size())) +
							bool(data::input_options & InputOptions::DrawPtr);

						for (std::size_t pi = data::pre.size(),
							 i = bound_lower_, d = 0;
							 i < m && pi < buffer.width();
							 i++, pi++)
						{
							if ((data::input_options & InputOptions::DrawPtr) &&
								i == ptr_)
							{
								d = 1;
							}
							else
							{
								auto& px = buffer.at(pi);
								if (i - d >= hlower && i - d < hupper)
								{
									px = color.mask(
										data::marked_mask
									);
								}
								else
									px = color;

								if (data::input_options & InputOptions::Censor)
								{
									px.v = '*';
								}
								else
								{
									px.v = data::text[i - d];
								}
							}
						}
					}
					if (const auto off = ptr_ + pre.size() - bound_lower_;
						(data::input_options & InputOptions::DrawPtr) &&
						off < buffer.width())
					{
						_write_ptr(
							buffer.at(off)
						);
					}
				}
			}
		};
	}
}

#endif // D2_INPUT_HPP
