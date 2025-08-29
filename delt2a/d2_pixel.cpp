#include "d2_pixel.hpp"
#include "d2_exceptions.hpp"

namespace d2
{
	//
	// Pixel Types
	//

	// Background

	PixelBackground::scalar PixelBackground::to_scalar(const PixelBackground& px) noexcept
	{
		scalar result = 0x00;
		result |= scalar(px.r);
		result |= scalar(px.g) << 8;
		result |= scalar(px.b) << 16;
		result |= scalar(px.a) << 24;
		return result;
	}
	PixelBackground PixelBackground::from_scalar(const scalar& px) noexcept
	{
		PixelBackground bg{};
		bg.r = component(px);
		bg.g = component(px >> 8);
		bg.b = component(px >> 16);
		bg.a = component(px >> 24);
		return bg;
	}

	PixelForeground PixelBackground::extend(value_type v, component style) const noexcept
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
	PixelBackground PixelBackground::alpha(float value) const noexcept
	{
		auto cpy = PixelBackground(*this);
		cpy.a = value * 255;
		return cpy;
	}
    PixelBackground PixelBackground::stylize(component style) const noexcept
    {
        return PixelForeground(*this).stylize(style);
    }
	PixelBackground PixelBackground::invert() const noexcept
	{
		return PixelBackground{
			static_cast<component>(255 - r),
			static_cast<component>(255 - g),
			static_cast<component>(255 - b),
			a
		};
	}
	PixelBackground PixelBackground::mask(const PixelBackground& px, float progress) const noexcept
	{
		return PixelBackground{
			.r = static_cast<component>((r + progress * (px.r - r)) / 2),
			.g = static_cast<component>((g + progress * (px.g - g)) / 2),
			.b = static_cast<component>((b + progress * (px.b - b)) / 2),
			.a = static_cast<component>((a + progress * (px.a - a)) / 2),
		};
	}

	PixelForeground PixelBackground::operator|(component flag) const noexcept
	{
		return extend(' ', flag);
	}

	PixelBackground::operator PixelForeground() const noexcept
	{
		return PixelForeground{
			.r = r,
			.g = g,
			.b = b,
			.a = a,
		};
	}
	PixelBackground::operator PixelBase() const noexcept
	{
		return PixelBase{
			.r = r,
			.g = g,
			.b = b,
			.a = a,
			.af = 255
		};
	}

	PixelBackground& PixelBackground::operator=(const PixelForeground& copy) noexcept
	{
		r = copy.r;
		g = copy.g;
		b = copy.b;
		a = copy.a;
		return *this;
	}
	PixelBackground& PixelBackground::operator=(const PixelBase& copy) noexcept
	{
		r = copy.r;
		g = copy.g;
		b = copy.b;
		a = copy.a;
		return *this;
	}
	PixelForeground& PixelForeground::operator=(const PixelBase& copy) noexcept
	{
		r = copy.rf;
		g = copy.gf;
		b = copy.bf;
		a = copy.af;
		v = copy.v;
		style = copy.style;
		return *this;
	}

	// Foreground

	PixelForeground PixelForeground::alpha(float value) const noexcept
	{
		auto cpy = PixelForeground(*this);
		cpy.a = value * 255;
		return cpy;
	}
    PixelForeground PixelForeground::stylize(component style) const noexcept
    {
        auto cpy = PixelForeground(*this);
        cpy.style = style;
        return cpy;
    }
	PixelForeground PixelForeground::extend(value_type v, component style) const noexcept
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
	PixelForeground PixelForeground::invert() const noexcept
	{
		return PixelForeground{
			static_cast<component>(255 - r),
			static_cast<component>(255 - g),
			static_cast<component>(255 - b),
			a,
			style,
			v
		};
	}
	PixelForeground PixelForeground::mask(const PixelForeground& px, float progress) const noexcept
	{
		return PixelForeground{
			static_cast<component>((r + progress * (px.r - r)) / 2),
			static_cast<component>((g + progress * (px.g - g)) / 2),
			static_cast<component>((b + progress * (px.b - b)) / 2),
			static_cast<component>((a + progress * (px.a - a)) / 2),
			style,
			v
		};
	}

