#ifndef D2_LOADING_INDICATORS_HPP
#define D2_LOADING_INDICATORS_HPP

#include "../d2_dsl.hpp"
#include <vector>

namespace d2::ctm
{
    D2_STYLESHEET_BEGIN(load_indicator_braille)
        D2_INTERP_GEN_AUTO(1000, Sequential, Value, std::vector<d2::string>{
            "⠇", "⠋", "⠙", "⠸", "⢰", "⣠", "⣄", "⡆"
        })
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(load_indicator_at)
        D2_INTERP_GEN_AUTO(1000, Sequential, Value, std::vector<d2::string>{
            ".", "o", "O", "@", "O", "o",
        });
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(load_indicator_slash)
        D2_INTERP_GEN_AUTO(1000, Sequential, Value, std::vector<d2::string>{
            "|", "/", "-", "\\"
        });
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(load_indicator_crossroads)
        D2_INTERP_GEN_AUTO(1000, Sequential, Value, std::vector<d2::string>{
            "┤", "┘", "┴", "└", "├", "┌", "┬", "┐"
        });
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(load_indicator_snake)
        D2_INTERP_GEN_AUTO(1000, Sequential, Value, std::vector<d2::string>{
            "(   )", "(=  )", "(== )", "(===)", "( ==)", "(  =)"
        });
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(load_indicator_binary)
        D2_INTERP_GEN_AUTO(1000, Sequential, Value, std::vector<d2::string>{
            "1000", "0010", "1010", "0101", "0100", "0001", "1001", "0110"
        });
    D2_STYLESHEET_END
}

#endif // D2_LOADING_INDICATORS_HPP
