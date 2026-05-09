#include "d2_main_worker.hpp"

namespace d2
{
    void MainWorker::wait() noexcept
    {
        wait(std::chrono::steady_clock::time_point::max());
    }
} // namespace d2