	PixelForeground::operator PixelBackground() const noexcept
	{
		return PixelBackground{
			.r = r,
			.g = g,
			.b = b,
			.a = a,
		};
	}
	PixelForeground::operator PixelBase() const noexcept
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

	PixelForeground PixelForeground::operator|(component style) const noexcept
	{
		PixelForeground copy = *this;
		copy.style |= style;
		return copy;
	}

	PixelForeground& PixelForeground::operator=(const PixelBackground& copy) noexcept
	{
		r = copy.r;
		g = copy.g;
		b = copy.b;
		a = copy.a;
		return *this;
	}

	// Base

	PixelBase PixelBase::combine(const PixelForeground& fg, const PixelBackground& bg) noexcept
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
	PixelBase PixelBase::interpolate(const PixelBase& from, const PixelBase& to, float progress) noexcept
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

	PixelBase PixelBase::cform(std::uint64_t len) noexcept
	{
		PixelBase px;
		*reinterpret_cast<std::uint64_t*>(&px.r) = len;
		px.style = Style::Reserved;
		return px;
	}
	std::size_t PixelBase::is_cform(const PixelBase& px) noexcept
	{
		if (px.style == Style::Reserved)
			return *reinterpret_cast<const std::uint64_t*>(&px.r);
		return 0;
	}

	void PixelBase::blend(const PixelBase& src) noexcept
	{
		auto do_blend = [](component source, component dest, float alpha)
		{
			return static_cast<component>(
				source * alpha + dest * (1.f - alpha) + 0.5f
			);
		};

		const auto special_case = (src.v == ' ' && src.a < a);
		const auto src_af = special_case ? src.a : src.af;
		if (src_af == 255)
		{
			rf = src.rf;
			gf = src.gf;
			bf = src.bf;
			af = 255;
		}
		else if (src.af != 0)
		{
			const auto alpha_src = src_af / 255.f;
			const auto alpha_dest = af / 255.f;
			rf = do_blend(src.rf, rf, alpha_src);
			gf = do_blend(src.gf, gf, alpha_src);
			bf = do_blend(src.bf, bf, alpha_src);
			af = static_cast<component>(
				(alpha_src + alpha_dest * (1.f - alpha_src)) * 255.f + 0.5f
			);
		}

		if (src.a == 255)
		{
			r = src.r;
			g = src.g;
			b = src.b;
			a = 255;
		}
		else if (src.a != 0)
		{
			const auto alpha_src = src.a / 255.f;
			const auto alpha_dest = a / 255.f;
			r = do_blend(src.r, r, alpha_src);
			g = do_blend(src.g, g, alpha_src);
			b = do_blend(src.b, b, alpha_src);
			a = static_cast<component>(
				(alpha_src + alpha_dest * (1.f - alpha_src)) * 255.f + 0.5f
			);
		}

		if (src_af >= af && !special_case)
		{
			v = src.v;
			style = src.style;
		}
	}

	bool PixelBase::compare_colors(const PixelBase& px) const noexcept
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

	PixelBase PixelBase::alpha(float v, float vf) const
	{
		auto cpy = PixelBase(*this);
		cpy.a = v * 255;
		cpy.af = vf * 255;
		return cpy;
	}
	PixelBase PixelBase::falpha(float value) const
	{
		auto cpy = PixelBase(*this);
		cpy.af = value * 255;
		return cpy;
	}
	PixelBase PixelBase::balpha(float value) const
	{
		auto cpy = PixelBase(*this);
		cpy.a = value * 255;
		return cpy;
	}
    PixelBase PixelBase::stylize(component style) const noexcept
    {
        auto cpy = PixelBase(*this);
        cpy.style = style;
        return cpy;
    }
	PixelBase PixelBase::extend(value_type ch) const noexcept
	{
		auto cpy = PixelBase(*this);
		cpy.v = ch;
		return cpy;
	}

