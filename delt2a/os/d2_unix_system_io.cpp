#include "d2_unix_system_io.hpp"
#include <absl/strings/escaping.h>
#include <iostream>
#include <optional>
#include <charconv>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <any>
#include <span>
#include <atomic>

namespace d2::sys
{
	//
	// Input
	//

	UnixTerminalInput::keyboard_keymap& UnixTerminalInput::_c_kpoll()
	{
		return _keys_poll[_poll_swap_index];
	}
	UnixTerminalInput::keyboard_keymap& UnixTerminalInput::_p_kpoll()
	{
		return _keys_poll[!_poll_swap_index];
	}
	UnixTerminalInput::mouse_keymap& UnixTerminalInput::_c_mpoll()
	{
		return _mouse_poll[_poll_swap_index];
	}
	UnixTerminalInput::mouse_keymap& UnixTerminalInput::_p_mpoll()
	{
		return _mouse_poll[!_poll_swap_index];
	}

	void UnixTerminalInput::_set(unsigned int ch)
	{
		_c_kpoll().set(ch);
	}
	void UnixTerminalInput::_setm(MouseKey ch)
	{
		_c_mpoll().set(ch);
	}
	void UnixTerminalInput::_relm(MouseKey ch)
	{
		_c_mpoll().reset(ch);
	}
	void UnixTerminalInput::_resetm()
	{
		_c_mpoll().reset();
	}

	void UnixTerminalInput::_enable_raw_mode()
	{
		termios io;
		tcgetattr(STDIN_FILENO, &_restore_termios);
		io = _restore_termios;
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
		tcsetattr(STDIN_FILENO, TCSANOW, &_restore_termios);
        std::cout << "\033[?25h" << "\033[?1003l" << "\x1b[?1006l" << std::flush;
	}

	bool UnixTerminalInput::_is_pressed_impl(keytype ch, KeyMode mode)
	{
		std::shared_lock lock(_mtx);

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
		std::shared_lock lock(_mtx);
		return _input_poll;
	}

	bool UnixTerminalInput::_is_pressed_mouse_impl(MouseKey key, KeyMode mode)
	{
		std::shared_lock lock(_mtx);

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
		std::shared_lock lock(_mtx);
		return _mouse_pos;
	}

	std::pair<std::size_t, std::size_t> UnixTerminalInput::_screen_size_impl()
	{
		std::shared_lock lock(_mtx);
		return _screen_size;
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
		std::shared_lock lock(_mtx);
		return !_input_poll.empty();
	}
	bool UnixTerminalInput::_is_key_input_impl()
	{
		std::shared_lock lock(_mtx);
		return
			_c_kpoll().test(SpecialKeyMax) ||
			_p_kpoll().test(SpecialKeyMax);
	}
	bool UnixTerminalInput::_is_mouse_input_impl()
	{
		std::shared_lock lock(_mtx);
		return
			_c_mpoll().test(MouseKeyMax) ||
			_p_mpoll().test(MouseKeyMax);
	}
	bool UnixTerminalInput::_is_screen_resize_impl()
	{
		std::shared_lock lock(_mtx);
		return _had_screen_resize;
	}

	void UnixTerminalInput::_begincycle_impl() { }
	void UnixTerminalInput::_endcycle_impl()
	{
		std::unique_lock lock(_mtx);

		// Update screen size

		{
			struct winsize ws;
			if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
			{
				_screen_size = { 0, 0 };
				_had_screen_resize = true;
			}
			else
			{
				const std::pair<std::size_t, std::size_t> src = { ws.ws_col, ws.ws_row };
				_had_screen_resize =
					src.first != _screen_size.first ||
					src.second != _screen_size.second;
				_screen_size = src;
			}
		}

		// Clear state

		// Reset key state
		{
			_input_poll.clear();
			_p_kpoll().reset();
		}
		// Reset mouse state
		{
			_p_mpoll() = _c_mpoll();
			_p_mpoll().reset(MouseKey::ScrollDown);
			_p_mpoll().reset(MouseKey::ScrollUp);
		}
		// Swap
		_poll_swap_index = !_poll_swap_index;

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
                            std::from_chars<int>(off1 + 1, off2, _mouse_pos.first);
                            std::from_chars<int>(off2 + 1, off3, _mouse_pos.second);
                            _mouse_pos.first--;
                            _mouse_pos.second--;

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
				_input_poll.push_back(ch);

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
		std::unique_lock lock(_mtx);

		struct termios io;
		tcgetattr(STDIN_FILENO, &io);
		io.c_lflag &= ~ISIG;
		tcsetattr(STDIN_FILENO, TCSANOW, &io);
	}
	void UnixTerminalInput::unmask_interrupts()
	{
		std::unique_lock lock(_mtx);

		struct termios io;
		tcgetattr(STDIN_FILENO, &io);
		io.c_lflag |= ISIG;
		tcsetattr(STDIN_FILENO, TCSANOW, &io);
	}

