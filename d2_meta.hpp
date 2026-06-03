#pragma once

#include <algorithm>
#include <string_view>

namespace d2::meta
{
    template <std::size_t Count>
    struct ConstString
    {
        constexpr ConstString(const char (&str)[Count])
        {
            std::copy(str, str + Count, data.begin());
        }
        constexpr ConstString(std::string_view str)
        {
            std::copy(str.begin(), str.begin() + Count, data.begin());
            data[Count - 1] = '\0';
        }
        std::array<char, Count> data{};

        constexpr auto view() const
        {
            return std::string_view(data.data());
        }
        constexpr auto view()
        {
            return std::string_view(data.data());
        }
    };
}

