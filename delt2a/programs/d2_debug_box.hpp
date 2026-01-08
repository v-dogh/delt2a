#ifndef D2_DEBUG_BOX_HPP
#define D2_DEBUG_BOX_HPP

#include "../elements/d2_std.hpp"
#include "../d2_colors.hpp"
#include "../d2_dsl.hpp"

namespace d2::prog
{
	// Ima just put these rad af load animations in here for later bcs why not
    // D2_INTERP_GEN(1000, Sequential, Text, value, std::vector<d2::string>{ "⠇", "⠋", "⠙", "⠸", "⢰", "⣠", "⣄", "⡆" });
    // D2_INTERP_GEN(1000, Sequential, Text, value, std::vector<d2::string>{ ".", "o", "O", "@", "O", "o", });
    // D2_INTERP_GEN(1000, Sequential, Text, value, std::vector<d2::string>{ "|", "/", "-", "\\" });
    // D2_INTERP_GEN(1000, Sequential, Text, value, std::vector<d2::string>{ "┤", "┘", "┴", "└", "├", "┌", "┬", "┐" });
    // D2_INTERP_GEN(1000, Sequential, Text, value, std::vector<d2::string>{
	// 	"(   )", "(=  )", "(== )", "(===)", "( ==)", "(  =)"
	// });
    // D2_INTERP_GEN(1000, Sequential, Text, value, std::vector<d2::string>{
	// 	"1000", "0010", "1010", "0101", "0100", "0001", "1001", "0110"
	// });

    namespace impl
    {
        std::uint64_t ts_to_ns(const timespec& ts);
        float bytes_to_mb(std::uint64_t b);
        std::uint64_t total_ram_bytes();
        std::uint64_t read_proc_self_rss_bytes();
        std::uint64_t heap_used_bytes();
    }
    D2_STATELESS_TREE_ROOT_FORWARD(debug, dx::VirtualBox)
}

/*
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
                    D2_INTERP_TWOWAY_AUTO(Hovered, 500, Linear, BackgroundColor, colors::w::silver);
                    D2_INTERP_TWOWAY_AUTO(Clicked, 100, Linear, BackgroundColor, colors::w::white);
                    D2_INTERP_TWOWAY_AUTO(Clicked, 100, Linear, ForegroundColor, colors::w::black);
                D2_ELEM_END
            D2_ELEM_END
        D2_ELEM_END
*/

#endif // D2_DEBUG_BOX_HPP