	//
	// Output
	//

    std::uint32_t _kitty_id()
    {
        static std::uint32_t id = 1;
        return id++;
    }
    void UnixTerminalOutput::_kitty_write(const std::string& payload)
    {
        std::string seq;
        seq.reserve(7 + payload.size());
        seq += "\x1b_G";
        seq += payload;
        seq += "\x1b\\";
        ::write(STDOUT_FILENO, seq.data(), seq.size());
    }
    void UnixTerminalOutput::_kitty_write_id(std::uint32_t id)
    {
        auto sp = [](std::string_view str) -> std::span<const unsigned char> {
            return std::span(
                reinterpret_cast<const unsigned char*>(str.data()),
                str.size()
            );
        };

        const auto c1 = sp("\x1b_Ga=p,i=");
        const auto c3 = sp(",q=2\x1b\\");

        std::array<char, 10> id_out;
        const auto [ end, _ ] = std::to_chars(
            id_out.data(),
            id_out.data() + id_out.size(),
            id
        );
        const auto c2 = sp(std::string_view(id_out.data(), end));

        out_.insert(out_.end(), c1.begin(), c1.end());
        out_.insert(out_.end(), c2.begin(), c2.end());
        out_.insert(out_.end(), c3.begin(), c3.end());
    }
    std::optional<std::string> _make_shm(const void* data, std::size_t len)
    {
        static std::atomic<std::uint64_t> counter = 0;
        for (std::size_t attempt = 0; attempt < 64; ++attempt)
        {
            const auto name = std::format(
                "/tty-graphics-protocol-{}-{}",
                ::getpid(), ++counter
            );
            const auto fd = ::shm_open(name.c_str(),
                O_CREAT | O_EXCL | O_RDWR
#               ifdef O_CLOEXEC
                    | O_CLOEXEC
#               endif
                ,
                0600
            );
            if (fd < 0)
            {
                if (errno == EEXIST)
                    continue;
                return std::nullopt;
            }
            if (::ftruncate(fd, static_cast<off_t>(len)) != 0)
            {
                ::close(fd);
                ::shm_unlink(name.c_str());
                return std::nullopt;
            }

            void* p = ::mmap(nullptr, len, PROT_WRITE, MAP_SHARED, fd, 0);
            if (p == MAP_FAILED)
            {
                ::close(fd);
                ::shm_unlink(name.c_str());
                return std::nullopt;
            }
            std::memcpy(p, data, len);
            ::msync(p, len, MS_SYNC);
            ::munmap(p, len);
            ::close(fd);
            return name;
        }
        return std::nullopt;
    }