	PixelBase PixelBase::mask(const PixelBase& px) const noexcept
	{
		return PixelBase{
			.r = static_cast<component>((r + px.r) / 2),
			.g = static_cast<component>((g + px.g) / 2),
			.b = static_cast<component>((b + px.b) / 2),
			.rf = static_cast<component>((rf + px.rf) / 2),
			.gf = static_cast<component>((gf + px.gf) / 2),
			.bf = static_cast<component>((bf + px.bf) / 2),
			.a = static_cast<component>((a + px.a) / 2),
			.af = static_cast<component>((af + px.af) / 2),
		};
	}
	PixelBase PixelBase::mask_foreground(const PixelBase& px) const noexcept
	{
		return PixelBase{
			.r = r,
			.g = g,
			.b = b,
			.rf = static_cast<component>((rf + px.rf) / 2),
			.gf = static_cast<component>((gf + px.gf) / 2),
			.bf = static_cast<component>((bf + px.bf) / 2),
			.a = a,
			.af = static_cast<component>((af + px.af) / 2),
		};
	}
	PixelBase PixelBase::mask_background(const PixelBase& px) const noexcept
	{
		return PixelBase{
			.r = static_cast<component>((r + px.r) / 2),
			.g = static_cast<component>((g + px.g) / 2),
			.b = static_cast<component>((b + px.b) / 2),
			.rf = rf,
			.gf = gf,
			.bf = bf,
			.a = static_cast<component>((a + px.a) / 2),
			.af = af,
		};
	}

	PixelBase PixelBase::invert() const noexcept
	{
		return PixelBase{
			static_cast<component>(255 - r),
			static_cast<component>(255 - g),
			static_cast<component>(255 - b),
			a,
			static_cast<component>(255 - rf),
			static_cast<component>(255 - gf),
			static_cast<component>(255 - bf),
			af
		};
	}

	PixelBase::operator PixelBackground() const noexcept
	{
		return {
			.r = r,
			.g = g,
			.b = b,
			.a = a,
		};
	}
	PixelBase::operator PixelForeground() const noexcept
	{
		return {
			.r = rf,
			.g = gf,
			.b = bf,
			.a = af,
			.v = v
		};
	}

	PixelBase& PixelBase::operator=(const PixelBackground& copy) noexcept
	{
		r = copy.r;
		g = copy.g;
		b = copy.b;
		a = copy.a;
		return *this;
	}
	PixelBase& PixelBase::operator=(const PixelForeground& copy) noexcept
	{
		rf = copy.r;
		gf = copy.g;
		bf = copy.b;
		af = copy.a;
		v = copy.v;
		return *this;
	}

	//
	// Pixel Buffer
	//

	// View

