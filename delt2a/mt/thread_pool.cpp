#include "thread_pool.hpp"
#include <algorithm>
#include <numeric>

namespace mt
{
    ThreadPool::Worker::Worker(wptr ptr, unsigned char flags)
        : _ptr(ptr), _flags(flags)
    {}
    ThreadPool::Worker::~Worker()
    {
        stop();
    }

    ThreadPool::ThreadPool(Config cfg) :
        _cfg([](Config cfg) {
            cfg.min_threads = std::max<std::size_t>(1, cfg.min_threads);
            return cfg;
        }(std::move(cfg))),
        _nodes(_node_count())
    {
        for (decltype(auto) it : _nodes)
            it = std::make_shared<Node>(_cfg.max_threads, _cfg.default_dist);
    }

    bool ThreadPool::Worker::start() noexcept
    {
        const auto ptr = _ptr.lock();
        if (ptr == nullptr)
            return false;

        const auto nid = ptr->_node_id();
        auto node = ptr->_nodes[nid];
        auto& main = node->threads[0];
        auto full = false;
        auto run = false;

        if (_flags & MainWorker)
        {
            if (main.handle.joinable())
                run = true;
            _tid = 0;
        }
        else
            run = true;
        ++main.task_cnt;
        ++node->task_cnt;

        if (run)
        {
            std::atomic_flag flag{ false };
            flag.test_and_set(std::memory_order::acquire);
            main.tasks.enqueue(Task{
                .callback = [&]() -> Task::Token {
                    if (!(_flags & MainWorker))
                    {
                        if (node->thread_cnt < ptr->_cfg.max_threads &&
                            node->task_cnt / node->thread_cnt > ptr->_cfg.growth_trigger_ratio)
                        {
                            ptr->_launch_worker(nid);
                            _tid = node->thread_cnt - 1;
                        }
                        else
                            full = true;
                    }
                    else
                        main.stop = true;
                    flag.clear(std::memory_order::release);
                    flag.notify_all();
                    return Task::Token::Discard;
                }
            });
            while (true)
            {
                if (!flag.test_and_set(std::memory_order::acquire)) break;
                else flag.wait(true, std::memory_order::relaxed);
            }
        }
        if (_flags & MainWorker)
        {
            main.handle.join();
            main.stop = false;
            ++node->thread_cnt;
            node->thread_cnt.notify_all();
        }

        return !full;
    }
    void ThreadPool::Worker::stop() noexcept
    {
        const auto ptr = _ptr.lock();
        if (ptr == nullptr)
            return;
        const auto nid = ptr->_node_id();
        auto node = ptr->_nodes[nid];
        if (_tid == 0)
        {
            auto& main = node->threads[0];
            --node->thread_scnt;
            node->thread_scnt.notify_all();
            main.stop = true;
            --node->thread_cnt;
            node->thread_cnt.notify_all();
        }
        else
        {
            ptr->_node_shrink(1, *node);
            ptr->_worker_close(node, &node->threads[_tid]);
        }
        _tid = ~0ull;
    }
    void ThreadPool::Worker::wait(std::chrono::milliseconds max) const noexcept
    {
        const auto ptr = _ptr.lock();
        if (ptr == nullptr)
            return;

        context::set(ptr);

        const auto nid = ptr->_node_id();
        auto node = ptr->_nodes[nid];
        auto* state = &node->threads[_tid];

        if (!state->stop)
            ptr->_worker_wait_task(*node, *state, max);

        if (_tid == 0)
            ptr->_worker_manage(node, state);
    }
    void ThreadPool::Worker::wait() const noexcept
    {
        wait(std::chrono::milliseconds::max());
    }
    bool ThreadPool::Worker::active() const noexcept
    {
        return !_ptr.expired() && _tid != ~0ull;
    }

