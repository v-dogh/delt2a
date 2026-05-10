#include "d2_debug_box.hpp"

#include <d2_std.hpp>
#include <elements/d2_std.hpp>
#include <logs/runtime_logs.hpp>

namespace d2::ex::impl
{
    static float GiB(std::uint64_t b)
    {
        return float(b) / (1024.f * 1024.f * 1024.f);
    }
    static float MiB(std::uint64_t b)
    {
        return float(b) / (1024.f * 1024.f);
    }
    static float KiB(std::uint64_t b)
    {
        return float(b) / 1024.f;
    }
    static std::string MeM(std::uint64_t b)
    {
        if (b >= 1024 * 1024 * 1024)
            return std::format("{}GiB", std::size_t(GiB(b)));
        if (b >= 1024 * 1024)
            return std::format("{}MiB", std::size_t(MiB(b)));
        if (b >= 1024)
            return std::format("{}KiB", std::size_t(KiB(b)));
        return std::format("{}b", b);
    }
} // namespace d2::ex::impl

#ifdef __linux__

#include <cstdio>
#include <fstream>
#include <malloc.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>

namespace d2::ex::impl
{
    static float cpu_usage()
    {
        static auto ts_to_ns = [](const timespec& ts) -> std::uint64_t
        { return ts.tv_sec * 1000000000ull + ts.tv_nsec; };
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

        const std::uint64_t cpu_ns_now = ts_to_ns(cpu);
        const std::uint64_t cpu_ns_last = ts_to_ns(last_cpu);
        const std::uint64_t cpu_ns_delta =
            (cpu_ns_now >= cpu_ns_last) ? (cpu_ns_now - cpu_ns_last) : 0;

        const auto wall_ns_delta =
            std::chrono::duration_cast<std::chrono::nanoseconds>(wall_now - last_wall).count();
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
} // namespace d2::ex::impl

#else

namespace d2::ex::impl
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
} // namespace d2::ex::impl

#endif

