#ifndef D2_PIXEL_HPP
#define D2_PIXEL_HPP

#include <functional>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>
#include "d2_locale.hpp"

namespace d2
{
	struct PixelBase;
	struct PixelBackground;
	struct PixelForeground;

	struct PixelBackground
	{
		using component = std::uint8_t;
		using scalar = std::uint32_t;

		component r{ 0 };
		component g{ 0 };
		component b{ 0 };
		component a{ 255 };

		static scalar to_scalar(const PixelBackground& px) noexcept;
		static PixelBackground from_scalar(const scalar& px) noexcept;

		PixelForeground extend(value_type v = ' ', component style = 0x00) const noexcept;
		PixelBackground alpha(float value) const noexcept;
		PixelBackground invert() const noexcept;
		PixelBackground mask(const PixelBackground& px, float progress = 0.5f) const noexcept;

		PixelForeground operator|(component flag) const noexcept;

		operator PixelBase() const noexcept;
		operator PixelForeground() const noexcept;

		PixelBackground& operator=(const PixelBackground&) = default;
		PixelBackground& operator=(const PixelForeground& copy) noexcept;
		PixelBackground& operator=(const PixelBase& copy) noexcept;

		auto operator<=>(const PixelBackground&) const = default;
	};
	struct PixelForeground
	{
		enum Style : std::uint8_t
		{
			Bold = 1 << 0,
			Underline = 1 << 1,
			Italic = 1 << 2,
			Strikethrough = 1 << 3,
			Blink = 1 << 4,
			Inverse = 1 << 5,
			Hidden = 1 << 6,
			Reserved = 1 << 7
		};

		using component = std::uint8_t;

		component r{ 255 };
		component g{ 255 };
		component b{ 255 };
		component a{ 255 };
		component style{ 0x00 };
		value_type v{ ' ' };

		PixelForeground alpha(float value) const noexcept;
		PixelForeground extend(value_type v, component style = 0x00) const noexcept;
		PixelForeground invert() const noexcept;
		PixelForeground mask(const PixelForeground& px, float progress = 0.5f) const noexcept;

		operator PixelBase() const noexcept;
		operator PixelBackground() const noexcept;

		PixelForeground operator|(component style) const noexcept;

		PixelForeground& operator=(const PixelForeground&) = default;
		PixelForeground& operator=(const PixelBackground& copy) noexcept;
		PixelForeground& operator=(const PixelBase& copy) noexcept;

		auto operator<=>(const PixelForeground&) const = default;
	};
	struct PixelBase
	{
		using Style = PixelForeground::Style;
		using value_type = ::d2::value_type;
		using component = std::uint8_t;

		component r{ 0 };
		component g{ 0 };
		component b{ 0 };
		component rf{ 255 };
		component gf{ 255 };
		component bf{ 255 };
		component a{ 255 };
		component af{ 255 };
		component style{ 0x00 };
		value_type v{ ' ' };

		static PixelBase combine(const PixelForeground& fg, const PixelBackground& bg) noexcept;
		static PixelBase interpolate(const PixelBase& from, const PixelBase& to, float progress) noexcept;
		static PixelBase cform(std::uint64_t len) noexcept;
		static std::size_t is_cform(const PixelBase& px) noexcept;

		void blend(const PixelBase& src) noexcept;

		bool compare_colors(const PixelBase& px) const noexcept;

		PixelBase alpha(float v, float vf) const;
		PixelBase falpha(float value) const;
		PixelBase balpha(float value) const;
		PixelBase extend(value_type ch) const noexcept;

		PixelBase mask(const PixelBase& px) const noexcept;
		PixelBase mask_foreground(const PixelBase& px) const noexcept;
		PixelBase mask_background(const PixelBase& px) const noexcept;

		PixelBase invert() const noexcept;

		operator PixelBackground() const noexcept;
		operator PixelForeground() const noexcept;

		PixelBase& operator=(const PixelBase&) = default;
		PixelBase& operator=(const PixelBackground& copy) noexcept;
		PixelBase& operator=(const PixelForeground& copy) noexcept;

		auto operator<=>(const PixelBase&) const = default;
	};

	// A single RGBAV value
	// RGBA standard color components and V locale independent value
	using Pixel = PixelBase;

