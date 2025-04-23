#ifndef D2_PIXEL_HPP
#define D2_PIXEL_HPP

#include <functional>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>
#include "d2_exceptions.hpp"
#include "d2_locale.hpp"

namespace d2
{
	struct PixelBase;
	struct PixelBackground;
	struct PixelForeground;

	struct PixelBackground
	{
		using scalar = std::uint32_t;

		std::uint8_t r{ 0 };
		std::uint8_t g{ 0 };
		std::uint8_t b{ 0 };
		std::uint8_t a{ 255 };

		static scalar to_scalar(const PixelBackground& px) noexcept
		{
			scalar result = 0x00;
			result |= scalar(px.r);
			result |= scalar(px.g) << 8;
			result |= scalar(px.b) << 16;
			result |= scalar(px.a) << 24;
			return result;
		}
		static PixelBackground from_scalar(const scalar& px) noexcept
		{
			PixelBackground bg{};
			bg.r = std::uint8_t(px);
			bg.g = std::uint8_t(px >> 8);
			bg.b = std::uint8_t(px >> 16);
			bg.a = std::uint8_t(px >> 24);
			return bg;
		}

		PixelForeground extend(value_type v = ' ', std::uint8_t style = 0x00) const noexcept;
		PixelBackground alpha(float value) const noexcept
		{
			auto cpy = PixelBackground(*this);
			cpy.a = value * 255;
			return cpy;
		}
		PixelBackground invert() const noexcept
		{
			return PixelBackground{
				static_cast<std::uint8_t>(255 - r),
				static_cast<std::uint8_t>(255 - g),
				static_cast<std::uint8_t>(255 - b),
				a
			};
		}
		PixelBackground mask(const PixelBackground& px, float progress = 0.5f) const noexcept
		{
			return PixelBackground{
				.r = static_cast<std::uint8_t>((r + progress * (px.r - r)) / 2),
				.g = static_cast<std::uint8_t>((g + progress * (px.g - g)) / 2),
				.b = static_cast<std::uint8_t>((b + progress * (px.b - b)) / 2),
				.a = static_cast<std::uint8_t>((a + progress * (px.a - a)) / 2),
			};
		}

		PixelForeground operator|(std::uint8_t flag) const noexcept;

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

		std::uint8_t r{ 255 };
		std::uint8_t g{ 255 };
		std::uint8_t b{ 255 };
		std::uint8_t a{ 255 };
		std::uint8_t style{ 0x00 };
		value_type v{ ' ' };

		PixelForeground alpha(float value) const noexcept
		{
			auto cpy = PixelForeground(*this);
			cpy.a = value * 255;
			return cpy;
		}
		PixelForeground extend(value_type v, std::uint8_t style = 0x00) const noexcept
		{
			return {
				.r = r,
				.g = g,
				.b = b,
				.a = a,
				.style = style,
				.v = v,
			};
		}
		PixelForeground invert() const noexcept
		{
			return PixelForeground{
				static_cast<std::uint8_t>(255 - r),
				static_cast<std::uint8_t>(255 - g),
				static_cast<std::uint8_t>(255 - b),
				a,
				style,
				v
			};
		}
		PixelForeground mask(const PixelForeground& px, float progress = 0.5f) const noexcept
		{
			return PixelForeground{
				static_cast<std::uint8_t>((r + progress * (px.r - r)) / 2),
				static_cast<std::uint8_t>((g + progress * (px.g - g)) / 2),
				static_cast<std::uint8_t>((b + progress * (px.b - b)) / 2),
				static_cast<std::uint8_t>((a + progress * (px.a - a)) / 2),
				style,
				v
			};
		}

		operator PixelBase() const noexcept;
		operator PixelBackground() const noexcept;

		PixelForeground operator|(std::uint8_t style) const noexcept
		{
			PixelForeground copy = *this;
			copy.style |= style;
			return copy;
		}

		PixelForeground& operator=(const PixelForeground&) = default;
		PixelForeground& operator=(const PixelBackground& copy) noexcept
		{
			r = copy.r;
			g = copy.g;
			b = copy.b;
			a = copy.a;
			return *this;
		}
		PixelForeground& operator=(const PixelBase& copy) noexcept;

