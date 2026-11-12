#include "core/types/d2_pixel.hpp"
#include <core/utils/d2_exceptions.hpp>
#include <stdexcept>
#include <limits>
#include <utility>
#include <climits>

namespace d2
{
    // View

    std::span<const pixel> PixelBuffer::View::data() const
    {
        if (!buffer_)
            return {};
        if (buffer_->compressed())
        {
            if (xoff_ || yoff_ || width_ != buffer_->width() || height_ != buffer_->height())
                throw std::logic_error("Compressed subview has no contiguous data");
            return buffer_->data();
        }
        if (height_ > 1 && width_ != buffer_->width())
            throw std::logic_error("Strided view has no contiguous data");
        if (empty())
            return {};
        return buffer_->data().subspan(std::size_t(yoff_) * buffer_->width() + xoff_,
                                      std::size_t(width_) * height_);
    }

    void PixelBuffer::View::clear()
    {
        D2_ASSERT(buffer_ != nullptr)
        fill(pixel{});
    }
    void PixelBuffer::View::fill(pixel px)
    {
        D2_ASSERT(buffer_ != nullptr)
        buffer_->fill(px, xoff_, yoff_, width_, height_);
    }
    void PixelBuffer::View::fill(pixel px, int x, int y, int width, int height)
    {
        if (compressed() || x < 0 || y < 0 || width < 0 || height < 0 ||
            x > width_ || y > height_ || width > width_ - x || height > height_ - y)
            throw std::out_of_range("Pixel fill bounds");
        buffer_->fill(px, x + xoff_, y + yoff_, width, height);
    }
    void PixelBuffer::View::fill_blend(pixel px)
    {
        buffer_->fill_blend(px, xoff_, yoff_, width_, height_);
    }
    void PixelBuffer::View::fill_blend(pixel px, int x, int y, int width, int height)
    {
        if (compressed() || x < 0 || y < 0 || width < 0 || height < 0 ||
            x > width_ || y > height_ || width > width_ - x || height > height_ - y)
            throw std::out_of_range("Pixel fill bounds");
        buffer_->fill_blend(px, x + xoff_, y + yoff_, width, height);
    }

