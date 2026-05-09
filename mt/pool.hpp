#ifndef POOL_HPP
#define POOL_HPP

#include "future.hpp"
#include "task_ring.hpp"
#include <any>
#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace mt
{
    class Barrier
    {
    private:
        std::atomic_flag _flag{false};
    public:
        Barrier();

        void release() noexcept;
        void wait() noexcept;
    };

    class ConcurrentPool;
    class Node;
    class Worker;

    enum class ErrorCode
    {
        // Unexpected exception was thrown
        Exception,
        // Scheduling was aborted due to pressure
        Pressure,
        // Scheduling failed because the task was rejected
        Full,
        // Rescheduling at a later time also failed
        RescheduleFailure,
        // Pool was not started but a running pool was required for the operation
        NotStarted,
        // The growth callback didn't return a proper worker count
        InvalidGrowthCb,
        // A shrink operation could not be performed
        ShrinkFailure,
    };

    struct PoolException : std::runtime_error
    {
        ErrorCode code;
        PoolException(ErrorCode code, std::string str) noexcept :
            code(code), runtime_error(std::move(str))
        {
        }
    };

    class Task
    {
    public:
        enum Type : unsigned char
        {
            Empty = 0x00,
            Immediate = 1 << 0,
            Delayed = 1 << 1,
            Periodic = 1 << 2,
            System = 1 << 3,

            All = Immediate | Delayed | Periodic
        };

        enum class Token
        {
            Continue,
            Discard,
        };
        enum class Query
        {
            // void -> std::chrono::milliseconds
            Timing,
            // void -> unsigned char
            Type,
            // void -> Token
            Run,
            // Public + N is free for the library user
            Public
        };
        using callback = std::function<void(Query, std::any&)>;
    protected:
        callback _cb{};
    public:
        Task() = default;
        Task(callback cb) : _cb(std::move(cb)) {}
        Task(Task&&) = default;
        Task(const Task&) = default;

        template<Query Qt> auto query() const
        {
            std::any out;
            if constexpr (Qt == Query::Run)
            {
                _cb(Query::Run, out);
                return std::any_cast<Token>(out);
            }
            else if constexpr (Qt == Query::Type)
            {
                _cb(Query::Type, out);
                return std::any_cast<unsigned char>(out);
            }
            else if constexpr (Qt == Query::Timing)
            {
                _cb(Query::Timing, out);
                return std::any_cast<std::chrono::milliseconds>(out);
            }
        }

        const callback& value() const noexcept;
        callback& value() noexcept;

        bool operator==(std::nullptr_t) const noexcept;
        bool operator!=(std::nullptr_t) const noexcept;

        Task& operator=(const Task&) = default;
        Task& operator=(Task&&) = default;
    };
    class PeriodicTask : public Task
    {
    private:
        static constexpr auto _cyclic_epsilon = std::chrono::milliseconds(5);

        std::chrono::steady_clock::time_point _last{};
    public:
        using Task::Task;

        bool ready() const;
        void update() noexcept;
        std::chrono::steady_clock::time_point deadline() const noexcept;
        std::chrono::milliseconds left() const noexcept;
    };

    class context
    {
    public:
        using pool = std::shared_ptr<ConcurrentPool>;
        struct Guard
        {
            pool ptr;
            Guard();
            ~Guard();
        };
    private:
        static inline thread_local pool _pool_ptr{nullptr};
    public:
        static pool ptr() noexcept;
        static ConcurrentPool& get() noexcept;
        static void set(pool ptr) noexcept;
    };

    // Represents an execution resource
    class Worker : public std::enable_shared_from_this<Worker>
    {
    public:
        template<typename Type> using vptr = std::shared_ptr<Type>;
        using ptr = std::shared_ptr<Worker>;
        using wptr = std::weak_ptr<Worker>;
    private:
        static constexpr auto _tmax =
            std::numeric_limits<std::chrono::steady_clock::time_point::rep>::max();

        const unsigned char _cap{0x00};

        std::vector<PeriodicTask> _periodic_tasks{};

        std::atomic<bool> _stop{true};
        std::atomic<std::size_t> _task_cnt{0};
        std::atomic<std::size_t> _ptask_cnt{0};

        std::atomic<std::chrono::steady_clock::time_point::rep> _last_task{_tmax};
        std::atomic<std::chrono::steady_clock::time_point::rep> _last_task_np{_tmax};
        std::atomic<std::chrono::steady_clock::time_point::rep> _periodic_deadline{_tmax};
    protected:
        bool _process(PeriodicTask& task);
        bool _process(Task& task);
        void _tick();

        virtual bool _try_accept_impl(Task& task) noexcept = 0;
        virtual void _accept_impl(Task task) noexcept = 0;
        virtual void _ping_impl() = 0;
        virtual void _start_impl() = 0;
        virtual void _stop_impl() noexcept = 0;
        virtual ptr _clone_impl() const = 0;
    public:
        friend class Node;

        template<typename Type, typename... Argv> static vptr<Type> make(Argv&&... args)
        {
            return std::make_shared<Type>(std::forward<Argv>(args)...);
        }

        Worker(unsigned char capabilities) : _cap(capabilities) {}
        Worker(const Worker&) = delete;
        Worker(Worker&&) = delete;
        virtual ~Worker();

        ptr clone() const;

        bool try_accept(Task& task) noexcept;
        void accept(Task task);
        void ping();

        void start();
        std::vector<PeriodicTask> stop() noexcept;

        unsigned char capabilities() const noexcept;
        bool is_runnable(unsigned char tags) const noexcept;
        bool is_running() const noexcept;

        std::size_t tasks() const noexcept;
        std::size_t ptasks() const noexcept;

        std::chrono::steady_clock::time_point deadline() const noexcept;
        std::chrono::steady_clock::time_point last_task() const noexcept;
        std::chrono::steady_clock::time_point last_immediate_task() const noexcept;

        Worker& operator=(const Worker&) = delete;
        Worker& operator=(Worker&&) = delete;
    };
    // Represents a computing node for NUMA awareness
    class Node : public std::enable_shared_from_this<Node>
    {
    public:
        using ptr = std::shared_ptr<Node>;
        using wptr = std::weak_ptr<Node>;

        enum Flags : unsigned char
        {
            AutoGrow = 1 << 0,
            AutoShrink = 1 << 1,
            RejectThrow = 1 << 2,
        };
        enum class Distribution
        {
            Random,
            RandomSampled,
            Fast,
            Optimal
        };

        struct RejectionToken
        {
            enum class Action
            {
                Reschedule,
                Discard,
                Throw,
            } action;
            std::chrono::milliseconds delay{};
        };
        struct Snapshot
        {
            std::size_t tasks{};
            std::size_t workers{};
        };
        struct Config
        {
            using ptr = std::shared_ptr<const Config>;

            template<typename Ret, typename... Argv>
            using event = std::vector<std::function<Ret(Argv...)>>;

            unsigned char flags{Flags::AutoGrow | Flags::AutoShrink};

            std::size_t min_workers{1};
            std::size_t max_workers{
                static_cast<std::size_t>(std::thread::hardware_concurrency() * 2)
            };

            Distribution default_distribution{Distribution::Optimal};

            // Ratios Task/Worker

            float growth_trigger_ratio{5.f};
            float pressure_trigger_ratio{15.f};

            // If the worker count is 0, the first worker in the returned vector will be selected
            // as the system worker for the current Node
            // Signature: Workers -> Count Hint (0 if no hint) | Snapshot
            std::function<std::vector<Worker::ptr>(std::size_t, const Snapshot&)> growth_callback{};
            std::function<RejectionToken(Task&, ErrorCode, const Snapshot&)> reject_callback{};

            // Events

            event<void> on_start{};
            event<void> on_stop{};

            event<void, Worker&, const Snapshot&> on_worker_start{};
            event<void, Worker&, const Snapshot&> on_worker_stop{};

            // First argument is the increase in workers
            event<void, std::size_t, const Snapshot&> on_grow{};
            // All of these need to pass for the worker to be removed
            event<bool, Worker&, const Snapshot&> on_shrink{};

            event<void, ErrorCode> on_error{};
            event<void, ErrorCode, const Snapshot&> on_reject{};
        };
        using reconfigure_callback = std::function<void(Config&)>;
    private:
        struct WorkerList
        {
            std::vector<Worker::ptr> workers;
            // Immediate, Delayed, Periodic
            std::array<std::vector<Worker::ptr>, 3> categories;
            Worker::ptr main{};
        };
        static constexpr auto _fast_default_lookahead = 4;
        static constexpr auto _random_sampled_default_samples = 3;
    private:
        const std::weak_ptr<ConcurrentPool> _pool{};

        // Stores the current version of the config
        std::atomic<Config::ptr> _cfg{std::make_shared<Config>()};

        std::atomic<std::size_t> _task_cnt{0};
        std::atomic<std::size_t> _sheduler_ctr{0};
        std::atomic<bool> _stop{true};

        std::atomic<std::shared_ptr<const WorkerList>> _workers{};

        std::size_t
        _schedule_random(std::span<const std::shared_ptr<Worker>> workers, std::size_t samples);
        std::size_t
        _schedule_fast(std::span<const std::shared_ptr<Worker>> workers, std::size_t lookahead);
        std::size_t _schedule_optimal(std::span<const std::shared_ptr<Worker>> workers);

        template<typename Exception> [[noreturn]] void _throw(Exception ex)
        {
            const auto cfg = _cfg.load();
            _event(cfg->on_error, *cfg, ex.code);
            throw std::move(ex);
        }
        template<typename Func, typename... Argv>
        auto _safe(Func&& callback, const Config& cfg, Argv&&... args) noexcept
        {
            using ret = std::invoke_result_t<Func, Argv...>;
            context::Guard guard;
            try
            {
                if constexpr (std::is_same_v<ret, void>)
                {
                    std::invoke(std::forward<Func>(callback), std::forward<Argv>(args)...);
                }
                else
                {
                    return std::optional<ret>(
                        std::invoke(std::forward<Func>(callback), std::forward<Argv>(args)...)
                    );
                }
            }
            catch (...)
            {
                for (decltype(auto) it : cfg.on_error)
                {
                    try
                    {
                        it(ErrorCode::Exception);
                    }
                    catch (...)
                    {
                        // I guess what can we do really lol
                    }
                }
                if constexpr (!std::is_same_v<ret, void>)
                    return std::optional<ret>(std::nullopt);
            }
        }
        template<typename Type, typename... Argv>
        auto _event(Type& ev, const Config& cfg, Argv&&... args)
        {
            if (ev.empty())
                return;
            using ret = std::invoke_result_t<typename Type::value_type, Argv...>;
            if constexpr (std::is_same_v<ret, void>)
            {
                _safe(
                    [&]()
                    {
                        for (decltype(auto) it : ev)
                            it(args...);
                    },
                    cfg
                );
            }
            else if (std::is_same_v<ret, bool>)
            {
                bool out = true;
                _safe(
                    [&]()
                    {
                        for (decltype(auto) it : ev)
                            out = out && it(args...);
                    },
                    cfg
                );
                return out;
            }
            else
            {
                static_assert(false, "Invalid signature");
            }
        }
    public:
        friend class Worker;

        static ptr make();

        Node() = default;
        Node(const Node&) = delete;
        Node(Node&&) = delete;

        void start();
        void stop() noexcept;

        template<typename Type, typename... Argv> void grow(unsigned char cap, Argv&&... args)
        {
            grow({Worker::make<Type>(cap, std::forward<Argv>(args)...)});
        }

        void grow(std::vector<Worker::ptr> workers);
        void grow(std::size_t count);
        void grow();
        void shrink(Worker::ptr worker, bool sync = false);

        float load() const noexcept;
        std::size_t workers() const noexcept;

        // Allocates a copy of the current configs and provides the version for writing
        // Subsequently it commits the write
        void reconfigure(reconfigure_callback callback);
        void schedule(Task task, std::optional<Distribution> dist = std::nullopt);
        Snapshot snapshot() const noexcept;
        Config::ptr config() const noexcept;

        Node& operator=(const Node&) = delete;
        Node& operator=(Node&&) = delete;
    };

    // Default worker
    // Represents a single thread
    // Accepts tasks to a static ring buffer
    class RingWorker : public Worker
    {
    protected:
        TaskRing<Task, 16> _tasks{};
        std::jthread _handle{};
        std::mutex _startup_mtx{};
        std::atomic<bool> _stop_handle{true};
        std::chrono::milliseconds _max_idle_time{std::chrono::milliseconds::max()};

        virtual bool _try_accept_impl(Task& task) noexcept override;
        virtual void _accept_impl(Task task) noexcept override;
        virtual void _ping_impl() override;
        virtual void _start_impl() override;
        virtual void _stop_impl() noexcept override;
        virtual ptr _clone_impl() const override;
    public:
        using Worker::Worker;

        RingWorker(unsigned char cap, std::chrono::milliseconds max_idle_time) :
            Worker(cap), _max_idle_time(max_idle_time)
        {
        }
    };

    class Topology
    {
    public:
        virtual ~Topology() = default;
        virtual std::vector<Node::ptr> nodes() const = 0;
        virtual std::size_t active() const noexcept = 0;
    };

    // NUMA unaware
    // Only one default node
    class SimpleTopology : public Topology
    {
    public:
        virtual std::vector<Node::ptr> nodes() const override;
        virtual std::size_t active() const noexcept override;
    };

    class ConcurrentPool : public std::enable_shared_from_this<ConcurrentPool>
    {
    public:
        using ptr = std::shared_ptr<ConcurrentPool>;
        using wptr = std::weak_ptr<ConcurrentPool>;
    private:
        std::unique_ptr<Topology> _topology{nullptr};
        std::atomic<bool> _changing{false};
        std::atomic<bool> _stop{true};
        std::atomic<std::shared_ptr<std::vector<Node::ptr>>> _nodes{};

        void _schedule(Task task, std::optional<Node::Distribution> dist);
        void _ctx();
        std::shared_ptr<std::vector<Node::ptr>> _ensure_started();

        template<typename Future, typename Ret> static consteval void _future_check()
        {
            static_assert(
                std::is_same_v<Ret, typename Future::ret>,
                "Future does not match function return type"
            );
        }
    public:
        template<typename Topology> static ptr make()
        {
            auto ptr = std::make_shared<ConcurrentPool>();
            ptr->_topology = std::make_unique<Topology>();
            return ptr;
        }

        ConcurrentPool() = default;
        ConcurrentPool(const ConcurrentPool&) = delete;
        ConcurrentPool(ConcurrentPool&&) = delete;

        void start(std::function<void(std::size_t, Node::Config&)> callback);
        void start();
        void stop() noexcept;

        template<typename Type, typename... Argv>
        void grow(unsigned char capabilities, Argv&&... args)
        {
            grow({Worker::make<Type>(capabilities, std::forward<Argv>(args)...)});
        }
        template<typename Type, typename... Argv>
        void grow_node(unsigned char capabilities, std::size_t node, Argv&&... args)
        {
            grow({Worker::make<Type>(capabilities, std::forward<Argv>(args)...)}, node);
        }

        void grow(std::vector<Worker::ptr> workers);
        void grow(std::vector<Worker::ptr> workers, std::size_t node);
        void shrink(Worker::ptr ptr, std::size_t node);
        void shrink(Worker::ptr ptr);

        void reconfigure(Node::reconfigure_callback callback);
        void reconfigure(Node::reconfigure_callback callback, std::size_t node);

        std::size_t active() const noexcept;

        Node::ptr node();
        Node::ptr node(std::size_t id);

        // Overrides

        // The type of the future will have to match the passed function's return type
        template<typename Future> struct TaskConfig
        {
            using future_type = Future;
            std::optional<Node::Distribution> distribution{std::nullopt};
            Future future{nullptr};
        };

        template<typename Future, typename Func, typename... Argv>
        auto launchd(Func&& callback, TaskConfig<Future> cfg, Argv&&... args)
        {
            _ctx();
            using ret = std::invoke_result_t<Func, Argv...>;
            _future_check<Future, ret>();

            const auto task = cfg.future == nullptr ? Future::make() : cfg.future;
            _schedule(
                {[block = std::weak_ptr(task.block()),
                  callback = std::forward<Func>(callback),
                  ... args = std::forward<Argv>(args)](Task::Query query, std::any& out) mutable
                 {
                     if (query == Task::Query::Type)
                     {
                         out = static_cast<unsigned char>(Task::Type::Immediate);
                     }
                     else if (query == Task::Query::Run)
                     {
                         const auto ptr = block.lock();
                         if (!(ptr && ptr->is_discarded()))
                         {
                             try
                             {
                                 if (ptr)
                                 {
                                     if constexpr (std::is_same_v<void, ret>)
                                     {
                                         callback(std::forward<Argv>(args)...);
                                         ptr->set_value();
                                     }
                                     else
                                         ptr->set_value(callback(std::forward<Argv>(args)...));
                                 }
                                 else
                                 {
                                     callback(std::forward<Argv>(args)...);
                                 }
                             }
                             catch (...)
                             {
                                 if (ptr)
                                     ptr->set_exception(std::current_exception());
                             }
                             out = Task::Token::Discard;
                         }
                     }
                 }},
                cfg.distribution
            );
            return task;
        }

        template<typename Func, typename... Argv>
        void launchd_void(Func&& callback, std::optional<Node::Distribution> dist, Argv&&... args)
        {
            _ctx();
            using ret = std::invoke_result_t<Func, Argv...>;
            _schedule(
                {[callback = std::forward<Func>(callback),
                  ... args = std::forward<Argv>(args)](Task::Query query, std::any& out) mutable
                 {
                     if (query == Task::Query::Type)
                     {
                         out = static_cast<unsigned char>(Task::Type::Immediate);
                     }
                     else if (query == Task::Query::Run)
                     {
                         try
                         {
                             callback(std::forward<Argv>(args)...);
                         }
                         catch (...)
                         {
                         }
                         out = Task::Token::Discard;
                     }
                 }},
                dist
            );
        }

        template<typename Future, typename Func, typename... Argv>
        auto launchd_cyclic(
            std::chrono::milliseconds time, Func&& callback, TaskConfig<Future> cfg, Argv&&... args
        )
        {
            _ctx();
            using ret = std::invoke_result_t<Func, std::nullptr_t, Argv...>;
            _future_check<Future, ret>();
            const auto task = cfg.future == nullptr ? Future::make() : cfg.future;
            _schedule(
                {[block = task.block(),
                  callback = std::forward<Func>(callback),
                  ... args = std::forward<Argv>(args),
                  time](Task::Query query, std::any& out) mutable
                 {
                     if (query == Task::Query::Type)
                     {
                         out = static_cast<unsigned char>(Task::Type::Periodic);
                     }
                     else if (query == Task::Query::Timing)
                     {
                         out = time;
                     }
                     else if (query == Task::Query::Run)
                     {
                         const auto fut = future<ret>(block);
                         if (!block->is_discarded() && !block->is_paused())
                         {
                             try
                             {
                                 if constexpr (std::is_same_v<void, ret>)
                                 {
                                     callback(fut, args...);
                                     block->set_tick();
                                 }
                                 else
                                     block->set_tick(callback(fut, args...));
                             }
                             catch (...)
                             {
                                 block->set_tick_exception(std::current_exception());
                             }
                         }
                         out = block->is_discarded() ? Task::Token::Discard : Task::Token::Continue;
                     }
                 }},
                cfg.distribution
            );
            return task;
        }

        template<typename Future, typename Func, typename... Argv>
        auto launchd_deferred(
            std::chrono::milliseconds time, Func&& callback, TaskConfig<Future> cfg, Argv&&... args
        )
        {
            _ctx();
            using ret = std::invoke_result_t<Func, Argv...>;
            _future_check<Future, ret>();
            const auto task = cfg.future == nullptr ? Future::make() : cfg.future;
            _schedule(
                {[block = std::weak_ptr(task.block()),
                  callback = std::forward<Func>(callback),
                  ... args = std::forward<Argv>(args),
                  time](Task::Query query, std::any& out) mutable
                 {
                     if (query == Task::Query::Type)
                     {
                         out = static_cast<unsigned char>(Task::Type::Delayed);
                     }
                     else if (query == Task::Query::Timing)
                     {
                         out = time;
                     }
                     else if (query == Task::Query::Run)
                     {
                         const auto ptr = block.lock();
                         if (!(ptr && ptr->is_discarded()))
                         {
                             try
                             {
                                 if (ptr)
                                 {
                                     if (ptr->is_paused())
                                     {
                                         out = Task::Token::Continue;
                                         return;
                                     }
                                     if constexpr (std::is_same_v<void, ret>)
                                     {
                                         callback(std::forward<Argv>(args)...);
                                         ptr->set_value();
                                     }
                                     else
                                         ptr->set_value(callback(std::forward<Argv>(args)...));
                                 }
                                 else
                                 {
                                     callback(std::forward<Argv>(args)...);
                                 }
                             }
                             catch (...)
                             {
                                 if (ptr)
                                     ptr->set_exception(std::current_exception());
                             }
                         }
                         out = Task::Token::Discard;
                     }
                 }},
                cfg.distribution
            );

            return task;
        }

        template<typename Func, typename... Argv>
        auto co_launchd(Func&& callback, std::optional<Node::Distribution> dist, Argv&&... args)
        {
            _ctx();
            using ret = std::invoke_result_t<Func, Argv...>;
            const auto task = co_future<ret>::make();
            _schedule(
                {[block = std::weak_ptr(task.block()),
                  callback = std::forward<Func>(callback),
                  ... args = std::forward<Argv>(args)](Task::Query query, std::any& out) mutable
                 {
                     if (query == Task::Query::Type)
                     {
                         out = static_cast<unsigned char>(Task::Type::Immediate);
                     }
                     else if (query == Task::Query::Run)
                     {
                         const auto ptr = block.lock();
                         if (!(ptr && ptr->is_discarded()))
                         {
                             try
                             {
                                 if (ptr)
                                 {
                                     if constexpr (std::is_same_v<void, ret>)
                                     {
                                         callback(std::forward<Argv>(args)...);
                                         ptr->set_value();
                                     }
                                     else
                                         ptr->set_value(callback(std::forward<Argv>(args)...));
                                 }
                                 else
                                 {
                                     callback(std::forward<Argv>(args)...);
                                 }
                             }
                             catch (...)
                             {
                                 if (ptr)
                                     ptr->set_exception(std::current_exception());
                             }
                         }
                         out = Task::Token::Discard;
                     }
                 }},
                dist
            );

            return task;
        }

        // Defaults

        template<typename Func, typename... Argv> auto launch(Func&& callback, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, Argv...>;
            return launchd(
                std::forward<Func>(callback), TaskConfig<future<ret>>(), std::forward<Argv>(args)...
            );
        }

        template<typename Func, typename... Argv> void launch_void(Func&& callback, Argv&&... args)
        {
            launchd_void(std::forward<Func>(callback), std::nullopt, std::forward<Argv>(args)...);
        }

        template<typename Func, typename... Argv>
        auto launch_cyclic(std::chrono::milliseconds time, Func&& callback, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, std::nullptr_t, Argv...>;
            return launchd_cyclic(
                time,
                std::forward<Func>(callback),
                TaskConfig<future<ret>>(),
                std::forward<Argv>(args)...
            );
        }

        template<typename Func, typename... Argv>
        auto launch_deferred(std::chrono::milliseconds time, Func&& callback, Argv&&... args)
        {
            using ret = std::invoke_result_t<Func, Argv...>;
            return launchd_deferred(
                time,
                std::forward<Func>(callback),
                TaskConfig<future<ret>>(),
                std::forward<Argv>(args)...
            );
        }

        template<typename Func, typename... Argv> auto co_launch(Func&& callback, Argv&&... args)
        {
            return co_launchd(
                std::forward<Func>(callback), std::nullopt, std::forward<Argv>(args)...
            );
        }

        ConcurrentPool& operator=(const ConcurrentPool&) = delete;
        ConcurrentPool& operator=(ConcurrentPool&&) = delete;
    };
} // namespace mt

#endif
