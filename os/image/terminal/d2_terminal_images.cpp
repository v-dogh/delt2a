#include "d2_terminal_images.hpp"
#include <core/io/d2_context.hpp>
#include <core/screen/d2_screen.hpp>
#include <absl/strings/escaping.h>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <format>
#ifdef D2_IMAGE_PNG
#include <png.h>
#endif

namespace d2::image
{
    std::string encode_kitty(const Composite& image, std::uint32_t id)
    {
        if (image.empty()) return {};
        static_assert(sizeof(RGBA) == 4);
        const auto bytes = std::string_view(reinterpret_cast<const char*>(image.pixels.data()), image.pixels.size() * 4);
        const auto encoded = absl::Base64Escape(bytes);
        std::string out = "\x1b" "7";
        for (std::size_t begin = 0; begin < encoded.size(); begin += 4096)
        {
            const auto count = std::min<std::size_t>(4096, encoded.size() - begin);
            const auto more = begin + count < encoded.size();
            out += begin ? std::format("\x1b_Gm={};", int(more)) :
                std::format("\x1b_Ga=t,t=d,f=32,s={},v={},i={},q=2,m={};", image.width(), image.height(), id, int(more));
            out.append(encoded, begin, count);
            out += "\x1b\\";
        }
        out += std::format("\x1b[{};{}H\x1b_Ga=p,i={},p=1,c={},r={},z=0,C=1,q=2;\x1b\\\x1b" "8",
                           image.cells.y + 1, image.cells.x + 1, id, image.cells.width, image.cells.height);
        return out;
    }
    std::string encode_sixel(const Composite& image, std::span<const pixel> cells, int columns)
    {
        if (image.empty()) return {};
        std::vector<int> indices(image.pixels.size(), -1);
        std::array<bool, 216> used{};
        for (int y = 0; y < image.height(); ++y)
            for (int x = 0; x < image.width(); ++x)
            {
                const auto index = std::size_t(y) * image.width() + x;
                const auto color = image.pixels[index];
                if (!color.a) continue;
                const auto& bg = cells[std::size_t(image.cells.y + y / image.cell_pixels.height) * columns +
                                       image.cells.x + x / image.cell_pixels.width];
                const auto channel = [&](int value, int background)
                { return ((value * color.a + background * (255 - color.a) + 127) / 255 * 5 + 127) / 255; };
                const auto key = channel(color.r, bg.r) * 36 + channel(color.g, bg.g) * 6 + channel(color.b, bg.b);
                indices[index] = key;
                used[key] = true;
            }
        std::string out = std::format("\x1b" "7\x1b[?80s\x1b[?80l\x1b[{};{}H\x1bP0;1;0q\"1;1;{};{}",
                                      image.cells.y + 1, image.cells.x + 1, image.width(), image.height());
        for (int i = 0; i < 216; ++i)
            if (used[i]) out += std::format("#{};2;{};{};{}", i, (i / 36) * 20, (i / 6 % 6) * 20, (i % 6) * 20);
        std::string row(image.width(), '?');
        for (int band = 0; band < image.height(); band += 6)
        {
            std::array<bool, 216> band_colors{};
            for (int y = band; y < std::min(band + 6, image.height()); ++y)
                for (int x = 0; x < image.width(); ++x)
                    if (const auto key = indices[std::size_t(y) * image.width() + x]; key >= 0)
                        band_colors[key] = true;
            for (int color = 0; color < 216; ++color)
            {
                if (!band_colors[color]) continue;
                std::fill(row.begin(), row.end(), '?');
                for (int x = 0; x < image.width(); ++x)
                    for (int y = band; y < std::min(band + 6, image.height()); ++y)
                        if (indices[std::size_t(y) * image.width() + x] == color)
                            row[x] += char(1 << (y - band));
                const auto last = row.find_last_not_of('?') + 1;
                out += std::format("#{}", color);
                for (std::size_t i = 0; i < last;)
                {
                    auto end = i + 1;
                    while (end < last && row[end] == row[i]) ++end;
                    if (end - i >= 4) out += std::format("!{}{}", end - i, row[i]);
                    else out.append(end - i, row[i]);
                    i = end;
                }
                out += '$';
            }
            if (band + 6 < image.height()) out += '-';
        }
        return out + "\x1b\\\x1b[?80r\x1b" "8";
    }
}
namespace d2::sys
{
    void TerminalImages::configure(Config config)
    {
        if (config.cell_pixels.width < 0 || config.cell_pixels.height < 0 ||
            bool(config.cell_pixels.width) != bool(config.cell_pixels.height))
            throw std::invalid_argument("Image cell metrics must be both zero or both positive");
        if (config.protocol != Protocol::Off && config.protocol != Protocol::Kitty && config.protocol != Protocol::Sixel)
            throw std::invalid_argument("Unknown image protocol");
        _config = config;
        _invalid = true;
        if (auto ctx = context())
            if (auto screen = ctx->sys_if<SystemScreen>(); screen && screen->root() != nullptr)
                internal::ElementView::from(screen->root()).signal_write(Element::WriteType::Style);
    }
    SystemImages::Plan TerminalImages::prepare(std::span<pixel> cells, BoundingBox viewport,
                                              const image::Scene& scene, BoundingBox metrics)
    {
        if (viewport.width < 0 || viewport.height < 0 || cells.size() != std::size_t(viewport.width) * viewport.height)
            throw std::invalid_argument("Image output frame dimensions");
        if (_config.protocol == Protocol::Off) return reset();
        if (_config.cell_pixels.width) metrics = _config.cell_pixels;
        if (metrics.width <= 0 || metrics.height <= 0)
        {
            if (_config.protocol == Protocol::Sixel && !scene.empty())
                throw std::invalid_argument("Sixel requires terminal cell pixel dimensions");
            metrics = {8, 16};
        }
        const auto changed = _invalid || _previous != _config.protocol || _scene != scene ||
                             _viewport != viewport || _composite.cell_pixels != metrics;
        auto composite = changed ? image::compose(scene, viewport, metrics) : image::Composite{};
        const auto& current = changed ? composite : _composite;
        for (int y = 0; y < current.cells.height; ++y)
            for (int x = 0; x < current.cells.width; ++x)
                if (current.occupied[std::size_t(y) * current.cells.width + x])
                {
                    auto& cell = cells[std::size_t(current.cells.y + y) * viewport.width + current.cells.x + x];
                    cell.v = ' ';
                    cell.style = px::style{};
                }
        const auto text_changed = _config.protocol == Protocol::Sixel &&
            (_cells.size() != cells.size() || !std::equal(_cells.begin(), _cells.end(), cells.begin()));
        const auto redraw = changed || (_config.protocol == Protocol::Sixel && text_changed);
        Plan plan;
        if (redraw)
        {
            if (_placed && _previous == Protocol::Kitty)
                plan.before = std::format("\x1b_Ga=d,d=I,i={},q=2;\x1b\\", _id);
            plan.repaint = (_placed && _previous == Protocol::Sixel) ||
                           (!current.empty() && _config.protocol == Protocol::Sixel);
            if (plan.repaint) plan.before += "\x1b[2J";
            if (!current.empty())
            {
                if (!_id)
                {
                    static std::atomic<std::uint64_t> next{1};
                    const auto id = next.fetch_add(1);
                    if (id > UINT32_MAX) throw std::overflow_error("Image ID space exhausted");
                    _id = std::uint32_t(id);
                }
                plan.after = _config.protocol == Protocol::Kitty ? image::encode_kitty(current, _id) :
                    image::encode_sixel(current, cells, viewport.width);
            }
        }
        _placed = !current.empty();
        if (changed) { _composite = std::move(composite); _scene = scene; }
        _viewport = viewport;
        if (_config.protocol == Protocol::Sixel) _cells.assign(cells.begin(), cells.end());
        else _cells.clear();
        _previous = _config.protocol;
        _invalid = false;
        return plan;
    }
    SystemImages::Plan TerminalImages::reset()
    {
        Plan plan;
        if (_placed && _previous == Protocol::Kitty)
            plan.before = std::format("\x1b_Ga=d,d=I,i={},q=2;\x1b\\", _id);
        plan.repaint = _placed && _previous == Protocol::Sixel;
        if (plan.repaint) plan.before += "\x1b[2J";
        _placed = false;
        _previous = Protocol::Off;
        _scene.clear(); _composite = {}; _cells.clear();
        _invalid = true;
        return plan;
    }
    TerminalImages::Status TerminalImages::_unload_impl()
    {
        if (auto ctx = context())
            if (auto output = ctx->sys_if<SystemOutput>(); output)
                output->clear_images();
        return Status::Ok;
    }
    image::Raster::ptr TerminalImages::decode_png(std::span<const unsigned char> bytes) const
    {
#ifdef D2_IMAGE_PNG
        if (bytes.empty() || bytes.size() > 64 * 1024 * 1024)
            throw std::invalid_argument("PNG input byte limit");
        png_image png{};
        png.version = PNG_IMAGE_VERSION;
        struct Cleanup { png_image& png; ~Cleanup() { png_image_free(&png); } } cleanup{png};
        if (!png_image_begin_read_from_memory(&png, bytes.data(), bytes.size()))
            throw std::invalid_argument(std::string("PNG header: ") + png.message);
        if (!png.width || !png.height || png.width > image::Raster::max_pixels / png.height)
            throw std::length_error("PNG exceeds image pixel limit");
        png.format = PNG_FORMAT_RGBA;
        std::vector<image::RGBA> pixels(std::size_t(png.width) * png.height);
        if (!png_image_finish_read(&png, nullptr, pixels.data(), 0, nullptr))
            throw std::invalid_argument(std::string("PNG decode: ") + png.message);
        return image::Raster::rgba(int(png.width), int(png.height), std::move(pixels));
#else
        throw std::logic_error("PNG decoding requires a build with libpng");
#endif
    }
    image::Raster::ptr TerminalImages::load_png(const std::string& path) const
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) throw std::runtime_error("Cannot open PNG file: " + path);
        const auto size = file.tellg();
        if (size < 0 || size > std::streamoff(64 * 1024 * 1024))
            throw std::length_error("PNG input exceeds byte limit");
        std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
        file.seekg(0);
        if (!file.read(reinterpret_cast<char*>(bytes.data()), size))
            throw std::runtime_error("Cannot read PNG file: " + path);
        return decode_png(bytes);
    }
}