    bool PixelBuffer::View::compressed() const
    {
        D2_ASSERT(buffer_ != nullptr)
        return buffer_->compressed_;
    }
    bool PixelBuffer::View::empty() const
    {
        return buffer_ == nullptr || width_ <= 0 || height_ <= 0 || buffer_->empty();
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

    const pixel& PixelBuffer::View::at(int x, int y) const
    {
        if (!buffer_ || x < 0 || y < 0 || x >= width_ || y >= height_)
            throw std::out_of_range("Pixel view coordinates");
        return std::as_const(*buffer_).at(x + xoff_, y + yoff_);
    }
    pixel& PixelBuffer::View::at(int x, int y)
    {
        const auto& cell = std::as_const(*this).at(x, y);
        buffer_->images_.erase({x + xoff_, y + yoff_, 1, 1});
        return const_cast<pixel&>(cell);
    }

    const pixel& PixelBuffer::View::at(int c) const
    {
        if (c < 0 || width_ <= 0 || height_ <= 0 ||
            std::size_t(c) >= std::size_t(width_) * height_)
            throw std::out_of_range("Pixel view index");
        const auto idxx = c % width_;
        const auto idxy = c / width_;
        return at(idxx, idxy);
    }
    pixel& PixelBuffer::View::at(int c)
    {
        std::as_const(*this).at(c);
        return at(c % width_, c / width_);
    }

    void PixelBuffer::View::inscribe(int x, int y, const View& sub)
    {
        if (empty() || sub.empty())
            return;
        if (buffer_->compressed())
            throw std::logic_error("Cannot write to a compressed pixel buffer");
        if (buffer_ == sub.buffer_)
        {
            if (static_cast<long long>(xoff_) + x == sub.xoff_ &&
                static_cast<long long>(yoff_) + y == sub.yoff_)
                return;
            PixelBuffer snapshot = *sub.buffer_;
            inscribe(x, y, View(&snapshot, sub.xoff_, sub.yoff_, sub.width_, sub.height_));
            return;
        }
        auto blend = [&](int sx, int sy, const pixel& px)
        {
            const auto dx = static_cast<long long>(x) + sx;
            const auto dy = static_cast<long long>(y) + sy;
            if (dx >= 0 && dy >= 0 && dx < width_ && dy < height_)
            {
                const auto tx = int(dx) + xoff_, ty = int(dy) + yoff_;
                if (px.a || (px.af && (px.v != ' ' || px.style != px::style{})))
                    buffer_->images_.erase({tx, ty, 1, 1});
                buffer_->buffer_[std::size_t(ty) * buffer_->width_ + tx].blend(px);
            }
        };
        if (sub.compressed())
        {
            PixelBuffer::rle_mdwalk(sub.buffer_->data(), sub.buffer_->width(), sub.buffer_->height(),
                [&](int sx, int sy, const pixel& px)
                {
                    sx -= sub.xoff_;
                    sy -= sub.yoff_;
                    if (sy >= sub.height_)
                        return RleBreak::SkipRest;
                    if (sx >= 0 && sy >= 0 && sx < sub.width_)
                        blend(sx, sy, px);
                    return RleBreak::Continue;
                });
        }
        else
        {
            for (int sy = 0; sy < sub.height_; ++sy)
                for (int sx = 0; sx < sub.width_; ++sx)
                    blend(sx, sy, sub.at(sx, sy));
        }
        const auto dx = static_cast<long long>(xoff_) + x;
        const auto dy = static_cast<long long>(yoff_) + y;
        if (dx >= INT_MIN && dx <= INT_MAX && dy >= INT_MIN && dy <= INT_MAX)
            buffer_->images_.append(sub.buffer_->images_, {sub.xoff_, sub.yoff_, sub.width_, sub.height_},
                                     {int(dx), int(dy)}, {xoff_, yoff_, width_, height_});
    }

    void PixelBuffer::View::image(image::Raster::ptr source, image::Rect destination, image::Rect crop)
    {
        if (!buffer_ || compressed())
            throw std::logic_error("Cannot place an image in this pixel view");
        const auto x = static_cast<long long>(destination.x) + xoff_;
        const auto y = static_cast<long long>(destination.y) + yoff_;
        if (x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX)
            throw std::out_of_range("Image placement position");
        destination.x = int(x); destination.y = int(y);
        buffer_->images_.place(std::move(source), destination, {xoff_, yoff_, width_, height_}, crop);
    }
    image::Scene PixelBuffer::View::image_scene() const
    {
        image::Scene scene;
        if (buffer_)
            scene.append(buffer_->images_, {xoff_, yoff_, width_, height_}, {0, 0}, {0, 0, width_, height_});
        return scene;
    }

    std::vector<pixel> PixelBuffer::View::unpack() const
    {
        if (empty()) return {};
        const auto unpacked = compressed() ? PixelBuffer::rle_unpack(buffer_->data()) : std::vector<pixel>{};
        const auto cells = compressed() ? std::span<const pixel>(unpacked) : std::as_const(*buffer_).data();
        std::vector<pixel> out;
        out.reserve(std::size_t(width_) * height_);
        for (int y = 0; y < height_; ++y)
        {
            const auto begin = cells.begin() + std::size_t(y + yoff_) * buffer_->width_ + xoff_;
            out.insert(out.end(), begin, begin + width_);
        }
        return out;
    }

    // RLE

    PixelBuffer::RleIterator::RleIterator(std::span<const px::combined> buffer) :
        _buffer(buffer), _current(buffer.begin())
    {
        if (!buffer.empty() && (_rem = (_buffer[0]).is_cform()) > 0)
        {
            ++_current;
            if (_current == _buffer.end())
                throw std::invalid_argument("Truncated RLE run");
            --_rem;
        }
    }

    const px::combined& PixelBuffer::RleIterator::value() const
    {
        if (is_end())
            throw std::out_of_range("RLE iterator is at end");
        return *_current;
    }
    void PixelBuffer::RleIterator::increment()
    {
        if (is_end())
            return;
        if (!_rem)
        {
            ++_current;
            if (!is_end() && (_rem = (*_current).is_cform()) > 0)
            {
                ++_current;
                if (_current == _buffer.end())
                    throw std::invalid_argument("Truncated RLE run");
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

    std::vector<pixel> PixelBuffer::rle_pack(std::span<const pixel> buffer)
    {
        if (buffer.empty())
            return {};

        std::vector<pixel> result;
        result.reserve(buffer.size());

        for (std::size_t i = 0;;)
        {
            pixel current = buffer[i];
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
                result.push_back(px::combined::cform(len));
                result.push_back(current);
            }

            if (i >= buffer.size())
                break;
        }

        result.shrink_to_fit();
        return result;
    }
    std::vector<pixel> PixelBuffer::rle_unpack(std::span<const pixel> buffer)
    {
        std::vector<pixel> result;
        result.reserve(buffer.size() * 3);

        for (auto it = buffer.begin(); it != buffer.end(); ++it)
        {
            if (const auto len = (*it).is_cform())
            {
                if (++it == buffer.end())
                    throw std::invalid_argument("Truncated RLE run");
                const auto& px = *it;
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
    PixelBuffer::rle_walk(std::span<const pixel> buffer, std::function<bool(const pixel&)> func)
    {
        for (auto it = buffer.begin(); it != buffer.end(); ++it)
        {
            if (const auto len = (*it).is_cform())
            {
                if (++it == buffer.end())
                    throw std::invalid_argument("Truncated RLE run");
                const auto& px = *it;
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
        std::span<const pixel> buffer,
        int width,
        int height,
        std::function<RleBreak(int, int, const pixel&)> func
    )
    {
        if (width <= 0 || height <= 0)
            return;
        auto it = rle_iterator(buffer);
        for (int y = 0; y < height && !it.is_end(); ++y)
        {
            bool skip = false;
            for (int x = 0; x < width && !it.is_end(); ++x, it.increment())
            {
                if (skip)
                    continue;
                const auto result = func(x, y, it.value());
                if (result == RleBreak::SkipRest)
                    return;
                skip = result == RleBreak::SkipLine;
            }
        }
    }

    PixelBuffer::RleIterator PixelBuffer::rle_iterator(std::span<const pixel> buffer)
    {
        return RleIterator(buffer);
    }

    std::span<const pixel> PixelBuffer::data() const
    {
        return {buffer_.begin(), buffer_.end()};
    }
    std::span<const pixel> PixelBuffer::data()
    {
        return {buffer_.begin(), buffer_.end()};
    }

    void PixelBuffer::clear()
    {
        images_.clear();
        buffer_.clear();
        buffer_.shrink_to_fit();
        width_ = 0;
        height_ = 0;
        compressed_ = false;
    }
    void PixelBuffer::fill(pixel px)
    {
        if (compressed_)
            throw std::logic_error("Cannot write to a compressed pixel buffer");
        images_.clear();
        std::fill(buffer_.begin(), buffer_.end(), px);
    }
    void PixelBuffer::fill(pixel px, int x, int y, int width, int height)
    {
        if (compressed_ || x < 0 || y < 0 || width < 0 || height < 0 ||
            x > width_ || y > height_ || width > width_ - x || height > height_ - y)
            throw std::out_of_range("Pixel fill bounds");
        images_.erase({x, y, width, height});
        for (std::size_t i = y; i < height + y; i++)
            for (std::size_t j = x; j < width + x; j++)
            {
                buffer_[(i * width_) + j] = px;
            }
    }
    void PixelBuffer::fill_blend(pixel px)
    {
        if (compressed_)
            throw std::logic_error("Cannot write to a compressed pixel buffer");
        if (px.a || (px.af && (px.v != ' ' || px.style != px::style{})))
            images_.clear();
        for (decltype(auto) it : buffer_)
            it.blend(px);
    }
    void PixelBuffer::fill_blend(pixel px, int x, int y, int width, int height)
    {
        if (compressed_ || x < 0 || y < 0 || width < 0 || height < 0 ||
            x > width_ || y > height_ || width > width_ - x || height > height_ - y)
            throw std::out_of_range("Pixel fill bounds");
        if (px.a || (px.af && (px.v != ' ' || px.style != px::style{})))
            images_.erase({x, y, width, height});
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
        if (w < 0 || h < 0)
            throw std::invalid_argument("Negative pixel buffer dimensions");
        std::vector<pixel> data;
        if (h && std::size_t(w) > data.max_size() / std::size_t(h))
            throw std::length_error("Pixel buffer dimensions overflow");
        data.resize(std::size_t(w) * std::size_t(h));
        images_.clear();
        buffer_ = std::move(data);
        width_ = w;
        height_ = h;
        compressed_ = false;
    }
    void PixelBuffer::reset(std::vector<pixel> data, int w, int h)
    {
        if (w < 0 || h < 0)
            throw std::invalid_argument("Negative pixel buffer dimensions");
        if (h && std::size_t(w) > data.max_size() / std::size_t(h))
            throw std::length_error("Pixel buffer dimensions overflow");
        data.resize(std::size_t(w) * std::size_t(h));
        images_.clear();
        buffer_ = std::move(data);
        width_ = w;
        height_ = h;
        compressed_ = false;
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

    const pixel& PixelBuffer::at(int x, int y) const
    {
        if (compressed_)
            throw std::logic_error("Compressed pixel buffer has no random access");
        if (x < 0 || y < 0 || x >= width_ || y >= height_)
            throw std::out_of_range("Pixel buffer coordinates");
        return buffer_[std::size_t(y) * width_ + x];
    }
    pixel& PixelBuffer::at(int x, int y)
    {
        const auto& cell = std::as_const(*this).at(x, y);
        images_.erase({x, y, 1, 1});
        return const_cast<pixel&>(cell);
    }
    const pixel& PixelBuffer::at(int c) const
    {
        if (compressed_)
            throw std::logic_error("Compressed pixel buffer has no random access");
        return buffer_.at(c);
    }
    pixel& PixelBuffer::at(int c)
    {
        std::as_const(*this).at(c);
        return at(c % width_, c / width_);
    }
    void PixelBuffer::inscribe(int xf, int yf, View view)
    {
        View(this, 0, 0, width_, height_).inscribe(xf, yf, view);
    }
} // namespace d2
