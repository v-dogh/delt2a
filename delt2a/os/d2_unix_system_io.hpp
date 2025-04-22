#ifndef LINUX_IO_HANDLERS_HPP
#define LINUX_IO_HANDLERS_HPP

#include <sys/uio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>
#include <shared_mutex>
#include <iostream>
#include <charconv>
#include <vector>
#include <bitset>

#include "../d2_io_handler.hpp"
#include "../d2_locale.hpp"

namespace d2::sys
{
	class UnixTerminalInput : public SystemInput
	{
	private:
		std::shared_mutex mtx_{};

		std::pair<std::size_t, std::size_t> screen_size_{ -1, -1 };
		std::pair<int, int> mouse_pos_{ 0, 0 };

		// c - current, p - previous
		// c for hold
		// c and !p for press
		// !c and p for release

		bool had_screen_resize_ = true;
		bool poll_swap_index_ = false;
		std::array<std::bitset<SpecialKeyMax + 1>, 2> keys_poll_{};
		std::array<std::bitset<MouseKeyMax + 1>, 2> mouse_poll_{};

		string input_poll_{};
		termios restore_termios_{};

		auto& _c_kpoll() noexcept
		{
			return keys_poll_[poll_swap_index_];
		}
		auto& _p_kpoll() noexcept
		{
			return keys_poll_[!poll_swap_index_];
		}
		auto& _c_mpoll() noexcept
		{
			return mouse_poll_[poll_swap_index_];
		}
		auto& _p_mpoll() noexcept
		{
			return mouse_poll_[!poll_swap_index_];
		}

		void _set(unsigned int ch) noexcept
		{
			_c_kpoll().set(ch);
		}
		void _setm(MouseKey ch) noexcept
		{
			_c_mpoll().set(ch);
		}
		void _relm(MouseKey ch) noexcept
		{
			_c_mpoll().reset(ch);
		}
		void _resetm() noexcept
		{
			_c_mpoll().reset();
		}

		void _enable_raw_mode()
		{
			termios io;
			tcgetattr(STDIN_FILENO, &restore_termios_);
			io = restore_termios_;
			io.c_lflag &= ~(ICANON | ECHO);
			io.c_cc[VMIN] = 1;
			io.c_cc[VTIME] = 0;
			tcsetattr(STDIN_FILENO, TCSANOW, &io);
			fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
			// Disable cursor | enable mouse tracking
			std::cout << "\033[?25l" << "\033[?1003h" << std::flush;

		}
		void _disable_raw_mode()
		{
			tcsetattr(STDIN_FILENO, TCSANOW, &restore_termios_);
			std::cout << "\033[?25h" << "\033[?1003l" << std::flush;
		}
	protected:
		virtual bool _is_pressed_impl(keytype ch, KeyMode mode) override
		{
			std::shared_lock lock(mtx_);

			if (mode == KeyMode::Hold)
				return _c_kpoll().test(ch);
			else if (mode == KeyMode::Press)
				return _c_kpoll().test(ch) && !_p_kpoll().test(ch);
			else if (mode == KeyMode::Release)
				return !_c_kpoll().test(ch) && _p_kpoll().test(ch);
			return false;
		}
		virtual const string& _key_sequence_input_impl() override
		{
			std::shared_lock lock(mtx_);
			return input_poll_;
		}

		virtual bool _is_pressed_mouse_impl(MouseKey key, KeyMode mode) override
		{
			std::shared_lock lock(mtx_);

			if (mode == KeyMode::Hold)
				return _c_mpoll().test(key);
			else if (mode == KeyMode::Press)
				return _c_mpoll().test(key) && !_p_mpoll().test(key);
			else if (mode == KeyMode::Release)
				return !_c_mpoll().test(key) && _p_mpoll().test(key);
			return false;
		}
		virtual std::pair<int, int> _mouse_position_impl() override
		{
			std::shared_lock lock(mtx_);
			return mouse_pos_;
		}

