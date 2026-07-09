#pragma once

#include <d2_locale.hpp>
#include <d2_vtypes.hpp>
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
            }
            explicit View(const View& ptr, int x, int y, int width, int height) :
                buffer_(ptr.buffer_), xoff_(x + ptr.xoff_), yoff_(y + ptr.yoff_), width_(width),
                height_(height)
            {
            }

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
            std::size_t _rem{0};
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
        int width_{0};
        int height_{0};
        bool compressed_{false};
    public:
        static std::vector<Pixel> rle_pack(std::span<const Pixel> buffer);
        static std::vector<Pixel> rle_unpack(std::span<const Pixel> buffer);
        static void rle_walk(std::span<const Pixel> buffer, std::function<bool(const Pixel&)> func);
        static void rle_mdwalk(
            std::span<const Pixel> buffer,
            int width,
            int height,
            std::function<RleBreak(int, int, const Pixel&)> func
        );
        static RleIterator rle_iterator(std::span<const Pixel> buffer);

        PixelBuffer() = default;
        PixelBuffer(int w, int h) : width_(w), height_(h) {}
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
} // namespace d2