	void ThreadPool::_node_shrink(std::size_t cnt, Node& node) noexcept
	{
		while (cnt--)
		{
            if (int(node.thread_cnt) - int(++node.thread_scnt) >= int(_cfg.min_threads))
			{
				// Instead of stopping this thread specifically we stop the last one in the list
				// So access is simpler (i.e. if we have N threads they are the first N in the threads array)

				auto idx = node.thread_cnt - 1;
				auto* dstate = &node.threads[idx];
				bool expected = false;
				while (!dstate->stop.compare_exchange_strong(expected, true))
				{
					dstate = &node.threads[--idx];
					expected = false;
				}
			}
			else
				node.thread_scnt--;
		}
		_event(_cfg.event_shrink, node.thread_cnt);
	}
	void ThreadPool::_node_shutdown(Node& node) noexcept
	{
		for (auto it = node.threads.rbegin(); it != node.threads.rend(); ++it)
		{
			++node.thread_scnt;
			it->stop = true;
            it->tasks.enqueue(Task{});
		}
		_event(_cfg.event_shrink, 0);
	}
	void ThreadPool::_node_shutdown_sync() noexcept
	{
		for (std::size_t i = 0; i < _nodes.size(); i++)
		{
            auto& node = *_nodes[i];
			auto expected = node.thread_cnt.load();
			while (expected != 0)
			{
				node.thread_cnt.wait(expected);
				expected = node.thread_cnt.load();
			}
		}
	}

	std::size_t ThreadPool::_node_count() const noexcept
	{
		return 1;
	}
	std::size_t ThreadPool::_node_id() const noexcept
	{
		return 0;
	}

    std::size_t ThreadPool::_distribute_random() noexcept
    {
        // Not implemented yet
        return _distribute_fast();
    }
    std::size_t ThreadPool::_distribute_fast() noexcept
    {
        const auto node = _nodes[_node_id()];

        auto expected = node->thread_cnt.load();
        while (expected == 0)
        {
            node->thread_cnt.wait(expected);
            expected = node->thread_cnt.load();
        }

        const auto max_iters = std::min<std::size_t>(1, node->threads.size() / 3);
        const auto avg = node->task_cnt ? node->thread_cnt / node->task_cnt : 0;

        std::size_t backoff = 0;
        std::size_t id = 0;
        while (backoff++ != max_iters &&
               node->threads[(id = (node->thread_ctr++ % node->thread_cnt))].task_cnt < avg);

        return node->threads[id].stop ? 0 : id;
    }
    std::size_t ThreadPool::_distribute_optimal() noexcept
    {
        // Not implemented yet
        return _distribute_fast();
    }
    std::size_t ThreadPool::_distribute(Distribution algo) noexcept
    {
        static constexpr auto table = std::array{
            &ThreadPool::_distribute_random,
            &ThreadPool::_distribute_fast,
            &ThreadPool::_distribute_optimal
        };
        return (this->*table[std::size_t(algo)])();
    }

    void ThreadPool::_schedule(Task task, std::optional<Distribution> dist) noexcept
	{
        auto node = _nodes[_node_id()];
        if (_cfg.flags & Config::Flags::MainCyclicWorker)
        {
            auto& thread = node->threads[0];
            ++thread.task_cnt;
            ++node->task_cnt;
            thread.tasks.enqueue(
                std::move(task)
            );
        }
        else
        {
            auto& thread = node->threads[_distribute(
                dist.has_value() ?
                    dist.value() : node->dist.load()
            )];
            ++thread.task_cnt;
            ++node->task_cnt;
            thread.tasks.enqueue(
                std::move(task)
            );
        }
	}
	bool ThreadPool::_check_overload() noexcept
	{
		if (_cfg.flags & Config::ManageOverload)
		{
			const auto nid = _node_id();
            const auto avg = _nodes[nid]->task_cnt ? _nodes[nid]->thread_cnt / _nodes[nid]->task_cnt : 0;
			if (avg >= _cfg.pressure_trigger_ratio)
			{
				return true;
			}
		}
		return false;
	}

