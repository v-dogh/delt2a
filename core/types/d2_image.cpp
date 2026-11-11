#include "d2_image.hpp"
#include <algorithm>
#include <climits>
#include <stdexcept>

namespace d2::image
{
    Rect intersect(Rect a, Rect b)
    {
        const auto x = std::max(a.x, b.x), y = std::max(a.y, b.y);
        const auto right = std::min(static_cast<long long>(a.x) + std::max(0, a.width),
                                    static_cast<long long>(b.x) + std::max(0, b.width));
        const auto bottom = std::min(static_cast<long long>(a.y) + std::max(0, a.height),
                                     static_cast<long long>(b.y) + std::max(0, b.height));
        return {x, y, int(std::max(0LL, right - x)), int(std::max(0LL, bottom - y))};
    }
    Raster::Raster(int width, int height, std::vector<RGBA> pixels) :
        _width(width), _height(height), _pixels(std::move(pixels))
    {
        if (width <= 0 || height <= 0 || std::size_t(width) > max_pixels / std::size_t(height) ||
            _pixels.size() != std::size_t(width) * height)
            throw std::invalid_argument("Invalid RGBA image dimensions or data");
    }
    Raster::ptr Raster::rgba(int width, int height, std::vector<RGBA> pixels)
    {
        return std::make_shared<Raster>(width, height, std::move(pixels));
    }
    Raster::ptr Raster::rgb(int width, int height, std::span<const unsigned char> bytes)
    {
        if (width <= 0 || height <= 0 || std::size_t(width) > max_pixels / std::size_t(height) ||
            bytes.size() != std::size_t(width) * height * 3)
            throw std::invalid_argument("Invalid RGB image dimensions or data");
        std::vector<RGBA> pixels(std::size_t(width) * height);
        for (std::size_t i = 0; i < pixels.size(); ++i)
            pixels[i] = {bytes[i * 3], bytes[i * 3 + 1], bytes[i * 3 + 2], 255};
        return rgba(width, height, std::move(pixels));
    }
    bool Placement::at(int x, int y) const
    {
        if (x < clip.x || y < clip.y || static_cast<long long>(x) - clip.x >= clip.width || static_cast<long long>(y) - clip.y >= clip.height)
            return false;
        return visible.empty() || visible[std::size_t(y - clip.y) * clip.width + x - clip.x];
    }
    void Scene::place(Raster::ptr raster, Rect destination, Rect clip, Rect source)
    {
        if (!raster)
            throw std::invalid_argument("Null image source");
        if (source == Rect{})
            source = {0, 0, raster->width(), raster->height()};
        if (source.empty() || source.x < 0 || source.y < 0 ||
            static_cast<long long>(source.x) + source.width > raster->width() ||
            static_cast<long long>(source.y) + source.height > raster->height())
            throw std::out_of_range("Image source rectangle");
        clip = intersect(destination, clip);
        if (destination.empty() || clip.empty())
            return;
        if (std::size_t(clip.width) > Raster::max_pixels / std::size_t(clip.height))
            throw std::length_error("Image placement exceeds cell limit");
        if (static_cast<long long>(clip.x) + clip.width > INT_MAX ||
            static_cast<long long>(clip.y) + clip.height > INT_MAX)
            throw std::out_of_range("Image clip coordinates");
        _placements.push_back({std::move(raster), source, destination, clip, {}});
    }
    void Scene::erase(Rect area)
    {
        for (auto& placement : _placements)
        {
            const auto cut = intersect(area, placement.clip);
            if (cut.empty())
                continue;
            if (cut == placement.clip)
            {
                placement.raster.reset();
                continue;
            }
            if (placement.visible.empty())
                placement.visible.assign(std::size_t(placement.clip.width) * placement.clip.height, 1);
            for (int y = 0; y < cut.height; ++y)
                std::fill_n(placement.visible.begin() + std::size_t(cut.y + y - placement.clip.y) *
                            placement.clip.width + cut.x - placement.clip.x, cut.width, 0);
        }
        std::erase_if(_placements, [](const auto& placement) { return !placement.raster; });
    }
    void Scene::append(const Scene& source, Rect source_view, Position destination, Rect clip)
    {
        if (this == &source)
        {
            const auto copy = source;
            append(copy, source_view, destination, clip);
            return;
        }
        const auto dx = static_cast<long long>(destination.x) - source_view.x;
        const auto dy = static_cast<long long>(destination.y) - source_view.y;
        for (const auto& old : source._placements)
        {
            auto visible = intersect(old.clip, source_view);
            const auto vx = visible.x + dx, vy = visible.y + dy;
            const auto px = old.destination.x + dx, py = old.destination.y + dy;
            if (vx < INT_MIN || vx > INT_MAX || vy < INT_MIN || vy > INT_MAX ||
                px < INT_MIN || px > INT_MAX || py < INT_MIN || py > INT_MAX)
                continue;
            visible.x = int(vx); visible.y = int(vy);
            visible = intersect(visible, clip);
            if (visible.empty() || static_cast<long long>(visible.x) + visible.width > INT_MAX ||
                static_cast<long long>(visible.y) + visible.height > INT_MAX)
                continue;
            Placement next{old.raster, old.source,
                {int(px), int(py), old.destination.width, old.destination.height}, visible, {}};
            if (!old.visible.empty())
            {
                next.visible.resize(std::size_t(visible.width) * visible.height);
                for (int y = 0; y < visible.height; ++y)
                    for (int x = 0; x < visible.width; ++x)
                        next.visible[std::size_t(y) * visible.width + x] =
                            old.at(int(visible.x + x - dx), int(visible.y + y - dy));
            }
            _placements.push_back(std::move(next));
        }
    }
    Composite compose(const Scene& scene, BoundingBox viewport, BoundingBox cell_pixels)
    {
        if (viewport.width < 0 || viewport.height < 0 || cell_pixels.width <= 0 || cell_pixels.height <= 0)
            throw std::invalid_argument("Image compositor geometry");
        Rect bounds;
        for (const auto& layer : scene.placements())
        {
            const auto clip = intersect(layer.clip, {0, 0, viewport.width, viewport.height});
            if (clip.empty()) continue;
            if (bounds.empty()) bounds = clip;
            else
            {
                const auto right = std::max(bounds.x + bounds.width, clip.x + clip.width);
                const auto bottom = std::max(bounds.y + bounds.height, clip.y + clip.height);
                bounds.x = std::min(bounds.x, clip.x); bounds.y = std::min(bounds.y, clip.y);
                bounds.width = right - bounds.x; bounds.height = bottom - bounds.y;
            }
        }
        Composite out{bounds, cell_pixels, {}, {}};
        if (bounds.empty()) return out;
        const auto width = static_cast<long long>(bounds.width) * cell_pixels.width;
        const auto height = static_cast<long long>(bounds.height) * cell_pixels.height;
        if (width > INT_MAX || height > INT_MAX || width > Raster::max_pixels / height)
            throw std::length_error("Image composition exceeds pixel limit");
        out.pixels.resize(std::size_t(width) * height);
        out.occupied.resize(std::size_t(bounds.width) * bounds.height);
        for (const auto& layer : scene.placements())
        {
            const auto clip = intersect(layer.clip, bounds);
            for (int cy = clip.y; cy < clip.y + clip.height; ++cy)
                for (int cx = clip.x; cx < clip.x + clip.width; ++cx)
                {
                    if (!layer.at(cx, cy)) continue;
                    for (int y = 0; y < cell_pixels.height; ++y)
                        for (int x = 0; x < cell_pixels.width; ++x)
                        {
                            const auto sx = layer.source.x + int(((static_cast<long double>(cx) - layer.destination.x) *
                                cell_pixels.width + x) * layer.source.width /
                                (static_cast<long long>(layer.destination.width) * cell_pixels.width));
                            const auto sy = layer.source.y + int(((static_cast<long double>(cy) - layer.destination.y) *
                                cell_pixels.height + y) * layer.source.height /
                                (static_cast<long long>(layer.destination.height) * cell_pixels.height));
                            const auto src = layer.raster->pixels()[std::size_t(sy) * layer.raster->width() + sx];
                            if (!src.a) continue;
                            auto& dst = out.pixels[(std::size_t(cy - bounds.y) * cell_pixels.height + y) * width +
                                                   (cx - bounds.x) * cell_pixels.width + x];
                            const unsigned alpha = src.a * 255u + dst.a * (255u - src.a);
                            const auto blend = [&](unsigned a, unsigned b)
                            { return static_cast<unsigned char>((a * src.a * 255u + b * dst.a * (255u - src.a) + alpha / 2) / alpha); };
                            dst = {blend(src.r, dst.r), blend(src.g, dst.g), blend(src.b, dst.b),
                                   static_cast<unsigned char>((alpha + 127) / 255)};
                            out.occupied[std::size_t(cy - bounds.y) * bounds.width + cx - bounds.x] = 1;
                        }
                }
        }
        if (std::none_of(out.occupied.begin(), out.occupied.end(), [](auto v) { return v; }))
            return {{}, cell_pixels, {}, {}};
        return out;
    }
} // namespace d2::image
