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

		static scalar to_scalar(const PixelBackground& px);
		static PixelBackground from_scalar(const scalar& px);

		PixelForeground extend(value_type v = ' ', component style = 0x00) const;
		PixelBackground alpha(float value) const;
        PixelBackground alpha(component value) const;
		PixelBackground invert() const;
		PixelBackground mask(const PixelBackground& px, float progress = 0.5f) const;
        PixelBackground stylize(component style) const;

		PixelForeground operator|(component flag) const;

		operator PixelBase() const;
		operator PixelForeground() const;

		PixelBackground& operator=(const PixelBackground&) = default;
		PixelBackground& operator=(const PixelForeground& copy);
		PixelBackground& operator=(const PixelBase& copy);

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

        PixelForeground alpha(float value) const;
        PixelForeground alpha(component value) const;
		PixelForeground extend(value_type v, component style = 0x00) const;
		PixelForeground invert() const;
        PixelForeground stylize(component style) const;
		PixelForeground mask(const PixelForeground& px, float progress = 0.5f) const;

		operator PixelBase() const;
		operator PixelBackground() const;

		PixelForeground operator|(component style) const;

		PixelForeground& operator=(const PixelForeground&) = default;
		PixelForeground& operator=(const PixelBackground& copy);
		PixelForeground& operator=(const PixelBase& copy);

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

		static PixelBase combine(const PixelForeground& fg, const PixelBackground& bg);
		static PixelBase interpolate(const PixelBase& from, const PixelBase& to, float progress);
		static PixelBase cform(std::uint64_t len);
		static std::size_t is_cform(const PixelBase& px);

		void blend(const PixelBase& src);

		bool compare_colors(const PixelBase& px) const;

		PixelBase alpha(float v, float vf) const;
        PixelBase alpha(component v, component vf) const;
		PixelBase falpha(float value) const;
        PixelBase falpha(component value) const;
		PixelBase balpha(float value) const;
        PixelBase balpha(component value) const;
		PixelBase extend(value_type ch) const;
        PixelBase stylize(component style) const;

		PixelBase mask(const PixelBase& px) const;
		PixelBase mask_foreground(const PixelBase& px) const;
		PixelBase mask_background(const PixelBase& px) const;

		PixelBase invert() const;

		operator PixelBackground() const;
		operator PixelForeground() const;

		PixelBase& operator=(const PixelBase&) = default;
		PixelBase& operator=(const PixelBackground& copy);
		PixelBase& operator=(const PixelForeground& copy);

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

			std::span<const Pixel> data() const;

			void clear();
			void fill(Pixel px);
			void fill(Pixel px, int x, int y, int width, int height);
            void fill_blend(Pixel px);
            void fill_blend(Pixel px, int x, int y, int width, int height);

			bool compressed() const;
			bool empty() const;

			int width() const;
			int height() const;
			int xpos() const;
			int ypos() const;

			const Pixel& at(int x, int y) const;
			Pixel& at(int x, int y);

			const Pixel& at(int c) const;
			Pixel& at(int c);

			void inscribe(int x, int y, const View& sub);

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

			const PixelBase& value() const;
			void increment();
			void increment(std::size_t cnt);
			bool is_end() const;

			RleIterator& operator=(const RleIterator&) = default;
			RleIterator& operator=(RleIterator&&) = default;
		};
	protected:
		std::vector<Pixel> buffer_{};
		int width_{ 0 };
		int height_{ 0 };
		bool compressed_{ false };
	public:
		static std::vector<Pixel> rle_pack(std::span<const Pixel> buffer);
		static std::vector<Pixel> rle_unpack(std::span<const Pixel> buffer);
		static void rle_walk(std::span<const Pixel> buffer, std::function<bool(const Pixel&)> func);
		static void rle_mdwalk(std::span<const Pixel> buffer, int width, int height, std::function<RleBreak(int, int, const Pixel&)> func);
		static RleIterator rle_iterator(std::span<const Pixel> buffer);

		PixelBuffer() = default;
		PixelBuffer(int w, int h)
			: width_(w), height_(h) {}
		PixelBuffer(const PixelBuffer&) = default;
		PixelBuffer(PixelBuffer&&) = default;

		std::span<const Pixel> data() const;
		std::span<const Pixel> data();

		void clear();
		void fill(Pixel px);
		void fill(Pixel px, int x, int y, int width, int height);
        void fill_blend(Pixel px);
        void fill_blend(Pixel px, int x, int y, int width, int height);

		bool empty() const;
		bool compressed() const;

		void set_size(int w, int h);
		void reset(std::vector<Pixel> data, int w, int h);
		void compress();

		int width() const;
		int height() const;

		const Pixel& at(int x, int y) const;
		Pixel& at(int x, int y);

		const Pixel& at(int c) const;
		Pixel& at(int c);

		void inscribe(int xf, int yf, View view);

		PixelBuffer& operator=(const PixelBuffer&) = default;
		PixelBuffer& operator=(PixelBuffer&&) = default;
	};
}

#endif // D2_PIXEL_HPP
