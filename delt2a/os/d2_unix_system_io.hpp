#ifndef LINUX_IO_HANDLERS_HPP
#define LINUX_IO_HANDLERS_HPP

#include <any>
#include <termios.h>
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <bitset>

#include "../d2_io_handler.hpp"
#include "../d2_locale.hpp"

namespace d2::sys
{
	class UnixTerminalInput : public SystemInput
	{
    public:
        static constexpr auto tsafe = true;
	private:
		using keyboard_keymap = std::bitset<SpecialKeyMax + 1>;
		using mouse_keymap = std::bitset<MouseKeyMax + 1>;
	private:
        std::shared_mutex _mtx{};

        std::pair<std::size_t, std::size_t> _screen_size{ -1, -1 };
        std::pair<int, int> _mouse_pos{ 0, 0 };

		// c - current, p - previous
		// c for hold
		// c and !p for press
		// !c and p for release

        bool _had_screen_resize = true;
        bool _poll_swap_index = false;
        std::array<keyboard_keymap, 2> _keys_poll{};
        std::array<mouse_keymap, 2> _mouse_poll{};

        string _input_poll{};
        termios _restore_termios{};

		keyboard_keymap& _c_kpoll();
		keyboard_keymap& _p_kpoll();
		mouse_keymap& _c_mpoll();
		mouse_keymap& _p_mpoll();

		void _set(unsigned int ch);
		void _setm(MouseKey ch);
		void _relm(MouseKey ch);
		void _resetm();

		void _enable_raw_mode();
		void _disable_raw_mode();
	protected:
		virtual bool _is_pressed_impl(keytype ch, KeyMode mode) override;
		virtual const string& _key_sequence_input_impl() override;

		virtual bool _is_pressed_mouse_impl(MouseKey key, KeyMode mode) override;
		virtual std::pair<int, int> _mouse_position_impl() override;

		virtual std::pair<std::size_t, std::size_t> _screen_size_impl() override;
		virtual std::pair<std::size_t, std::size_t> _screen_capacity_impl() override;

		virtual bool _has_mouse_impl() override;

		virtual bool _is_key_sequence_input_impl() override;
		virtual bool _is_key_input_impl() override;
		virtual bool _is_mouse_input_impl() override;
		virtual bool _is_screen_resize_impl() override;

		virtual void _begincycle_impl() override;
		virtual void _endcycle_impl() override;

        virtual Status _load_impl() override
        {
            _enable_raw_mode();
            return Status::Ok;
        }
        virtual Status _unload_impl() override
        {
            _disable_raw_mode();
            return Status::Ok;
        }
	public:
        using SystemInput::SystemInput;

		void mask_interrupts();
		void unmask_interrupts();
	};
	class UnixTerminalOutput : public SystemOutput
	{
    public:
        static constexpr auto tsafe = false;
	private:		
		static constexpr auto max_color_len_ =
			std::string_view("\033[RX;RX;RX;RX;RX;RX;RX;m").size() +
			(std::string_view("38;2;255;255;255").size() * 2);
		static constexpr auto max_pos_len_ =
			std::string_view("\x1b[XXXX;XXXXH").size();
        static constexpr auto cls_code_ = std::string_view("\033[H");
	private:
        struct KittyImageState
        {
            std::uint32_t id{ 0 };
            std::size_t width{ 0 };
            std::size_t height{ 0 };
        };
		using position_type = std::pair<std::array<char, UnixTerminalOutput::max_pos_len_>, std::size_t>;
        using color_type =  std::pair<std::array<char, UnixTerminalOutput::max_color_len_>, std::size_t>;
    private:
		std::chrono::microseconds frame_time_{ 0 };
		std::vector<unsigned char> out_{};
		std::vector<Pixel> swapframe_{};
		std::size_t pbuffer_size_{ 0 };
		std::size_t buffer_size_{ 0 };
		std::uint8_t track_style_{};
		PixelForeground track_foreground_{};
		PixelBackground track_background_{};

		std::mutex mtx_{};
		std::unordered_map<
            std::string,
            std::any
        > images_{};

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
        virtual std::uint32_t image_id(const std::string& path) override;

		virtual void write(std::span<const Pixel> buffer, std::size_t width, std::size_t height) override;

        virtual std::size_t delta_size() override;
        virtual std::size_t swapframe_size() override;
	};
} // d2

#endif // LINUX_IO_HANDLERS_HPP
