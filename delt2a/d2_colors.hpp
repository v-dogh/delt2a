#ifndef D2_COLORS_HPP
#define D2_COLORS_HPP

#include "d2_pixel.hpp"

namespace d2::colors
{
	using pf = PixelBackground;
	namespace r
	{
		constexpr pf red{ .r = 255, .g = 0, .b = 0 };
		constexpr pf coral{ .r = 255, .g = 127, .b = 80 };
		constexpr pf chrome_yellow{ .r = 255, .g = 167, .b = 0 };
        constexpr pf yellow{ .r = 255, .g = 255, .b = 0 };
		constexpr pf magenta{ .r = 255, .b = 255 };
		constexpr pf crimson{ .r = 220, .g = 20, .b = 60 };
		constexpr pf salmon{ .r = 250, .g = 128, .b = 114 };
		constexpr pf firebrick{ .r = 178, .g = 34, .b = 34 };
	}
	namespace g
	{
		constexpr pf green{ .r = 0, .g = 255, .b = 0 };
		constexpr pf sea_green{ .r = 46, .g = 139, .b = 87 };
		constexpr pf olive{ .r = 128, .g = 128 };
		constexpr pf aquamarine{ .r = 127, .g = 255, .b = 212 };
		constexpr pf lime{ .r = 50, .g = 205, .b = 50 };
		constexpr pf forest_green{ .r = 34, .g = 139, .b = 34 };
		constexpr pf chartreuse{ .r = 127, .g = 255, .b = 0 };
	}
	namespace b
	{
		constexpr pf blue{ .r = 0, .g = 0, .b = 255 };
		constexpr pf slate_blue{ .r = 106, .g = 90, .b = 205 };
		constexpr pf dark_slate_blue{ .r = 72, .g = 61, .b = 139 };
		constexpr pf indigo{ .r = 75, .g = 0, .b = 130 };
		constexpr pf turquoise{ .r = 64, .g = 224, .b = 208 };
		constexpr pf cyan{ .g = 255, .b = 255 };
		constexpr pf royal_blue{ .r = 65, .g = 105, .b = 225 };
		constexpr pf steel_blue{ .r = 70, .g = 130, .b = 180 };
		constexpr pf midnight_blue{ .r = 25, .g = 25, .b = 112 };
	}
	namespace w
	{
		constexpr pf white{ .r = 255, .g = 255, .b = 255 };
		constexpr pf light_gray{ .r = 210, .g = 210, .b = 210 };
		constexpr pf gray{ .r = 128, .g = 128, .b = 128 };
		constexpr pf dark_gray{ .r = 76, .g = 76, .b = 76 };
		constexpr pf silver{ .r = 192, .g = 192, .b = 192 };
		constexpr pf black{ .r = 0, .g = 0, .b = 0 };
		constexpr pf transparent{ .a = 0 };
	}
}

#endif // D2_COLORS_HPP
