#pragma once

#include <any>
#include <shared_mutex>

#include <d2_locale.hpp>
#include <d2_module.hpp>
#include <mods/d2_core.hpp>

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
        virtual void _accept_impl(mt::Task task) noexcept override;
        virtual void _ping_impl() override;
        virtual void _start_impl() override;
        virtual void _stop_impl() noexcept override;
        virtual Worker::ptr _clone_impl() const override;
    private:
        std::atomic<std::thread::id> _main_thread{};
        mt::TaskRing<mt::Task, 16> _tasks{};
        int _wake_read{-1};
        int _wake_write{-1};
    private:
        static void _closefd(int& fd) noexcept;
        static int _poll(std::chrono::steady_clock::time_point deadline) noexcept;

        void _wake() noexcept;
        void _drain() noexcept;
        void _process_tasks();
    public:
        virtual void wait(std::chrono::steady_clock::time_point deadline) noexcept override;
    };

    class UnixTerminalInput : public SystemInput,
                              public ConcreteModule<UnixTerminalInput, Load::Immediate>
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
                               public ConcreteModule<UnixTerminalOutput, Load::Immediate>
    {
    private:
        static constexpr auto _max_color_len =
            std::string_view("\033[RX;RX;RX;RX;RX;RX;RX;m").size() +
            (std::string_view("38;2;255;255;255").size() * 2);
        static constexpr auto _max_pos_len = std::string_view("\x1b[XXXX;XXXXH").size();
        static constexpr auto _cls_code = std::string_view("\033[H");
    private:
        struct KittyImageState
        {
            std::uint32_t id{0};
            std::size_t width{0};
            std::size_t height{0};
        };
        using position_type =
            std::pair<std::array<char, UnixTerminalOutput::_max_pos_len>, std::size_t>;
        using color_type =
            std::pair<std::array<char, UnixTerminalOutput::_max_color_len>, std::size_t>;
    private:
        std::chrono::microseconds _frame_time{0};
        std::vector<unsigned char> _out{};
        std::vector<Pixel> _swapframe{};
        std::size_t _pbuffer_size{0};
        std::size_t _buffer_size{0};
        std::uint8_t _track_style{};
        PixelForeground _track_foreground{};
        PixelBackground _track_background{};

        std::mutex mtx_{};
        std::unordered_map<std::string, std::any> images_{};

        void _kitty_write_id(std::uint32_t id);
        void _kitty_write(const std::string& cmd);

        position_type _generate_position(int x, int y, bool skip = false);
        color_type _generate_color(const Pixel& px, bool force = false);

        void _push(const Pixel& px);
        int _write(std::span<const unsigned char> buffer);
        int _write(std::span<const char> buffer);

        void _release_image(std::any data);
    protected:
        virtual Status _load_impl() override
        {
            return Status::Ok;
        }
        virtual Status _unload_impl() override
        {
            for (decltype(auto) it : images_)
                _release_image(it.second);
            return Status::Ok;
        }
    public:
        using SystemOutput::SystemOutput;

        std::chrono::microseconds frame_time() const;

        virtual void load_image(const std::string& path, ImageInstance img) override;
        virtual void release_image(const std::string& path) override;
        virtual ImageInstance image_info(const std::string& path) override;
        virtual image image_id(const std::string& path) override;

        virtual void
        write(std::span<const Pixel> buffer, std::size_t width, std::size_t height) override;

        virtual std::size_t delta_size() override;
        virtual std::size_t swapframe_size() override;
    };
} // namespace d2::sys
