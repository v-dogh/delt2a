#ifndef D2_CHARDEF_HPP
#define D2_CHARDEF_HPP

#include "d2_locale.hpp"

namespace d2
{
	template<Encoding Ed>
	struct Charset;

	template<>
	struct Charset<Encoding::Ascii>
	{
		static constexpr auto top_fix = true;
		static constexpr auto corners = false;
		static constexpr auto box_top_bar = '.';
		static constexpr auto box_tr_corner = '+';
		static constexpr auto box_tl_corner = '+';
		static constexpr auto box_br_corner = '+';
		static constexpr auto box_bl_corner = '+';
		static constexpr auto box_tr_corner_smooth = '+';
		static constexpr auto box_tl_corner_smooth = '+';
		static constexpr auto box_br_corner_smooth = '+';
		static constexpr auto box_bl_corner_smooth = '+';
		static constexpr auto box_border_vertical = '|';
		static constexpr auto box_border_horizontal = '_';

		static constexpr auto checkbox_true = 'x';
		static constexpr auto checkbox_false = 'o';
		static constexpr auto switch_separator_horizontal = '-';
		static constexpr auto switch_separator_vertical = '|';
		static constexpr auto slider_horizontal = '-';
		static constexpr auto slider_vertical = '|';
		static constexpr auto slider_thumb_vertical = ' ';
		static constexpr auto slider_thumb_horizontal = ' ';
		static constexpr auto radio_checked = '*';
		static constexpr auto radio_unchecked = 'o';

		static constexpr auto arrow_up = '^';
		static constexpr auto arrow_down = 'v';
		static constexpr auto arrow_left = '<';
		static constexpr auto arrow_right = '>';
	};

	template<>
	struct Charset<Encoding::Unicode>
	{
		static constexpr auto top_fix = false;
		static constexpr auto corners = true;
		static constexpr auto box_top_bar = u'░';
		static constexpr auto box_tr_corner = u'┐';
		static constexpr auto box_tl_corner = u'┌';
		static constexpr auto box_br_corner = u'┘';
		static constexpr auto box_bl_corner = u'└';
		static constexpr auto box_tr_corner_smooth = u'╮';
		static constexpr auto box_tl_corner_smooth = u'╭';
		static constexpr auto box_br_corner_smooth = u'╯';
		static constexpr auto box_bl_corner_smooth = u'╰';
		static constexpr auto box_border_vertical = u'│';
		static constexpr auto box_border_horizontal = u'─';

		static constexpr auto checkbox_true = u'☑';
		static constexpr auto checkbox_false = u'☐';
		static constexpr auto switch_separator_horizontal = u'━';
		static constexpr auto switch_separator_vertical = u'┃';
		static constexpr auto slider_horizontal = u'─';
		static constexpr auto slider_vertical = u'│';
		static constexpr auto slider_thumb_vertical = u'█';
		static constexpr auto slider_thumb_horizontal = u'█';
		static constexpr auto radio_checked = u'◉';
		static constexpr auto radio_unchecked = u'◯';

		static constexpr auto arrow_up = u'↑';
		static constexpr auto arrow_down = u'↓';
		static constexpr auto arrow_left = u'←';
		static constexpr auto arrow_right = u'→';
	};

	using charset = Charset<encoding>;
}

#endif // D2_CHARDEF_HPP