	std::span<const Pixel> PixelBuffer::View::data() const noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		return buffer_->data();
	}

	void PixelBuffer::View::clear() noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		buffer_->clear();
	}
	void PixelBuffer::View::fill(Pixel px) noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		buffer_->fill(
			px, xoff_, yoff_, width_, height_
		);
	}
	void PixelBuffer::View::fill(Pixel px, int x, int y, int width, int height) noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		buffer_->fill(
			px, x + xoff_, y + yoff_, width, height
		);
	}

	bool PixelBuffer::View::compressed() const noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		return buffer_->compressed_;
	}
	bool PixelBuffer::View::empty() const noexcept
	{
		return buffer_ == nullptr || buffer_->empty();
	}

	int PixelBuffer::View::width() const noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		return width_;
	}
	int PixelBuffer::View::height() const noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		return height_;
	}
	int PixelBuffer::View::xpos() const noexcept
	{
		return xoff_;
	}
	int PixelBuffer::View::ypos() const noexcept
	{
		return yoff_;
	}

	const Pixel& PixelBuffer::View::at(int x, int y) const noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		return buffer_->buffer_[
			((y + yoff_) * buffer_->width_) + (x + xoff_)
		];
	}
	Pixel& PixelBuffer::View::at(int x, int y) noexcept
	{
		return const_cast<Pixel&>(const_cast<const View*>(this)->at(x, y));
	}

	const Pixel& PixelBuffer::View::at(int c) const noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		const auto idxx = c % width_;
		const auto idxy = c / width_;
		return at(idxx, idxy);
	}
	Pixel& PixelBuffer::View::at(int c) noexcept
	{
		return const_cast<Pixel&>(const_cast<const View*>(this)->at(c));
	}

	void PixelBuffer::View::inscribe(int x, int y, const View& sub) noexcept
	{
		D2_ASSERT(buffer_ != nullptr)
		return buffer_->inscribe(x, y, sub);
	}

	// RLE

	PixelBuffer::RleIterator::RleIterator(std::span<const PixelBase> buffer)
		: _buffer(buffer), _current(buffer.begin())
	{
		if (!buffer.empty() &&
			(_rem = PixelBase::is_cform(_buffer[0])) > 0)
		{
			++_current;
			--_rem;
		}
	}

	const PixelBase& PixelBuffer::RleIterator::value() const noexcept
	{
		return *_current;
	}
	void PixelBuffer::RleIterator::increment() noexcept
	{
		D2_ASSERT(!is_end())
		if (!_rem)
		{
			++_current;
			if (!is_end() && (_rem = PixelBase::is_cform(*_current)) > 0)
			{
				++_current;
				--_rem;
			}
		}
		else
		{
			--_rem;
		}
	}
	void PixelBuffer::RleIterator::increment(std::size_t cnt) noexcept
	{
		while (cnt--)
			increment();
	}
	bool PixelBuffer::RleIterator::is_end() const noexcept
	{
		return _current == _buffer.end();
	}

	// Implementation

	std::vector<Pixel> PixelBuffer::rle_pack(std::span<const Pixel> buffer) noexcept
	{
		if (buffer.empty())
			return {};

		std::vector<Pixel> result;
		result.reserve(buffer.size());

		for (std::size_t i = 0;;)
		{
			Pixel current = buffer[i];
			i++;

			std::size_t len = 1;
			while (i < buffer.size() && current == buffer[i])
			{
				i++;
				len++;
			}

			if (len == 1)
			{
				result.push_back(current);
			}
			else if (len == 2)
			{
				result.push_back(current);
				result.push_back(current);
			}
			else
			{
				result.push_back(PixelBase::cform(len));
				result.push_back(current);
			}

			if (i >= buffer.size())
				break;
		}

		result.shrink_to_fit();
		return result;
	}
	std::vector<Pixel> PixelBuffer::rle_unpack(std::span<const Pixel> buffer) noexcept
	{
		std::vector<Pixel> result;
		result.reserve(buffer.size() * 3);

		for (auto it = buffer.begin(); it != buffer.end(); ++it)
		{
			if (const auto len = PixelBase::is_cform(*it))
			{
				const auto& px = *(++it);
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
	void PixelBuffer::rle_walk(std::span<const Pixel> buffer, std::function<bool(const Pixel&)> func) noexcept
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
	void PixelBuffer::rle_mdwalk(std::span<const Pixel> buffer, int width, int height, std::function<RleBreak(int, int, const Pixel&)> func) noexcept
	{
		int x = 0;
		int y = 0;
		int skip = 0;
		for (auto it = buffer.begin(); it != buffer.end(); ++it)
		{
			if (const auto len = PixelBase::is_cform(*it))
			{
				auto& px = *(++it);
				for (std::size_t i = 0; i < len; i++)
				{
					if (!skip)
					{
						const auto res = func(x, y, px);
						if (res == RleBreak::SkipLine)
						{
							skip = width - x - 1;
							// For now naive approach
							// For this one we need to consider the X/Y offsets after the transformation but its kinda late rn fr fr
							// const auto req = width - x - 1;
							// if (req > len - i)
							// {
							// 	skip = req - (len - i);
							// 	break;
							// }
							// else
							// {
							// 	i += req;
							// 	if (i >= len)
							// 		break;
							// }
						}
						else if (res == RleBreak::SkipRest)
						{
							return;
						}
					}
					else
						skip--;

					if (++x == width)
					{
						x = 0;
						y++;
					}
				}
				if (it == buffer.end())
					break;
			}
			else
			{
				if (!skip)
				{
					const auto res = func(x, y, *it);
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
					skip--;

				if (++x == width)
				{
					x = 0;
					y++;
				}
			}
			if (y >= height)
				break;
		}
	}
	PixelBuffer::RleIterator PixelBuffer::rle_iterator(std::span<const Pixel> buffer) noexcept
	{
		return RleIterator(buffer);
	}

	std::span<const Pixel> PixelBuffer::data() const noexcept
	{
		return { buffer_.begin(), buffer_.end() };
	}
	std::span<const Pixel> PixelBuffer::data() noexcept
	{
		return { buffer_.begin(), buffer_.end() };
	}

	void PixelBuffer::clear() noexcept
	{
		buffer_.clear();
		buffer_.shrink_to_fit();
		width_ = 0;
		height_ = 0;
		compressed_ = false;
	}
	void PixelBuffer::fill(Pixel px) noexcept
	{
		std::fill(buffer_.begin(), buffer_.end(), px);
	}
	void PixelBuffer::fill(Pixel px, int x, int y, int width, int height) noexcept
	{
		for (std::size_t i = y; i < height; i++)
			for (std::size_t j = x; j < width; j++)
			{
				buffer_[(i * width_) + j] = px;
			}
	}

	bool PixelBuffer::empty() const noexcept
	{
		return buffer_.empty();
	}
	bool PixelBuffer::compressed() const noexcept
	{
		return compressed_;
	}

	void PixelBuffer::set_size(int w, int h) noexcept
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
	void PixelBuffer::reset(std::vector<Pixel> data, int w, int h) noexcept
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
	void PixelBuffer::compress()
	{
		buffer_ = rle_pack(buffer_);
		compressed_ = true;
	}

	int PixelBuffer::width() const noexcept
	{
		return width_;
	}
	int PixelBuffer::height() const noexcept
	{
		return height_;
	}

	const Pixel& PixelBuffer::at(int x, int y) const noexcept
	{
		D2_ASSERT(x >= 0 && y >= 0 && x < width_ && y < height_)
		return buffer_[(y * width_) + x];
	}
	Pixel& PixelBuffer::at(int x, int y) noexcept
	{
		D2_ASSERT(x >= 0 && y >= 0 && x < width_ && y < height_)
		return buffer_[(y * width_) + x];
	}

	const Pixel& PixelBuffer::at(int c) const noexcept
	{
		D2_ASSERT(c >= 0 && c < width_ * height_)
		return buffer_[c];
	}
	Pixel& PixelBuffer::at(int c) noexcept
	{
		D2_ASSERT(c >= 0 && c < width_ * height_)
		return buffer_[c];
	}

	void PixelBuffer::inscribe(int xf, int yf, View view) noexcept
	{
		if (view.empty())
			return;

		const int xdiff = (xf < 0) * -xf;
		const int ydiff = (yf < 0) * -yf;
		if (view.compressed())
		{
			rle_mdwalk(view.data(), view.width(), view.height(),
				[&xdiff, &ydiff, &xf, &yf, this](int xs, int ys, const Pixel& px) -> RleBreak {
					const auto xoff = xs + xf;
					const auto yoff = ys + yf;
					if (xoff < 0 || yoff < 0)
						return RleBreak::Continue;
					else if (yoff >= height_)
						return RleBreak::SkipRest;
					else if (xoff >= width_)
						return RleBreak::SkipLine;
					auto& dest = at(xoff, yoff);
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
}
