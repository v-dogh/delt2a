#include "check.hpp"
#include "mt/pool.hpp"
#include <barrier>
#include <latch>
#include <thread>

mt::future<int> relay(mt::future<int> source)
{
    co_return (co_await source) + 1;
}
mt::future<int> fail()
{
    throw std::runtime_error("original");
    co_return 0;
}

void queues()
{
    mt::TaskQueue<std::unique_ptr<int>> fifo;
    for (int i = 0; i < 10000; ++i)
        fifo.enqueue(std::make_unique<int>(i));
    for (int i = 0; i < 10000; ++i)
    {
        std::unique_ptr<int> value;
        CHECK(fifo.try_dequeue(value));
        CHECK(*value == i);
    }
    CHECK(fifo.empty());

    constexpr int count = 20000;
    mt::TaskQueue<int> queue;
    std::vector<std::atomic<int>> seen(count);
    std::vector<std::jthread> consumers, producers;
    std::atomic<int> received = 0;
    for (int i = 0; i < 4; ++i)
        consumers.emplace_back([&]
        {
            int value;
            while (received < count)
            {
                if (queue.dequeue(value, std::chrono::milliseconds(1)))
                {
                    CHECK(value >= 0 && value < count);
                    ++seen[value];
                    ++received;
                }
            }
        });
    for (int i = 0; i < 4; ++i)
        producers.emplace_back([&, i]
        {
            for (int j = i; j < count; j += 4)
                queue.enqueue(j);
        });
    producers.clear();
    consumers.clear();
    for (const auto& value : seen)
        CHECK(value == 1);
    int value;
    CHECK(!queue.dequeue(value, std::chrono::milliseconds(1)));
    mt::TaskQueue<int, true> ordered;
    std::atomic<int> turn = 0;
    for (int thread = 0; thread < 4; ++thread)
        producers.emplace_back([&, thread]
        {
            for (int i = thread; i < 1000; i += 4)
            {
                auto current = turn.load();
                while (current != i)
                {
                    turn.wait(current);
                    current = turn.load();
                }
                ordered.enqueue(i);
                ++turn;
                turn.notify_all();
            }
        });
    producers.clear();
    for (int i = 0; i < 1000; ++i)
        CHECK(ordered.try_dequeue(value) && value == i);

}

void futures()
{
    auto f = mt::future<int>::make();
    auto consumer = relay(f);
    CHECK(!consumer.has_value());
    f.block()->set_value(41);
    CHECK(consumer.value() == 42);
    CHECK(consumer.is_discarded());
    throws<mt::TaskDiscarded>([&] { consumer.value(); });

    auto cancelled = mt::future<int>::make();
    auto waiting = relay(cancelled);
    cancelled.discard();
    throws<mt::TaskDiscarded>([&] { waiting.value(); });

    auto failed = fail();
    std::weak_ptr weak = failed.block();
    try { failed.value(); CHECK(false); }
    catch (const std::runtime_error& ex) { CHECK(std::string(ex.what()) == "original"); }
    failed = nullptr;
    CHECK(weak.expired());

    mt::ControlBlock<std::unique_ptr<int>> movable;
    movable.set_value(std::make_unique<int>(7));
    CHECK(*movable.value() == 7);
    mt::ControlBlock<int> ticks;
    ticks.set_tick(1);
    CHECK(ticks.value() == 1 && !ticks.is_discarded());
    ticks.set_tick(2);
    ticks.set_tick(3);
    CHECK(ticks.value() == 3);
    ticks.discard();
    throws<mt::TaskDiscarded>([&] { ticks.value(); });

    for (int i = 0; i < 500; ++i)
    {
        auto source = mt::future<int>::make();
        std::barrier gate(2);
        std::jthread producer([&]
        {
            gate.arrive_and_wait();
            source.block()->set_value(i);
        });
        gate.arrive_and_wait();
        auto result = relay(source);
        CHECK(result.value() == i + 1);
    }

    bool destroyed = false;
    {
        mt::ManagedControlBlock<void, bool*> managed(&destroyed);
        managed.set_hook(decltype(managed)::Event::Destroyed,
            +[](mt::ManagedControlBlock<void, bool*>&, bool*& flag) { *flag = true; });
    }
    CHECK(destroyed);
}

mt::Task immediate(std::function<void()> callback)
{
    return mt::Task([callback = std::move(callback)](mt::Task::Query query, std::any& out)
    {
        if (query == mt::Task::Query::Type)
            out = static_cast<unsigned char>(mt::Task::Immediate);
        else if (query == mt::Task::Query::Run)
        {
            callback();
            out = mt::Task::Token::Discard;
        }
    });
}

void workers()
{
    auto worker = mt::Worker::make<mt::RingWorker>(mt::Task::All, std::chrono::milliseconds::max());
    for (int pass = 0; pass < 3; ++pass)
    {
        worker->start();
        std::latch started(1), release(1);
        worker->accept(immediate([&] { started.count_down(); release.wait(); }));
        started.wait();
        std::atomic<int> completed = 0;
        for (int i = 0; i < 2000; ++i)
            worker->accept(immediate([&] { ++completed; }));
        release.count_down();
        worker->stop();
        CHECK(completed == 2000);
        CHECK(worker->tasks() == 0);
    }
    worker->start();
    std::atomic<int> accepted = 0, ran = 0;
    std::latch started(4);
    std::vector<std::jthread> producers;
    for (int i = 0; i < 4; ++i)
        producers.emplace_back([&]
        {
            auto first = immediate([&] { ++ran; });
            CHECK(worker->try_accept(first));
            ++accepted;
            started.count_down();
            for (int j = 0; j < 1000; ++j)
            {
                auto task = immediate([&] { ++ran; });
                if (!worker->try_accept(task))
                    break;
                ++accepted;
            }
        });
    started.wait();
    worker->stop();
    producers.clear();
    CHECK(accepted == ran && worker->tasks() == 0);

}

int main()
{
    queues();
    futures();
    workers();
    auto pool = mt::ConcurrentPool::make<>();
    pool->start();
    auto delayed = pool->launch_deferred(std::chrono::hours(1), [] { return 7; });
    pool->stop();
    CHECK(delayed.is_discarded());
    throws<mt::TaskDiscarded>([&] { delayed.value(); });
}
