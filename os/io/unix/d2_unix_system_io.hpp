#pragma once
#include <core/mods/d2_images.hpp>

#include <any>
#include <shared_mutex>

#include <core/io/d2_module.hpp>
#include <core/mods/d2_core.hpp>
#include <core/platform/d2_locale.hpp>

#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

namespace d2::sys
{
    class UnixWorker : public MainWorker
    {
    protected:
        virtual bool _is_current_thread_impl() const noexcept override;
        virtual bool _try_accept_impl(mt::Task& task) noexcept override;
        virtual void _accept_impl(mt::Task task) override;
        virtual void _ping_impl() override;
        virtual void _start_impl() override;
        virtual void _stop_impl() noexcept override;
        virtual Worker::ptr _clone_impl() const override;
    private:
        std::atomic<std::thread::id> _main_thread{};
        mt::TaskQueue<mt::Task> _tasks{};
        int _wake_read{-1};
        int _wake_write{-1};
    private:
        static void _closefd(int& fd) noexcept;
        static int _poll(std::chrono::steady_clock::time_point deadline) noexcept;

        void _wake() noexcept;
        void _drain() noexcept;
        void _process_tasks();
    public:
        ~UnixWorker() override { stop(); }
        virtual void wait(std::chrono::steady_clock::time_point deadline) noexcept override;
    };

    class UnixTerminalInput : public SystemInput,
                              public ModuleImpl<UnixTerminalInput, Load::Immediate>
    {
    private:
        std::shared_mutex _mtx{};
        in::InputFrame::ptr _in{in::InputFrame::make()};

        termios _restore_termios{};

        void _enable_raw_mode();
        void _disable_raw_mode();
    protected:
        virtual void _lock_frame_impl() override;
        virtual void _unlock_frame_impl() override;

        virtual in::InputFrame::ptr _frame_impl() override;

        virtual void _begincycle_impl() override;
        virtual void _endcycle_impl() override;

        virtual Status _load_impl() override;
        virtual Status _unload_impl() override;
        virtual MainWorker::ptr _worker_impl() override;
    public:
        using SystemInput::SystemInput;
    };
    class UnixTerminalOutput : public SystemOutput,
                               public ModuleImpl<UnixTerminalOutput, Load::Immediate>
    {
    private:
        static constexpr std::size_t _max_color_len = 96;
        static constexpr auto _max_pos_len = std::string_view("\x1b[XXXXXXXXXX;XXXXXXXXXXH").size();
        static constexpr auto _cls_code = std::string_view("\033[H");
    private:
        using position_type =
            std::pair<std::array<char, UnixTerminalOutput::_max_pos_len>, std::size_t>;
        using color_type =
            std::pair<std::array<char, UnixTerminalOutput::_max_color_len>, std::size_t>;
    private:
        std::chrono::microseconds _frame_time{0};
        std::vector<unsigned char> _out{};
        std::vector<pixel> _swapframe{};
        std::vector<pixel> _displayframe{};
        std::size_t _pbuffer_width{0}, _pbuffer_height{0};
        std::size_t _pbuffer_size{0};
        std::size_t _buffer_size{0};
        px::style _track_style{};
        px::foreground _track_foreground{};
        px::background _track_background{};

        std::mutex mtx_{};
        std::shared_ptr<SystemImages> _active_images{};
        std::string _image_cleanup{};
        std::vector<pixel> _sourceframe{};
        std::size_t _source_width{0}, _source_height{0};
        void _render(std::span<const pixel> buffer, std::size_t width, std::size_t height,
                     const image::Scene& images, bool suppress_images);

        position_type _generate_position(int x, int y, bool skip = false);
        color_type _generate_color(const pixel& px, bool force = false);

        void _push(const pixel& px);
        int _write(std::span<const unsigned char> buffer);
        int _write(std::span<const char> buffer);

    protected:
        virtual Status _load_impl() override
        {
            return Status::Ok;
        }
        virtual Status _unload_impl() override
        {
            clear_images();
            return Status::Ok;
        }
    public:
        using SystemOutput::SystemOutput;

        std::chrono::microseconds frame_time() const;

        void clear_images() override;
        void write(std::span<const pixel> buffer, std::size_t width, std::size_t height) override;
        void write(std::span<const pixel> buffer, std::size_t width, std::size_t height,
                   const image::Scene& images) override;

        virtual std::size_t delta_size() override;
        virtual std::size_t swapframe_size() override;
    };
} // namespace d2::sys
