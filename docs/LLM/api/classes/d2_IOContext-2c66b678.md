# `d2::IOContext`

**Kind:** class
**Declared in:** `core/io/d2_context.hpp:21`

**Bases:** `d2::Signals`

## Functions

### `set`

```cpp
void d2::IOContext::set(ptr) noexcept
```

Source: `core/io/d2_context.hpp:112`

### `get`

```cpp
IOContext::ptr d2::IOContext::get() noexcept
```

Source: `core/io/d2_context.hpp:113`

### `make`

```cpp
template<typename... Modules>
static ptr d2::IOContext::make(rs::Config logs_cfg={})
```

Source: `core/io/d2_context.hpp:115`

### `IOContext`

```cpp
d2::IOContext::IOContext(mt::ConcurrentPool::ptr scheduler, rs::RuntimeLogs::ptr logs)
```

Source: `core/io/d2_context.hpp:144`

### `~IOContext`

```cpp
d2::IOContext::~IOContext()
```

Source: `core/io/d2_context.hpp:145`

### `run`

```cpp
template<typename Tree, typename... Trees>
void d2::IOContext::run(auto &&... themes)
```

Source: `core/io/d2_context.hpp:149`

### `stop`

```cpp
void d2::IOContext::stop()
```

Source: `core/io/d2_context.hpp:150`

### `ping`

```cpp
void d2::IOContext::ping()
```

Source: `core/io/d2_context.hpp:151`

### `deadline`

```cpp
std::chrono::steady_clock::time_point d2::IOContext::deadline(std::chrono::steady_clock::time_point tp)
```

Source: `core/io/d2_context.hpp:153`

### `deadline`

```cpp
std::chrono::steady_clock::time_point d2::IOContext::deadline(std::chrono::milliseconds ms)
```

Source: `core/io/d2_context.hpp:154`

### `is_suspended`

```cpp
bool d2::IOContext::is_suspended() const
```

Source: `core/io/d2_context.hpp:156`

### `suspend`

```cpp
void d2::IOContext::suspend(bool state)
```

Source: `core/io/d2_context.hpp:157`

### `scheduler`

```cpp
mt::ConcurrentPool::ptr d2::IOContext::scheduler()
```

Source: `core/io/d2_context.hpp:161`

### `is_synced`

```cpp
bool d2::IOContext::is_synced() const
```

Source: `core/io/d2_context.hpp:163`

### `sync`

```cpp
template<typename Func, typename... Argv>
std::invoke_result_t< Func &, Argv... > d2::IOContext::sync(Func &&callback, Argv &&... args)
```

Source: `core/io/d2_context.hpp:166`

### `sync_async`

```cpp
template<typename Func, typename... Argv>
mt::future< std::invoke_result_t< Func &, Argv... > > d2::IOContext::sync_async(Func &&callback, Argv &&... args)
```

Source: `core/io/d2_context.hpp:174`

### `launch_thread`

```cpp
template<typename Func, typename... Argv>
std::jthread d2::IOContext::launch_thread(Func &&callback, Argv &&... args)
```

Source: `core/io/d2_context.hpp:222`

### `launch_thread`

```cpp
void d2::IOContext::launch_thread()
```

Source: `core/io/d2_context.hpp:256`

### `unload`

```cpp
template<typename Module>
void d2::IOContext::unload()
```

Source: `core/io/d2_context.hpp:260`

### `unload`

```cpp
template<typename Module>
void d2::IOContext::unload(const std::string &id)
```

Source: `core/io/d2_context.hpp:267`

### `load`

```cpp
template<typename... Modules>
void d2::IOContext::load()
```

Source: `core/io/d2_context.hpp:274`

### `load`

```cpp
template<typename Module>
void d2::IOContext::load(const std::string &id)
```

Source: `core/io/d2_context.hpp:293`

### `sysenum`

```cpp
void d2::IOContext::sysenum(std::function< void(sys::SystemModule::ModInfo, sys::SystemModule::ModPreset, std::optional< sys::module< sys::SystemModule > >, std::optional< std::string >)> callback)
```

Source: `core/io/d2_context.hpp:299`

### `sysenum`

```cpp
template<typename Module>
void d2::IOContext::sysenum(std::function< void(sys::SystemModule::ModInfo, sys::SystemModule::ModPreset, std::optional< sys::module< sys::SystemModule > >, std::optional< std::string >)> callback)
```

Source: `core/io/d2_context.hpp:308`

### `sys`

```cpp
template<typename Module>
module< Module > d2::IOContext::sys()
```

Source: `core/io/d2_context.hpp:320`

### `sys_if`

```cpp
template<typename Module>
module< Module > d2::IOContext::sys_if()
```

Source: `core/io/d2_context.hpp:324`

### `sys`

```cpp
template<typename Module>
module< Module > d2::IOContext::sys(const std::string &id)
```

Source: `core/io/d2_context.hpp:332`

### `sys_if`

```cpp
template<typename Module>
module< Module > d2::IOContext::sys_if(const std::string &id)
```

Source: `core/io/d2_context.hpp:336`

### `input`

```cpp
IOContext::module< sys::input > d2::IOContext::input()
```

Source: `core/io/d2_context.hpp:344`

### `output`

```cpp
IOContext::module< sys::output > d2::IOContext::output()
```

Source: `core/io/d2_context.hpp:345`

### `screen`

```cpp
IOContext::module< sys::screen > d2::IOContext::screen()
```

Source: `core/io/d2_context.hpp:346`


## Typedefs

### `module`

```cpp
template<typename Type>
using d2::IOContext::module = sys::module<Type>
```

Source: `core/io/d2_context.hpp:25`

### `future`

```cpp
template<typename Type>
using d2::IOContext::future = mt::future<Type>
```

Source: `core/io/d2_context.hpp:26`

### `ptr`

```cpp
using d2::IOContext::ptr = std::shared_ptr<IOContext>
```

Source: `core/io/d2_context.hpp:27`

### `wptr`

```cpp
using d2::IOContext::wptr = std::weak_ptr<IOContext>
```

Source: `core/io/d2_context.hpp:28`
