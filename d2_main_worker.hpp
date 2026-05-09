#ifndef D2_MAIN_WORKER_HPP
#define D2_MAIN_WORKER_HPP

#include <mt/pool.hpp>

namespace d2
{
    class MainWorker : public mt::Worker
    {
    public:
        using ptr = mt::Worker::vptr<MainWorker>;
        MainWorker() : Worker(mt::Task::Periodic | mt::Task::Delayed) {}
        void wait() noexcept;
        virtual void wait(std::chrono::steady_clock::time_point deadline) noexcept = 0;
    };
} // namespace d2

#endif
