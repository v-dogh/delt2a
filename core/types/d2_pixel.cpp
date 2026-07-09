#include "d2_pixel.hpp"
#include <d2_exceptions.hpp>

namespace d2
{
    // View

    std::span<const Pixel> PixelBuffer::View::data() const
    {
        return buffer_->data();
    }

    void PixelBuffer::View::clear()
    {
        D2_ASSERT(buffer_ != nullptr)
        buffer_->clear();
    }
    void PixelBuffer::View::fill(Pixel px)
    {
        D2_ASSERT(buffer_ != nullptr)
        buffer_->fill(px, xoff_, yoff_, width_, height_);
    }
    void PixelBuffer::View::fill(Pixel px, int x, int y, int width, int height)
    {
        D2_ASSERT(x + width <= width_ && y + height <= height_ && x >= 0 && y >= 0)
        buffer_->fill(px, x + xoff_, y + yoff_, width, height);
    }
    void PixelBuffer::View::fill_blend(Pixel px)
    {
        buffer_->fill_blend(px, xoff_, yoff_, width_, height_);
    }
    void PixelBuffer::View::fill_blend(Pixel px, int x, int y, int width, int height)
    {
        D2_ASSERT(x + width <= width_ && y + height <= height_ && x >= 0 && y >= 0)
        buffer_->fill_blend(px, x + xoff_, y + yoff_, width, height);
    }

    bool PixelBuffer::View::compressed() const
    {
        D2_ASSERT(buffer_ != nullptr)
        return buffer_->compressed_;
    }
    bool PixelBuffer::View::empty() const
    {
        return buffer_ == nullptr || buffer_->empty();
    }

    int PixelBuffer::View::width() const
    {
        D2_ASSERT(buffer_ != nullptr)
        return width_;
    }
    int PixelBuffer::View::height() const
    {
        D2_ASSERT(buffer_ != nullptr)
        return height_;
    }
    int PixelBuffer::View::xpos() const
    {
        return xoff_;
    }
    int PixelBuffer::View::ypos() const
    {
        return yoff_;
    }

    const Pixel& PixelBuffer::View::at(int x, int y) const
    {
        D2_ASSERT(x < width_ && y < height_ && x >= 0 && y >= 0)
        return buffer_->buffer_[((y + yoff_) * buffer_->width_) + (x + xoff_)];
    }
    Pixel& PixelBuffer::View::at(int x, int y)
    {
        return const_cast<Pixel&>(const_cast<const View*>(this)->at(x, y));
    }

    const Pixel& PixelBuffer::View::at(int c) const
    {
        D2_ASSERT(c < width_ * height_ && c >= 0)
        const auto idxx = c % width_;
        const auto idxy = c / width_;
        return at(idxx, idxy);
    }
    Pixel& PixelBuffer::View::at(int c)
    {
        return const_cast<Pixel&>(const_cast<const View*>(this)->at(c));
    }

    void PixelBuffer::View::inscribe(int x, int y, const View& sub)
    {
        return buffer_->inscribe(x, y, sub);
    }

    // RLE

    PixelBuffer::RleIterator::RleIterator(std::span<const PixelBase> buffer) :
        _buffer(buffer), _current(buffer.begin())
    {
        if (!buffer.empty() && (_rem = PixelBase::is_cform(_buffer[0])) > 0)
        {
            ++_current;
            --_rem;
        }
    }

    const PixelBase& PixelBuffer::RleIterator::value() const
    {
        return *_current;
    }
    void PixelBuffer::RleIterator::increment()
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
    void PixelBuffer::RleIterator::increment(std::size_t cnt)
    {
        while (cnt--)
            increment();
    }
    bool PixelBuffer::RleIterator::is_end() const
    {
        return _current == _buffer.end();
    }

    // Implementation