		auto operator<=>(const PixelForeground&) const = default;
	};
	struct PixelBase
	{
		using Style = PixelForeground::Style;
		using value_type = ::d2::value_type;

		std::uint8_t r{ 0 };
		std::uint8_t g{ 0 };
		std::uint8_t b{ 0 };
		std::uint8_t rf{ 255 };
		std::uint8_t gf{ 255 };
		std::uint8_t bf{ 255 };
		std::uint8_t a{ 255 };
		std::uint8_t af{ 255 };
		std::uint8_t style{ 0x00 };
		value_type v{ ' ' };

		static PixelBase combine(const PixelForeground& fg, const PixelBackground& bg) noexcept
		{
			PixelBase px;
			px.r = bg.r;
			px.g = bg.g;
			px.b = bg.b;
			px.rf = fg.r;
			px.gf = fg.g;
			px.bf = fg.b;
			px.a = bg.a;
			px.af = fg.a;
			px.v = fg.v;
			px.style = fg.style;
			return px;
		}
		static PixelBase interpolate(const PixelBase& from, const PixelBase& to, float progress) noexcept
		{
			PixelBase value{};
			value.r = from.r + progress * (to.r - from.r);
			value.g = from.g + progress * (to.g - from.g);
			value.b = from.b + progress * (to.b - from.b);
			value.a = from.a + progress * (to.a - from.a);
			value.rf = from.rf + progress * (to.rf - from.rf);
			value.gf = from.gf + progress * (to.gf - from.gf);
			value.bf = from.bf + progress * (to.bf - from.bf);
			value.af = from.af + progress * (to.af - from.af);
			return value;
		}

		static PixelBase cform(std::uint64_t len) noexcept
		{
			PixelBase px;
			*reinterpret_cast<std::uint64_t*>(&px.r) = len;
			px.style = Style::Reserved;
			return px;
		}
		static std::size_t is_cform(const PixelBase& px) noexcept
		{
			if (px.style == Style::Reserved)
				return *reinterpret_cast<const std::uint64_t*>(&px.r);
			return 0;
		}

		void blend(const PixelBase& src) noexcept
		{
			static auto blend = [](std::uint8_t src, std::uint8_t dest, double alpha_src)
			{
				return static_cast<std::uint8_t>(
					(src * alpha_src) +
					(dest * (1 - alpha_src))
				);
			};

			// Foreground
			if (af != 255 || src.af != 255)
			{
				const auto alpha_src = src.af / 255.0;
				const auto alpha_dest = af / 255.0;
				rf = blend(src.rf, rf, alpha_src);
				gf = blend(src.gf, gf, alpha_src);
				bf = blend(src.bf, bf, alpha_src);
				af = static_cast<std::uint8_t>((alpha_src + alpha_dest * (1 - alpha_src)) * 255);
			}
			else
			{
				rf = src.rf;
				gf = src.gf;
				bf = src.bf;
				af = src.af;
			}

			// Background
			if (a != 255 || src.a != 255)
			{
				const auto alpha_src = src.a / 255.0;
				const auto alpha_dest = a / 255.0;
				r = blend(src.r, r, alpha_src);
				g = blend(src.g, g, alpha_src);
				b = blend(src.b, b, alpha_src);
				a = static_cast<std::uint8_t>((alpha_src + alpha_dest * (1 - alpha_src)) * 255);
			}
			else
			{
				r = src.r;
				g = src.g;
				b = src.b;
				a = src.a;
			}

			// Value
			if (src.af >= af &&
				!(src.a <= a && src.v == ' '))
			{
				v = src.v;
				style = src.style;
			}
		}

		// Used for color-wise comparison
		// (does not include the alpha or value since they do not affect the color right now)
		bool compare_colors(const PixelBase& px) const noexcept
		{
			return
				r == px.r &&
				g == px.g &&
				b == px.b &&
				rf == px.rf &&
				gf == px.gf &&
				bf == px.bf &&
				style == px.style;
		}

