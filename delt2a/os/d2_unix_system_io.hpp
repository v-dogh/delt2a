#ifndef LINUX_IO_HANDLERS_HPP
#define LINUX_IO_HANDLERS_HPP

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
	private:
		using keyboard_keymap = std::bitset<SpecialKeyMax + 1>;
		using mouse_keymap = std::bitset<MouseKeyMax + 1>;
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
		std::array<keyboard_keymap, 2> keys_poll_{};
		std::array<mouse_keymap, 2> mouse_poll_{};

		string input_poll_{};
		termios restore_termios_{};

		keyboard_keymap& _c_kpoll() noexcept;
		keyboard_keymap& _p_kpoll() noexcept;
		mouse_keymap& _c_mpoll() noexcept;
		mouse_keymap& _p_mpoll() noexcept;

		void _set(unsigned int ch) noexcept;
		void _setm(MouseKey ch) noexcept;
		void _relm(MouseKey ch) noexcept;
		void _resetm() noexcept;

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

		virtual void _begincycle_impl() noexcept override;
		virtual void _endcycle_impl() noexcept override;
	public:
		UnixTerminalInput(std::weak_ptr<IOContext> ctx) : SystemInput(ctx)
		{
			_enable_raw_mode();
		}
		virtual ~UnixTerminalInput()
		{
			_disable_raw_mode();
		}

		void mask_interrupts();
		void unmask_interrupts();
	};
	class UnixTerminalOutput : public SystemOutput
	{
	private:		
		static constexpr auto max_color_len_ =
			std::string_view("\033[RX;RX;RX;RX;RX;RX;RX;m").size() +
			(std::string_view("38;2;255;255;255").size() * 2);
		static constexpr auto max_pos_len_ =
			std::string_view("\x1b[XXXX;XXXXH").size();
		static constexpr auto max_sixel_len_ = std::string_view("63;63;63m").size();
		static constexpr auto cls_code_ = std::string_view("\033[H");
	private:
		using sixel_type = std::pair<std::array<char, UnixTerminalOutput::max_sixel_len_>, std::size_t>;
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
			ImageInstance::id,
			std::pair<ImageInstance::wptr, std::vector<unsigned char>>>
		images_{};

		unsigned char _u8component_to_sixel(unsigned int value) noexcept;
		sixel_type _generate_sixel(unsigned char r, unsigned char g, unsigned char b) noexcept;
		std::vector<unsigned char> _image_to_sixel(const ImageInstance::data& data, std::size_t width, std::size_t height);

		position_type _generate_position(int x, int y, bool skip = false) noexcept;
		color_type _generate_color(const Pixel& px, bool force = false) noexcept;

        void _push(const Pixel& px) noexcept;
		void _write(std::span<const unsigned char> buffer) noexcept;
		void _write(std::span<const char> buffer) noexcept;
	public:
		using SystemOutput::SystemOutput;
		virtual ~UnixTerminalOutput() = default;

		std::chrono::microseconds frame_time() const noexcept;
		std::size_t buffer_size() const noexcept;

		virtual ImageInstance::ptr make_image(ImageInstance::data data, std::size_t width, std::size_t height) override;
		virtual void release_image(ImageInstance::ptr ptr) override;

		virtual void write(std::span<const Pixel> buffer, std::size_t width, std::size_t height) override;
	};
} // d2

namespace d2
{
	template<>
	struct OSConfig<std::size_t(OS::Unix)> : OSConfig<std::size_t(OS::Default)>
	{
		using input = sys::UnixTerminalInput;
		using output = sys::UnixTerminalOutput;
		using clipboard = sys::ext::LocalSystemClipboard;
	};
}

#endif // LINUX_IO_HANDLERS_HPP