    std::vector<Pixel> PixelBuffer::rle_pack(std::span<const Pixel> buffer)
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
    std::vector<Pixel> PixelBuffer::rle_unpack(std::span<const Pixel> buffer)
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
    void
    PixelBuffer::rle_walk(std::span<const Pixel> buffer, std::function<bool(const Pixel&)> func)
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
    void PixelBuffer::rle_mdwalk(
        std::span<const Pixel> buffer,
        int width,
        int height,
        std::function<RleBreak(int, int, const Pixel&)> func
    )
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
                            // For this one we need to consider the X/Y offsets after the
                            // transformation but its kinda late rn fr fr const auto req = width - x
                            // - 1; if (req > len - i)
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
    PixelBuffer::RleIterator PixelBuffer::rle_iterator(std::span<const Pixel> buffer)
    {
        return RleIterator(buffer);
    }

    std::span<const Pixel> PixelBuffer::data() const
    {
        return {buffer_.begin(), buffer_.end()};
    }
    std::span<const Pixel> PixelBuffer::data()
    {
        return {buffer_.begin(), buffer_.end()};
    }

    void PixelBuffer::clear()
    {
        buffer_.clear();
        buffer_.shrink_to_fit();
        width_ = 0;
        height_ = 0;
        compressed_ = false;
    }
    void PixelBuffer::fill(Pixel px)
    {
        std::fill(buffer_.begin(), buffer_.end(), px);
    }
    void PixelBuffer::fill(Pixel px, int x, int y, int width, int height)
    {
        D2_ASSERT(x + width <= width_ && y + height <= height_ && x >= 0 && y >= 0)
        for (std::size_t i = y; i < height + y; i++)
            for (std::size_t j = x; j < width + x; j++)
            {
                buffer_[(i * width_) + j] = px;
            }
    }
    void PixelBuffer::fill_blend(Pixel px)
    {
        for (decltype(auto) it : buffer_)
            it.blend(px);
    }
    void PixelBuffer::fill_blend(Pixel px, int x, int y, int width, int height)
    {
        D2_ASSERT(x + width <= width_ && y + height <= height_ && x >= 0 && y >= 0)
        for (std::size_t i = y; i < height + y; i++)
            for (std::size_t j = x; j < width + x; j++)
            {
                buffer_[(i * width_) + j].blend(px);
            }
    }

    bool PixelBuffer::empty() const
    {
        return buffer_.empty();
    }
    bool PixelBuffer::compressed() const
    {
        return compressed_;
    }

    void PixelBuffer::set_size(int w, int h)
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
    void PixelBuffer::reset(std::vector<Pixel> data, int w, int h)
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
        if (!compressed_)
        {
            buffer_ = rle_pack(buffer_);
            compressed_ = true;
        }
    }

    int PixelBuffer::width() const
    {
        return width_;
    }
    int PixelBuffer::height() const
    {
        return height_;
    }

    const Pixel& PixelBuffer::at(int x, int y) const
    {
        D2_ASSERT(x < width_ && y < height_ && x >= 0 && y >= 0)
        return buffer_[(y * width_) + x];
    }
    Pixel& PixelBuffer::at(int x, int y)
    {
        D2_ASSERT(x < width_ && y < height_ && x >= 0 && y >= 0)
        return buffer_[(y * width_) + x];
    }

    const Pixel& PixelBuffer::at(int c) const
    {
        D2_ASSERT(c < width_ * height_ && c >= 0)
        return buffer_[c];
    }
    Pixel& PixelBuffer::at(int c)
    {
        D2_ASSERT(c < width_ * height_ && c >= 0)
        return buffer_[c];
    }

    void PixelBuffer::inscribe(int xf, int yf, View view)
    {
        if (view.empty())
            return;

        const int xdiff = (xf < 0) * -xf;
        const int ydiff = (yf < 0) * -yf;
        if (view.compressed())
        {
            rle_mdwalk(
                view.data(),
                view.width(),
                view.height(),
                [&xdiff, &ydiff, &xf, &yf, this](int xs, int ys, const Pixel& px) -> RleBreak
                {
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
} // namespace d2
