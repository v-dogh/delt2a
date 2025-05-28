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
		using clipboard = void;
		using audio = void;
		using notifications = void;
	};

	// D2_LOCALE_MODE specifies how characters should be handled
	// Either ASCII or UNICODE
	// The latter treats the characters as 8-bit ascii
	// The former considers utf-8 sequences when rendering
	// UNICODE mode may result in lower performance/higher memory usage since more pixels have to be allocated
	// Additionally in many situations additional conversions/computations need to be performed on utf-8 strings

#	ifndef D2_LOCALE_MODE
#	define D2_LOCALE_MODE ASCII
#	endif

#	if D2_LOCALE_MODE == ASCII
#	define D2_STRING(s) s
#	define D2_CHAR(c) c
	constexpr auto encoding = Encoding::Ascii;
	using string = std::string;
	using string_view = std::string;
	using value_type = char;
#	elif D2_LOCALE_MODE == UNICODE
#	define D2_STRING(s) u##s
#	define D2_CHAR(c) u##c
	constexpr auto encoding = Encoding::Unicode;
	using string = std::u32string;
	using string_view = std::u32string_view;
	using value_type = char32_t;
#	endif
}

#endif // D2_LOCALE_HPP
