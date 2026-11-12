#include "d2_kitty_images.hpp"
#include <algorithm>
#include <format>
#include <absl/strings/escaping.h>
#include <atomic>

namespace d2
{
    namespace sys 
    {
        std::string SystemImagesKitty::_encode(const image::Composite& image, std::span<const pixel>, int)
        {
            if (!_id)
            {
                static std::atomic<std::uint64_t> next{1};
                const auto id = next.fetch_add(1);
                if (id > UINT32_MAX) throw std::overflow_error("Image ID space exhausted");
                _id = std::uint32_t(id);
            }
            return image::encode_kitty(image, _id);
        }
        std::string SystemImagesKitty::_erase() const
        {
            return std::format("\x1b_Ga=d,d=I,i={},q=2;\x1b\\", _id);
        }
    }

    namespace image 
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
    }
}
