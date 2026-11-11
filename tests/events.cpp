#include "check.hpp"
#include "core/io/d2_signal_handler.hpp"
#include "elements/d2_text.hpp"
#include <latch>

enum class Event { Value };
using Manifest = d2::SignalManifest<d2::EvDefault<Event, const int&>>;

struct Element : d2::dx::Text
{
    Element() : d2::dx::Text("test", nullptr) {}
    void emit() { setstate(State::Clicked, !getstate(State::Clicked)); }
};

int main()
{
    auto element = std::make_shared<Element>();
    int calls = 0;
    auto handle = element->listen(d2::Element::State::Clicked,
        [&](auto self, auto) { ++calls; self.destroy(); });
    handle.mute();
    handle.mute();
    element->emit();
    CHECK(calls == 0 && !element->provides_cursor_sink());
    handle.unmute();
    handle.unmute();
    element->emit();
    element->emit();
    handle.destroy();
    CHECK(calls == 1 && !element->provides_cursor_sink());

    auto pool = mt::ConcurrentPool::make<>();
    std::atomic<int> errors = 0;
    pool->start([&](auto, mt::Node::Config& cfg)
    {
        cfg.flags = 0;
        cfg.on_exception.emplace_back([&](std::exception_ptr) { ++errors; return true; });
    });
    auto signals = d2::Signals::make(pool);
    auto slot = signals->connect<Manifest>({d2::Signals::Async});
    std::latch delivered(1);
    std::atomic<int> calls_async = 0;
    auto listener = signals->listen(Event::Value, [&](const int& value)
    {
        if (value == 0)
            throw std::runtime_error("listener failure");
        ++calls_async;
        if (value == 1000)
            delivered.count_down();
    });
    std::latch blocked(1), release(1);
    pool->launch_void([&] { blocked.count_down(); release.wait(); });
    blocked.wait();
    for (int i = 0; i <= 1000; ++i)
        signals->signal(Event::Value, int(i));
    release.count_down();
    delivered.wait();
    pool->stop();
    CHECK(calls_async == 1000);
    CHECK(errors == 1);

    pool->start([](auto, mt::Node::Config& cfg) { cfg.flags = 0; });
    auto combined = signals->connect<Manifest>(d2::sigspec(1), {d2::Signals::Async | d2::Signals::Combine});
    std::latch blocked2(1), release2(1), latest(1);
    std::atomic<int> latest_value = 0, latest_calls = 0;
    auto combined_listener = signals->listen(Event::Value, d2::sigspec(1), [&](const int& value)
    {
        latest_value = value;
        ++latest_calls;
        latest.count_down();
    });
    pool->launch_void([&] { blocked2.count_down(); release2.wait(); });
    blocked2.wait();
    for (int i = 1; i <= 1000; ++i)
        signals->signal(Event::Value, d2::sigspec(1), int(i));
    release2.count_down();
    latest.wait();
    pool->stop();
    CHECK(latest_value == 1000 && latest_calls == 1);
}