	bool ThreadPool::_worker_handle_task(Task& task, Node& node, Thread& state) noexcept
	{
		const auto token = task.callback();
		return token == Task::Token::Continue;
	}
    void ThreadPool::_worker_wait_task(Node& node, Thread& state, std::chrono::milliseconds wait) noexcept
	{
        std::chrono::milliseconds max{ wait };
		if (state.id != 0 &&
			node.thread_cnt != _cfg.min_threads &&
			(_cfg.flags & Config::AutoShrink))
		{
            max = std::min(max, std::min(
				state.cyclic_wait,
                std::chrono::duration_cast<std::chrono::milliseconds>(
					_cfg.max_idle_time
				)
            ));
		}
		else
		{
            max = std::min(wait, state.cyclic_wait);
		}

		std::array<Task, 6> tasks;
        if (max == std::chrono::milliseconds::max() ?
			state.tasks.dequeue(tasks[0]) :
			state.tasks.dequeue(tasks[0], max))
		{
			std::size_t cnt = 1;
			while (cnt < tasks.size())
			{
				if (state.tasks.try_dequeue(tasks[cnt])) cnt++;
				else break;
			}

			for (std::size_t i = 0; i < cnt; i++)
			{
                --state.task_cnt;
                --node.task_cnt;
				auto& task = tasks[i];
                if (task.callback)
                {
                    if (task.timing != task.oneoff)
                    {
                        ++node.cyclic_task_cnt;
                        state.cyclic_tasks.push_back({
                            std::move(task.callback),
                            task.timing
                        });
                        state.cyclic_wait = std::min(task.timing, state.cyclic_wait);
                    }
                    else
                    {
                        _worker_handle_task(task, node, state);
                    }
                }
			}
			state.last_task = std::chrono::steady_clock::now();
		}
		else
		{
            std::chrono::milliseconds wait{ std::chrono::milliseconds::max() };
			bool removed = false;
            std::size_t task_cnt = 0;
			for (decltype(auto) it : state.cyclic_tasks)
			{
				if (it.ready())
                {
                    ++task_cnt;
                    ++state.task_cnt;
                    ++node.task_cnt;
                    it.update();
                    if (!_worker_handle_task(it, node, state))
					{
						it.callback = nullptr;
						removed = true;
                    }
                }
                wait = std::min(wait, it.timing - it.left());
			}
            state.task_cnt -= task_cnt;
            node.task_cnt -= task_cnt;
            state.cyclic_wait = wait;
			if (removed)
			{
				state.cyclic_tasks.erase(
					std::remove_if(state.cyclic_tasks.begin(), state.cyclic_tasks.end(), [](const auto& v) {
						return v.callback == nullptr;
					}),
					state.cyclic_tasks.end()
				);
			}
		}

		if (std::chrono::steady_clock::now() - state.last_task >= _cfg.max_idle_time)
			_node_shrink(1, node);
	}

    void ThreadPool::_worker_main(std::size_t id, wptr ptr, std::chrono::milliseconds wait) noexcept
	{
		context::set(ptr.lock());

		const auto nid = _node_id();
		Node::ptr node = nullptr;
		Thread* state = nullptr;
		// Safely initialize state pointers
		{
			auto pool = ptr.lock();
			if (!pool)
				return;
			node = pool->_nodes[nid];
			state = &pool->_nodes[nid]->threads[id];
		}
		// Wait for tasks
		while (!state->stop)
		{
			auto pool = ptr.lock();
            if (pool)
                _worker_manage(node, state);
			else
				return;
            _worker_wait_task(*node, *state, wait);
		}
		--node->thread_cnt;
		--node->thread_scnt;
        node->thread_cnt.notify_all();
        node->thread_scnt.notify_all();
	}
    void ThreadPool::_worker_normal(std::size_t id, wptr ptr, std::chrono::milliseconds wait) noexcept
    {
		context::set(ptr.lock());

		Node::ptr node = nullptr;
		Thread* state = nullptr;
		// Safely initialize state pointers
		{
			const auto nid = _node_id();
			auto pool = ptr.lock();
			if (!pool)
				return;
			node = pool->_nodes[nid];
			state = &pool->_nodes[nid]->threads[id];
		}
		// Wait for tasks
		while (!state->stop)
		{
			auto pool = ptr.lock();
			[[ unlikely ]] if (!pool)
				return;
            _worker_wait_task(*node, *state, wait);
		}
		--node->thread_cnt;
		--node->thread_scnt;
        node->thread_cnt.notify_all();
        node->thread_scnt.notify_all();
        _worker_close(node, state);
	}
    void ThreadPool::_worker_manage(Node::ptr node, Thread* state) noexcept
    {
        const auto nid = _node_id();
        if ((_cfg.flags & Config::AutoGrow) &&
            node->thread_cnt < _cfg.max_threads &&
            node->task_cnt / node->thread_cnt > _cfg.growth_trigger_ratio)
        {
            _launch_worker(
                nid,
                &ThreadPool::_worker_normal,
                _cfg.growth_callback(node->thread_cnt, node->task_cnt)
            );
        }
    }
    void ThreadPool::_worker_close(Node::ptr node, Thread* state) noexcept
    {
        // Redistribute the tasks to other running threads if present
        // Wait for the next one to prevent lost tasks and an invalid thread state
        if (state->id != node->threads.size() - 1)
        {
            std::size_t idx = node->threads.size() - 1;
            while (idx != state->id)
            {
                auto& nx = node->threads[idx];
                auto expected = nx.stop.load();
                while (expected != true)
                {
                    nx.stop.wait(expected);
                    expected = nx.stop.load();
                }
                idx--;
            }
        }

        Task task;
        while (state->tasks.try_dequeue(task))
            node
                ->threads[0].tasks
                .enqueue(std::move(task));

        auto cyclic_tasks = std::move(state->cyclic_tasks);
        for (decltype(auto) it : cyclic_tasks)
            _schedule(std::move(it), std::nullopt);
    }

