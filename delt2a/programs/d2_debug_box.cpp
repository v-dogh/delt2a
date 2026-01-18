#include "d2_debug_box.hpp"

namespace d2::prog::impl
{
    static float MiB(std::uint64_t b)
    {
        return float(b) / (1024.0f * 1024.0f);
    }
    static float KiB(std::uint64_t b)
    {
        return float(b) / 1024.0f;
    }
}

#ifdef __linux__

#include <fstream>
#include <cstdio>
#include <unistd.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <malloc.h>

namespace d2::prog::impl
{
    static float cpu_usage()
    {
        static auto ts_to_ns = [](const timespec& ts) -> std::uint64_t
        {
            return ts.tv_sec * 1000000000ull + ts.tv_nsec;
        };
        static bool init = false;
        static timespec last_cpu{};
        static std::chrono::steady_clock::time_point last_wall{};

        timespec cpu{};
        if (::clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu) != 0)
            return 0.0f;

        auto wall_now = std::chrono::steady_clock::now();
        if (!init)
        {
            init = true;
            last_cpu = cpu;
            last_wall = wall_now;
            return 0.0f;
        }

        const std::uint64_t cpu_ns_now  = ts_to_ns(cpu);
        const std::uint64_t cpu_ns_last = ts_to_ns(last_cpu);
        const std::uint64_t cpu_ns_delta = (cpu_ns_now >= cpu_ns_last) ? (cpu_ns_now - cpu_ns_last) : 0;

        const auto wall_ns_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(wall_now - last_wall).count();
        if (wall_ns_delta <= 0)
        {
            last_cpu = cpu;
            last_wall = wall_now;
            return 0.0f;
        }

        last_cpu = cpu;
        last_wall = wall_now;

        const auto pct = (double(cpu_ns_delta) / double(wall_ns_delta)) * 100.0;
        return float(pct);
    }
    static std::uint64_t total_ram_bytes()
    {
        struct ::sysinfo info{};
        if (::sysinfo(&info) == 0)
            return std::uint64_t(info.totalram) * std::uint64_t(info.mem_unit);
        return 0;
    }
    static std::uint64_t total_rss_bytes()
    {
        auto* f = std::fopen("/proc/self/statm", "r");
        if (!f)
            return 0;

        unsigned long size_pages = 0;
        unsigned long resident_pages = 0;
        const auto n = std::fscanf(f, "%lu %lu", &size_pages, &resident_pages);
        std::fclose(f);

        if (n != 2)
            return 0;
        const auto page_size = ::sysconf(_SC_PAGESIZE);
        if (page_size <= 0)
            return 0;
        return std::uint64_t(resident_pages) * std::uint64_t(page_size);
    }
    static std::uint64_t heap_used_bytes()
    {
        auto mi = ::mallinfo2();
        return std::uint64_t(mi.uordblks);
    }
    static std::uint64_t thread_count()
    {
        std::ifstream file(std::format("/proc/{}/status", getpid()));
        if (!file.is_open())
            return 0;

        std::string line;
        while (std::getline(file, line))
            if (line.rfind("Threads:", 0) == 0)
            {
                const auto pos = line.find_first_of("0123456789");
                if (pos == std::string::npos)
                    return 0;
                return std::stoi(line.substr(pos));
            }
        return 0;
    }
}

#else

namespace d2::prog::impl
{
    static float cpu_usage()
    {
        return 0;
    }
    static std::uint64_t total_ram_bytes()
    {
        return 0;
    }
    static std::uint64_t total_rss_bytes()
    {
        return 0;
    }
    static std::uint64_t heap_used_bytes()
    {
        return 0;
    }
    static std::uint64_t thread_count()
    {
        return 0;
    }
}

#endif

namespace d2::prog
{
    D2_TREE_DEFINE(debug)
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
                D2_STYLE(OnChangeValues, [](d2::TreeIter<Switch> ptr, d2::string n, d2::string o) {
                    auto root = ptr->state()->core();
                    if (!o.empty()) root->at(o)->setstate(Element::Display, false);
                    root->at(n)->setstate(Element::Display, true);
                })
            D2_ELEM_END
            D2_ELEM(Text)
                D2_STYLE(ZIndex, 1)
                D2_STYLE(Value, " ")
                D2_STYLE(X, 5.0_pxi)
                D2_INTERP_TWOWAY_AUTO(
                    Hovered, 500, Linear,
                    ForegroundColor, d2::colors::b::turquoise
                );
                D2_INTERP_GEN_AUTO(1000, Sequential, Value,
                    std::vector<d2::string>{ "⠇", "⠋", "⠙", "⠸", "⢰", "⣠", "⣄", "⡆" }
                );
                D2_ON(Clicked)
                    state->screen()->suspend(
                        !state->screen()->is_suspended()
                    );
                D2_ON_END
            D2_ELEM_END
            D2_ELEM(Text)
                D2_STYLE(ZIndex, 1)
                D2_STYLE(Value, "<X>")
                D2_STYLE(X, 1.0_pxi)
                D2_INTERP_TWOWAY_AUTO(
                    Hovered, 500, Linear,
                    ForegroundColor, d2::colors::r::crimson
                );
                D2_ON(Clicked)
                    state->core()->parent()->remove(
                        state->core()
                    );
                D2_ON_END
            D2_ELEM_END
        D2_ELEM_END
        // Overview
        D2_ELEM(FlowBox, Overview)
            struct Metric
            {
                bool pin_min{ true };
                bool enabled{ false };
                bool round{ true };
                float min{ 0.f };
                float max{ 1.f };
                float rescale{ 1.f };
                float measurement{ 0.f };
                std::string unit{ "" };
                std::function<float(Screen::ptr)> callback{ nullptr };
                std::chrono::milliseconds resolution{ 100 };
                IOContext::future<void> task{ nullptr };