		virtual std::pair<std::size_t, std::size_t> _screen_size_impl() override
		{
			std::shared_lock lock(mtx_);
			return screen_size_;
		}
		virtual std::pair<std::size_t, std::size_t> _screen_capacity_impl() override
		{
			return _screen_size_impl();
		}

		virtual bool _has_mouse_impl() override
		{
			return true;
		}

		virtual bool _is_key_sequence_input_impl() override
		{
			std::shared_lock lock(mtx_);
			return !input_poll_.empty();
		}
		virtual bool _is_key_input_impl() override
		{
			std::shared_lock lock(mtx_);
			return
				_c_kpoll().test(SpecialKeyMax) ||
				_p_kpoll().test(SpecialKeyMax);
		}
		virtual bool _is_mouse_input_impl() override
		{
			std::shared_lock lock(mtx_);
			return
				_c_mpoll().test(MouseKeyMax) ||
				_p_mpoll().test(MouseKeyMax);
		}
		virtual bool _is_screen_resize_impl() override
		{
			std::shared_lock lock(mtx_);
			return had_screen_resize_;
		}

		virtual void _begincycle_impl() noexcept override { }
		virtual void _endcycle_impl() noexcept override
		{
			std::unique_lock lock(mtx_);

			// Update screen size

			{
				struct winsize ws;
				if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
				{
					screen_size_ = { 0, 0 };
					had_screen_resize_ = true;
				}
				else
				{
					const std::pair<std::size_t, std::size_t> src = { ws.ws_col, ws.ws_row };
					had_screen_resize_ =
						src.first != screen_size_.first ||
						src.second != screen_size_.second;
					screen_size_ = src;
				}
			}

			// Clear state

			// Reset key state
			{
				input_poll_.clear();
				_p_kpoll().reset();
			}
			// Reset mouse state
			{
				_p_mpoll() = _c_mpoll();
				_p_mpoll().reset(MouseKey::ScrollDown);
				_p_mpoll().reset(MouseKey::ScrollUp);
			}
			// Swap
			poll_swap_index_ = !poll_swap_index_;

			// Keyboard input

			fd_set fds;
			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;

			FD_ZERO(&fds);
			FD_SET(STDIN_FILENO, &fds);

			bool is_input = ::select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &timeout) > 0;
			_c_kpoll().set(SpecialKeyMax, is_input);
			_c_mpoll().reset(MouseKeyMax);
			while (is_input)
			{
				char ch;
				::read(STDIN_FILENO, &ch, 1);

				// Special keys :) (for now only some of them since im sigma skividi rn fr fr)
				if (ch == '\n')
				{
					_set(Enter);
				}
				// Tabulator exterminans
				else if (ch == '\t')
				{
					_set(Tab);
				}
				// Backspace
				else if (ch == 127)
				{
					_set(Backspace);
				}
				// Control
				else if (ch >= 1 && ch <= 26)
				{
					const auto res = 'A' + (ch - 1);
					_set(LeftControl);
					_set(res - keymin());
				}
				// Escape
				else if (ch == '\e')
				{
					std::array<char, 6> seq{ ch };
					std::size_t i = 1;
					while (i < seq.size() && ::read(STDIN_FILENO, &seq[i], 1) > 0) i++;

					if (i > 1)
					{
						// Other
						if (seq[1] == 'O')
						{
							if (seq[2] >= 'P' && seq[2] <= 'P' + 12)
							{
								char n = seq[2] - 'P';
								_set(Fn + n + 1);
							}
						}
						else if (seq[1] == '[')
						{
							// Mouse input
							if (seq[2] == 'M')
							{
								const int button = seq[3] - 32;
								switch (button)
								{
								case 0: _setm(MouseKey::Left); break;
								case 1: _setm(MouseKey::Middle); break;
								case 2: _setm(MouseKey::Right); break;
								case 3: _resetm(); break;
								case 4: _relm(MouseKey::Middle); break;
								case -127: _setm(MouseKey::SideTop); break;
								case -128: _setm(MouseKey::SideBottom); break;
								case -95: _relm(MouseKey::SideTop); break;
								case 64: _setm(MouseKey::ScrollUp); break;
								case 65: _setm(MouseKey::ScrollDown); break;
								}

								mouse_pos_.first = int(*reinterpret_cast<const unsigned char*>(&seq[4])) - 33;
								mouse_pos_.second = int(*reinterpret_cast<const unsigned char*>(&seq[5])) - 33;
								_c_mpoll().set(MouseKeyMax);
							}
							else if (seq[2] == 'Z')
							{
								_set(Shift);
								_set(Tab);
							}
							else
							{
								std::size_t basis = 0;
								if (seq[2] == '1' && seq[3] == ';' && seq[4] == '2')
								{
									_set(LeftControl);
									_set(Shift);
									basis = 3;
								}

								switch (seq[2 + basis])
								{
								case 'A': _set(ArrowUp); break;
								case 'B': _set(ArrowDown); break;
								case 'C': _set(ArrowRight); break;
								case 'D': _set(ArrowLeft); break;
								case 'H': _set(Home); break;
								case 'F': _set(End); break;
								case '3': _set(Delete); break;
								}
							}
						}
					}
					// Literally escape
					else
					{
						_set(Escape);
					}
				}
				// Normal keys :(
				else
				{
					// Sequence
					input_poll_.push_back(ch);

					// Key
					if (ch >= keymin() && ch <= keymax())
					{
						if (ch >= 'A' && ch <= 'Z')
							_set(Shift);
						_set(_resolve_key(ch));
					}
				}
				is_input = ::select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &timeout) > 0;
			}
		}
	public:
		UnixTerminalInput(std::weak_ptr<IOContext> ctx) : SystemInput(ctx)
		{
			_enable_raw_mode();
		}
		virtual ~UnixTerminalInput()
		{
			_disable_raw_mode();
		}

		void mask_interrupts()
		{
			std::unique_lock lock(mtx_);

			struct termios io;
			tcgetattr(STDIN_FILENO, &io);
			io.c_lflag &= ~ISIG;
			tcsetattr(STDIN_FILENO, TCSANOW, &io);
		}
		void unmask_interrupts()
		{
			std::unique_lock lock(mtx_);

			struct termios io;
			tcgetattr(STDIN_FILENO, &io);
			io.c_lflag |= ISIG;
			tcsetattr(STDIN_FILENO, TCSANOW, &io);
		}
	};
	class UnixOutput : public SystemOutput
	{
	private:		
		static constexpr auto max_color_len_ =
			std::string_view("\033[RX;RX;RX;RX;RX;RX;RX;m").size() +
			(std::string_view("38;2;255;255;255").size() * 2);
		static constexpr auto max_pos_len_ =
			std::string_view("\x1b[XXXX;XXXXH").size();
		static constexpr auto max_sixel_len_ = std::string_view("63;63;63m").size();
		static constexpr auto cls_code_ = std::string_view("\033[H");

		std::chrono::microseconds frame_time_{ 0 };
		std::vector<unsigned char> out_{};
		std::vector<Pixel> previous_frame_{};
		std::size_t buffer_size_{ 0 };
		std::uint8_t track_style_{};
		PixelForeground track_foreground_{};
		PixelBackground track_background_{};

		std::mutex mtx_{};
		std::unordered_map<
			ImageInstance::id,
			std::pair<ImageInstance::wptr, std::vector<unsigned char>>>
		images_{};

		unsigned char _u8component_to_sixel(unsigned int value) noexcept
		{
			return (value * 63) / 255;
		}
		auto _generate_sixel(unsigned char r, unsigned char g, unsigned char b) noexcept
		{
			std::array<char, max_sixel_len_> code{};
			const auto [ ptr1, _1 ] = std::to_chars(code.begin(), code.end(), r);
			const auto [ ptr2, _2 ] = std::to_chars(ptr1 + 1, code.end(), g);
			const auto [ ptr3, _3 ] = std::to_chars(ptr2 + 1, code.end(), b);

			*ptr1 = ';';
			*ptr2 = ';';
			*ptr3 = 'm';

			return std::make_pair(code, ptr3 - code.data() + 1);
		}
		std::vector<unsigned char> _image_to_sixel(const ImageInstance::data& data, std::size_t width, std::size_t height)
		{
			constexpr std::string_view sixel_begin = "\033P1;2";
			constexpr std::string_view sixel_end ="\033\\";
			std::vector<unsigned char> img;

			// Worst case
			img.reserve(sixel_begin.size() + sixel_end.size() + 10 * data.size());
			img.insert(img.end(), sixel_begin.begin(), sixel_begin.end());

			for (std::size_t j = 0; j < height; j++)
				for (std::size_t i = 0; i < width; i++)
				{
					const auto& px = data[j * width + i];

					const auto rs = _u8component_to_sixel(px.r);
					const auto gs = _u8component_to_sixel(px.g);
					const auto bs = _u8component_to_sixel(px.b);
					const auto [ code, off ] = _generate_sixel(rs, bs, gs);
					img.insert(img.end(), code.begin(), code.begin() + off);
				}

			img.insert(img.end(), sixel_end.begin(), sixel_end.end());
			img.shrink_to_fit();

			return img;
		}

		auto _generate_position(int x, int y, bool skip = false) noexcept
		{
			D2_EXPECT(x <= 9999 && y <= 9999)

			using buffer = std::array<char, max_pos_len_>;

			if (skip)
				return std::make_pair(buffer(), 0);

			buffer result{ "\x1b[" };

			const auto [ ptr1, _1 ] = std::to_chars(result.begin() + 2, result.end(), y + 1);
			const auto [ ptr2, _2 ] = std::to_chars(ptr1 + 1, result.end(), x + 1);

			*ptr1 = ';';
			*ptr2 = 'H';

			return std::make_pair(result, int(ptr2 - result.begin() + 1));
		}
		auto _generate_color(const Pixel& px, bool force = false) noexcept
		{
			using buffer = std::array<char, max_color_len_>;

			constexpr auto style_code_table = std::array<char, 7>{
				'1', '4', '3', '9', '5', '7', '8'
			};
			constexpr auto style_code = std::string_view("\033[");
			constexpr auto bg_code = std::string_view("48;2;");
			constexpr auto fg_code = std::string_view("38;2;");

			// Preds

			const auto do_background =
				force ||
				px.r != track_background_.r ||
				px.g != track_background_.g ||
				px.b != track_background_.b;
			const auto do_foreground =
				force ||
				px.rf != track_foreground_.r ||
				px.gf != track_foreground_.g ||
				px.bf != track_foreground_.b;

			if (px.style == track_style_ && !do_background && !do_foreground)
				return std::make_pair(buffer(), 0);

			buffer code{ "\033[" };

			// Styles

			buffer::iterator ptr = code.begin() + style_code.size();
			if (force || px.style != track_style_)
			{
				for (std::size_t i = 0; i < 7; i++)
				{

					const auto ns = px.style & (1 << i);
					const auto ps = track_style_ & (1 << i);
					if (ns != ps)
					{
						if (!ns)
							*(ptr++) = '2';
						*(ptr++) = style_code_table[i];
						*(ptr++) = ';';
					}
				}
				track_style_ = px.style;
			}

			if (!(do_foreground || do_background))
				*ptr = 'm';

			// Background
			buffer::iterator ptrbe = ptr;
			if (do_background)
			{
				const auto ptrb = std::copy(bg_code.begin(), bg_code.end(), ptrbe);

				const auto [ ptr1b, _1b ] = std::to_chars(ptrb, code.end(), px.r);
				const auto [ ptr2b, _2b ] = std::to_chars(ptr1b + 1, code.end(), px.g);
				const auto [ ptr3b, _3b ] = std::to_chars(ptr2b + 1, code.end(), px.b);

				*ptr1b = ';';
				*ptr2b = ';';
				*ptr3b = do_foreground ? ';' : 'm';

				ptrbe = ptr3b + 1;

				track_background_.r = px.r;
				track_background_.g = px.g;
				track_background_.b = px.b;
			}

			// Foreground
			buffer::iterator ptrfe = ptrbe;
			if (do_foreground)
			{
				const auto ptrf = std::copy(fg_code.begin(), fg_code.end(), ptrfe);

				const auto [ ptr1f, _1f ] = std::to_chars(ptrf, code.end(), px.rf);
				const auto [ ptr2f, _2f ] = std::to_chars(ptr1f + 1, code.end(), px.gf);
				const auto [ ptr3f, _3f ] = std::to_chars(ptr2f + 1, code.end(), px.bf);

				*ptr1f = ';';
				*ptr2f = ';';
				*ptr3f = 'm';

				ptrfe = ptr3f;

				track_foreground_.r = px.rf;
				track_foreground_.g = px.gf;
				track_foreground_.b = px.bf;
			}

			return std::make_pair(code, int(ptrfe - code.begin() + 1));
		}

		void _write(std::span<const unsigned char> buffer) noexcept
		{
			::write(STDOUT_FILENO, buffer.data(), buffer.size());
		}
		void _write(std::span<const char> buffer) noexcept
		{
			::write(STDOUT_FILENO, buffer.data(), buffer.size());
		}
		void _writev(std::span<const iovec> vecs) noexcept
		{
			::writev(STDOUT_FILENO, vecs.data(), vecs.size());
		}
	public:
		using SystemOutput::SystemOutput;
		virtual ~UnixOutput() = default;

		std::chrono::microseconds frame_time() const noexcept
		{
			return frame_time_;
		}
		std::size_t buffer_size() const noexcept
		{
			return buffer_size_;
		}

		virtual ImageInstance::ptr make_image(ImageInstance::data data, std::size_t width, std::size_t height) override
		{
			// The image instance is left empty
			// We convert the image data to sixel encoding
			auto ptr = std::make_shared<ImageInstance>(ImageInstance::data{}, width, height);
			std::unique_lock lock(mtx_);
			images_[ptr->code()] = std::make_pair(
				ptr,
				_image_to_sixel(data, width, height)
			);
			return ptr;
		}
		virtual void release_image(ImageInstance::ptr ptr) override
		{
			std::unique_lock lock(mtx_);
			images_.erase(ptr->code());
		}

		virtual void write(std::span<const Pixel> buffer, std::size_t width, std::size_t height) override
		{
			std::unique_lock lock(mtx_);

			const auto beg = std::chrono::high_resolution_clock::now();

			track_style_ = 0x00;
			track_foreground_ = PixelForeground(255, 255, 255);
			track_background_ = PixelBackground(0, 0, 0);

			out_.reserve(
				(buffer.size()) * sizeof(value_type) +
				(height + 1) +
				cls_code_.size() +
				max_color_len_ * 5
			);

			// Clean redraw
			if (buffer.size() != previous_frame_.size() ||
				// Check all corners, if changed it is likely that the entire frame changed
				(buffer[0] != previous_frame_[0] &&
				 buffer[buffer.size() - 1] != previous_frame_[buffer.size() - 1] &&
				 buffer[width - 1] != previous_frame_[width - 1] &&
				 buffer[buffer.size() - width] != previous_frame_[buffer.size() - width]))
			{
				out_.insert(out_.end(), cls_code_.begin(), cls_code_.end());

				for (auto it = buffer.begin(); it != buffer.end();)
				{
					const auto& px = *it;
					const auto [ code, len ] = _generate_color(px);

					auto sit = it;
					while (sit != buffer.end() && px.compare_colors(*sit)) ++sit;

					const auto abs_start = it - buffer.begin();
					const auto abs_end = sit - buffer.begin();
					const auto edls = (abs_end - abs_start) / width;

					out_.reserve(out_.size() + len + ((sit - it) * sizeof(value_type)) + edls);
					out_.insert(out_.end(), code.begin(), code.begin() + len);

					while (it != sit)
					{
						const auto abs = it - buffer.begin();
						if (abs && !(abs % width) && abs != abs_start)
							out_.push_back('\n');
						out_.push_back(it->v);
						++it;
					}

					if (sit == buffer.end())
						break;
				}
			}
			else
			{
				bool sequential = false;
				bool linear = false;
				for (auto it = buffer.begin(); it != buffer.end();)
				{
					if (const auto& px = *it;
						px != previous_frame_[it - buffer.begin()])
					{
						const auto idx = it - buffer.begin();
						const auto x = idx % width;
						const auto y = idx / width;
						const auto [ pos, plen ] = _generate_position(x, y, linear && sequential);
						const auto [ code, len ] = _generate_color(px, !sequential);

						auto sit = it;
						auto cnt = 0;
						while (sit != buffer.end() &&
							   *sit != previous_frame_[idx + cnt] &&
							   px.compare_colors(*sit))
						{
							++sit;
							++cnt;
						}

						const auto end = sit - buffer.begin();
						const auto edls = (end - idx) / width;

						out_.reserve(out_.size() + len + plen + ((sit - it) * sizeof(value_type)) + edls);
						out_.insert(out_.end(), pos.begin(), pos.begin() + plen);
						out_.insert(out_.end(), code.begin(), code.begin() + len);

						linear = true;
						sequential = true;
						while (it != sit)
						{
							const auto abs = it - buffer.begin();
							if (abs && !(abs % width) && abs != idx)
							{
								linear = false;
								out_.push_back('\n');
							}
							out_.push_back(it->v);
							++it;
						}

						if (sit == buffer.end())
							break;
					}
					else
					{
						const auto abs = it - buffer.begin();
						linear = !(abs && !(abs % width));
						sequential = false;
						++it;
					}
				}
			}

			if (!out_.empty())
			{
				constexpr auto chunk = 1024ull;
				constexpr auto frame_upper_bound = 56ull * 1024ull;
				D2_EXPECT(out_.size() <= frame_upper_bound)

				// Results in visual tearing with large buffers (same as unchunked)
				// std::array<iovec, frame_upper_bound / chunk> chunks;
				// const auto len = (out_.size() + chunk - 1) / chunk;
				// for (std::size_t i = 0; i < len; i++)
				// {
				// 	const auto idx = i * chunk;
				// 	const auto rem = out_.size() - idx;
				// 	chunks[i] = iovec{
				// 		.iov_base = out_.data() + idx,
				// 		.iov_len = std::min(chunk, rem)
				// 	};
				// }
				// _writev({ chunks.begin(), chunks.begin() + len });

				const auto len = (out_.size() + chunk - 1) / chunk;
				for (std::size_t i = 0; i < len; i++)
				{
					const auto idx = i * chunk;
					const auto rem = out_.size() - idx;
					_write({ out_.begin() + idx, out_.begin() + idx + std::min(chunk, rem) });
				}

				previous_frame_.clear();
				previous_frame_.insert(previous_frame_.end(), buffer.begin(), buffer.end());
			}
			buffer_size_ = out_.size();
			out_.clear();

			frame_time_ = std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::high_resolution_clock::now() - beg
			);
		}
	};
} // d2

namespace d2
{
	template<>
	struct OSConfig<std::size_t(OS::Unix)> : OSConfig<std::size_t(OS::Default)>
	{
		using input = sys::UnixTerminalInput;
		using output = sys::UnixOutput;
		using clipboard = sys::ext::LocalSystemClipboard;
	};
}

#endif // LINUX_IO_HANDLERS_HPP
