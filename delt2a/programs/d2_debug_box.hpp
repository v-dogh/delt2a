#ifndef D2_DEBUG_BOX_HPP
#define D2_DEBUG_BOX_HPP

#include "../d2_screen.hpp"
#include "../d2_colors.hpp"
#include "../elements/d2_text.hpp"
#include "../elements/d2_draggable_box.hpp"
#include "../elements/d2_scrollbox.hpp"
#include "../elements/d2_input.hpp"
#include "../elements/d2_button.hpp"
#include "../d2_dsl.hpp"
#include "../d2_tree.hpp"
#include "delt2a/os/d2_unix_system_io.hpp"

#include <delt2a/elements/d2_flow_box.hpp>
#include <delt2a/elements/d2_separator.hpp>

namespace d2::prog
{
	// Ima just put these rad af load animations in here for later bcs why not
    // D2_INTERPOLATE_GEN(1000, Sequential, Text, value, std::vector<d2::string>{ "⠇", "⠋", "⠙", "⠸", "⢰", "⣠", "⣄", "⡆" });
	// D2_INTERPOLATE_GEN(1000, Sequential, Text, value, std::vector<d2::string>{ ".", "o", "O", "@", "O", "o", });
	// D2_INTERPOLATE_GEN(1000, Sequential, Text, value, std::vector<d2::string>{ "|", "/", "-", "\\" });
	// D2_INTERPOLATE_GEN(1000, Sequential, Text, value, std::vector<d2::string>{ "┤", "┘", "┴", "└", "├", "┌", "┬", "┐" });
	// D2_INTERPOLATE_GEN(1000, Sequential, Text, value, std::vector<d2::string>{
	// 	"(   )", "(=  )", "(== )", "(===)", "( ==)", "(  =)"
	// });
	// D2_INTERPOLATE_GEN(1000, Sequential, Text, value, std::vector<d2::string>{
	// 	"1000", "0010", "1010", "0101", "0100", "0001", "1001", "0110"
	// });

