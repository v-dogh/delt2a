#include "d2_unix_system_io.hpp"
#include <iostream>
#include <charconv>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

namespace d2::sys
{
	//
	// Input
	//

	UnixTerminalInput::keyboard_keymap& UnixTerminalInput::_c_kpoll() noexcept
	{
		return keys_poll_[poll_swap_index_];
	}
	UnixTerminalInput::keyboard_keymap& UnixTerminalInput::_p_kpoll() noexcept
	{
		return keys_poll_[!poll_swap_index_];
	}
	UnixTerminalInput::mouse_keymap& UnixTerminalInput::_c_mpoll() noexcept
	{
		return mouse_poll_[poll_swap_index_];
	}
	UnixTerminalInput::mouse_keymap& UnixTerminalInput::_p_mpoll() noexcept
	{
		return mouse_poll_[!poll_swap_index_];
	}

	void UnixTerminalInput::_set(unsigned int ch) noexcept
	{
		_c_kpoll().set(ch);
	}
	void UnixTerminalInput::_setm(MouseKey ch) noexcept
	{
		_c_mpoll().set(ch);
	}
	void UnixTerminalInput::_relm(MouseKey ch) noexcept
	{
		_c_mpoll().reset(ch);
	}
	void UnixTerminalInput::_resetm() noexcept
	{
		_c_mpoll().reset();
	}