		PixelBase alpha(float v, float vf) const
		{
			auto cpy = PixelBase(*this);
			cpy.a = v * 255;
			cpy.af = vf * 255;
			return cpy;
		}
		PixelBase falpha(float value) const
		{
			auto cpy = PixelBase(*this);
			cpy.af = value * 255;
			return cpy;
		}
		PixelBase balpha(float value) const
		{
			auto cpy = PixelBase(*this);
			cpy.a = value * 255;
			return cpy;
		}
		PixelBase extend(value_type ch) const noexcept
		{
			auto cpy = PixelBase(*this);
			cpy.v = ch;
			return cpy;
		}

		PixelBase mask(const PixelBase& px) const noexcept
		{
			return PixelBase{
				.r = static_cast<std::uint8_t>((r + px.r) / 2),
				.g = static_cast<std::uint8_t>((g + px.g) / 2),
				.b = static_cast<std::uint8_t>((b + px.b) / 2),
				.rf = static_cast<std::uint8_t>((rf + px.rf) / 2),
				.gf = static_cast<std::uint8_t>((gf + px.gf) / 2),
				.bf = static_cast<std::uint8_t>((bf + px.bf) / 2),
				.a = static_cast<std::uint8_t>((a + px.a) / 2),
				.af = static_cast<std::uint8_t>((af + px.af) / 2),
			};
		}
		PixelBase mask_foreground(const PixelBase& px) const noexcept
		{
			return PixelBase{
				.r = r,
				.g = g,
				.b = b,
				.rf = static_cast<std::uint8_t>((rf + px.rf) / 2),
				.gf = static_cast<std::uint8_t>((gf + px.gf) / 2),
				.bf = static_cast<std::uint8_t>((bf + px.bf) / 2),
				.a = a,
				.af = static_cast<std::uint8_t>((af + px.af) / 2),
			};
		}
		PixelBase mask_background(const PixelBase& px) const noexcept
		{
			return PixelBase{
				.r = static_cast<std::uint8_t>((r + px.r) / 2),
				.g = static_cast<std::uint8_t>((g + px.g) / 2),
				.b = static_cast<std::uint8_t>((b + px.b) / 2),
				.rf = rf,
				.gf = gf,
				.bf = bf,
				.a = static_cast<std::uint8_t>((a + px.a) / 2),
				.af = af,
			};
		}

		PixelBase invert() const noexcept
		{
			return PixelBase{
				static_cast<std::uint8_t>(255 - r),
				static_cast<std::uint8_t>(255 - g),
				static_cast<std::uint8_t>(255 - b),
				a,
				static_cast<std::uint8_t>(255 - rf),
				static_cast<std::uint8_t>(255 - gf),
				static_cast<std::uint8_t>(255 - bf),
				af
			};
		}

		operator PixelBackground() const noexcept
		{
			return {
				.r = r,
				.g = g,
				.b = b,
				.a = a,
			};
		}
		operator PixelForeground() const noexcept
		{
			return {
				.r = rf,
				.g = gf,
				.b = bf,
				.a = af,
				.v = v
			};
		}

		PixelBase& operator=(const PixelBase&) = default;
		PixelBase& operator=(const PixelBackground& copy) noexcept
		{
			r = copy.r;
			g = copy.g;
			b = copy.b;
			a = copy.a;
			return *this;
		}
		PixelBase& operator=(const PixelForeground& copy) noexcept
		{
			rf = copy.r;
			gf = copy.g;
			bf = copy.b;
			af = copy.a;
			v = copy.v;
			return *this;
		}

		bool operator==(const PixelBase& copy) const noexcept
		{
			return
				r == copy.r &&
				g == copy.g &&
				b == copy.b &&
				rf == copy.rf &&
				gf == copy.gf &&
				bf == copy.bf &&
				a == copy.a &&
				af == copy.af &&
				v == copy.v;
		}
		bool operator!=(const PixelBase& copy) const noexcept
		{
			return !const_cast<const PixelBase*>(this)->operator==(copy);
		}
	};

	inline PixelForeground::operator PixelBackground() const noexcept
	{
		return PixelBackground{
			.r = r,
			.g = g,
			.b = b,
			.a = a,
		};
	}
	inline PixelForeground::operator PixelBase() const noexcept
	{
		return PixelBase{
			.rf = r,
			.gf = g,
			.bf = b,
			.a = 0,
			.af = a,
			.style = style,
			.v = v
		};
	}