                float min_smooth{ 0.f };
                float max_smooth{ 1.f };
                int max_hold_left{ 0 };
                int max_hold_samples{ 10 };

                void update(float mes)
                {
                    constexpr auto headroom = 2;
                    constexpr auto attack  = 0.25f;
                    constexpr auto release = 0.02f;

                    const auto tmp_min = min;
                    const auto tmp_max = max;

                    if (pin_min) min = 0;

                    measurement = mes;
                    if (!std::isfinite(measurement))
                        return;

                    if (measurement < min_smooth) min_smooth = measurement;
                    else min_smooth += (measurement - min_smooth) * release;

                    if (measurement > max_smooth)
                    {
                        max_smooth += (measurement - max_smooth) * attack;
                        max_hold_left = max_hold_samples;
                    }
                    else
                    {
                        if (max_hold_left > 0) --max_hold_left;
                        else max_smooth += (measurement - max_smooth) * release;
                    }

                    if (min_smooth < 0.f) min_smooth = 0.f;
                    if (max_smooth < min_smooth) max_smooth = min_smooth;

                    static auto step = [](float x)
                    {
                        if (x <= 0.f)
                            return 1.f;
                        const auto exp10 = std::pow(10.f, std::floor(std::log10(x)));
                        const auto f = x / exp10;
                        const auto nf = (f <= 1.f) ? 1.f : (f <= 2.f) ? 2.f : (f <= 5.f) ? 5.f : 10.f;
                        return nf * exp10;
                    };
                    static auto range = [](float& mn, float& mx, int ticks = 5)
                    {
                        const auto span = mx - mn;
                        if (!(span > 0.f))
                            return;
                        const auto st = step(span / std::max(1, ticks - 1));
                        mn = std::floor(mn / st) * st;
                        mx = (std::ceil(mx / st) + headroom) * st;
                    };

                    const auto span = max_smooth - min_smooth;
                    const auto pad = span * 0.10f;
                    auto mn = min_smooth - pad;
                    auto mx = max_smooth + pad;

                    if (round)
                        range(mn, mx, 5);

                    if (mn < 0.f) mn = 0.f;
                    if (!(mx > mn)) mx = mn + 1.f;

                    if (pin_min) min = 0;
                    else min = mn;
                    max = mx;

                    if (min != tmp_min || max != tmp_max)
                    {
                        const auto pspan = tmp_max - tmp_min;
                        const auto nspan = max - min;
                        rescale = pspan / nspan;
                    }
                    else
                        rescale = 1.f;
                }
                float normalized()
                {
                    return std::clamp((measurement - min) / (max - min), 0.f, 1.f);
                }
            };
            D2_STATIC(metrics, std::unordered_map<std::string, Metric>)
            D2_STATIC(metric_reloads, std::vector<std::function<void()>>)
            D2_STATIC(active_metric, std::pair<std::string, Metric*>)
            D2_CONTEXT(state->core(), d2::ParentElement)
                D2_OFF(Created)
                    for (decltype(auto) it : *metrics)
                    {
                        it.second.task.discard();
                        it.second.task.sync_value();
                    }
                D2_ON_END
            D2_CONTEXT_END
            D2_ANCHOR(FlowBox, output)
            D2_ANCHOR(Text, gvalue)
            D2_ANCHOR(Text, minmax)
            *metrics = {
                { "Delta", Metric{
                    .unit = "us",
                    .callback = [](Screen::ptr src) -> float {
                        return src->delta().count();
                    },
                }},
                { "FPS", Metric{
                    .callback = [](Screen::ptr src) -> float {
                        return src->fps();
                    },
                }},
                { "Elements", Metric{
                    .callback = [](Screen::ptr src) -> float {
                        std::function<std::size_t(d2::TreeIter<>)> func = nullptr;
                        auto count_elements_recursive = [&](d2::TreeIter<> elem) -> std::size_t {
                            std::size_t cnt = 0;
                            elem.foreach([&](d2::TreeIter<> h) {
                                cnt++;
                                if (h.is_type<ParentElement>())
                                    cnt += func(h);
                                return true;
                            });
                            return cnt;
                        };
                        func = count_elements_recursive;
                        return count_elements_recursive(src->root()) + 1;
                    },
                    .resolution = std::chrono::milliseconds(500)
                }},
                { "Framebuffer", Metric{
                    .unit = "B",
                    .callback = [](Screen::ptr src) -> float {
                        return src->context()->output()->delta_size();
                    },
                }},
                { "Swapframe", Metric{
                    .unit = "B",
                    .callback = [](Screen::ptr src) -> float {
                        return src->context()->output()->swapframe_size();
                    },
                }},
                { "Threads", Metric{
                    .callback = [](Screen::ptr src) -> float {
                        return impl::thread_count();
                    },
                    .resolution = std::chrono::milliseconds(300)
                }},
                { "CPU/AVG", Metric{
                    .unit = "% core",
                    .callback = [](Screen::ptr) -> float {
                        return impl::cpu_usage();
                    },
                }},
                { "Heap", Metric{
                    .round = true,
                    .unit = "KiB",
                    .callback = [](Screen::ptr) -> float {
                        return impl::KiB(impl::heap_used_bytes());
                    },
                }},
                { "Memory", Metric{
                    .round = true,
                    .unit = "MiB",
                    .callback = [](Screen::ptr) -> float {
                        return impl::MiB(impl::total_rss_bytes());
                    }
                }},
            };
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
                        "Framebuffer", "Swapframe", "Delta", "FPS",
                        "Elements", "Animations", "Dynamic Variables",
                        "Logs", "Warnings", "Errors"
                    })
                    D2_STYLE(OnChangeValues, [=](d2::TreeIter<VerticalSwitch>, d2::string n, d2::string o) {
                        if (active_metric->second)
                            active_metric->second->task.pause();
                        if (output->exists(o))
                            output->at(o)->setstate(Element::Display, false);
                        auto f = metrics->find(n);
                        if (f != metrics->end())
                        {
                            D2_CONTEXT_AUTO(output)
                                if (!ptr->exists(n)) D2_ELEM_STR(graph::CyclicVerticalBar, n)
                                    D2_STYLE(Width, 1.0_pc)
                                    D2_STYLE(Height, 1.0_pc)
                                D2_ELEM_END
                                else ptr->at(n)->setstate(Element::Display, true);
                            D2_CONTEXT_END
                            f->second.task.unpause();
                            *active_metric = { n, &f->second };
                            gvalue->set<Text::Value>(std::format("Value: {}{}",
                                f->second.measurement, f->second.unit
                            ));
                            minmax->set<Text::Value>(std::format("Min/Max: [ {} {} ]{}",
                                f->second.min, f->second.max, f->second.unit
                            ));
                        }
                        else
                        {
                            gvalue->set<Text::Value>("Value:");
                            minmax->set<Text::Value>("Min/Max:");
                            *active_metric = { "", nullptr };
                            if (output->exists(o))
                                output->at(o)->setstate(Element::Display, false);
                        }
                        for (decltype(auto) it : *metric_reloads)
                            it();
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
                    int ctr = 0;
                    auto option = [&](d2::string label, auto&& change, auto&& val, bool def = false) {
                        D2_ELEM(Checkbox)
                            D2_STYLE(Value, def)
                            D2_STYLE(X, 1.0_relative)
                            D2_STYLE(BackgroundColor, d2::colors::w::transparent)
                            if (ctr++) D2_STYLE(Y, 0.0_relative)
                            D2_STYLE(OnSubmit, [=](d2::TreeIter<Checkbox>, bool value) {
                                const auto& [ name, metric ] = *active_metric;
                                if (metric != nullptr)
                                    change(name, *metric, value);
                            })
                            (*metric_reloads).push_back([=]() {
                                if (active_metric->second == nullptr)
                                {
                                    ptr.as<Checkbox>()->set<Checkbox::Value>(def);
                                }
                                else
                                {
                                    ptr.as<Checkbox>()->set<Checkbox::Value>(
                                        val(*active_metric->second)
                                    );
                                }
                            });
                        D2_ELEM_END
                        D2_ELEM(Text)
                            D2_STYLE(Value, label)
                            D2_STYLE(X, 1.0_relative)
                        D2_ELEM_END
                    };
                    option("track", [=](const std::string& name, Metric& metric, bool value) {
                            if (value)
                            {
                                metric.task = state->context()->scheduler()->launch_cyclic(metric.resolution, [&](auto&&...) {
                                    auto graph = output->at(name).as<graph::CyclicVerticalBar>();
                                    const auto measurement = metric.callback(state->screen());
                                    const auto pmin = metric.min;
                                    const auto pmax = metric.max;
                                    const auto pmes = metric.measurement;
                                    metric.update(measurement);
                                    if (pmin != metric.min || pmax != metric.max)
                                    {
                                        minmax->set<Text::Value>(
                                            std::format("[ {} {} ]{}",
                                                std::floor(metric.min),
                                                std::floor(metric.max),
                                                metric.unit
                                            )
                                        );
                                    }
                                    if (pmes != metric.measurement)
                                    {
                                        gvalue->set<Text::Value>(
                                            std::format("Value: {}{}",
                                                metric.round ? std::floor(measurement) : measurement,
                                                metric.unit
                                            )
                                        );
                                    }
                                    graph->rescale(metric.rescale);
                                    graph->push(metric.normalized());
                                });
                            }
                            else
                            {
                                metric.task.discard();
                            }
                            metric.enabled = value;
                        },
                        [](Metric& metric) {
                            return metric.enabled;
                        },
                        false
                    );
                D2_ELEM_END
            D2_ELEM_END
            // Core
            D2_ELEM(FlowBox)
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
                    D2_STYLE(X, 1.0_px)
                    D2_STYLE(Y, 0.0_relative)
                    auto handle = state->context()->scheduler()->launch_cyclic(std::chrono::milliseconds(200), [=](auto&&...) {
                        auto focused = ptr->screen()->focused();
                        auto info = std::format(
                            "Focused: {}",
                            focused == nullptr ? "<Unknown>" : focused->name()
                        );
                        ptr->set<Text::Value>(std::move(info));
                    });
                    D2_OFF_EXPR(Created, handle.discard())
                D2_ELEM_END
            D2_ELEM_END
            // Info
            D2_ELEM(FlowBox)
                D2_STYLE(ZIndex, 1)
                D2_STYLE(X, 0.5_pc)
                D2_STYLE(Y, 0.5_pc)
                D2_STYLE(Width, 0.5_pc)
                D2_STYLE(Height, 0.5_pc)
                D2_STYLE(ContainerBorder, true)
                D2_STATIC(grabbed, bool)
                D2_STATIC(listener, d2::IOContext::Handle)
                D2_ON(Clicked)
                    if (ptr->mouse_object_space().x == 0)
                        *grabbed = true;
                D2_ON_END
                D2_OFF_EXPR(Swapped, listener.unmute())
                D2_ON_EXPR(Swapped, listener.mute())
                *listener = D2_ON_EVENT(MouseInput)
                    const auto in = ctx->input();
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
                D2_ON_EVENT_END
                *grabbed = false;
                D2_ELEM(Text)
                    D2_STYLE(ZIndex, Box::overlap)
                    D2_STYLE(Value, "<info>")
                    D2_STYLE(X, 0.0_center)
                D2_ELEM_END
                D2_ELEM_ANCHOR(gvalue)
                    D2_STYLE(Value, "Value:")
                    D2_STYLE(X, 1.0_relative)
                D2_ELEM_END
                D2_ELEM_ANCHOR(minmax)
                    D2_STYLE(Value, "Min/Max:")
                    D2_STYLE(X, 1.0_relative)
                    D2_STYLE(Y, 0.0_relative)
                D2_ELEM_END
                D2_ELEM_ANCHOR(output)
                    D2_STYLE(Width, 1.0_pc)
                    D2_STYLE(Height, 2.0_pxi)
                    D2_STYLE(Y, 2.0_px)
                    D2_STYLE(ContainerBorder, true)
                D2_ELEM_END
            D2_ELEM_END
            ptr->setstate(Element::Display, false);
        D2_ELEM_END
        D2_ELEM(ScrollFlowBox, Modules)
            D2_STYLE(Y, 1.0_px)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 1.0_pxi)
            auto handle = state->context()->scheduler()->launch_cyclic(std::chrono::seconds(5), [=](auto&&...) {
                auto update_mod_view = [=](TreeIter<> ptr, sys::SystemComponent* mod)
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

                std::vector<TreeIter<>> rem;
                ptr->foreach([&](TreeIter<> ptr) {
                    auto f = mods.find(ptr->name());
                    if (f == mods.end())
                    {
                        rem.push_back(ptr);
                    }
                    else
                    {
                        update_mod_view(ptr, f->second);
                    }
                    return true;
                });
            });
            D2_OFF_EXPR(Created, handle.discard())
            ptr->setstate(Element::Display, false);
        D2_ELEM_END
        D2_ELEM(Box, Inspector)
            ptr->setstate(Element::Display, false);
        D2_ELEM_END
        D2_ELEM(Box, Logs)
            ptr->setstate(Element::Display, false);
        D2_ELEM_END
    D2_TREE_DEFINITION_END
}
