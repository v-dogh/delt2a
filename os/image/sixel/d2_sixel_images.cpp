#include "d2_sixel_images.hpp"
#include <algorithm>
#include <format>

namespace d2
{
    namespace sys 
    {
        std::string SystemImagesSixel::_encode(const image::Composite& image, std::span<const pixel> cells, int columns)
        {
            return image::encode_sixel(image, cells, columns);
        }
        std::string SystemImagesSixel::_erase() const { return "\x1b[2J"; }
    }

    namespace image 
    {
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
}
