# `d2::MainWorker`

**Kind:** class
**Declared in:** `core/io/d2_main_worker.hpp:7`

**Bases:** `mt::Worker`

## Functions

### `MainWorker`

```cpp
d2::MainWorker::MainWorker()
```

Source: `core/io/d2_main_worker.hpp:11`

### `wait`

```cpp
void d2::MainWorker::wait() noexcept
```

Source: `core/io/d2_main_worker.hpp:12`

### `wait`

```cpp
virtual void d2::MainWorker::wait(std::chrono::steady_clock::time_point deadline) noexcept=0
```

Source: `core/io/d2_main_worker.hpp:13`


## Typedefs

### `ptr`

```cpp
using d2::MainWorker::ptr = mt::Worker::vptr<MainWorker>
```

Source: `core/io/d2_main_worker.hpp:10`
