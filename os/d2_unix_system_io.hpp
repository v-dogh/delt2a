#ifndef LINUX_IO_HANDLERS_HPP
#define LINUX_IO_HANDLERS_HPP

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
        virtual bool _try_accept_impl(mt::Task& task) noexcept override;
        virtual void _accept_impl(mt::Task task) noexcept override;
        virtual void _ping_impl() override;
        virtual void _start_impl() override;
        virtual void _stop_impl() noexcept override;
        virtual Worker::ptr _clone_impl() const override;
    private:
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

    class UnixTerminalInput
        : public SystemInput,
          public ConcreteModule<UnixTerminalInput, Access::TSafe, Load::Immediate>
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

        void mask_interrupts();
        void unmask_interrupts();
    };
    class UnixTerminalOutput
        : public SystemOutput,
          public ConcreteModule<UnixTerminalOutput, Access::TUnsafe, Load::Immediate>
    {
    private:
        static constexpr auto max_color_len_ =
            std::string_view("\033[RX;RX;RX;RX;RX;RX;RX;m").size() +
            (std::string_view("38;2;255;255;255").size() * 2);
        static constexpr auto max_pos_len_ = std::string_view("\x1b[XXXX;XXXXH").size();
        static constexpr auto cls_code_ = std::string_view("\033[H");
    private:
        struct KittyImageState
        {
            std::uint32_t id{0};
            std::size_t width{0};
            std::size_t height{0};
        };
        using position_type =
            std::pair<std::array<char, UnixTerminalOutput::max_pos_len_>, std::size_t>;
        using color_type =
            std::pair<std::array<char, UnixTerminalOutput::max_color_len_>, std::size_t>;
    private:
        std::chrono::microseconds frame_time_{0};
        std::vector<unsigned char> out_{};
        std::vector<Pixel> swapframe_{};
        std::size_t pbuffer_size_{0};
        std::size_t buffer_size_{0};
        std::uint8_t track_style_{};
        PixelForeground track_foreground_{};
        PixelBackground track_background_{};

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

#endif // LINUX_IO_HANDLERS_HPP