	UnixTerminalOutput::position_type UnixTerminalOutput::_generate_position(int x, int y, bool skip)
	{
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
	UnixTerminalOutput::color_type UnixTerminalOutput::_generate_color(const Pixel& px, bool force)
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

    void UnixTerminalOutput::_push(const Pixel& px)
    {
#       if D2_LOCALE_MODE == UNICODE
            if (global_extended_code_page.is_extended(px.v))
            {
                const auto ext = global_extended_code_page.read(px.v);
                out_.insert(out_.end(), ext.begin(), ext.end());
            }
            else
            {
                out_.push_back(px.v);
            }
#       else
            out_.push_back(px.v);
#       endif
    }
    int UnixTerminalOutput::_write(std::span<const unsigned char> buffer)
	{
        return ::write(STDOUT_FILENO, buffer.data(), buffer.size());
	}
    int UnixTerminalOutput::_write(std::span<const char> buffer)
	{
        return ::write(STDOUT_FILENO, buffer.data(), buffer.size());
    }

	std::chrono::microseconds UnixTerminalOutput::frame_time() const
	{
		return frame_time_;
	}

    void UnixTerminalOutput::_release_image(std::any data)
    {
        _kitty_write(std::format("a=d,d=I,i={};", std::any_cast<KittyImageState&>(data).id));
    }

    void UnixTerminalOutput::load_image(const std::string& path, ImageInstance img)
    {
        if (images_.contains(path))
            release_image(path);

        const auto fmt = static_cast<Format>(img.format());
        const auto bytes = img.read();
        if (bytes.empty())
            return;

        const auto* fkey =
            fmt == Format::PNG ?
                "f=100" : (fmt == Format::RGBA8 ? "f=32" : "f=24");
        std::optional<std::pair<int, int>> dims;
        if (fmt != Format::PNG)
        {
            dims = {
                img.width(),
                img.height()
            };
        }

        const auto shm_name = _make_shm(bytes.data(), bytes.size());
        if (!shm_name)
            return;

        const auto kid = _kitty_id();
        std::ostringstream out;
        out << std::format("a=t,q=2,i={},t=s,{}", kid, fkey);
        if (dims)
            out << std::format(",s={},v={}", dims->first, dims->second);
        out << std::format(",S={};", bytes.size());
        out << absl::Base64Escape(*shm_name);
        _kitty_write(out.str());

        images_.emplace(
            path,
            KittyImageState{
                .id = kid,
                .width = img.width(),
                .height = img.height()
            }
        );
    }
    void UnixTerminalOutput::release_image(const std::string& path)
    {
        auto it = images_.find(path);
        if (it != images_.end())
        {
            _release_image(it->second);
            images_.erase(it);
        }
    }
    UnixTerminalOutput::ImageInstance UnixTerminalOutput::image_info(const std::string& path)
    {
        const auto& state = std::any_cast<KittyImageState&>(images_.at(path));
        return ImageInstance({}, state.width, state.height);
    }
    SystemOutput::image UnixTerminalOutput::image_id(const std::string& path)
    {
        return std::any_cast<KittyImageState&>(images_.at(path)).id;
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

        // Move to zero
        {
            const auto [ pos, plen ] = _generate_position(0, 0, false);
            _write(pos);
        }

		// Clean redraw
        if (buffer.size() != pbuffer_size_ || corner_check)
		{
            std::size_t x = 0;
            std::size_t y = 0;
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
                    {
                        out_.push_back('\n');
                        y++;
                        x = 0;
                    }
                    else
                        x++;

                    // Image support
                    [[ unlikely ]] if (it->v == image_constant.v)
                    {
                        // Explicitly move cursor
                        const auto [ pos, plen ] = _generate_position(x + 1, y + 1, false);
                        out_.insert(out_.end(), pos.begin(), pos.begin() + plen);
                        ++it;
                        // Get ID
                        std::uint32_t id = 0;
                        id |= static_cast<std::uint32_t>(it->v); ++it;
                        id |= (static_cast<std::uint32_t>(it->v) << 1) & 0xF0; ++it;
                        id |= (static_cast<std::uint32_t>(it->v) << 2) & 0xF00; ++it;
                        id |= (static_cast<std::uint32_t>(it->v) << 3) & 0xF000; ++it;
                        _kitty_write_id(id);
                        for (std::size_t i = 0; i < 5; i++)
                            _push({});
                    }
                    else
                    {
                        _push(*it);
                        ++it;
                    }
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
                        // Image support
                        [[ unlikely ]] if (it->v == image_constant.v)
                        {
                            // Explicitly move cursor
                            const auto [ pos, plen ] = _generate_position(x + 1, y + 1, false);
                            out_.insert(out_.end(), pos.begin(), pos.begin() + plen);
                            ++it;
                            // Get ID
                            std::uint32_t id = 0;
                            id |= static_cast<std::uint32_t>(it->v); ++it;
                            id |= (static_cast<std::uint32_t>(it->v) << 1) & 0xF0; ++it;
                            id |= (static_cast<std::uint32_t>(it->v) << 2) & 0xF00; ++it;
                            id |= (static_cast<std::uint32_t>(it->v) << 3) & 0xF000; ++it;
                            _kitty_write_id(id);
                            for (std::size_t i = 0; i < 5; i++)
                                _push({});
                        }
                        else
                        {
                            _push(*it);
                            ++it;
                        }
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
            constexpr std::size_t chunk = 1024;
            for (std::size_t idx = 0; idx < out_.size(); )
            {
                const auto size = std::min(chunk, out_.size() - idx);
                std::size_t off = 0;
                while (off < size)
                {
                    off += _write({
                        out_.data() + idx + off,
                        size - off
                    });
                }
                idx += size;
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

    std::size_t UnixTerminalOutput::delta_size()
    {
        return buffer_size_;
    }
    std::size_t UnixTerminalOutput::swapframe_size()
    {
        return swapframe_.size();
    }
}
