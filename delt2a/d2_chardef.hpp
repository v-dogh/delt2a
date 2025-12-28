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
        static constexpr auto box_top_drag_spot = ',';
		static constexpr auto box_tr_corner = '+';
		static constexpr auto box_tl_corner = '+';
		static constexpr auto box_br_corner = '+';
		static constexpr auto box_bl_corner = '+';
		static constexpr auto box_tr_corner_smooth = '+';
		static constexpr auto box_tl_corner_smooth = '+';
		static constexpr auto box_br_corner_smooth = '+';
		static constexpr auto box_bl_corner_smooth = '+';
        static constexpr auto box_tr_corner_rough = '+';
        static constexpr auto box_tl_corner_rough = '+';
        static constexpr auto box_br_corner_rough = '+';
        static constexpr auto box_bl_corner_rough = '+';
		static constexpr auto box_border_vertical = '|';
		static constexpr auto box_border_horizontal = '_';
        static constexpr auto box_border_connector_left = "|";
        static constexpr auto box_border_connector_right = "|";
        static constexpr auto box_border_connector_bottom = "_";
        static constexpr auto box_border_connector_top = "_";

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

        static constexpr auto shade_transparent = ' ';
        static constexpr auto shade_opaque = ' ';
	};

	template<>
    struct Charset<Encoding::Unicode>
	{
		static constexpr auto top_fix = false;
		static constexpr auto corners = true;
        static constexpr auto box_top_bar = ".";
        static constexpr auto box_top_drag_spot = ",";
        static constexpr auto box_tr_corner_rough = "┐";
        static constexpr auto box_tl_corner_rough = "┌";
        static constexpr auto box_br_corner_rough = "┘";
        static constexpr auto box_bl_corner_rough = "└";
        static constexpr auto box_tr_corner_smooth = "╮";
        static constexpr auto box_tl_corner_smooth = "╭";
        static constexpr auto box_br_corner_smooth = "╯";
        static constexpr auto box_bl_corner_smooth = "╰";
        static constexpr auto box_tr_corner = box_tr_corner_smooth;
        static constexpr auto box_tl_corner = box_tl_corner_smooth;
        static constexpr auto box_br_corner = box_br_corner_smooth;
        static constexpr auto box_bl_corner = box_bl_corner_smooth;
        static constexpr auto box_border_vertical = "│";
        static constexpr auto box_border_horizontal = "─";
        static constexpr auto box_border_connector_left = "▌";
        static constexpr auto box_border_connector_right = "▐";
        static constexpr auto box_border_connector_bottom = "▄";
        static constexpr auto box_border_connector_top = "▀";

        static constexpr auto box_border_junction_left = "┤";
        static constexpr auto box_border_junction_right = "├";
        static constexpr auto box_border_junction_up = "┴";
        static constexpr auto box_border_junction_down = "┬";
        static constexpr auto box_border_junction_cross = "┼";

        static constexpr auto checkbox_true = "◉";
        static constexpr auto checkbox_false = "○";
        static constexpr auto switch_separator_horizontal = "━";
        static constexpr auto switch_separator_vertical = "┃";
        static constexpr auto slider_horizontal = "━";
        static constexpr auto slider_vertical = "┃";
        static constexpr auto slider_thumb_vertical = "█";
        static constexpr auto slider_thumb_horizontal = "█";
        static constexpr auto radio_checked = "◉";
        static constexpr auto radio_unchecked = "○";

        static constexpr auto arrow_up = "↑";
        static constexpr auto arrow_down = "↓";
        static constexpr auto arrow_left = "←";
        static constexpr auto arrow_right = "→";

        static constexpr auto shade_transparent = "░";
        static constexpr auto shade_opaque = "█";

        struct icon
        {
            static constexpr auto triangle_left = "◀";
            static constexpr auto triangle_right = "▶";
            static constexpr auto triangle_up = "▲";
            static constexpr auto triangle_down = "▼";

            static constexpr auto arrow_left = "←";
            static constexpr auto arrow_right = "→";
            static constexpr auto arrow_up = "↑";
            static constexpr auto arrow_down = "↓";

            static constexpr auto play = "▶";
            static constexpr auto pause = "⏸";
            static constexpr auto stop = "⏹";
            static constexpr auto record = "⏺";
            static constexpr auto next = "⏭";
            static constexpr auto previous = "⏮";

            static constexpr auto checkbox_unchecked = "☐";
            static constexpr auto checkbox_checked  = "☑";
            static constexpr auto checkbox_crossed = "☒";

            static constexpr auto volume_muted = "🔇";
            static constexpr auto volume_low = "🔈";
            static constexpr auto volume_medium = "🔉";
            static constexpr auto volume_high = "🔊";

            static constexpr auto info = "🛈";
            static constexpr auto help = "?⃝";
            static constexpr auto warning = "⚠";
            static constexpr auto error = "❌";
            static constexpr auto check = "✔";
            static constexpr auto cross = "✖";
            static constexpr auto exclamation = "❗";
            static constexpr auto question = "❓";

            static constexpr auto keyboard = "⌨";
            static constexpr auto home = "⌂";
            static constexpr auto gear = "⚙";
            static constexpr auto wrench = "🔧";
            static constexpr auto hammer = "🔨";

            static constexpr auto document = "📄";
            static constexpr auto folder = "📁";
            static constexpr auto trash = "🗑";
            static constexpr auto scissors = "✂";

            static constexpr auto star_filled = "★";
            static constexpr auto star_empty = "☆";
            static constexpr auto heart = "❤";
            static constexpr auto heart_empty = "♡";

            static constexpr auto envelope = "✉";
            static constexpr auto phone = "☎";
        };


        enum Cell
        {
            TopLeft = 1 << 0, TopRight = 1 << 3,
            MidTopLeft = 1 << 1, MidTopRight = 1 << 4,
            MidBotLeft = 1 << 2, MidBotRight = 1 << 5,
            BotLeft = 1 << 6, BotRight = 1 << 7,
        };
        static std::string cell(uint8_t mask)
        {
            const char32_t ch = 0x2800 + mask;
            std::string s;
            if (ch <= 0xFFFF)
            {
                s.push_back(char(0xE0 | ((ch >> 12) & 0x0F)));
                s.push_back(char(0x80 | ((ch >> 6)  & 0x3F)));
                s.push_back(char(0x80 | (ch & 0x3F)));
            }
            return s;
        }
	};

    using charset = Charset<encoding>;
}

#endif // D2_CHARDEF_HPP
