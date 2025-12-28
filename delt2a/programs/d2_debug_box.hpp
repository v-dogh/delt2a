#ifndef D2_DEBUG_BOX_HPP
#define D2_DEBUG_BOX_HPP

#include "../elements/d2_std.hpp"
#include "../d2_colors.hpp"
#include "../d2_dsl.hpp"

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

    D2_STATELESS_TREE_ROOT(debug, dx::VirtualBox)
        D2_STYLE(Title, "<Debug Box>")
        D2_STYLE(ZIndex, 127)
        D2_STYLE(Width, 82.0_px)
        D2_STYLE(Height, 26.0_px)
        D2_STYLE(ContainerBorder, true)
        // Controls
        D2_ELEM(FlowBox)
            D2_STYLE(X, 0.0_px)
            D2_STYLE(Y, 0.0_px)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 1.0_px)
            D2_ELEM(Switch)
                D2_STYLE(ZIndex, 0)
                D2_STYLE(Options, Switch::opts{ "Overview", "Modules", "Inspector", "Logs" })
                D2_STYLE(Width, 1.0_pc)
                D2_STYLE(OnChangeValues, [](d2::TypedTreeIter<Switch> ptr, d2::string n, d2::string o) {
                    auto root = ptr->state()->core();
                    if (!o.empty()) root->at(o)->setstate(Element::Display, false);
                    root->at(n)->setstate(Element::Display, true);
                })
            D2_ELEM_END
            D2_ELEM(Text)
                D2_STYLE(ZIndex, 1)
                D2_STYLE(Value, " ")
                D2_STYLE(X, 5.0_pxi)
                D2_INTERPOLATE_TWOWAY_AUTO(
                    Hovered, 500, Linear,
                    ForegroundColor, d2::colors::b::turquoise
                );
                D2_INTERPOLATE_GEN_AUTO(1000, Sequential, Value,
                    std::vector<d2::string>{ "⠇", "⠋", "⠙", "⠸", "⢰", "⣠", "⣄", "⡆" }
                );
                D2_LISTEN(Clicked, true)
                    state->screen()->suspend(
                        !state->screen()->is_suspended()
                    );
                D2_LISTEN_END
            D2_ELEM_END
            D2_ELEM(Text)
                D2_STYLE(ZIndex, 1)
                D2_STYLE(Value, "<X>")
                D2_STYLE(X, 1.0_pxi)
                D2_INTERPOLATE_TWOWAY_AUTO(
                    Hovered, 500, Linear,
                    ForegroundColor, d2::colors::r::crimson
                )
                D2_LISTEN(Clicked, true)
                    state->core()->parent()->remove(
                        state->core()
                    );
                D2_LISTEN_END
            D2_ELEM_END
        D2_ELEM_END
        // Overview
        D2_ELEM(FlowBox, Overview)
            struct Metric
            {
                bool enabled{ false };
                float min{ 1.f };
                float max{ 0.f };
                std::function<float()> callback{ nullptr };
                std::function<std::pair<float, float>()> min_max{ nullptr };
            };
            D2_STATIC(metrics, std::unordered_map<std::string, Metric>)
            D2_STYLE(Y, 1.0_px)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 1.0_pxi)
            // Setup
            D2_ELEM(FlowBox)
                D2_STYLE(ZIndex, 0)
                D2_STYLE(Width, 0.5_pc)
                D2_STYLE(Height, 1.0_pc)
                D2_STYLE(ContainerBorder, true)
                D2_ELEM(Text)
                    D2_STYLE(ZIndex, Box::overlap)
                    D2_STYLE(Value, "<setup>")
                    D2_STYLE(X, 0.0_center)
                D2_ELEM_END
                D2_ELEM(VerticalSwitch)
                    D2_STYLE(Width, 1.0_pc)
                    D2_STYLE(Height, 5.0_px)
                    D2_STYLE(Y, 1.0_relative)
                    D2_STYLE(Options, VerticalSwitch::opts{
                        "Threads", "CPU/AVG", "Memory", "Heap",
                        "Framebuffer", "Delta", "FPS",
                        "Elements", "Animations", "Dynamic Variables",
                        "Logs", "Warnings", "Errors"
                    })
                    D2_STYLE(OnChangeValues, [=](d2::TypedTreeIter<VerticalSwitch>, d2::string n, d2::string) {

                    })
                D2_ELEM_END
                D2_ELEM(ScrollFlowBox)
                    D2_ELEM(Text)
                        D2_STYLE(ZIndex, Box::overlap)
                        D2_STYLE(Value, "<options>")
                        D2_STYLE(X, 0.0_center)
                    D2_ELEM_END
                    D2_STYLE(Width, 1.0_pc)
                    D2_STYLE(Height, 7.0_pxi)
                    D2_STYLE(Y, 1.0_relative)
                    D2_STYLE(ContainerBorder, true)
                    auto option = [&](d2::string name, d2::string label, auto&& callback, bool def = false) {
                        const auto brk = !ptr->empty();
                        D2_CONTEXT(ptr, ScrollFlowBox)
                            D2_ELEM_STR(Checkbox, name)
                                D2_STYLE(Value, def)
                                if (brk) D2_STYLE(Y, 0.0_relative)
                                D2_STYLE(X, 1.0_relative)
                                D2_STYLE(OnSubmit, [=](d2::TypedTreeIter<Checkbox>, bool value) {
                                    callback(value);
                                })
                            D2_ELEM_END
                            D2_ELEM(Text)
                                D2_STYLE(Value, label)
                                D2_STYLE(X, 1.0_relative)
                            D2_ELEM_END
                        D2_CONTEXT_END
                    };
                    option("track", "track", [=](bool value) {
                        metrics->at("");
                    }, false);
                D2_ELEM_END
            D2_ELEM_END
            // Core
            D2_ELEM(Box)
                D2_STYLE(Width, 0.5_pc)
                D2_STYLE(Height, 0.5_pc)
                D2_STYLE(X, 0.5_pc)
                D2_STYLE(ContainerBorder, true)
                D2_ELEM(Text)
                    D2_STYLE(ZIndex, Box::overlap)
                    D2_STYLE(Value, "<core>")
                    D2_STYLE(X, 0.0_center)
                D2_ELEM_END
                D2_ELEM(Text)
                    state->context()->scheduler()->launch_cyclic(std::chrono::milliseconds(50), [=](auto&&...) {
                        auto focused = ptr->screen()->focused();
                        auto info = std::format(
                            R"(Focused: {})",
                            focused == nullptr ? "<Unknown>" : focused->name()
                        );
                        ptr->set<Text::Value>(std::move(info));
                    });
                D2_ELEM_END
            D2_ELEM_END
            // Info
            D2_ELEM(Box)
                D2_STYLE(ZIndex, 1)
                D2_STYLE(X, 0.5_pc)
                D2_STYLE(Y, 0.5_pc)
                D2_STYLE(Width, 0.5_pc)
                D2_STYLE(Height, 0.5_pc)
                D2_STYLE(ContainerBorder, true)
                D2_STATIC(grabbed, bool)
                D2_STATIC(listener, d2::IOContext::AutoEventListener)
                D2_LISTEN(Clicked, true)
                    if (ptr->mouse_object_space().x == 0)
                        *grabbed = true;
                D2_LISTEN_END
                D2_LISTEN_EXPR(Swapped, false, listener.unmute())
                D2_LISTEN_EXPR(Swapped, true, listener.mute())
                *listener = state->context()->listen<d2::IOContext::Event::MouseInput>(
                    [=](IOContext::EventListener, TreeState::ptr state) {
                        const auto in = state->context()->input();
                        if (in->is_pressed_mouse(sys::input::Left))
                        {
                            if (*grabbed)
                            {
                                const auto pos = ptr->parent()->mouse_object_space().x;
                                const auto w = ptr->layout(Element::Layout::Width);
                                const auto pw = ptr->parent()->layout(Element::Layout::Width);
                                const auto position = Unit(float(pos) / pw, Unit::Pc);
                                ptr->set<Box::Width>(Unit(1.f - position.raw(), Unit::Pc));
                                ptr->set<Box::X>(position);
                            }
                        }
                        else
                            *grabbed = false;
                    }
                );
                *grabbed = false;
                D2_ELEM(Text)
                    D2_STYLE(ZIndex, Box::overlap)
                    D2_STYLE(Value, "<info>")
                    D2_STYLE(X, 0.0_center)
                D2_ELEM_END
                D2_ELEM(Box)
                    D2_STYLE(Width, 1.0_pc)
                    D2_STYLE(Height, 1.0_pxi)
                    D2_STYLE(Y, 1.0_px)
                    D2_STYLE(ContainerBorder, true)
                    D2_ELEM(graph::CyclicVerticalBar)
                        D2_STYLE(Width, 1.0_pc)
                        D2_STYLE(Height, 1.0_pc)
                    D2_ELEM_END
                D2_ELEM_END
            D2_ELEM_END
            ptr->setstate(Element::Display, false);
        D2_ELEM_END
        D2_ELEM(ScrollFlowBox, Modules)
            D2_STATIC(update, IOContext::future<void>)
            D2_STYLE(Y, 1.0_px)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 1.0_pxi)
            *update = state->context()->scheduler()->launch_cyclic(std::chrono::seconds(5), [=](auto&&...) {
                auto update_mod_view = [=](TreeIter ptr, sys::SystemComponent* mod)
                {
                    px::foreground color;
                    switch (mod->status())
                    {
                    case sys::SystemComponent::Status::Ok: color = colors::g::green; break;
                    case sys::SystemComponent::Status::Degraded: color = colors::r::yellow; break;
                    case sys::SystemComponent::Status::Failure: color = colors::r::red; break;
                    case sys::SystemComponent::Status::Offline: color = colors::b::blue; break;
                    }
                    ptr.asp()->at("indicator").as<Text>()
                        ->set<Text::ForegroundColor>(color);
                };
                auto create_mod_view = [=](sys::SystemComponent* mod) {
                    D2_CONTEXT(ptr, ScrollFlowBox)
                        D2_ELEM_STR(FlowBox, mod->name())
                            D2_STYLE(Width, 1.0_pc)
                            D2_STYLE(Y, 0.0_relative)
                            D2_STYLE(ContainerBorder, true)
                            D2_ELEM(Text, indicator)
                                D2_STYLE(ZIndex, Box::overlap)
                                D2_STYLE(Value, std::format("<{}>", mod->name()))
                                D2_STYLE(X, 0.0_center)
                            D2_ELEM_END
                            update_mod_view(ptr, mod);
                        D2_ELEM_END
                    D2_CONTEXT_END
                };

                std::unordered_map<std::string, sys::SystemComponent*> mods;
                ptr->state()->context()->sysenum([&](sys::SystemComponent* mod) {
                    mods[mod->name()] = mod;
                    if (!ptr->exists(mod->name()))
                        create_mod_view(mod);
                });

                std::vector<TreeIter> rem;
                ptr->foreach([&](TreeIter ptr) {
                    auto f = mods.find(ptr->name());
                    if (f == mods.end())
                    {
                        rem.push_back(ptr);
                    }
                    else
                    {
                        update_mod_view(ptr, f->second);
                    }
                });
            });
            ptr->setstate(Element::Display, false);
        D2_ELEM_END
        D2_ELEM(Box, Inspector)
            ptr->setstate(Element::Display, false);
        D2_ELEM_END
        D2_ELEM(Box, Logs)
            ptr->setstate(Element::Display, false);
        D2_ELEM_END
    D2_TREE_END
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
                    D2_INTERPOLATE_TWOWAY_AUTO(Hovered, 500, Linear, BackgroundColor, colors::w::silver);
                    D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, BackgroundColor, colors::w::white);
                    D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, ForegroundColor, colors::w::black);
                D2_ELEM_END
            D2_ELEM_END
        D2_ELEM_END
*/

#endif // D2_DEBUG_BOX_HPP
