#ifndef D2_LOCALE_HPP
#define D2_LOCALE_HPP

#include <string>
#include <limits>
#include "delt2a/d2_extended_page.hpp"

namespace d2
{
	enum class OS : std::size_t
	{
		Default,
		Unix,
		Linux,
		Windows,
		MacOS,
		Reserved = MacOS + std::numeric_limits<std::size_t>::max() / 2
	};
	enum class Encoding
	{
		Ascii,
        Unicode
	};

	template<std::size_t ID>
	struct OSConfig
	{
		using input = void;
		using output = void;
        using screen = void;
		using clipboard = void;
		using audio = void;
		using notifications = void;
	};

#	if D2_LOCALE_MODE == ASCII
	constexpr auto encoding = Encoding::Ascii;
	using string = std::string;
	using string_view = std::string;
    using value_type = char;
#	elif D2_LOCALE_MODE == UNICODE
    constexpr auto encoding = Encoding::Unicode;
    using string = std::string;
    using string_view = std::string_view;
    using value_type = AutoValueType;
#	else
#	error(Invalid locale mode)
#	endif
}

#endif // D2_LOCALE_HPP