	void UnixTerminalInput::_enable_raw_mode()
	{
		termios io;
		tcgetattr(STDIN_FILENO, &restore_termios_);
		io = restore_termios_;
		io.c_lflag &= ~(ICANON | ECHO);
		io.c_cc[VMIN] = 1;
		io.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, TCSANOW, &io);
		fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
        // Disable cursor | enable mouse tracking | extended mode
        std::cout << "\033[?25l" << "\033[?1003h" << "\x1b[?1006h" << std::flush;

	}
	void UnixTerminalInput::_disable_raw_mode()
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &restore_termios_);
        std::cout << "\033[?25h" << "\033[?1003l" << "\x1b[?1006l" << std::flush;
	}

	bool UnixTerminalInput::_is_pressed_impl(keytype ch, KeyMode mode)
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
	const string& UnixTerminalInput::_key_sequence_input_impl()
	{
		std::shared_lock lock(mtx_);
		return input_poll_;
	}

	bool UnixTerminalInput::_is_pressed_mouse_impl(MouseKey key, KeyMode mode)
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
	std::pair<int, int> UnixTerminalInput::_mouse_position_impl()
	{
		std::shared_lock lock(mtx_);
		return mouse_pos_;
	}

	std::pair<std::size_t, std::size_t> UnixTerminalInput::_screen_size_impl()
	{
		std::shared_lock lock(mtx_);
		return screen_size_;
	}
	std::pair<std::size_t, std::size_t> UnixTerminalInput::_screen_capacity_impl()
	{
		return _screen_size_impl();
	}

	bool UnixTerminalInput::_has_mouse_impl()
	{
		return true;
	}

	bool UnixTerminalInput::_is_key_sequence_input_impl()
	{
		std::shared_lock lock(mtx_);
		return !input_poll_.empty();
	}
	bool UnixTerminalInput::_is_key_input_impl()
	{
		std::shared_lock lock(mtx_);
		return
			_c_kpoll().test(SpecialKeyMax) ||
			_p_kpoll().test(SpecialKeyMax);
	}
	bool UnixTerminalInput::_is_mouse_input_impl()
	{
		std::shared_lock lock(mtx_);
		return
			_c_mpoll().test(MouseKeyMax) ||
			_p_mpoll().test(MouseKeyMax);
	}
	bool UnixTerminalInput::_is_screen_resize_impl()
	{
		std::shared_lock lock(mtx_);
		return had_screen_resize_;
	}

	void UnixTerminalInput::_begincycle_impl() noexcept { }
	void UnixTerminalInput::_endcycle_impl() noexcept
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
                std::array<char, 18> seq{ ch };
				std::size_t i = 1;
                while (i < seq.size() &&
                       ::read(STDIN_FILENO, &seq[i], 1) > 0 &&
                       std::tolower(seq[i]) != 'm') i++;

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
                        if (seq[2] == '<')
						{
                            int button = 0;
                            const auto off1 = std::find(seq.begin() + 2, seq.end(), ';');
                            const auto off2 = std::find(off1 + 1, seq.end(), ';');
                            const auto off3 = std::find(off2 + 1, seq.end(), ';');
                            std::from_chars<int>(seq.data() + 3, off1, button);
                            std::from_chars<int>(off1 + 1, off2, mouse_pos_.first);
                            std::from_chars<int>(off2 + 1, off3, mouse_pos_.second);
                            mouse_pos_.first--;
                            mouse_pos_.second--;

                            const auto is_release = (seq[i] == 'm');
                            const auto is_motion  = (button & 32) != 0;
                            const auto is_wheel   = (button & 64) != 0;

                            if (is_wheel)
                            {
                                if ((button & 1) == 0) _setm(MouseKey::ScrollUp);
                                else                   _setm(MouseKey::ScrollDown);
                            }
                            else
                            {
                                switch (button & 3)
                                {
                                case 0: is_release ? _relm(MouseKey::Left)   : _setm(MouseKey::Left);   break;
                                case 1: is_release ? _relm(MouseKey::Middle) : _setm(MouseKey::Middle); break;
                                case 2: is_release ? _relm(MouseKey::Right)  : _setm(MouseKey::Right);  break;
                                }
                            }

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

	void UnixTerminalInput::mask_interrupts()
	{
		std::unique_lock lock(mtx_);

		struct termios io;
		tcgetattr(STDIN_FILENO, &io);
		io.c_lflag &= ~ISIG;
		tcsetattr(STDIN_FILENO, TCSANOW, &io);
	}
	void UnixTerminalInput::unmask_interrupts()
	{
		std::unique_lock lock(mtx_);

		struct termios io;
		tcgetattr(STDIN_FILENO, &io);
		io.c_lflag |= ISIG;
		tcsetattr(STDIN_FILENO, TCSANOW, &io);
	}

	//
	// Output
	//

	unsigned char UnixTerminalOutput::_u8component_to_sixel(unsigned int value) noexcept
	{
		return (value * 63) / 255;
	}
	UnixTerminalOutput::sixel_type UnixTerminalOutput::_generate_sixel(unsigned char r, unsigned char g, unsigned char b) noexcept
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
	std::vector<unsigned char> UnixTerminalOutput::_image_to_sixel(const ImageInstance::data& data, std::size_t width, std::size_t height)
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

	UnixTerminalOutput::position_type UnixTerminalOutput::_generate_position(int x, int y, bool skip) noexcept
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
	UnixTerminalOutput::color_type UnixTerminalOutput::_generate_color(const Pixel& px, bool force) noexcept
	{
        using buffer = std::array<char, max_color_len_>;

        constexpr std::array<const char*, 7> style_on  = { "1", "4", "3", "9", "5", "7", "8" };
        constexpr std::array<const char*, 7> style_off = { "22", "24", "23", "29", "25", "27", "28" };
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
                    if (ns)
                    {
                        *(ptr++) = style_on[i][0];
                    }
                    else
                    {
                        *(ptr++) = style_off[i][0];
                        *(ptr++) = style_off[i][1];
                    }
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

    void UnixTerminalOutput::_push(const Pixel& px) noexcept
    {
#       if D2_LOCALE_MODE == 32
            if (global_extended_code_page.is_extended(px.v))
            {
                const auto ext = global_extended_code_page.read(px.v);
                out_.insert(out_.end(), ext.begin(), ext.end());
            }
            else
                out_.push_back(px.v);
#       else
            out_.push_back(px.v);
#       endif
    }
	void UnixTerminalOutput::_write(std::span<const unsigned char> buffer) noexcept
	{
		::write(STDOUT_FILENO, buffer.data(), buffer.size());
	}
	void UnixTerminalOutput::_write(std::span<const char> buffer) noexcept
	{
		::write(STDOUT_FILENO, buffer.data(), buffer.size());
	}

	std::chrono::microseconds UnixTerminalOutput::frame_time() const noexcept
	{
		return frame_time_;
	}
	std::size_t UnixTerminalOutput::buffer_size() const noexcept
	{
		return buffer_size_;
	}

	UnixTerminalOutput::ImageInstance::ptr UnixTerminalOutput::make_image(ImageInstance::data data, std::size_t width, std::size_t height)
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
	void UnixTerminalOutput::release_image(ImageInstance::ptr ptr)
	{
		std::unique_lock lock(mtx_);
		images_.erase(ptr->code());
	}

	void UnixTerminalOutput::write(std::span<const Pixel> buffer, std::size_t width, std::size_t height)
	{
		std::unique_lock lock(mtx_);

		const auto beg = std::chrono::high_resolution_clock::now();

		track_style_ = 0x00;
		track_foreground_ = PixelForeground(255, 255, 255);
		track_background_ = PixelBackground(0, 0, 0);

		out_.reserve(
			(buffer.size()) * sizeof(value_type) +
			(height + 1) +
			max_color_len_ * 2
		);

		const auto compressed = PixelBuffer::rle_pack(buffer);

		bool corner_check = false;
		if (!swapframe_.empty())
		{
			PixelBuffer::RleIterator it(swapframe_);
			corner_check =
				it.value() != buffer[0] &&
				swapframe_.back() != buffer.back();
		}

		// Clean redraw
		if (buffer.size() != pbuffer_size_ || corner_check)
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
                    _push(*it);
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
			PixelBuffer::RleIterator pv_it(swapframe_);
			for (auto it = buffer.begin(); it != buffer.end();)
			{
				if (const auto& px = *it;
					px != pv_it.value())
				{
					const auto idx = it - buffer.begin();
					const auto x = idx % width;
					const auto y = idx / width;
					const auto [ pos, plen ] = _generate_position(x, y, linear && sequential);
					const auto [ code, len ] = _generate_color(px, !sequential);

					auto sit = it;
					while (sit != buffer.end() &&
						   *sit != pv_it.value() &&
						   px.compare_colors(*sit))
					{
						pv_it.increment();
						++sit;
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
                        _push(*it);
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
					pv_it.increment();
					++it;
				}
			}
		}

		if (!out_.empty())
		{
			constexpr auto chunk = 1024ull;
			constexpr auto frame_upper_bound = 56ull * 1024ull;
			D2_EXPECT(out_.size() <= frame_upper_bound)

			const auto len = (out_.size() + chunk - 1) / chunk;
			for (std::size_t i = 0; i < len; i++)
			{
				const auto idx = i * chunk;
				const auto rem = out_.size() - idx;
				_write({ out_.begin() + idx, out_.begin() + idx + std::min(chunk, rem) });
			}

			pbuffer_size_ = width * height;
			swapframe_.clear();
			swapframe_.insert(swapframe_.end(), compressed.begin(), compressed.end());
		}
		buffer_size_ = out_.size();
		out_.clear();

		frame_time_ = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::high_resolution_clock::now() - beg
		);
	}
}
