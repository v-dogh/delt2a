#ifndef D2_LOCALE_HPP
#define D2_LOCALE_HPP

#include <string>
#include <limits>

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

	template<std::size_t ID>
	struct OSConfig
	{
		using input = void;
		using output = void;
		using clipboard = void;
		using audio = void;
		using notifications = void;
	};

	// D2_LOCALE_MODE specifies how characters should be handled
	// Either ASCII or UTF8
	// The latter treats the characters as 8-bit ascii
	// The former considers utf-8 sequences when rendering
	// UTF8 mode may result in lower performance/higher memory usage since more pixels have to be allocated
	// Additionally in many situations additional conversions/computations need to be performed on utf-8 strings

#	ifndef D2_LOCALE_MODE
#	define D2_LOCALE_MODE ASCII
#	endif

#	if D2_LOCALE_MODE == ASCII
#	define D2_STRING
#	define D2_CHAR
	using string = std::string;
	using string_view = std::string;
	using value_type = char;
#	elif D2_LOCALE_MODE == UTF8
#	define D2_STRING u32
#	define D2_CHAR u32
	using string = std::u32string;
	using string_view = std::u32string_view;
	using value_type = char32_t;
#	endif
}

#endif // D2_LOCALE_HPP
