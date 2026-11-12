#pragma once

#include <core/types/d2_vtypes.hpp>
#include <memory>
#include <span>
#include <vector>

namespace d2::image
{
    struct RGBA
    {
        unsigned char r{0}, g{0}, b{0}, a{0};
        auto operator<=>(const RGBA&) const = default;
    };
    struct Rect
    {
        int x{0}, y{0}, width{0}, height{0};
        bool empty() const { return width <= 0 || height <= 0; }
        auto operator<=>(const Rect&) const = default;
    };
    Rect intersect(Rect first, Rect second);

    class Raster
    {
    private:
        int _width, _height;
        std::vector<RGBA> _pixels;
    public:
        using ptr = std::shared_ptr<const Raster>;
        static constexpr std::size_t max_pixels = 16 * 1024 * 1024;
        Raster(int width, int height, std::vector<RGBA> pixels);
        static ptr rgba(int width, int height, std::vector<RGBA> pixels);
        static ptr rgb(int width, int height, std::span<const unsigned char> bytes);
        int width() const { return _width; }
        int height() const { return _height; }
        std::span<const RGBA> pixels() const { return _pixels; }
    };
    struct Placement
    {
        Raster::ptr raster;
        Rect source;
        Rect destination;
        Rect clip;
        std::vector<unsigned char> visible;
        bool operator==(const Placement&) const = default;
        bool at(int x, int y) const;
    };
    class Scene
    {
    private:
        std::vector<Placement> _placements;
    public:
        void place(Raster::ptr raster, Rect destination, Rect clip, Rect source = {});
        void erase(Rect area);
        void append(const Scene& source, Rect source_view, Position destination, Rect clip);
        void clear() { _placements.clear(); }
        bool empty() const { return _placements.empty(); }
        const std::vector<Placement>& placements() const { return _placements; }
        bool operator==(const Scene&) const = default;
    };
    struct Composite
    {
        Rect cells;
        BoundingBox cell_pixels;
        std::vector<RGBA> pixels;
        std::vector<unsigned char> occupied;
        int width() const { return cells.width * cell_pixels.width; }
        int height() const { return cells.height * cell_pixels.height; }
        bool empty() const { return pixels.empty(); }
    };
    Composite compose(const Scene& scene, BoundingBox viewport, BoundingBox cell_pixels);
} // namespace d2::image