    void ThreadPool::_launch_worker(std::size_t nid, std::size_t cnt)
    {
        auto node = _nodes[nid];
        while (cnt--)
        {
            for (std::size_t i = node->thread_cnt; i < _cfg.max_threads; i++)
            {
                if (bool expected = true;
                    node->threads[i].stop.compare_exchange_strong(expected, false))
                {
                    node->thread_cnt++;
                    const auto id = i;
                    auto& state = node->threads[id];
                    state.id = id;
                    state.last_task = std::chrono::steady_clock::now();
                    break;
                }
            }
        }
        _event(_cfg.event_grow, node->thread_cnt);
    }

	ThreadPool::ptr ThreadPool::make(Config cfg)
	{
        return std::make_shared<ThreadPool>(std::move(cfg));
	}
	ThreadPool::ptr ThreadPool::make()
	{
		return make(Config());
	}

	std::size_t ThreadPool::threads(std::size_t node) const noexcept
	{
		return _nodes[node]->thread_cnt;
	}
	std::size_t ThreadPool::threads() const noexcept
	{
		return std::accumulate(_nodes.begin(), _nodes.end(), std::size_t{0}, [](auto c, const auto& n) {
			return c + n->thread_cnt;
		});
	}

	std::size_t ThreadPool::tasks(std::size_t node) const noexcept
	{
		return _nodes[node]->task_cnt;
	}
	std::size_t ThreadPool::tasks() const noexcept
	{
		return std::accumulate(_nodes.begin(), _nodes.end(), std::size_t{0}, [](auto c, const auto& n) {
			return c + n->task_cnt;
		});
	}

    ThreadPool::Distribution ThreadPool::distribution(std::size_t node) const noexcept
    {
        return _nodes[node]->dist;
    }
    ThreadPool::Distribution ThreadPool::distribution() const noexcept
    {
        return _nodes[_node_id()]->dist;
    }

    void ThreadPool::set_distribution(std::size_t node, Distribution dist) const noexcept
    {
        _nodes[node]->dist = dist;
    }
    void ThreadPool::set_distribution(Distribution dist) const noexcept
    {
        _nodes[_node_id()]->dist = dist;
    }

    void ThreadPool::start() noexcept
	{
        stop();
		for (std::size_t i = 0; i < _nodes.size(); i++)
		{
            _launch_worker(i, &ThreadPool::_worker_main);
			_launch_worker(i, &ThreadPool::_worker_normal, _cfg.min_threads - 1);
		}
	}
	void ThreadPool::stop() noexcept
	{
		for (std::size_t i = 0; i < _nodes.size(); i++)
			_node_shutdown(*_nodes[i]);
		_node_shutdown_sync();
	}
	void ThreadPool::bind() noexcept
	{
		context::set(shared_from_this());
	}
	void ThreadPool::resize(std::size_t size) noexcept
	{
		for (std::size_t i = 0; i < _nodes.size(); i++)
			resize(i, size);
	}
	void ThreadPool::resize(std::size_t node, std::size_t size) noexcept
	{
		const auto cnt = _nodes[node]->thread_cnt.load();
		if (cnt < size)
		{
			_launch_worker(
				node,
				&ThreadPool::_worker_normal,
				size - cnt
			);
		}
		else if (cnt > size)
		{
			_node_shrink(cnt - size, *_nodes[node]);
		}
	}

    ThreadPool::Worker ThreadPool::worker(unsigned char flags) noexcept
    {
        return Worker(weak_from_this(), flags);
    }
}
