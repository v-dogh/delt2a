#pragma once

#include <core/platform/d2_extended_page.hpp>

#include <limits>
#include <string>

namespace d2
{
    enum class Encoding
    {
        Ascii,
        Unicode
    };

    template<std::size_t ID> struct OSConfig
    {
        using input = void;
        using output = void;
        using screen = void;
        using clipboard = void;
        using audio = void;
        using notifications = void;
    };

#if D2_LOCALE_MODE == ASCII
    constexpr auto encoding = Encoding::Ascii;
    using string = std::string;
    using string_view = std::string;
    using value_type = char;
#elif D2_LOCALE_MODE == UNICODE
    constexpr auto encoding = Encoding::Unicode;
    using string = std::string;
    using string_view = std::string_view;
    using value_type = AutoValueType;
#else
#error (Invalid locale mode)
#endif
} // namespace d2