	inline PixelBackground::operator PixelForeground() const noexcept
	{
		return PixelForeground{
			.r = r,
			.g = g,
			.b = b,
			.a = a,
		};
	}
	inline PixelBackground::operator PixelBase() const noexcept
	{
		return PixelBase{
			.r = r,
			.g = g,
			.b = b,
			.a = a,
			.af = 255
		};
	}

	inline PixelBackground& PixelBackground::operator=(const PixelForeground& copy) noexcept
	{
		r = copy.r;
		g = copy.g;
		b = copy.b;
		a = copy.a;
		return *this;
	}
	inline PixelBackground& PixelBackground::operator=(const PixelBase& copy) noexcept
	{
		r = copy.r;
		g = copy.g;
		b = copy.b;
		a = copy.a;
		return *this;
	}
	inline PixelForeground& PixelForeground::operator=(const PixelBase& copy) noexcept
	{
		r = copy.rf;
		g = copy.gf;
		b = copy.bf;
		a = copy.af;
		v = copy.v;
		style = copy.style;
		return *this;
	}

	inline PixelForeground PixelBackground::extend(value_type v, std::uint8_t style) const noexcept
	{
		return PixelForeground{
			.r = r,
			.g = g,
			.b = b,
			.a = a,
			.style = style,
			.v = v,
		};
	}
	inline PixelForeground PixelBackground::operator|(std::uint8_t flag) const noexcept
	{
		return extend(' ', flag);
	}

	// A single RGBAV value
	// RGBA standard color components and V locale independent value
	using Pixel = PixelBase;

	// Aliases
	namespace px
	{
		using style = Pixel::Style;
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

			std::span<const Pixel> data() const noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				return buffer_->data();
			}