	// Aliases
	namespace px
	{
		using style = Pixel::Style;
		using component = Pixel::component;
		using combined = Pixel;
		using background = PixelBackground;
		using foreground = PixelForeground;
	}

	// Handles a standard pixel buffer
	class PixelBuffer
	{
	public:
		enum class RleBreak
		{
			Continue,
			SkipRest,
			SkipLine
		};
		class View
		{
		private:
			PixelBuffer* buffer_{ nullptr };
			int xoff_{ 0 };
			int yoff_{ 0 };
			int width_{ 0 };
			int height_{ 0 };
		public:
			View() = default;
			View(std::nullptr_t) {}
			View(View&&) = default;
			View(const View&) = default;
			explicit View(PixelBuffer* ptr, int x, int y, int width, int height)
				: buffer_(ptr), xoff_(x), yoff_(y), width_(width), height_(height) {}
			explicit View(const View& ptr, int x, int y, int width, int height) :
				buffer_(ptr.buffer_),
				xoff_(x + ptr.xoff_), yoff_(y + ptr.yoff_),
				width_(width), height_(height)
			{}

			std::span<const Pixel> data() const noexcept;

			void clear() noexcept;
			void fill(Pixel px) noexcept;
			void fill(Pixel px, int x, int y, int width, int height) noexcept;

			bool compressed() const noexcept;
			bool empty() const noexcept;

			int width() const noexcept;
			int height() const noexcept;
			int xpos() const noexcept;
			int ypos() const noexcept;

			const Pixel& at(int x, int y) const noexcept;
			Pixel& at(int x, int y) noexcept;

			const Pixel& at(int c) const noexcept;
			Pixel& at(int c) noexcept;

			void inscribe(int x, int y, const View& sub) noexcept;

			View& operator=(const View&) = default;
			View& operator=(View&&) = default;
		};
		class RleIterator
		{
		private:
			std::span<const PixelBase> _buffer{};
			std::span<const PixelBase>::iterator _current{};
			std::size_t _rem{ 0 };
		public:
			RleIterator() = default;
			RleIterator(std::nullptr_t) {}
			RleIterator(std::span<const PixelBase> buffer);
			RleIterator(const RleIterator&) = default;
			RleIterator(RleIterator&&) = default;

			const PixelBase& value() const noexcept;
			void increment() noexcept;
			void increment(std::size_t cnt) noexcept;
			bool is_end() const noexcept;

			RleIterator& operator=(const RleIterator&) = default;
			RleIterator& operator=(RleIterator&&) = default;
		};
	protected:
		std::vector<Pixel> buffer_{};
		int width_{ 0 };
		int height_{ 0 };
		bool compressed_{ false };
	public:
		static std::vector<Pixel> rle_pack(std::span<const Pixel> buffer) noexcept;
		static std::vector<Pixel> rle_unpack(std::span<const Pixel> buffer) noexcept;
		static void rle_walk(std::span<const Pixel> buffer, std::function<bool(const Pixel&)> func) noexcept;
		static void rle_mdwalk(std::span<const Pixel> buffer, int width, int height, std::function<RleBreak(int, int, const Pixel&)> func) noexcept;
		static RleIterator rle_iterator(std::span<const Pixel> buffer) noexcept;

		PixelBuffer() = default;
		PixelBuffer(int w, int h)
			: width_(w), height_(h) {}
		PixelBuffer(const PixelBuffer&) = default;
		PixelBuffer(PixelBuffer&&) = default;

		std::span<const Pixel> data() const noexcept;
		std::span<const Pixel> data() noexcept;

		void clear() noexcept;
		void fill(Pixel px) noexcept;
		void fill(Pixel px, int x, int y, int width, int height) noexcept;

		bool empty() const noexcept;
		bool compressed() const noexcept;

		void set_size(int w, int h) noexcept;
		void reset(std::vector<Pixel> data, int w, int h) noexcept;
		void compress();

		int width() const noexcept;
		int height() const noexcept;

		const Pixel& at(int x, int y) const noexcept;
		Pixel& at(int x, int y) noexcept;

		const Pixel& at(int c) const noexcept;
		Pixel& at(int c) noexcept;

		void inscribe(int xf, int yf, View view) noexcept;

		PixelBuffer& operator=(const PixelBuffer&) = default;
		PixelBuffer& operator=(PixelBuffer&&) = default;
	};
}

#endif // D2_PIXEL_HPP
