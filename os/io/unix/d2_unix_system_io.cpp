#include "os/io/unix/d2_unix_system_io.hpp"
#include <core/platform/d2_text_layout.hpp>
#include <core/io/d2_context.hpp>

#include <absl/strings/escaping.h>
#include <any>
#include <atomic>
#include <charconv>
#include <chrono>
#include <fcntl.h>
#include <mutex>
#include <optional>
#include <span>
#include <system_error>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <thread>
#include <unistd.h>

namespace d2::sys
{
    //
    // Worker
    //

    bool UnixWorker::_is_current_thread_impl() const noexcept
    {
        return std::this_thread::get_id() == _main_thread;
    }
    bool UnixWorker::_try_accept_impl(mt::Task& task) noexcept
    {
        if (!_tasks.try_enqueue(task))
            return false;
        _wake();
        return true;
    }
    void UnixWorker::_accept_impl(mt::Task task)
    {
        _tasks.enqueue(std::move(task));
        _wake();
    }
    void UnixWorker::_ping_impl()
    {
        _wake();
    }
    void UnixWorker::_start_impl()
    {
        int fds[2];
        if (::pipe2(fds, O_CLOEXEC | O_NONBLOCK) != 0)
            throw std::system_error(errno, std::generic_category(), "pipe2");
        _wake_read = fds[0];
        _wake_write = fds[1];
        _main_thread = std::this_thread::get_id();
    }
    void UnixWorker::_stop_impl() noexcept
    {
        _process_tasks();
        _wake();
        _closefd(_wake_read);
        _closefd(_wake_write);
    }
    mt::Worker::ptr UnixWorker::_clone_impl() const
    {
        return Worker::make<UnixWorker>();
    }
    void UnixWorker::_closefd(int& fd) noexcept
    {
        if (fd >= 0)
        {
            ::close(fd);
            fd = -1;
        }
    }
    int UnixWorker::_poll(std::chrono::steady_clock::time_point deadline) noexcept
    {
        if (deadline == std::chrono::steady_clock::time_point::max())
            return -1;
        const auto now = std::chrono::steady_clock::now();
        if (deadline <= now)
            return 0;
        const auto remaining =
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        if (remaining.count() <= 0)
            return 0;
        if (remaining.count() > INT_MAX)
            return INT_MAX;
        return static_cast<int>(remaining.count());
    }
    void UnixWorker::_wake() noexcept
    {
        std::uint8_t byte = 0;
        while (true)
        {
            const auto written = ::write(_wake_write, &byte, sizeof(byte));
            if (written == sizeof(byte))
                return;
            if (written < 0)
            {
                if (errno == EINTR)
                    continue;
                return;
            }
        }
    }
    void UnixWorker::_drain() noexcept
    {
        std::array<std::uint8_t, 256> buffer{};
        while (true)
        {
            const auto read_count = ::read(_wake_read, buffer.data(), buffer.size());
            if (read_count > 0)
                continue;
            if (read_count == 0)
                return;
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            return;
        }
    }
    void UnixWorker::_process_tasks()
    {
        std::array<mt::Task, 6> tasks;
        while (true)
        {
            std::size_t count = 0;
            while (count < tasks.size())
            {
                if (!_tasks.try_dequeue(tasks[count]))
                    break;
                ++count;
            }
            if (count == 0)
                return;
            for (std::size_t i = 0; i < count; ++i)
                _process(tasks[i]);
        }
    }
    void UnixWorker::wait(std::chrono::steady_clock::time_point deadline) noexcept
    {
        _process_tasks();

        pollfd fds[2]{};

        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        fds[1].fd = _wake_read;
        fds[1].events = POLLIN;

        const auto timeout_ms = _poll(std::min(deadline, this->deadline()));
        {
            while (true)
            {
                const auto rc = ::poll(fds, 2, timeout_ms);
                if (rc < 0)
                {
                    if (errno == EINTR)
                        break;
                }
                break;
            }
        }

        if (fds[1].revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL))
        {
            _drain();
            _process_tasks();
        }
        _tick();
    }

    //
    // Input
    //

    void UnixTerminalInput::_enable_raw_mode()
    {
        termios io;
        tcgetattr(STDIN_FILENO, &_restore_termios);
        io = _restore_termios;
        io.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
        io.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
        io.c_cflag |= CS8;
        io.c_cc[VMIN] = 0;
        io.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &io);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

        std::cout << "\033[?25l"
                  << "\033[?1003h" // any-motion tracking
                  << "\033[?1006h" // SGR extended coordinates
                  << std::flush;
    }
    void UnixTerminalInput::_disable_raw_mode()
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &_restore_termios);
        std::cout << "\033[?1006l"
                  << "\033[?1003l"
                  << "\033[?25h" << std::flush;
    }

    void UnixTerminalInput::_lock_frame_impl()
    {
        _mtx.lock_shared();
    }
    void UnixTerminalInput::_unlock_frame_impl()
    {
        _mtx.unlock_shared();
    }

    in::InputFrame::ptr UnixTerminalInput::_frame_impl()
    {
        return _in;
    }

    void UnixTerminalInput::_begincycle_impl()
    {
        std::unique_lock lock(_mtx);

        auto frame = in::internal::InputFrameView::from(_in.get());
        frame.swap();

        // Update screen size
        {
            struct winsize ws;
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
            {
                frame.set_screen_size({0, 0});
            }
            else
            {
                frame.set_screen_size({ws.ws_col, ws.ws_row});
            }
            frame.set_screen_capacity(_in->screen_size());
        }
        // Key input
        {
            string text;

            static std::string pending;
            static bool esc_waiting = false;
            static std::chrono::steady_clock::time_point esc_started{};

            constexpr auto esc_delay = std::chrono::milliseconds{25};
            const auto now = std::chrono::steady_clock::now();
            const auto is_ready = []
            {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(STDIN_FILENO, &fds);

                timeval timeout{};
                timeout.tv_sec = 0;
                timeout.tv_usec = 0;

                const auto res = ::select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &timeout);
                return res > 0 && FD_ISSET(STDIN_FILENO, &fds);
            };
            const auto read_av = [&]
            {
                while (is_ready())
                {
                    char buf[128];
                    const auto n = ::read(STDIN_FILENO, buf, sizeof(buf));

                    if (n > 0)
                    {
                        pending.append(buf, static_cast<std::size_t>(n));
                    }
                    else if (n < 0 && errno == EINTR)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            };

            const auto is_csi_final = [](unsigned char ch) { return ch >= 0x40 && ch <= 0x7e; };
            const auto parse_int = [](std::string_view sv, int& out)
            {
                if (sv.empty())
                    return false;

                const auto* first = sv.data();
                const auto* last = sv.data() + sv.size();

                const auto res = std::from_chars(first, last, out);
                return res.ec == std::errc{} && res.ptr == last;
            };
            const auto parse_params = [&](std::string_view sv)
            {
                std::vector<int> res;

                while (true)
                {
                    const auto off = sv.find(';');
                    const auto part = sv.substr(0, off);

                    int value = 0;
                    if (!part.empty())
                        parse_int(part, value);

                    res.push_back(value);

                    if (off == std::string_view::npos)
                        break;

                    sv.remove_prefix(off + 1);
                }

                if (res.size() == 1 && res[0] == 0 && sv.empty())
                    res.clear();

                return res;
            };
            const auto pulse_key = [&](char ch)
            {
                if (ch >= in::keymin() && ch <= in::keymax())
                {
                    if (ch >= 'A' && ch <= 'Z')
                        frame.pulse(in::special::Shift);

                    frame.pulse(in::key(ch));
                }
            };
            const auto pulse_key_raw = [&](char ch)
            {
                if (ch >= in::keymin() && ch <= in::keymax())
                    frame.pulse(in::key(ch));
            };
            const auto pulse_control = [&](char ch)
            {
                frame.pulse(in::special::LeftControl);
                pulse_key_raw(ch);
            };
            const auto apply_csi_modifier = [&](int mod)
            {
                if (mod <= 1)
                    return;

                const auto bits = mod - 1;
                if ((bits & 1) != 0)
                    frame.pulse(in::special::Shift);
                if ((bits & 2) != 0)
                    frame.pulse(in::special::LeftAlt);
                if ((bits & 4) != 0)
                    frame.pulse(in::special::LeftControl);
            };
            const auto pulse_fn_key = [&](int n) { frame.pulse(in::fn(n) + 1); };
            const auto parse_sgr_mouse = [&](std::string_view body, char final)
            {
                if (body.empty() || body[0] != '<')
                    return;

                body.remove_prefix(1);

                const auto off1 = body.find(';');
                if (off1 == std::string_view::npos)
                    return;

                const auto off2 = body.find(';', off1 + 1);
                if (off2 == std::string_view::npos)
                    return;

                int button = 0;
                int x = 0;
                int y = 0;

                if (!parse_int(body.substr(0, off1), button))
                    return;
                if (!parse_int(body.substr(off1 + 1, off2 - off1 - 1), x))
                    return;
                if (!parse_int(body.substr(off2 + 1), y))
                    return;

                Position mpos{x - 1, y - 1};

                const auto is_release = final == 'm';
                const auto is_wheel = (button & 64) != 0;

                if ((button & 4) != 0)
                    frame.pulse(in::special::Shift);
                if ((button & 8) != 0)
                    frame.pulse(in::special::LeftAlt);
                if ((button & 16) != 0)
                    frame.pulse(in::special::LeftControl);

                if (is_wheel)
                {
                    Position scroll_delta{0, 0};

                    if ((button & 1) == 0)
                        scroll_delta.y = 1;
                    else
                        scroll_delta.y = -1;

                    frame.set_scroll_delta(scroll_delta);
                }
                else
                {
                    switch (button & 3)
                    {
                    case 0:
                        is_release ? frame.set(in::mouse::Left, false) : frame.set(in::mouse::Left);
                        break;
                    case 1:
                        is_release ? frame.set(in::mouse::Middle, false)
                                   : frame.set(in::mouse::Middle);
                        break;
                    case 2:
                        is_release ? frame.set(in::mouse::Right, false)
                                   : frame.set(in::mouse::Right);
                        break;
                    }
                }

                frame.set_mouse_position(mpos);
            };
            const auto parse_csi = [&](std::string_view seq)
            {
                if (seq.size() < 3)
                    return;

                const auto final = seq.back();
                const auto body = seq.substr(2, seq.size() - 3);

                if ((final == 'M' || final == 'm') && !body.empty() && body[0] == '<')
                {
                    parse_sgr_mouse(body, final);
                    return;
                }
                if (final == 'Z')
                {
                    frame.pulse(in::special::Shift);
                    frame.pulse(in::special::Tab);
                    return;
                }

                const auto params = parse_params(body);
                if (params.size() >= 2)
                    apply_csi_modifier(params[1]);

                switch (final)
                {
                case 'A':
                    frame.pulse(in::special::ArrowUp);
                    break;
                case 'B':
                    frame.pulse(in::special::ArrowDown);
                    break;
                case 'C':
                    frame.pulse(in::special::ArrowRight);
                    break;
                case 'D':
                    frame.pulse(in::special::ArrowLeft);
                    break;
                case 'H':
                    frame.pulse(in::special::Home);
                    break;
                case 'F':
                    frame.pulse(in::special::End);
                    break;
                case 'P':
                    pulse_fn_key(0);
                    break;
                case 'Q':
                    pulse_fn_key(1);
                    break;
                case 'R':
                    pulse_fn_key(2);
                    break;
                case 'S':
                    pulse_fn_key(3);
                    break;

                case '~':
                {
                    if (params.empty())
                        break;
                    switch (params[0])
                    {
                    case 1:
                    case 7:
                        frame.pulse(in::special::Home);
                        break;
                    case 4:
                    case 8:
                        frame.pulse(in::special::End);
                        break;
                    case 3:
                        frame.pulse(in::special::Delete);
                        break;
                    case 15:
                        pulse_fn_key(4);
                        break;
                    case 17:
                        pulse_fn_key(5);
                        break;
                    case 18:
                        pulse_fn_key(6);
                        break;
                    case 19:
                        pulse_fn_key(7);
                        break;
                    case 20:
                        pulse_fn_key(8);
                        break;
                    case 21:
                        pulse_fn_key(9);
                        break;
                    case 23:
                        pulse_fn_key(10);
                        break;
                    case 24:
                        pulse_fn_key(11);
                        break;
                    }
                    break;
                }
                }
            };
            const auto parse_ss3 = [&](std::string_view seq)
            {
                if (seq.size() < 3)
                    return;

                const auto final = seq.back();
                const auto body = seq.substr(2, seq.size() - 3);
                const auto params = parse_params(body);

                if (params.size() >= 2)
                    apply_csi_modifier(params[1]);
                switch (final)
                {
                case 'A':
                    frame.pulse(in::special::ArrowUp);
                    break;
                case 'B':
                    frame.pulse(in::special::ArrowDown);
                    break;
                case 'C':
                    frame.pulse(in::special::ArrowRight);
                    break;
                case 'D':
                    frame.pulse(in::special::ArrowLeft);
                    break;
                case 'H':
                    frame.pulse(in::special::Home);
                    break;
                case 'F':
                    frame.pulse(in::special::End);
                    break;
                case 'P':
                    pulse_fn_key(0);
                    break;
                case 'Q':
                    pulse_fn_key(1);
                    break;
                case 'R':
                    pulse_fn_key(2);
                    break;
                case 'S':
                    pulse_fn_key(3);
                    break;
                }
            };
            const auto should_wait_for_escape = [&]
            {
                if (!esc_waiting)
                {
                    esc_waiting = true;
                    esc_started = now;
                    return true;
                }
                return now - esc_started < esc_delay;
            };

            read_av();

            std::size_t pos = 0;
            while (pos < pending.size())
            {
                const auto ch = static_cast<unsigned char>(pending[pos]);

                // Enter
                if (ch == '\n' || ch == '\r')
                {
                    frame.pulse(in::special::Enter);
                    pos++;
                }
                // Tabulator exterminans
                else if (ch == '\t')
                {
                    frame.pulse(in::special::Tab);
                    pos++;
                }
                // Backspace
                else if (ch == 127 || ch == '\b')
                {
                    frame.pulse(in::special::Backspace);
                    pos++;
                }
                // Escape / sequences
                else if (ch == '\x1b')
                {
                    if (pos + 1 >= pending.size())
                    {
                        if (should_wait_for_escape())
                            break;

                        frame.pulse(in::special::Escape);
                        esc_waiting = false;
                        pos++;
                        continue;
                    }
                    esc_waiting = false;

                    const auto next = pending[pos + 1];
                    if (next == '[')
                    {
                        std::size_t end = pos + 2;
                        while (end < pending.size() &&
                               !is_csi_final(static_cast<unsigned char>(pending[end])))
                        {
                            end++;
                        }
                        if (end >= pending.size())
                        {
                            if (should_wait_for_escape())
                                break;

                            frame.pulse(in::special::Escape);
                            esc_waiting = false;
                            pos++;
                            continue;
                        }
                        parse_csi(std::string_view{pending.data() + pos, end - pos + 1});
                        pos = end + 1;
                    }
                    else if (next == 'O')
                    {
                        std::size_t end = pos + 2;
                        while (end < pending.size() &&
                               !is_csi_final(static_cast<unsigned char>(pending[end])))
                        {
                            end++;
                        }
                        if (end >= pending.size())
                        {
                            if (should_wait_for_escape())
                                break;

                            frame.pulse(in::special::Escape);
                            esc_waiting = false;
                            pos++;
                            continue;
                        }
                        parse_ss3(std::string_view{pending.data() + pos, end - pos + 1});
                        pos = end + 1;
                    }
                    else
                    {
                        frame.pulse(in::special::LeftAlt);
                        pulse_key(next);
                        pos += 2;
                    }
                }
                // Control
                else if (ch >= 1 && ch <= 26)
                {
                    const auto res = static_cast<char>('A' + (ch - 1));
                    pulse_control(res);
                    pos++;
                }
                else if (ch == 0)
                {
                    // Ctrl+@ / Ctrl+Space
                    frame.pulse(in::special::LeftControl);
                    pulse_key_raw(' ');
                    pos++;
                }
                else if (ch >= 28 && ch <= 31)
                {
                    // Ctrl+\, Ctrl+], Ctrl+^, Ctrl+_
                    static constexpr char ctrl_map[] = {'\\', ']', '^', '_'};
                    pulse_control(ctrl_map[ch - 28]);
                    pos++;
                }
                // Normal keys :(
                else
                {
                    const auto c = static_cast<char>(ch);
                    // Sequence
                    text.push_back(c);
                    // Key
                    pulse_key(c);
                    pos++;
                }
            }
            pending.erase(0, pos);

            frame.set_text(std::move(text));
        }
    }
    void UnixTerminalInput::_endcycle_impl() {}

    UnixTerminalInput::Status UnixTerminalInput::_load_impl()
    {
        _enable_raw_mode();
        return Status::Ok;
    }
    UnixTerminalInput::Status UnixTerminalInput::_unload_impl()
    {
        _disable_raw_mode();
        return Status::Ok;
    }

    MainWorker::ptr UnixTerminalInput::_worker_impl()
    {
        return mt::Worker::make<UnixWorker>();
    }

    //
    // Output
    //

    UnixTerminalOutput::position_type
    UnixTerminalOutput::_generate_position(int x, int y, bool skip)
    {
        using buffer = std::array<char, _max_pos_len>;

        if (skip)
            return std::make_pair(buffer(), 0);

        buffer result{"\x1b["};

        const auto [ptr1, _1] = std::to_chars(result.begin() + 2, result.end(), y + 1);
        const auto [ptr2, _2] = std::to_chars(ptr1 + 1, result.end(), x + 1);

        *ptr1 = ';';
        *ptr2 = 'H';

        return std::make_pair(result, int(ptr2 - result.begin() + 1));
    }
    UnixTerminalOutput::color_type UnixTerminalOutput::_generate_color(const pixel& cell, bool force)
    {
        std::array<char, _max_color_len> code{};
        auto ptr = code.begin();
        auto append = [&](std::string_view value) { ptr = std::copy(value.begin(), value.end(), ptr); };
        const auto style = cell.style & ~px::style::Reserved;
        const auto background = force || cell.r != _track_background.r ||
                                cell.g != _track_background.g || cell.b != _track_background.b;
        const auto foreground = force || cell.rf != _track_foreground.r ||
                                cell.gf != _track_foreground.g || cell.bf != _track_foreground.b;
        if (!force && style == _track_style && !background && !foreground)
            return {code, 0};
        append("\033[");
        constexpr std::array<std::string_view, 7> on{"1", "4", "3", "9", "5", "7", "8"};
        constexpr std::array<std::string_view, 7> off{"22", "24", "23", "29", "25", "27", "28"};
        for (std::size_t i = 0; i < on.size(); ++i)
        {
            const auto bit = px::style(1 << i);
            if (force || (style & bit) != (_track_style & bit))
            {
                append(bool(style & bit) ? on[i] : off[i]);
                append(";");
            }
        }
        auto rgb = [&](unsigned char r, unsigned char g, unsigned char b)
        {
            for (auto value : {r, g, b})
            {
                ptr = std::to_chars(ptr, code.end(), value).ptr;
                *ptr++ = ';';
            }
        };
        if (background)
        {
            append("48;2;");
            rgb(cell.r, cell.g, cell.b);
        }
        if (foreground)
        {
            append("38;2;");
            rgb(cell.rf, cell.gf, cell.bf);
        }
        *(ptr - 1) = 'm';
        _track_style = style;
        _track_background = cell.as_background();
        _track_foreground = cell.as_foreground();
        return {code, static_cast<std::size_t>(ptr - code.begin())};
    }

    void UnixTerminalOutput::_push(const pixel& px)
    {
        if (px.v == '\t')
        {
            _out.push_back(' ');
            return;
        }

#if D2_LOCALE_MODE == UNICODE
        if (global_extended_code_page.is_extended(px.v))
        {
            const auto ext = global_extended_code_page.read(px.v);
            _out.insert(_out.end(), ext.begin(), ext.end());
        }
        else
        {
            _out.push_back(px.v);
        }
#else
        _out.push_back(px.v);
#endif
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
        return _frame_time;
    }

    void UnixTerminalOutput::clear_images()
    {
        std::vector<pixel> cells;
        std::size_t width, height;
        {
            std::unique_lock lock(mtx_);
            if (!_active_images && _image_cleanup.empty()) return;
            cells = _sourceframe;
            width = _source_width; height = _source_height;
        }
        _render(cells, width, height, {}, true);
    }
    void UnixTerminalOutput::write(std::span<const pixel> buffer, std::size_t width, std::size_t height)
    {
        _render(buffer, width, height, {}, false);
    }
    void UnixTerminalOutput::write(std::span<const pixel> buffer, std::size_t width, std::size_t height,
                                    const image::Scene& images)
    {
        _render(buffer, width, height, images, false);
    }
    void UnixTerminalOutput::_render(std::span<const pixel> buffer, std::size_t width, std::size_t height,
                                      const image::Scene& images, bool suppress_images)
    {
        std::unique_lock lock(mtx_);
        if ((height && width > std::numeric_limits<std::size_t>::max() / height) ||
            buffer.size() != width * height || width > INT_MAX || height > INT_MAX)
            throw std::invalid_argument("Terminal frame dimensions");
        const auto beg = std::chrono::high_resolution_clock::now();
        auto full = width != _pbuffer_width || height != _pbuffer_height ||
                    buffer.size() != _pbuffer_size;
        _sourceframe.assign(buffer.begin(), buffer.end());
        _source_width = width; _source_height = height;
        _displayframe.assign(buffer.begin(), buffer.end());
        std::shared_ptr<SystemImages> renderer;
        if (!suppress_images)
            if (auto ctx = context())
                if (auto module = ctx->sys_if<SystemImages>(); module)
                    renderer = module.ptr();
        SystemImages::Plan plan;
        if (renderer != _active_images)
        {
            if (_active_images)
            {
                plan = _active_images->reset();
                _image_cleanup += plan.before;
                plan.before.clear();
            }
            _active_images = renderer;
            full = true;
        }
        if (renderer)
        {
            winsize size{};
            BoundingBox metrics{};
            if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col && size.ws_row)
                metrics = {size.ws_xpixel / size.ws_col, size.ws_ypixel / size.ws_row};
            try
            {
                auto next = renderer->prepare(_displayframe, {int(width), int(height)}, images, metrics);
                plan.before += next.before;
                plan.after = std::move(next.after);
                plan.repaint |= next.repaint;
            }
            catch (...)
            {
                renderer->invalidate();
                _pbuffer_size = _pbuffer_width = _pbuffer_height = 0;
                throw;
            }
        }
        full |= plan.repaint;
        _image_cleanup += plan.before;
        _out.insert(_out.end(), _image_cleanup.begin(), _image_cleanup.end());
        for (std::size_t i = 0; i < _displayframe.size(); ++i)
        {
            _displayframe[i] = text::display_cell(_displayframe, i, width);
            if (_displayframe[i].v == char(127)) _displayframe[i].v = ' ';
        }
        buffer = _displayframe;
        const auto compressed = PixelBuffer::rle_pack(buffer);
        std::vector<bool> dirty(buffer.size(), full);
        if (!full)
        {
            PixelBuffer::RleIterator previous(_swapframe);
            for (std::size_t i = 0; i < buffer.size(); ++i, previous.increment())
                dirty[i] = buffer[i] != previous.value();
        }
        bool first = true;
        std::size_t next = buffer.size();
        auto changed = [&](std::size_t i)
        {
            return dirty[i] || (i % width && dirty[i - 1]) ||
                   (i % width + 1 < width && dirty[i + 1]);
        };
        for (std::size_t i = 0; i < buffer.size(); ++i)
        {
            const auto& cell = buffer[i];
            if (changed(i) && !text::continuation(cell))
            {
                const auto [pos, plen] = _generate_position(i % width, i / width, next == i && i % width);
                _out.insert(_out.end(), pos.begin(), pos.begin() + plen);
                const auto [color, clen] = _generate_color(cell, first);
                _out.insert(_out.end(), color.begin(), color.begin() + clen);
                first = false;
                _push(cell);
                next = i + 1;
                if (i % width + 1 < width && text::continuation(buffer[i + 1])) ++next;
            }
        }
        _out.insert(_out.end(), plan.after.begin(), plan.after.end());

        if (!_out.empty())
        {
            constexpr std::size_t chunk = 1024;
            for (std::size_t idx = 0; idx < _out.size();)
            {
                const auto size = std::min(chunk, _out.size() - idx);
                std::size_t off = 0;
                while (off < size)
                {
                    const auto written = _write({_out.data() + idx + off, size - off});

                    if (written > 0)
                    {
                        off += static_cast<std::size_t>(written);
                        continue;
                    }

                    if (written == -1 && errno == EINTR)
                        continue;

                    if (written == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        pollfd descriptor{STDOUT_FILENO, POLLOUT, 0};
                        int ready;
                        do { ready = ::poll(&descriptor, 1, -1); } while (ready < 0 && errno == EINTR);
                        if (ready > 0 && (descriptor.revents & POLLOUT)) continue;
                        if (ready >= 0) errno = EIO;
                    }
                    const auto error = written < 0 ? errno : EIO;
                    if (_active_images) _active_images->invalidate();
                    _image_cleanup.insert(0, "\x1b\\");
                    _out.clear();
                    _swapframe.clear();
                    _pbuffer_size = _pbuffer_width = _pbuffer_height = 0;
                    throw std::system_error(error, std::generic_category(), "Terminal write failed");
                }
                idx += size;
            }

        }
        _pbuffer_size = width * height;
        _pbuffer_width = width;
        _pbuffer_height = height;
        _swapframe.assign(compressed.begin(), compressed.end());
        _buffer_size = _out.size();
        _image_cleanup.clear();
        _out.clear();

        _frame_time = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - beg
        );
    }

    std::size_t UnixTerminalOutput::delta_size()
    {
        return _buffer_size;
    }
    std::size_t UnixTerminalOutput::swapframe_size()
    {
        return _swapframe.size();
    }
} // namespace d2::sys