namespace d2::ex
{
    void debug::construct(TreeCtx<dx::VirtualBox, TreeState> ctx)
    {
        using namespace dx;

        struct Metric
        {
            bool pin_min{true};
            bool enabled{false};
            bool round{true};

            float min{0.f};
            float max{1.f};
            float rescale{1.f};
            float measurement{0.f};

            std::string unit{""};
            std::function<float(IOContext::ptr)> callback{nullptr};

            std::chrono::milliseconds resolution{100};
            IOContext::future<void> task{nullptr};

            float min_smooth{0.f};
            float max_smooth{1.f};

            int max_hold_left{0};
            int max_hold_samples{10};

            void update(float mes)
            {
                constexpr auto headroom = 2;
                constexpr auto attack = 0.25f;
                constexpr auto release = 0.02f;

                const auto tmp_min = min;
                const auto tmp_max = max;

                if (pin_min)
                    min = 0;

                measurement = mes;
                if (!std::isfinite(measurement))
                    return;

                if (measurement < min_smooth)
                    min_smooth = measurement;
                else
                    min_smooth += (measurement - min_smooth) * release;

                if (measurement > max_smooth)
                {
                    max_smooth += (measurement - max_smooth) * attack;
                    max_hold_left = max_hold_samples;
                }
                else
                {
                    if (max_hold_left > 0)
                        --max_hold_left;
                    else
                        max_smooth += (measurement - max_smooth) * release;
                }

                if (min_smooth < 0.f)
                    min_smooth = 0.f;

                if (max_smooth < min_smooth)
                    max_smooth = min_smooth;

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
                if (mn < 0.f)
                    mn = 0.f;
                if (!(mx > mn))
                    mx = mn + 1.f;
                if (pin_min)
                    min = 0;
                else
                    min = mn;
                max = mx;

                if (min != tmp_min || max != tmp_max)
                {
                    const auto pspan = tmp_max - tmp_min;
                    const auto nspan = max - min;
                    rescale = pspan / nspan;
                }
                else
                {
                    rescale = 1.f;
                }
            }

            float normalized() const
            {
                return std::clamp((measurement - min) / (max - min), 0.f, 1.f);
            }
        };
        struct DebugState
        {
            std::unordered_map<std::string, Metric> metrics{};
            std::vector<std::function<void()>> metric_reloads{};

            std::pair<std::string, Metric*> active_metric{"", nullptr};

            TreeIter<FlowBox> output{};
            TreeIter<Text> gvalue{};
            TreeIter<Text> minmax{};

            bool grabbed{false};
            IOContext::Handle listener{};
        };

        auto& data = ctx.declare<DebugState>();

        data.metrics = {
            {"Delta",
             Metric{
                 .unit = "us",
                 .callback = [](IOContext::ptr ctx) -> float
                 { return ctx->screen()->delta().count(); },
             }},
            {"FPS",
             Metric{
                 .callback = [](IOContext::ptr ctx) -> float { return ctx->screen()->fps(); },
             }},
            {"Elements",
             Metric{
                 .callback = [](IOContext::ptr ctx) -> float
                 {
                     std::function<std::size_t(TreeIter<>)> func = nullptr;

                     auto count_elements_recursive = [&](TreeIter<> elem) -> std::size_t
                     {
                         std::size_t cnt = 0;

                         elem.foreach (
                             [&](TreeIter<> h)
                             {
                                 cnt++;

                                 if (h.is_type<ParentElement>())
                                     cnt += func(h);

                                 return true;
                             }
                         );

                         return cnt;
                     };

                     func = count_elements_recursive;

                     return count_elements_recursive(ctx->screen()->root()) + 1;
                 },
                 .resolution = std::chrono::milliseconds(500),
             }},
            {"Framebuffer",
             Metric{
                 .unit = "B",
                 .callback = [](IOContext::ptr ctx) -> float
                 { return ctx->output()->delta_size(); },
             }},
            {"Swapframe",
             Metric{
                 .unit = "B",
                 .callback = [](IOContext::ptr ctx) -> float
                 { return ctx->output()->swapframe_size(); },
             }},
            {"Threads",
             Metric{
                 .callback = [](IOContext::ptr) -> float { return impl::thread_count(); },
                 .resolution = std::chrono::milliseconds(300),
             }},
            {"CPU/AVG",
             Metric{
                 .unit = "% core",
                 .callback = [](IOContext::ptr) -> float { return impl::cpu_usage(); },
             }},
            {"Heap",
             Metric{
                 .round = true,
                 .unit = "KiB",
                 .callback = [](IOContext::ptr) -> float
                 { return impl::KiB(impl::heap_used_bytes()); },
             }},
            {"Memory",
             Metric{
                 .round = true,
                 .unit = "MiB",
                 .callback = [](IOContext::ptr) -> float
                 { return impl::MiB(impl::total_rss_bytes()); },
             }},
        };

        dstyle(Title, "<Debug Box>");
        dstyle(ZIndex, 127);
        dstyle(Width, 82.0_px);
        dstyle(Height, 26.0_px);
        dstyle(ContainerBorder, true);

        ctx.onv(
            Element::State::Created,
            false,
            [&data](Element::EventListener, TreeIter<VirtualBox>)
            {
                for (auto& [_, metric] : data.metrics)
                {
                    metric.task.discard();
                    metric.task.sync_value();
                }
            }
        );

        // Controls
        ctx.elem<FlowBox>(
            [](TreeCtx<FlowBox> ctx)
            {
                dstyle(X, 0.0_px);
                dstyle(Y, 0.0_px);
                dstyle(Width, 1.0_pc);
                dstyle(Height, 1.0_px);

                ctx.elem<Switch>(
                    [](TreeCtx<Switch> ctx)
                    {
                        dstyle(ZIndex, 0);
                        dstyle(Options, Switch::opts{"Overview", "Modules", "Inspector", "Logs"});
                        dstyle(Width, 1.0_pc);
                        dstyle(
                            OnChangeValues,
                            [](TreeIter<Switch> ptr, string n, string o)
                            {
                                auto root = ptr->state()->core();

                                if (!o.empty())
                                    root->at(o)->setstate(Element::Display, false);

                                root->at(n)->setstate(Element::Display, true);
                            }
                        );
                    }
                );
                ctx.elem<Text>(
                    [](TreeCtx<Text> ctx)
                    {
                        dstyle(ZIndex, 1);
                        dstyle(Value, " ");
                        dstyle(X, 5.0_pxi);

                        // D2_INTERP_TWOWAY_AUTO(
                        //     Hovered,
                        //     500,
                        //     Linear,
                        //     ForegroundColor,
                        //     colors::b::turquoise
                        // );
                        // D2_INTERP_GEN_AUTO(
                        //     1000,
                        //     Sequential,
                        //     Value,
                        //     std::vector<string>{"⠇", "⠋", "⠙", "⠸", "⢰", "⣠", "⣄", "⡆"}
                        // );

                        ctx.onv(
                            Element::State::Clicked,
                            true,
                            [](Element::EventListener, TreeIter<Text> ptr)
                            { ptr->context()->suspend(!ptr->context()->is_suspended()); }
                        );
                    }
                );
                ctx.elem<Text>(
                    [](TreeCtx<Text> ctx)
                    {
                        dstyle(ZIndex, 1);
                        dstyle(Value, "<X>");
                        dstyle(X, 1.0_pxi);

                        // D2_INTERP_TWOWAY_AUTO(
                        //     Hovered,
                        //     500,
                        //     Linear,
                        //     ForegroundColor,
                        //     colors::r::crimson
                        // );

                        ctx.onv(
                            Element::State::Clicked,
                            true,
                            [](Element::EventListener, TreeIter<Text> ptr)
                            { ptr->state()->core()->parent()->remove(ptr->state()->core()); }
                        );
                    }
                );
            }
        );
        // Overview
        ctx.elem<
            FlowBox
        >("Overview",
          [&data](TreeCtx<FlowBox> ctx)
          {
              dstyle(Y, 1.0_px);
              dstyle(Width, 1.0_pc);
              dstyle(Height, 1.0_pxi);

              // Setup
              ctx.elem<FlowBox>(
                  [&data](TreeCtx<FlowBox> ctx)
                  {
                      dstyle(ZIndex, 0);
                      dstyle(Width, 0.5_pc);
                      dstyle(Height, 1.0_pc);
                      dstyle(ContainerBorder, true);

                      ctx.elem<Text>(
                          [](TreeCtx<Text> ctx)
                          {
                              dstyle(ZIndex, Box::overlap);
                              dstyle(Value, "<setup>");
                              dstyle(X, 0.0_center);
                          }
                      );
                      ctx.elem<VerticalSwitch>(
                          [&data](TreeCtx<VerticalSwitch> ctx)
                          {
                              dstyle(Width, 1.0_pc);
                              dstyle(Height, 5.0_px);
                              dstyle(Y, 1.0_relative);
                              dstyle(
                                  Options,
                                  VerticalSwitch::opts{
                                      "Threads",
                                      "CPU/AVG",
                                      "Memory",
                                      "Heap",
                                      "Framebuffer",
                                      "Swapframe",
                                      "Delta",
                                      "FPS",
                                      "Elements",
                                      "Animations",
                                      "Dynamic Variables",
                                      "Logs",
                                      "Warnings",
                                      "Errors",
                                  }
                              );
                              dstyle(
                                  OnChangeValues,
                                  [&data](TreeIter<VerticalSwitch> ptr, string n, string o)
                                  {
                                      if (data.active_metric.second)
                                          data.active_metric.second->task.pause();
                                      if (data.output != nullptr && data.output->exists(o))
                                          data.output->at(o)->setstate(Element::Display, false);

                                      auto f = data.metrics.find(n);

                                      if (f != data.metrics.end())
                                      {
                                          TreeCtx<FlowBox> output_ctx(data.output);

                                          if (!data.output->exists(n))
                                          {
                                              output_ctx.elem<graph::CyclicVerticalBar>(
                                                  n,
                                                  [](TreeCtx<graph::CyclicVerticalBar> ctx)
                                                  {
                                                      dstyle(Width, 1.0_pc);
                                                      dstyle(Height, 1.0_pc);
                                                  }
                                              );
                                          }
                                          else
                                          {
                                              data.output->at(n)->setstate(Element::Display, true);
                                          }

                                          f->second.task.unpause();
                                          data.active_metric = {n, &f->second};

                                          data.gvalue->set<Text::Value>(std::format(
                                              "Value: {}{}", f->second.measurement, f->second.unit
                                          ));
                                          data.minmax->set<Text::Value>(std::format(
                                              "Min/Max: [ {} {} ]{}",
                                              f->second.min,
                                              f->second.max,
                                              f->second.unit
                                          ));
                                      }
                                      else
                                      {
                                          data.gvalue->set<Text::Value>("Value:");
                                          data.minmax->set<Text::Value>("Min/Max:");
                                          data.active_metric = {"", nullptr};

                                          if (data.output != nullptr && data.output->exists(o))
                                              data.output->at(o)->setstate(Element::Display, false);
                                      }

                                      for (auto& reload : data.metric_reloads)
                                          reload();
                                  }
                              );
                          }
                      );

                      ctx.elem<ScrollFlowBox>(
                          [&data](TreeCtx<ScrollFlowBox> ctx)
                          {
                              dstyle(Width, 1.0_pc);
                              dstyle(Height, 7.0_pxi);
                              dstyle(Y, 1.0_relative);
                              dstyle(ContainerBorder, true);

                              ctx.elem<Text>(
                                  [](TreeCtx<Text> ctx)
                                  {
                                      dstyle(ZIndex, Box::overlap);
                                      dstyle(Value, "<options>");
                                      dstyle(X, 0.0_center);
                                  }
                              );

                              int ctr = 0;
                              auto option =
                                  [&](string label, auto&& change, auto&& val, bool def = false)
                              {
                                  ctx.elem<Checkbox>(
                                      [&,
                                       label,
                                       change = std::forward<decltype(change)>(change),
                                       val = std::forward<decltype(val)>(val),
                                       def](TreeCtx<Checkbox> ctx)
                                      {
                                          dstyle(Value, def);
                                          dstyle(X, 1.0_relative);
                                          dstyle(BackgroundColor, colors::w::transparent);
                                          if (ctr++)
                                              dstyle(Y, 0.0_relative);
                                          dstyle(
                                              OnSubmit,
                                              [&data, change](TreeIter<Checkbox>, bool value)
                                              {
                                                  auto& [name, metric] = data.active_metric;

                                                  if (metric != nullptr)
                                                      change(name, *metric, value);
                                              }
                                          );

                                          auto ptr = ctx.ptr();
                                          data.metric_reloads.push_back(
                                              [&data, ptr, val, def]()
                                              {
                                                  if (data.active_metric.second == nullptr)
                                                  {
                                                      ptr.template as<Checkbox>()
                                                          ->template set<Checkbox::Value>(def);
                                                  }
                                                  else
                                                  {
                                                      ptr.template as<Checkbox>()
                                                          ->template set<Checkbox::Value>(
                                                              val(*data.active_metric.second)
                                                          );
                                                  }
                                              }
                                          );
                                      }
                                  );
                                  ctx.elem<Text>(
                                      [label = std::move(label)](TreeCtx<Text> ctx)
                                      {
                                          dstyle(Value, label);
                                          dstyle(X, 1.0_relative);
                                      }
                                  );
                              };

                              option(
                                  "track",
                                  [&data](const std::string& name, Metric& metric, bool value)
                                  {
                                      if (value)
                                      {
                                          metric.task =
                                              data.output->context()->scheduler()->launch_cyclic(
                                                  metric.resolution,
                                                  [&data, name, &metric](auto&&...)
                                                  {
                                                      auto graph = data.output->at(name)
                                                                       .template as<
                                                                           graph::CyclicVerticalBar
                                                                       >();

                                                      const auto measurement =
                                                          metric.callback(data.output->context());

                                                      const auto pmin = metric.min;
                                                      const auto pmax = metric.max;
                                                      const auto pmes = metric.measurement;

                                                      metric.update(measurement);

                                                      if (pmin != metric.min || pmax != metric.max)
                                                      {
                                                          data.minmax->set<Text::Value>(std::format(
                                                              "[ {} {} ]{}",
                                                              std::floor(metric.min),
                                                              std::floor(metric.max),
                                                              metric.unit
                                                          ));
                                                      }

                                                      if (pmes != metric.measurement)
                                                      {
                                                          data.gvalue->set<Text::Value>(std::format(
                                                              "Value: {}{}",
                                                              metric.round ? std::floor(measurement)
                                                                           : measurement,
                                                              metric.unit
                                                          ));
                                                      }

                                                      graph->rescale(metric.rescale);
                                                      graph->push(metric.normalized());
                                                  }
                                              );
                                      }
                                      else
                                      {
                                          metric.task.discard();
                                      }

                                      metric.enabled = value;
                                  },
                                  [](Metric& metric) { return metric.enabled; },
                                  false
                              );
                          }
                      );
                  }
              );
              // Core
              ctx.elem<FlowBox>(
                  [](TreeCtx<FlowBox> ctx)
                  {
                      dstyle(Width, 0.5_pc);
                      dstyle(Height, 0.5_pc);
                      dstyle(X, 0.5_pc);
                      dstyle(ContainerBorder, true);

                      ctx.elem<Text>(
                          [](TreeCtx<Text> ctx)
                          {
                              dstyle(ZIndex, Box::overlap);
                              dstyle(Value, "<core>");
                              dstyle(X, 0.0_center);
                          }
                      );
                      ctx.elem<Text>(
                          [](TreeCtx<Text> ctx)
                          {
                              dstyle(X, 1.0_px);
                              dstyle(Y, 0.0_relative);

                              auto handle = ctx->context()->scheduler()->launch_cyclic(
                                  std::chrono::milliseconds(200),
                                  [ptr = ctx.ptr()](auto&&...)
                                  {
                                      auto focused = ptr->screen()->focused();
                                      auto info = std::format(
                                          "Focused: {}",
                                          focused == nullptr ? "<Unknown>" : focused->name()
                                      );

                                      ptr->set<Text::Value>(std::move(info));
                                  }
                              );

                              ctx.onv(
                                  Element::State::Created,
                                  false,
                                  [handle](Element::EventListener, TreeIter<Text>) mutable
                                  { handle.discard(); }
                              );
                          }
                      );
                  }
              );
              // Info
              ctx.elem<FlowBox>(
                  [&data](TreeCtx<FlowBox> ctx)
                  {
                      dstyle(ZIndex, 1);
                      dstyle(X, 0.5_pc);
                      dstyle(Y, 0.5_pc);
                      dstyle(Width, 0.5_pc);
                      dstyle(Height, 0.5_pc);
                      dstyle(ContainerBorder, true);

                      ctx.onv(
                          Element::State::Clicked,
                          true,
                          [&data](Element::EventListener, TreeIter<FlowBox> ptr)
                          {
                              if (ptr->mouse_object_space().x == 0)
                                  data.grabbed = true;
                          }
                      );
                      ctx.onv(
                          Element::State::Swapped,
                          false,
                          [&data](Element::EventListener, TreeIter<FlowBox>)
                          { data.listener.unmute(); }
                      );
                      ctx.onv(
                          Element::State::Swapped,
                          true,
                          [&data](Element::EventListener, TreeIter<FlowBox>)
                          { data.listener.mute(); }
                      );
                      ctx.onv(
                          Element::State::Event,
                          true,
                          [&data](Element::EventListener, TreeIter<FlowBox> ptr)
                          {
                              auto& in = ptr->input_frame();
                              if (in.active(in::mouse::Left, in::mode::Press))
                              {
                                  if (data.grabbed)
                                  {
                                      const auto pos = ptr->parent()->mouse_object_space().x;
                                      const auto w = ptr->layout(Element::Layout::Width);
                                      const auto pw = ptr->parent()->layout(Element::Layout::Width);

                                      (void)w;

                                      const auto position = Unit(float(pos) / pw, Unit::Pc);

                                      ptr->set<Box::Width>(Unit(1.f - position.raw(), Unit::Pc));
                                      ptr->set<Box::X>(position);
                                  }
                              }
                              else
                              {
                                  data.grabbed = false;
                              }
                          }
                      );

                      ctx.elem<Text>(
                          [](TreeCtx<Text> ctx)
                          {
                              dstyle(ZIndex, Box::overlap);
                              dstyle(Value, "<info>");
                              dstyle(X, 0.0_center);
                          }
                      );
                      ctx.elem<Text>(
                          [&data](TreeCtx<Text> ctx)
                          {
                              data.gvalue = ctx.ptr();

                              dstyle(Value, "Value:");
                              dstyle(X, 1.0_relative);
                          }
                      );
                      ctx.elem<Text>(
                          [&data](TreeCtx<Text> ctx)
                          {
                              data.minmax = ctx.ptr();

                              dstyle(Value, "Min/Max:");
                              dstyle(X, 1.0_relative);
                              dstyle(Y, 0.0_relative);
                          }
                      );
                      ctx.elem<FlowBox>(
                          [&data](TreeCtx<FlowBox> ctx)
                          {
                              data.output = ctx.ptr();

                              dstyle(Width, 1.0_pc);
                              dstyle(Height, 2.0_pxi);
                              dstyle(Y, 2.0_px);
                              dstyle(ContainerBorder, true);
                          }
                      );
                  }
              );
              ctx->setstate(Element::Display, false);
          });

        // Modules
        ctx.elem<ScrollFlowBox>(
            "Modules",
            [](TreeCtx<ScrollFlowBox> ctx)
            {
                dstyle(Y, 1.0_px);
                dstyle(Width, 1.0_pc);
                dstyle(Height, 1.0_pxi);

                auto handle = ctx->context()->scheduler()->launch_cyclic(
                    std::chrono::seconds(5),
                    [ptr = ctx.ptr()](auto&&...)
                    {
                        auto update_mod_view = [=](TreeIter<> ptr, sys::SystemModule* mod)
                        {
                            px::foreground color;
                            switch (mod->status())
                            {
                            case sys::SystemModule::Status::Ok:
                                color = colors::g::green;
                                break;
                            case sys::SystemModule::Status::Degraded:
                                color = colors::r::yellow;
                                break;
                            case sys::SystemModule::Status::Failure:
                                color = colors::r::red;
                                break;
                            case sys::SystemModule::Status::Offline:
                                color = colors::b::blue;
                                break;
                            default:
                                color = colors::r::red;
                                break;
                            }
                            ptr.asp()
                                ->at("indicator")
                                .template as<Text>()
                                ->template set<Text::ForegroundColor>(color);
                        };

                        auto create_mod_view = [=](sys::SystemModule* mod)
                        {
                            TreeCtx<ParentElement> ctx(ptr);

                            ctx.elem<FlowBox>(
                                mod->name(),
                                [=](TreeCtx<FlowBox> ctx)
                                {
                                    std::string load_type = "";
                                    switch (mod->load_spec().type)
                                    {
                                    case sys::Load::Spec::Type::Immediate:
                                        load_type = "Immediate";
                                        break;
                                    case sys::Load::Spec::Type::Lazy:
                                        load_type = "Lazy";
                                        break;
                                    case sys::Load::Spec::Type::Deferred:
                                        load_type =
                                            std::format("Deferred ({})", mod->load_spec().ms);
                                        break;
                                    }

                                    dstyle(Width, 1.0_pc);
                                    dstyle(Y, 0.0_relative);
                                    dstyle(ContainerBorder, true);

                                    ctx.elem<Text>(
                                        "indicator",
                                        [=](TreeCtx<Text> ctx)
                                        {
                                            dstyle(ZIndex, Box::overlap);
                                            dstyle(Value, std::format("<{}>", mod->name()));
                                            dstyle(X, 0.0_center);
                                        }
                                    );
                                    ctx.elem<Text>(
                                        "static-memory",
                                        [=](TreeCtx<Text> ctx)
                                        {
                                            dstyle(
                                                Value,
                                                std::format(
                                                    "Static Memory: {}",
                                                    impl::MeM(mod->static_usage())
                                                )
                                            );
                                            dstyle(Y, 0.0_relative);
                                        }
                                    );
                                    ctx.elem<Text>(
                                        "load",
                                        [=](TreeCtx<Text> ctx)
                                        {
                                            dstyle(Value, std::format("Load: {}", load_type));
                                            dstyle(Y, 0.0_relative);
                                        }
                                    );
                                    ctx.elem<Text>(
                                        "access",
                                        [=](TreeCtx<Text> ctx)
                                        {
                                            dstyle(
                                                Value,
                                                std::format(
                                                    "Access: {}",
                                                    mod->access() == sys::Access::TSafe
                                                        ? "Thread Safe"
                                                        : "Not Thread Safe"
                                                )
                                            );
                                            dstyle(Y, 0.0_relative);
                                        }
                                    );
                                    ctx.elem<Text>(
                                        [](TreeCtx<Text> ctx)
                                        {
                                            dstyle(Value, "Dependencies:");
                                            dstyle(Y, 0.0_relative);
                                        }
                                    );
                                    const auto deps = mod->dependency_names();
                                    for (const auto& dep : deps)
                                    {
                                        ctx.elem<Text>(
                                            [dep](TreeCtx<Text> ctx)
                                            {
                                                dstyle(Value, dep);
                                                dstyle(X, 2.0_px);
                                                dstyle(Y, 0.0_relative);
                                            }
                                        );
                                    }
                                    update_mod_view(ctx.ptr(), mod);
                                }
                            );
                        };

                        std::unordered_map<std::string, sys::SystemModule*> mods;
                        ptr->state()->context()->sysenum(
                            [&](sys::SystemModule* mod)
                            {
                                mods[mod->name()] = mod;

                                if (!ptr->exists(mod->name()))
                                    create_mod_view(mod);
                            }
                        );
                        std::vector<TreeIter<>> rem;
                        ptr->foreach (
                            [&](TreeIter<> child)
                            {
                                auto f = mods.find(child->name());

                                if (f == mods.end())
                                {
                                    rem.push_back(child);
                                }
                                else
                                {
                                    update_mod_view(child, f->second);
                                }

                                return true;
                            }
                        );
                        for (auto child : rem)
                            ptr.asp()->remove(child);
                    }
                );
                ctx.onv(
                    Element::State::Created,
                    false,
                    [handle](Element::EventListener, TreeIter<ScrollFlowBox>) mutable
                    { handle.discard(); }
                );
                ctx->setstate(Element::Display, false);
            }
        );

        // Inspector
        ctx.elem<Box>(
            "Inspector",
            [](TreeCtx<Box> ctx)
            {
                dstyle(Y, 1.0_px);
                dstyle(Width, 1.0_pc);
                dstyle(Height, 1.0_pxi);

                ctx->setstate(Element::Display, false);
            }
        );
        // Logs
        ctx.elem<Box>(
            "Logs",
            [](TreeCtx<Box> ctx)
            {
                dstyle(Y, 1.0_px);
                dstyle(Width, 1.0_pc);
                dstyle(Height, 1.0_pxi);

                ctx.elem<MultiInput>(
                    [](TreeCtx<MultiInput> ctx)
                    {
                        dstyle(InputOptions, MultiInput::ReadOnly);
                        dstyle(Width, 2.0_pxi);
                        dstyle(Height, 1.0_pxi);
                        dstyle(X, 0.0_center);
                        dstyle(Y, 1.0_px);
                        dstyle(EnableScrollVert, true);
                        dstyle(EnableScrollHoriz, true);

                        class Sink : public rs::RuntimeLogSink
                        {
                        public:
                            explicit Sink(TreeIter<MultiInput> ptr) : ptr(ptr) {}

                            TreeIter<MultiInput> ptr;

                            void accept(rs::LogEntry entry) noexcept override
                            {
                                if (ptr.weak().expired())
                                    return;

                                const auto value = std::format(
                                    "{0:}({1:})<{2:}> : {3:}\n",
                                    "",
                                    severity_to_str(entry.severity),
                                    entry.module,
                                    entry.msg
                                );

                                const auto scroll = ptr->vertical_scollbar();
                                const auto synced = !scroll->getstate(Element::Display) ||
                                                    scroll->relative_value() == 1.f;

                                ptr->append(value);

                                if (synced)
                                    ptr->sync();
                            }
                        };
                        ctx->disable_history();
                        rs::context::get().sink<Sink>(ctx.ptr());
                        Sink tmp(ctx.ptr());
                        rs::context::get().page([&](rs::LogEntry entry) { tmp.accept(entry); });
                        ctx.onv(
                            Element::State::Created,
                            false,
                            [](Element::EventListener, TreeIter<MultiInput>)
                            { rs::context::get().unsink<Sink>(); }
                        );
                    }
                );
                ctx->setstate(Element::Display, false);
            }
        );
    }
} // namespace d2::ex
