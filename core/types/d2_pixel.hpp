#pragma once

#include <stdexcept>
#include <core/types/d2_image.hpp>

#include <core/platform/d2_locale.hpp>
#include <core/types/d2_vtypes.hpp>
#include <functional>
#include <span>
#include <vector>

namespace d2
{
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
            PixelBuffer* buffer_{nullptr};
            int xoff_{0};
            int yoff_{0};
            int width_{0};
            int height_{0};
        public:
            View() = default;
            View(std::nullptr_t) {}
            View(View&&) = default;
            View(const View&) = default;
            explicit View(PixelBuffer* ptr, int x, int y, int width, int height) :
                buffer_(ptr), xoff_(x), yoff_(y), width_(width), height_(height)
            {
                if (!ptr || x < 0 || y < 0 || width < 0 || height < 0 ||
                    x > ptr->width() || y > ptr->height() ||
                    width > ptr->width() - x || height > ptr->height() - y)
                    throw std::out_of_range("Pixel view bounds");
            }
            explicit View(const View& ptr, int x, int y, int width, int height) :
                buffer_(ptr.buffer_), width_(width), height_(height)
            {
                if (!buffer_ || x < 0 || y < 0 || width < 0 || height < 0 ||
                    x > ptr.width_ || y > ptr.height_ ||
                    width > ptr.width_ - x || height > ptr.height_ - y)
                    throw std::out_of_range("Pixel subview bounds");
                xoff_ = ptr.xoff_ + x;
                yoff_ = ptr.yoff_ + y;
            }

            std::span<const pixel> data() const;

            void clear();
            void fill(pixel px);
            void fill(pixel px, int x, int y, int width, int height);
            void fill_blend(pixel px);
            void fill_blend(pixel px, int x, int y, int width, int height);

            bool compressed() const;
            bool empty() const;

            int width() const;
            int height() const;
            int xpos() const;
            int ypos() const;

            const pixel& at(int x, int y) const;
            pixel& at(int x, int y);

            const pixel& at(int c) const;
            pixel& at(int c);

            void inscribe(int x, int y, const View& sub);
            void image(image::Raster::ptr source, image::Rect destination, image::Rect crop = {});
            image::Scene image_scene() const;
            bool has_images() const { return buffer_ && !buffer_->images_.empty(); }
            std::vector<pixel> unpack() const;

            View& operator=(const View&) = default;
            View& operator=(View&&) = default;
        };
        class RleIterator
        {
        private:
            std::span<const px::combined> _buffer{};
            std::span<const px::combined>::iterator _current{};
            std::size_t _rem{0};
        public:
            RleIterator() = default;
            RleIterator(std::nullptr_t) {}
            RleIterator(std::span<const px::combined> buffer);
            RleIterator(const RleIterator&) = default;
            RleIterator(RleIterator&&) = default;

            const px::combined& value() const;
            void increment();
            void increment(std::size_t cnt);
            bool is_end() const;

            RleIterator& operator=(const RleIterator&) = default;
            RleIterator& operator=(RleIterator&&) = default;
        };
    protected:
        std::vector<pixel> buffer_{};
        int width_{0};
        int height_{0};
        bool compressed_{false};
        image::Scene images_{};
    public:
        static std::vector<pixel> rle_pack(std::span<const pixel> buffer);
        static std::vector<pixel> rle_unpack(std::span<const pixel> buffer);
        static void rle_walk(std::span<const pixel> buffer, std::function<bool(const pixel&)> func);
        static void rle_mdwalk(
            std::span<const pixel> buffer,
            int width,
            int height,
            std::function<RleBreak(int, int, const pixel&)> func
        );
        static RleIterator rle_iterator(std::span<const pixel> buffer);

        PixelBuffer() = default;
        PixelBuffer(int w, int h) { set_size(w, h); }
        PixelBuffer(const PixelBuffer&) = default;
        PixelBuffer(PixelBuffer&&) = default;

        std::span<const pixel> data() const;
        std::span<const pixel> data();

        void clear();
        void fill(pixel px);
        void fill(pixel px, int x, int y, int width, int height);
        void fill_blend(pixel px);
        void fill_blend(pixel px, int x, int y, int width, int height);

        bool empty() const;
        bool compressed() const;

        void set_size(int w, int h);
        void reset(std::vector<pixel> data, int w, int h);
        void compress();

        int width() const;
        int height() const;

        const pixel& at(int x, int y) const;
        pixel& at(int x, int y);

        const pixel& at(int c) const;
        pixel& at(int c);

        void inscribe(int xf, int yf, View view);
        const image::Scene& image_scene() const { return images_; }

        PixelBuffer& operator=(const PixelBuffer&) = default;
        PixelBuffer& operator=(PixelBuffer&&) = default;
    };
} // namespace d2