	using namespace d2::dx;
    D2_STATELESS_TREE_ROOT(debug, VirtualBox)
        D2_STYLE(Title, "<Debug Box>")
        D2_STYLE(ZIndex, 127)
        D2_STYLE(Width, 48.0_px)
        D2_STYLE(Height, 17.0_px)
        D2_STYLE(ContainerBorder, true)
        // Controls
        D2_ELEM(Text, exit)
            D2_STYLE(Value, "<X>")
            D2_STYLE(Y, 0.0_px)
            D2_INTERPOLATE_TWOWAY_AUTO(
                Hovered, 500, Linear,
                ForegroundColor, d2::colors::r::crimson
            )
            D2_LISTEN(Clicked, true)
                state->screen()->stop_blocking();
            D2_LISTEN_END
        D2_ELEM_END
        D2_ELEM(Text, indicator)
            D2_STYLE(Y, 0.0_px)
            D2_STYLE(X, 4.0_px)
            D2_STYLE(Value, "(   )")
            D2_INTERPOLATE_TWOWAY_AUTO(
                Hovered, 500, Linear,
                ForegroundColor, d2::colors::b::turquoise
            );
            D2_INTERPOLATE_GEN_AUTO(1000, Sequential, Value, std::vector<d2::string>{
                "(   )", "(=  )", "(== )", "(===)", "( ==)", "(  =)"
            });
            D2_LISTEN(Clicked, true)
                state->screen()->suspend(
                    !state->screen()->is_suspended()
                );
            D2_LISTEN_END
        D2_ELEM_END
        // Main display
        D2_ELEM(ScrollFlowBox)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 1.0_pc)
            D2_STYLE(Y, 1.0_px)
            // Performance
            D2_ELEM(Box, performance-box)
                D2_STYLE(Width, 0.5_pc)
                D2_STYLE(Height, 1.0_pxi)
                D2_STYLE(X, 0.0_relative)
                D2_ELEM(Text)
                    D2_STYLE(Value, "Info")
                    D2_STYLE(X, 0.0_center)
                D2_ELEM_END
                D2_ELEM(Input, input)
                    D2_STYLE(Pre, "<fps>")
                    D2_STYLE(X, 1.0_px)
                    D2_STYLE(Y, 1.0_px)
                    D2_STYLE(DrawPtr, true)
                    D2_STYLE(Blink, true)
                    D2_STYLE(OnSubmit, [](d2::TreeIter ptr, d2::string value) {
                        try
                        {
                            ptr
                                ->state()
                                ->screen()
                                ->set_refresh_rate(
                                    d2::Screen::fps(std::stoul(value))
                                );
                        }
                        catch (...) {}
                    })
                D2_ELEM_END
                D2_ELEM(Text, performance-values)
                    D2_STYLE(X, 0.0_center)
                    D2_STYLE(Y, 2.0_px)
                    D2_STYLE(TextHandleNewlines, true)
                    D2_CYCLIC_TASK(1000)
                        static std::function<std::size_t(d2::TreeIter)> func = nullptr;
                        static auto count_elements_recursive = [](d2::TreeIter elem) -> std::size_t {
                            std::size_t cnt = 0;
                            elem.foreach([&](d2::TreeIter h) {
                                cnt++;
                                cnt += func(h);
                            });
                            return cnt;
                        };
                        if (func == nullptr)
                            func = count_elements_recursive;

                        std::size_t cnt = count_elements_recursive(state->screen()->root()) + 1;
                        std::string f = "-";
                        if (state->screen()->focused() != nullptr)
                        {
                            f = state->screen()->focused()->name();
                        }
                        else
                        {
                            f = "<null>";
                        }

                        ptr.as<Text>()->set<Text::Value>(std::format(
                            "Components: {}\nFps/avg: {}\nDelta: {}\nFbuf: {}\nAnimations: {}\nElements: {}\nFocused: {}\nMouse: [{} {}]\nDimensions: [{} {}]",
                            state->context()->syscnt(),
                            state->screen()->fps(),
                            state->screen()->delta(),
                            state->context()->output()->as<sys::UnixTerminalOutput>()->buffer_size(),
                            state->screen()->animations(),
                            cnt,
                            f,
                            state->context()->input()->mouse_position().first,
                            state->context()->input()->mouse_position().second,
                            state->context()->input()->screen_size().first,
                            state->context()->input()->screen_size().second
                        ));
                    D2_CYCLIC_END
                D2_ELEM_END
            D2_ELEM_END
            D2_ELEM(VerticalSeparator)
                D2_STYLE(Height, 1.0_pc)
                D2_STYLE(X, 0.0_relative)
            D2_ELEM_END
            // Clipboard
            D2_ELEM(FlowBox, clipboard-box)
                D2_STYLE(Width, 0.5_pc)
                D2_STYLE(Height, 1.0_pxi)
                D2_STYLE(X, 0.0_relative)
                D2_ELEM(Text)
                    D2_STYLE(Value, "Clipboard")
                    D2_STYLE(X, 0.0_center)
                    D2_STYLE(Y, 0.0_relative)
                D2_ELEM_END
                D2_ELEM(Text)
                    D2_STYLE(Value, "Status: ")
                    D2_STYLE(X, 0.0_center)
                    D2_STYLE(Y, 0.0_relative)
                    D2_CYCLIC_TASK(5000)
                        auto* clipboard = state->context()->sys_if<sys::ext::SystemClipboard>();
                        ptr.as<Text>()->set<Text::Value>(std::format("Status: {}",
                            !clipboard ? "not loaded" : std::format("loaded:{}",
                                clipboard->empty() ? "empty" : "full"
                            )
                        ));
                    D2_CYCLIC_END
                D2_ELEM_END
                D2_ELEM(Text)
                    D2_STYLE(Value, "Value: ")
                    D2_STYLE(X, 0.0_center)
                    D2_STYLE(Y, 0.0_relative)
                    D2_STYLE(TextHandleNewlines, true)
                    D2_CYCLIC_TASK(5000)
                        auto* clipboard = state->context()->sys_if<sys::ext::SystemClipboard>();
                        ptr.as<Text>()->set<Text::Value>(std::format("Value: {}",
                            (!clipboard || clipboard->empty()) ?
                                "<null>" :
                                std::format("'{}'", clipboard->paste())
                        ));
                    D2_CYCLIC_END
                D2_ELEM_END
                D2_ELEM(Input)
                    D2_STYLE(Pre, "Copy: ")
                    D2_STYLE(X, 0.0_center)
                    D2_STYLE(Y, 0.0_relative)
                    D2_STYLE(Width, 9.0_pxi)
                    D2_STYLE(OnSubmit, [](d2::TreeIter ptr, string value) {
                        ptr->state()->context()
                            ->sys<sys::ext::SystemClipboard>()
                            ->copy(value);
                    })
                D2_ELEM_END
                D2_ELEM(Button)
                    D2_STYLE(Value, "Clear Data")
                    D2_STYLE(BackgroundColor, colors::w::gray)
                    D2_STYLE(X, 0.0_center)
                    D2_STYLE(Y, 0.0_relative)
                    D2_STYLE(OnSubmit, [](d2::TreeIter ptr) {
                        ptr->state()->context()
                            ->sys<sys::ext::SystemClipboard>()
                            ->clear();
                    })
                    D2_INTERPOLATE_TWOWAY_AUTO(Hovered, 500, Linear, BackgroundColor, colors::w::silver);
                    D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, BackgroundColor, colors::w::white);
                    D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, ForegroundColor, colors::w::black);
                D2_ELEM_END
            D2_ELEM_END
        D2_ELEM_END
    D2_TREE_END
}

#endif // D2_DEBUG_BOX_HPP