			void clear() noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				buffer_->clear();
			}
			void fill(Pixel px) noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				buffer_->fill(
					px, xoff_, yoff_, width_, height_
				);
			}
			void fill(Pixel px, int x, int y, int width, int height) noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				buffer_->fill(
					px, x + xoff_, y + yoff_, width, height
				);
			}

			bool compressed() const noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				return buffer_->compressed_;
			}
			bool empty() const noexcept
			{
				return buffer_ == nullptr || buffer_->empty();
			}

			int width() const noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				return width_;
			}
			int height() const noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				return height_;
			}
			int xpos() const noexcept
			{
				return xoff_;
			}
			int ypos() const noexcept
			{
				return yoff_;
			}

			const Pixel& at(int x, int y) const noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				return buffer_->buffer_[
					((y + yoff_) * buffer_->width_) + (x + xoff_)
				];
			}
			Pixel& at(int x, int y) noexcept
			{
				return const_cast<Pixel&>(const_cast<const View*>(this)->at(x, y));
			}

			const Pixel& at(int c) const noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				const auto idxx = c % width_;
				const auto idxy = c / width_;
				return at(idxx, idxy);
			}
			Pixel& at(int c) noexcept
			{
				return const_cast<Pixel&>(const_cast<const View*>(this)->at(c));
			}

			void inscribe(int x, int y, const View& sub) noexcept
			{
				D2_ASSERT(buffer_ != nullptr)
				return buffer_->inscribe(x, y, sub);
			}

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
			RleIterator(std::span<const PixelBase> buffer)
				: _buffer(buffer), _current(buffer.begin())
			{
				if (!buffer.empty() &&
					(_rem = PixelBase::is_cform(_buffer[0])) > 0)
				{
					++_current;
				}
			}
			RleIterator(const RleIterator&) = default;
			RleIterator(RleIterator&&) = default;

			void increment() noexcept
			{
				if (!is_end() && !_rem)
				{
					++_current;
					if ((_rem = PixelBase::is_cform(*_current)) > 0)
					{
						++_current;
					}
				}
			}
			void increment(std::size_t cnt) noexcept
			{
				while (cnt--)
					increment();
			}
			bool is_end() const noexcept
			{
				return _current == _buffer.end();
			}
			const auto& value() const noexcept
			{
				return *_current;
			}

			RleIterator& operator=(const RleIterator&) = default;
			RleIterator& operator=(RleIterator&&) = default;
		};
	protected:
		std::vector<Pixel> buffer_{};
		int width_{ 0 };
		int height_{ 0 };
		bool compressed_{ false };
	public:
		static std::vector<Pixel> rle_pack(std::span<const Pixel> buffer) noexcept
		{
			if (buffer.empty())
				return {};

			std::vector<Pixel> result;
			result.reserve(buffer.size());

			Pixel current = buffer[0];
			std::size_t len = 0;
			for (auto it = buffer.begin(); it != buffer.end(); ++it)
			{
				const auto& px = *it;
				const auto cmp = px == current;
				len += cmp;
				if (!cmp || (it + 1) == buffer.end())
				{

					if (len > 2)
					{
						result.push_back(PixelBase::cform(len));
						result.push_back(current);
					}
					else if (len == 2)
					{
						result.push_back(current);
						result.push_back(current);
					}
					else
					{
						result.push_back(current);
					}
					current = px;
					len = 1;
				}
			}

			result.shrink_to_fit();
			return result;
		}
		static std::vector<Pixel> rle_unpack(std::span<const Pixel> buffer) noexcept
		{
			std::vector<Pixel> result;
			result.reserve(buffer.size() * 3);

			for (auto it = buffer.begin(); it != buffer.end(); ++it)
			{
				if (const auto len = PixelBase::is_cform(*it))
				{
					auto& px = *(++it);
					for (std::size_t i = 0; i < len; i++)
						result.push_back(px);
					if (it == buffer.end())
						break;
				}
				else
				{
					result.push_back(*it);
				}
			}

			result.shrink_to_fit();
			return result;
		}
		static void rle_walk(std::span<const Pixel> buffer, std::function<bool(const Pixel&)> func) noexcept
		{
			for (auto it = buffer.begin(); it != buffer.end(); ++it)
			{
				if (const auto len = PixelBase::is_cform(*it))
				{
					auto& px = *(++it);
					for (std::size_t i = 0; i < len; i++)
					{
						if (!func(px))
							return;
					}
					if (it == buffer.end())
						break;
				}
				else
				{
					if (!func(*it))
						return;
				}
			}
		}
		static void rle_mdwalk(std::span<const Pixel> buffer, int width, int height, int xf, int yf, std::function<RleBreak(int, int, const Pixel&)> func) noexcept
		{
			int x = xf;
			int y = yf;
			int skip = 0;
			for (auto it = buffer.begin(); it != buffer.end(); ++it)
			{
				if (const auto len = PixelBase::is_cform(*it))
				{
					auto& px = *(++it);
					for (std::size_t i = 0; i < len; i++)
					{
						if (x >= xf && x < xf + width &&
							y >= yf && y < yf + height)
						{
							if (!skip)
							{
								const auto res = func(x, y, px);
								if ((++x % width) == 0)
								{
									x = 0;
									y++;
								}

								if (res == RleBreak::SkipLine)
								{
									const auto req = width - x - 1;
									if (req > len - i)
									{
										skip = req - (len - i);
										break;
									}
									else
									{
										i += req;
										if (i >= len)
											break;
									}
								}
								else if (res == RleBreak::SkipRest)
								{
									return;
								}
							}
							else
							{
								if ((++x % width) == 0)
								{
									x = 0;
									y++;
								}
							}
						}
						else
						{
							++x;
							if (x >= xf + width)
							{
								x = 0;
								y++;
							}
						}
					}
					if (it == buffer.end())
						break;
				}
				else
				{
					if (x >= xf && x < xf + width &&
						y >= yf && y < yf + height)
					{
						if (!skip)
						{
							const auto res = func(x, y, *it);
							if ((++x % width) == 0)
							{
								x = 0;
								y++;
							}

							if (res == RleBreak::SkipLine)
							{
								skip = width - x - 1;
							}
							else if (res == RleBreak::SkipRest)
							{
								return;
							}
						}
						else
						{
							if ((++x % width) == 0)
							{
								x = 0;
								y++;
							}
						}
					}
					else
					{
						++x;
						if (x >= xf + width)
						{
							x = 0;
							y++;
						}
					}
				}
				if (y >= yf + height)
					break;
			}
		}
		static RleIterator rle_iterator(std::span<const Pixel> buffer) noexcept
		{
			return RleIterator(buffer);
		}

		PixelBuffer() = default;
		PixelBuffer(int w, int h)
			: width_(w), height_(h) {}
		PixelBuffer(const PixelBuffer&) = default;
		PixelBuffer(PixelBuffer&&) = default;

		std::span<const Pixel> data() const noexcept
		{
			return { buffer_.begin(), buffer_.end() };
		}
		std::span<const Pixel> data() noexcept
		{
			return { buffer_.begin(), buffer_.end() };
		}

		void clear() noexcept
		{
			buffer_.clear();
			buffer_.shrink_to_fit();
			width_ = 0;
			height_ = 0;
			compressed_ = false;
		}
		void fill(Pixel px) noexcept
		{
			std::fill(buffer_.begin(), buffer_.end(), px);
		}
		void fill(Pixel px, int x, int y, int width, int height) noexcept
		{
			for (std::size_t i = y; i < height; i++)
				for (std::size_t j = x; j < width; j++)
				{
					buffer_[(i * width_) + j] = px;
				}
		}

		bool empty() const noexcept
		{
			return buffer_.empty();
		}
		bool compressed() const noexcept
		{
			return compressed_;
		}

		void set_size(int w, int h) noexcept
		{
			if (w > 0 && h > 0)
			{
				compressed_ = false;
				width_ = w;
				height_ = h;
				buffer_.clear();
				buffer_.resize(w * h);
			}
			else
			{
				clear();
			}
		}
		void reset(std::vector<Pixel> data, int w, int h) noexcept
		{
			compressed_ = false;
			width_ = w;
			height_ = h;
			buffer_ = std::move(data);
			if (w * h > buffer_.size())
			{
				buffer_.resize(width_ * height_);
			}
		}
		void compress()
		{
			buffer_ = rle_pack(buffer_);
			compressed_ = true;
		}

		int width() const noexcept
		{
			return width_;
		}
		int height() const noexcept
		{
			return height_;
		}

		const Pixel& at(int x, int y) const noexcept
		{
			D2_ASSERT(x >= 0 && y >= 0 && x < width_ && y < height_)
			return buffer_[(y * width_) + x];
		}
		Pixel& at(int x, int y) noexcept
		{
			D2_ASSERT(x >= 0 && y >= 0 && x < width_ && y < height_)
			return buffer_[(y * width_) + x];
		}

		const Pixel& at(int c) const noexcept
		{
			D2_ASSERT(c >= 0 && c < width_ * height_)
			return buffer_[c];
		}
		Pixel& at(int c) noexcept
		{
			D2_ASSERT(c >= 0 && c < width_ * height_)
			return buffer_[c];
		}

		void inscribe(int xf, int yf, View view) noexcept
		{
			if (view.empty())
				return;

			const int xdiff = (xf < 0) * -xf;
			const int ydiff = (yf < 0) * -yf;
			if (view.compressed())
			{
				rle_mdwalk(view.data(),
					view.width(), view.height(),
					xdiff + view.xpos(), ydiff + view.ypos(),
					[&xdiff, &ydiff, &xf, &yf, this](int xs, int ys, const Pixel& px) -> RleBreak {
						if (ys + yf >= height_)
							return RleBreak::SkipRest;
						if (xs + xf >= width_)
							return RleBreak::SkipLine;
						auto& dest = at(xs + xf, ys + yf);
						dest.blend(px);
						return RleBreak::Continue;
					}
				);
			}
			else
			{
				for (std::size_t ys = ydiff; ys < view.height(); ys++)
				{
					if (ys + yf >= height_)
						break;
					for (std::size_t xs = xdiff; xs < view.width(); xs++)
					{
						if (xs + xf >= width_)
							break;
						const auto src = view.at(xs, ys);
						auto& dest = at(xs + xf, ys + yf);
						dest.blend(src);
					}
				}
			}
		}

		PixelBuffer& operator=(const PixelBuffer&) = default;
		PixelBuffer& operator=(PixelBuffer&&) = default;
	};
}

#endif // D2_PIXEL_HPP
