#include "pool.hpp"

#include <bit>
#include <chrono>
#include <random>

namespace mt
{
    Barrier::Barrier()
    {
        _flag.test_and_set(std::memory_order::acquire);
    }

    void Barrier::release() noexcept
    {
        _flag.clear(std::memory_order::release);
        _flag.notify_all();
    }
    void Barrier::wait() noexcept
    {
        while (true)
        {
            if (!_flag.test_and_set(std::memory_order::acquire))
                break;
            else
                _flag.wait(true, std::memory_order::acquire);
        }
    }

    std::chrono::steady_clock::time_point::rep
    tp_to_itg(std::chrono::steady_clock::time_point value)
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(value.time_since_epoch())
            .count();
    }
    std::chrono::steady_clock::time_point
    itg_to_tp(std::chrono::steady_clock::time_point::rep value)
    {
        return std::chrono::steady_clock::time_point(std::chrono::nanoseconds{value});
    }

    // Context

    context::Guard::Guard()
    {
        ptr = context::ptr();
    }
    context::Guard::~Guard()
    {
        context::set(ptr);
    }

    context::pool context::ptr() noexcept
    {
        return _pool_ptr;
    }
    ConcurrentPool& context::get() noexcept
    {
        return *_pool_ptr;
    }
    void context::set(pool ptr) noexcept
    {
        _pool_ptr = ptr;
    }

    // Other stuff

    const Task::callback& Task::value() const noexcept
    {
        return _cb;
    }
    Task::callback& Task::value() noexcept
    {
        return _cb;
    }

    bool Task::operator==(std::nullptr_t) const noexcept
    {
        return _cb == nullptr;
    }
    bool Task::operator!=(std::nullptr_t) const noexcept
    {
        return _cb != nullptr;
    }

    std::chrono::steady_clock::time_point PeriodicTask::deadline() const noexcept
    {
        return _last + Task::query<Query::Timing>() - _cyclic_epsilon;
    }
    bool PeriodicTask::ready() const
    {
        return std::chrono::steady_clock::now() - _last >=
               Task::query<Query::Timing>() - _cyclic_epsilon;
    }
    void PeriodicTask::update() noexcept
    {
        _last = std::chrono::steady_clock::now();
    }
    std::chrono::milliseconds PeriodicTask::left() const noexcept
    {
        const auto elapsed = std::chrono::steady_clock::now() - _last;
        const auto period = Task::query<Query::Timing>();
        return duration_cast<std::chrono::milliseconds>(period - elapsed);
    }

    // Worker

    bool Worker::_process(PeriodicTask& task)
    {
        const auto type = task.query<Task::Query::Type>();
        const auto token = task.query<Task::Query::Run>();
        _last_task = tp_to_itg(std::chrono::steady_clock::now());
        return token == Task::Token::Continue;
    }
    bool Worker::_process(Task& task)
    {
        const auto type = task.query<Task::Query::Type>();
        Task::Token token = Task::Token::Discard;
        if ((type & (Task::Periodic | Task::Delayed)) && task != nullptr)
        {
            const auto timing = task.query<Task::Query::Timing>();
            _periodic_deadline = tp_to_itg(
                std::min(
                    itg_to_tp(_periodic_deadline.load()), std::chrono::steady_clock::now() + timing
                )
            );
            _periodic_tasks.emplace_back(std::move(task.value()));
        }
        else
        {
            if (task != nullptr)
                token = task.query<Task::Query::Run>();
            _last_task = tp_to_itg(std::chrono::steady_clock::now());
        }
        _last_task_np = tp_to_itg(std::chrono::steady_clock::now());
        context::ptr()->node()->_task_cnt--;
        _task_cnt--;
        return token == Task::Token::Continue;
    }
    void Worker::_tick()
    {
        std::chrono::steady_clock::time_point deadline{
            std::chrono::steady_clock::time_point::max()
        };
        bool removed = false;
        std::size_t task_cnt = 0;
        for (decltype(auto) it : _periodic_tasks)
        {
            if (it.ready())
            {
                it.update();
                if (!_process(it))
                {
                    it = PeriodicTask{};
                    _ptask_cnt--;
                    removed = true;
                }
            }
            if (it != nullptr)
                deadline = std::min(deadline, it.deadline());
        }
        _periodic_deadline = tp_to_itg(deadline);
        if (removed)
        {
            _periodic_tasks.erase(
                std::remove_if(
                    _periodic_tasks.begin(),
                    _periodic_tasks.end(),
                    [](const auto& v) { return v == nullptr; }
                ),
                _periodic_tasks.end()
            );
        }
    }

    Worker::~Worker()
    {
        stop();
    }

    Worker::ptr Worker::clone() const
    {
        return _clone_impl();
    }

    bool Worker::try_accept(Task& task) noexcept
    {
        return _try_accept_impl(task);
    }
    void Worker::accept(Task task)
    {
        _accept_impl(std::move(task));
    }
    void Worker::ping()
    {
        _ping_impl();
    }

    void Worker::start()
    {
        _start_impl();
        _stop = false;
    }
    std::vector<PeriodicTask> Worker::stop() noexcept
    {
        if (_stop)
            return {};
        Barrier lock;
        std::vector<PeriodicTask> tasks;
        accept({[&](Task::Query query, std::any& out)
                {
                    if (query == Task::Query::Type)
                    {
                        out =
                            static_cast<unsigned char>(Task::Type::Immediate | Task::Type::System);
                    }
                    else if (query == Task::Query::Run)
                    {
                        tasks = std::move(_periodic_tasks);
                        lock.release();
                        out = Task::Token::Discard;
                    }
                }});
        lock.wait();
        _stop_impl();
        _stop = true;
        return tasks;
    }

    unsigned char Worker::capabilities() const noexcept
    {
        return _cap;
    }
    bool Worker::is_runnable(unsigned char tags) const noexcept
    {
        return (_cap & tags) == tags;
    }
    bool Worker::is_running() const noexcept
    {
        return !_stop.load();
    }

    std::size_t Worker::tasks() const noexcept
    {
        return _task_cnt.load();
    }
    std::size_t Worker::ptasks() const noexcept
    {
        return _ptask_cnt;
    }

    std::chrono::steady_clock::time_point Worker::deadline() const noexcept
    {
        return itg_to_tp(_periodic_deadline.load());
    }
    std::chrono::steady_clock::time_point Worker::last_task() const noexcept
    {
        return itg_to_tp(_last_task.load());
    }
    std::chrono::steady_clock::time_point Worker::last_immediate_task() const noexcept
    {
        return itg_to_tp(_last_task_np.load());
    }

    // Ring Worker

    bool RingWorker::_try_accept_impl(Task& task) noexcept
    {
        return _tasks.try_enqueue(task);
    }
    void RingWorker::_accept_impl(Task task) noexcept
    {
        _tasks.enqueue(std::move(task));
    }
    void RingWorker::_ping_impl()
    {
        _tasks.enqueue(Task{});
    }
    void RingWorker::_start_impl()
    {
        if (is_running())
        {
            // Wait until it is started
            std::lock_guard lock(_startup_mtx);
            return;
        }
        std::lock_guard lock(_startup_mtx);
        const auto pool = context::ptr();
        _handle = std::jthread(
            [=, this, ptr = shared_from_this()]()
            {
                context::set(pool);
                while (!_stop_handle)
                {
                    std::array<Task, 6> tasks;
                    const auto max = std::min(
                        _max_idle_time,
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            deadline() - std::chrono::steady_clock::now()
                        )
                    );
                    if (max == std::chrono::milliseconds::max() ? _tasks.dequeue(tasks[0])
                        : max == std::chrono::milliseconds(0)   ? _tasks.try_dequeue(tasks[0])
                                                                : _tasks.dequeue(tasks[0], max))
                    {
                        std::size_t cnt = 1;
                        while (cnt < tasks.size())
                        {
                            if (_tasks.try_dequeue(tasks[cnt]))
                                cnt++;
                            else
                                break;
                        }

                        for (std::size_t i = 0; i < cnt; i++)
                        {
                            _process(tasks[i]);
                        }
                    }
                    else
                        _tick();
                    const auto s = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - last_immediate_task()
                    );
                    if (s >= _max_idle_time)
                        pool->shrink(shared_from_this());
                }
            }
        );
        _stop_handle = false;
    }
    void RingWorker::_stop_impl() noexcept
    {
        if (!is_running())
        {
            // Wait until stopped
            std::lock_guard lock(_startup_mtx);
            return;
        }
        std::lock_guard lock(_startup_mtx);
        _stop_handle = true;
        _tasks.enqueue(Task{});
        _handle.join();
    }
    Worker::ptr RingWorker::_clone_impl() const
    {
        return Worker::make<RingWorker>(capabilities(), _max_idle_time);
    }

    // Node

    std::size_t
    Node::_schedule_random(std::span<const std::shared_ptr<Worker>> workers, std::size_t samples)
    {
        thread_local std::random_device dev;
        thread_local std::mt19937 rng{dev()};
        thread_local std::uniform_real_distribution<float> dist{0.f, 1.f};

        std::size_t min_tasks = std::numeric_limits<std::size_t>::max();
        std::size_t min_idx = 0;
        for (std::size_t i = 0; i < samples; i++)
        {
            const auto idx = static_cast<std::size_t>(dist(rng) * workers.size());
            if (const auto tasks = workers[idx]->tasks(); tasks < min_tasks)
            {
                min_tasks = tasks;
                min_idx = idx;
            }
        }

        return min_idx;
    }
    std::size_t
    Node::_schedule_fast(std::span<const std::shared_ptr<Worker>> workers, std::size_t lookahead)
    {
        std::size_t min_tasks = std::numeric_limits<std::size_t>::max();
        std::size_t min_idx = 0;
        for (std::size_t i = _sheduler_ctr.fetch_add(1); i < lookahead; i++)
        {
            if (const auto tasks = workers[(i + lookahead) % workers.size()]->tasks();
                tasks < min_tasks)
            {
                min_tasks = tasks;
                min_idx = i;
            }
        }
        return min_idx;
    }
    std::size_t Node::_schedule_optimal(std::span<const std::shared_ptr<Worker>> workers)
    {
        std::size_t min_tasks = std::numeric_limits<std::size_t>::max();
        std::size_t min_idx = 0;
        for (std::size_t i = 0; i < workers.size(); i++)
        {
            if (const auto tasks = workers[i]->tasks(); tasks < min_tasks)
            {
                min_tasks = tasks;
                min_idx = i;
            }
        }
        return min_idx;
    }

    Node::ptr Node::make()
    {
        return std::make_shared<Node>();
    }

    void Node::start()
    {
        if (!_stop)
            return;
        _stop = false;
        const auto cfg = _cfg.load();
        grow(cfg->min_workers);
        const auto workers = _workers.load();
        for (decltype(auto) it : workers->workers)
        {
            it->start();
            _event(cfg->on_worker_start, *cfg, *it, Snapshot());
        }
    }
    void Node::stop() noexcept
    {
        if (_stop)
            return;
        _stop = true;
        const auto cfg = _cfg.load();
        const auto workers = _workers.load();
        for (decltype(auto) it : workers->workers)
        {
            it->stop();
            _event(cfg->on_worker_stop, *cfg, *it, Snapshot());
        }
        _workers = nullptr;
        _event(cfg->on_stop, *cfg);
    }

    void Node::grow(std::vector<Worker::ptr> workers)
    {
        if (workers.empty() || _stop)
            return;
        static auto populate = [](WorkerList& list)
        {
            for (decltype(auto) it : list.categories)
                it.reserve(list.workers.size());
            for (decltype(auto) it : list.workers)
            {
                const auto cap = it->capabilities();
                if (cap & Task::Immediate)
                    list.categories[0].push_back(it);
                if (cap & Task::Delayed)
                    list.categories[1].push_back(it);
                if (cap & Task::Periodic)
                    list.categories[2].push_back(it);
            }
        };
        const auto lock = _workers.load();
        if (lock == nullptr)
        {
            const auto tmp = std::make_shared<WorkerList>();
            tmp->main = workers[0];
            tmp->workers = std::move(workers);
            populate(*tmp);
            _workers = tmp;
        }
        else
        {
            lock->main->accept(
                {[ptr = shared_from_this(),
                  workers = std::move(workers)](Task::Query query, std::any& out) mutable
                 {
                     if (query == Task::Query::Type)
                     {
                         out = static_cast<unsigned char>(Task::System | Task::Immediate);
                     }
                     else if (query == Task::Query::Run)
                     {
                         const auto lock = ptr->_workers.load();
                         const auto tmp = std::make_shared<WorkerList>();
                         tmp->main = lock->main;
                         tmp->workers = std::move(workers);
                         tmp->workers.reserve(lock->workers.size());
                         tmp->workers.insert(
                             tmp->workers.end(), lock->workers.begin(), lock->workers.end()
                         );
                         populate(*tmp);
                         ptr->_workers = tmp;
                         out = Task::Token::Discard;
                     }
                 }}
            );
        }
    }
    void Node::grow(std::size_t count)
    {
        if (_stop)
            return;
        const auto cfg = _cfg.load();
        auto result = cfg->growth_callback(count, snapshot());
        if (result.size() != count)
            _throw(
                PoolException{
                    ErrorCode::InvalidGrowthCb,
                    "The growth callback returned an invalid worker count"
                }
            );
        grow(std::move(result));
    }
    void Node::grow()
    {
        if (_stop)
            return;
        const auto cfg = _cfg.load();
        auto result = cfg->growth_callback(0, snapshot());
        if (!result.empty())
            grow(std::move(result));
    }
    void Node::shrink(Worker::ptr worker, bool sync)
    {
        if (_stop)
            return;
        const auto workers = _workers.load();
        Barrier lock;
        workers->main->accept(
            {[=, this, ptr = shared_from_this(), &lock](Task::Query query, std::any& out)
             {
                 const auto workers = _workers.load();
                 if (workers->main == worker)
                     _throw(
                         PoolException{
                             ErrorCode::ShrinkFailure,
                             "Could not remove a worker from a Node because it "
                             "is the main worker and the "
                             "Node is running"
                         }
                     );
                 auto cpy = std::make_shared<WorkerList>(*workers);
                 const auto f = std::find(cpy->workers.begin(), cpy->workers.end(), worker);
                 if (f == cpy->workers.end())
                     _throw(
                         PoolException{
                             ErrorCode::ShrinkFailure, "Failed to find the worker for removal"
                         }
                     );
                 cpy->workers.erase(f);
                 auto& c1 = cpy->categories[0];
                 auto& c2 = cpy->categories[1];
                 auto& c3 = cpy->categories[2];
                 const auto f1 = std::find(c1.begin(), c1.end(), worker);
                 const auto f2 = std::find(c2.begin(), c2.end(), worker);
                 const auto f3 = std::find(c3.begin(), c3.end(), worker);
                 if (f1 != c1.end())
                     c1.erase(f1);
                 if (f2 != c2.end())
                     c2.erase(f2);
                 if (f3 != c3.end())
                     c3.erase(f3);
                 _workers = cpy;
                 const auto tasks = worker->stop();
                 for (decltype(auto) it : tasks)
                     schedule(std::move(it));
                 if (sync)
                     lock.release();
             }}
        );
        if (sync)
            lock.wait();
    }

    float Node::load() const noexcept
    {
        const auto lock = _workers.load();
        return float(_task_cnt.load() + 1) / lock->workers.size();
    }
    Node::Snapshot Node::snapshot() const noexcept
    {
        const auto workers = _workers.load();
        return {
            .tasks = _task_cnt.load(), .workers = workers == nullptr ? 0 : workers->workers.size()
        };
    }
    Node::Config::ptr Node::config() const noexcept
    {
        return _cfg.load();
    }

    void Node::reconfigure(reconfigure_callback callback)
    {
        const auto lock = _cfg.load();
        auto tmp = lock == nullptr ? std::make_shared<Config>() : std::make_shared<Config>(*lock);
        callback(*tmp);
        tmp->min_workers = std::max<std::size_t>(1, tmp->min_workers);
        const auto workers = _workers.load();
        _cfg = tmp;
        if (workers == nullptr || workers->workers.size() < tmp->min_workers)
            grow(
                workers == nullptr ? tmp->min_workers : tmp->min_workers - workers->workers.size()
            );
    }
    void Node::schedule(Task task, std::optional<Distribution> distv)
    {
    RESCHEDULE:
        if (_stop)
            return;
        const auto cfg = _cfg.load();
        const auto l = load();
        if (l >= cfg->growth_trigger_ratio)
            grow();
        const auto dist = distv.has_value() ? distv.value() : cfg->default_distribution;
        const auto lock = _workers.load();
        const auto type = task.query<Task::Query::Type>();
        const auto workers = lock->categories[std::countr_zero(type)];
        ErrorCode code = ErrorCode::Full;
        bool accepted = false;
        std::size_t idx = 0;
        if (l >= cfg->pressure_trigger_ratio)
        {
            code = ErrorCode::Pressure;
        }
        else
        {
            switch (dist)
            {
            case Distribution::Random:
                idx = _schedule_random(workers, 1);
            case Distribution::RandomSampled:
                idx = _schedule_random(workers, _random_sampled_default_samples);
            case Distribution::Fast:
                idx = _schedule_fast(workers, _fast_default_lookahead);
            case Distribution::Optimal:
                idx = _schedule_optimal(workers);
            }
            accepted = workers[idx]->try_accept(task);
        }
        if (!accepted)
        {
            const auto snap = snapshot();
            const auto token = _safe(cfg->reject_callback, *cfg, task, code, snap);
            _event(cfg->on_reject, *cfg, ErrorCode::Full, snap);
            if (token.has_value())
            {
                if (token->action == RejectionToken::Action::Reschedule)
                {
                    if (token->delay == std::chrono::milliseconds(0))
                        goto RESCHEDULE;
                    schedule(Task(
                        [delay = token->delay,
                         task = std::move(task),
                         ptr = weak_from_this()](Task::Query query, std::any& out) mutable
                        {
                            if (query == Task::Query::Type)
                            {
                                out = static_cast<unsigned char>(Task::Type::System);
                            }
                            else if (query == Task::Query::Timing)
                            {
                                out = delay;
                            }
                            else if (query == Task::Query::Run)
                            {
                                auto lock = ptr.lock();
                                if (lock != nullptr)
                                    lock->schedule(std::move(task));
                                out = Task::Token::Discard;
                            }
                        }
                    ));
                    return;
                }
                else if (token->action == RejectionToken::Action::Discard)
                {
                    return;
                }
            }
            _throw(PoolException{code, "Failed to accept task with no fallback policy"});
        }
        else
        {
            workers[idx]->_task_cnt++;
            _task_cnt++;
        }
    }

    // Topology

    std::vector<Node::ptr> SimpleTopology::nodes() const
    {
        return {Node::make()};
    }
    std::size_t SimpleTopology::active() const noexcept
    {
        return 0;
    }

    // Pool

    void ConcurrentPool::_ctx()
    {
        context::set(shared_from_this());
    }
    std::shared_ptr<std::vector<Node::ptr>> ConcurrentPool::_ensure_started()
    {
        const auto lock = _nodes.load();
        if (_changing)
            _stop.wait(_stop.load());
        if (_stop == true)
            throw PoolException{ErrorCode::NotStarted, "Concurrent pool was not started"};
        return lock;
    }
    void ConcurrentPool::_schedule(Task task, std::optional<Node::Distribution> dist)
    {
        auto lock = _ensure_started();
        const auto nodes = _nodes.load();
        const auto node = nodes->at(_topology->active());
        node->schedule(std::move(task), dist);
    }

    void ConcurrentPool::start()
    {
        start([](std::size_t, Node::Config&) {});
    }
    void ConcurrentPool::start(std::function<void(std::size_t, Node::Config&)> callback)
    {
        _ctx();
        if (_stop == false)
            return;
        while (true)
        {
            if (_changing)
                _changing.wait(true);
            if (bool v = false; _changing.compare_exchange_strong(v, true))
                break;
        }

        _nodes = std::make_shared<std::vector<Node::ptr>>(_topology->nodes());
        const auto nodes = _nodes.load();
        for (std::size_t i = 0; i < nodes->size(); i++)
        {
            auto node = nodes->at(i);
            node->reconfigure([&](Node::Config& cfg) { callback(i, cfg); });
            node->start();
        }

        _stop = false;
        _stop.notify_all();
        _changing = false;
        _changing.notify_one();
    }
    void ConcurrentPool::stop() noexcept
    {
        _ctx();
        if (_stop == true)
            return;
        while (true)
        {
            if (_changing)
                _changing.wait(true);
            if (bool v = false; _changing.compare_exchange_strong(v, true))
                break;
        }

        const auto nodes = _nodes.load();
        for (decltype(auto) it : *nodes)
            it->stop();
        nodes->clear();
        nodes->shrink_to_fit();

        _stop = true;
        _stop.notify_all();
        _changing = false;
        _changing.notify_one();
    }

    void ConcurrentPool::grow(std::vector<Worker::ptr> workers)
    {
        auto lock = _ensure_started();
        _ctx();

        const auto nodes = _nodes.load();
        for (auto it = nodes->begin() + 1; it != nodes->end(); ++it)
        {
            std::vector<Worker::ptr> cpy;
            cpy.resize(workers.size());
            std::transform(
                workers.begin(),
                workers.end(),
                cpy.begin(),
                [](const auto& v) { return v->clone(); }
            );
            grow(std::move(cpy), it - nodes->begin());
        }
        grow(std::move(workers), 0);
    }
    void ConcurrentPool::grow(std::vector<Worker::ptr> workers, std::size_t node)
    {
        auto lock = _ensure_started();
        _ctx();

        const auto nodes = _nodes.load();
        nodes->at(node)->grow(std::move(workers));
    }
    void ConcurrentPool::shrink(Worker::ptr ptr)
    {
        shrink(ptr, active());
    }
    void ConcurrentPool::shrink(Worker::ptr ptr, std::size_t node)
    {
        auto lock = _ensure_started();
        _ctx();

        const auto nodes = _nodes.load();
        nodes->at(node)->shrink(ptr);
    }

    Node::ptr ConcurrentPool::node()
    {
        auto lock = _ensure_started();
        return _nodes.load()->at(active());
    }
    Node::ptr ConcurrentPool::node(std::size_t id)
    {
        auto lock = _ensure_started();
        return _nodes.load()->at(id);
    }

    void ConcurrentPool::reconfigure(Node::reconfigure_callback callback)
    {
        auto lock = _ensure_started();
        _ctx();

        const auto nodes = _nodes.load();
        for (decltype(auto) it : *nodes)
            it->reconfigure(callback);
    }
    void ConcurrentPool::reconfigure(Node::reconfigure_callback callback, std::size_t id)
    {
        auto lock = _ensure_started();
        _ctx();
        const auto nodes = _nodes.load();
        const auto node = nodes->at(id);
        node->reconfigure(std::move(callback));
    }

    std::size_t ConcurrentPool::active() const noexcept
    {
        return _topology->active();
    }
} // namespace mt
