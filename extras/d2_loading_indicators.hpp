#ifndef D2_LOADING_INDICATORS_HPP
#define D2_LOADING_INDICATORS_HPP

#include <chrono>
#include <d2_tree_construct.hpp>
#include <vector>

namespace d2::ex
{
    using load_indicator_braille =
        Stylesheet<[]<typename Type>(
                       TreeCtx<Type> ctx,
                       std::chrono::milliseconds period = std::chrono::milliseconds(1000)
                   )
                   {
                       ctx.template animate<anim::Sequential, Type::Value>(
                           period, std::vector<d2::string>{"⠇", "⠋", "⠙", "⠸", "⢰", "⣠", "⣄", "⡆"}
                       );
                   }>;
    using load_indicator_breathing =
        Stylesheet<[]<typename Type>(
                       TreeCtx<Type> ctx,
                       std::chrono::milliseconds period = std::chrono::milliseconds(1000)
                   )
                   {
                       ctx.template animate<anim::Sequential, Type::Value>(
                           period,
                           std::vector<d2::string>{
                               ".",
                               "o",
                               "O",
                               "@",
                               "O",
                               "o",
                           }

                       );
                   }>;
    using load_indicator_slash =
        Stylesheet<[]<typename Type>(
                       TreeCtx<Type> ctx,
                       std::chrono::milliseconds period = std::chrono::milliseconds(1000)
                   )
                   {
                       ctx.template animate<anim::Sequential, Type::Value>(
                           period, std::vector<d2::string>{"|", "/", "-", "\\"}
                       );
                   }>;
    using load_indicator_cross =
        Stylesheet<[]<typename Type>(
                       TreeCtx<Type> ctx,
                       std::chrono::milliseconds period = std::chrono::milliseconds(1000)
                   )
                   {
                       ctx.template animate<anim::Sequential, Type::Value>(
                           period, std::vector<d2::string>{"┤", "┘", "┴", "└", "├", "┌", "┬", "┐"}
                       );
                   }>;
    using load_indicator_scan =
        Stylesheet<[]<typename Type>(
                       TreeCtx<Type> ctx,
                       std::chrono::milliseconds period = std::chrono::milliseconds(1000)
                   )
                   {
                       ctx.template animate<anim::Sequential, Type::Value>(
                           period,
                           std::vector<d2::string>{
                               "(   )", "(=  )", "(== )", "(===)", "( ==)", "(  =)"
                           }
                       );
                   }>;
    using load_indicator_binary =
        Stylesheet<[]<typename Type>(
                       TreeCtx<Type> ctx,
                       std::chrono::milliseconds period = std::chrono::milliseconds(1000)
                   )
                   {
                       ctx.template animate<anim::Sequential, Type::Value>(
                           period,
                           std::vector<d2::string>{
                               "1000", "0010", "1010", "0101", "0100", "0001", "1001", "0110"
                           }
                       );
                   }>;
} // namespace d2::ex

#endif // D2_LOADING_INDICATORS_HPP
