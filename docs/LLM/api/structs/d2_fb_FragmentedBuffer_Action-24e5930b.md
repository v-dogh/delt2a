# `d2::fb::FragmentedBuffer::Action`

**Kind:** struct
**Declared in:** `core/platform/d2_fragmented_buffer.hpp:114`

## Enums

### `Type`

```cpp
enum Type
```

Source: `core/platform/d2_fragmented_buffer.hpp:117`

Values: `Apply`, `Revert`, `Delete`, `Merge`


## Functions

### `from`

```cpp
template<typename... Argv>
static auto d2::fb::FragmentedBuffer::Action::from(bool(*callback)(FragmentedBuffer *, void *, void *, Type), Argv &&... args)
```

Source: `core/platform/d2_fragmented_buffer.hpp:130`

### `from`

```cpp
template<typename Type>
static auto d2::fb::FragmentedBuffer::Action::from(bool(*callback)(FragmentedBuffer *, void *, void *, Type), Type *ptr)
```

Source: `core/platform/d2_fragmented_buffer.hpp:135`

### `alloc_state`

```cpp
template<typename State>
static auto * d2::fb::FragmentedBuffer::Action::alloc_state(State value, std::size_t len=0)
```

Source: `core/platform/d2_fragmented_buffer.hpp:140`

### `alloc_state`

```cpp
template<typename State, typename Type>
static auto * d2::fb::FragmentedBuffer::Action::alloc_state(State value, std::span< Type > data)
```

Source: `core/platform/d2_fragmented_buffer.hpp:148`

### `get_state`

```cpp
template<typename State, typename Buffer = unsigned char>
static std::pair< State *, Buffer * > d2::fb::FragmentedBuffer::Action::get_state(void *ptr)
```

Source: `core/platform/d2_fragmented_buffer.hpp:161`

### `delete_state`

```cpp
template<typename State>
static void d2::fb::FragmentedBuffer::Action::delete_state(void *ptr)
```

Source: `core/platform/d2_fragmented_buffer.hpp:168`

### `Action`

```cpp
d2::fb::FragmentedBuffer::Action::Action(bool(*callback)(FragmentedBuffer *, void *, void *, Type), void *data=nullptr)
```

Source: `core/platform/d2_fragmented_buffer.hpp:174`

### `Action`

```cpp
d2::fb::FragmentedBuffer::Action::Action(const Action &)=delete
```

Source: `core/platform/d2_fragmented_buffer.hpp:175`

### `Action`

```cpp
d2::fb::FragmentedBuffer::Action::Action(Action &&copy)
```

Source: `core/platform/d2_fragmented_buffer.hpp:176`

### `~Action`

```cpp
d2::fb::FragmentedBuffer::Action::~Action()
```

Source: `core/platform/d2_fragmented_buffer.hpp:177`

### `apply`

```cpp
void d2::fb::FragmentedBuffer::Action::apply(FragmentedBuffer *buf)
```

Source: `core/platform/d2_fragmented_buffer.hpp:179`

### `revert`

```cpp
void d2::fb::FragmentedBuffer::Action::revert(FragmentedBuffer *buf)
```

Source: `core/platform/d2_fragmented_buffer.hpp:180`

### `merge`

```cpp
bool d2::fb::FragmentedBuffer::Action::merge(Action &action)
```

Source: `core/platform/d2_fragmented_buffer.hpp:182`

### `operator=`

```cpp
Action & d2::fb::FragmentedBuffer::Action::operator=(const Action &)=delete
```

Source: `core/platform/d2_fragmented_buffer.hpp:184`

### `operator=`

```cpp
FragmentedBuffer::Action & d2::fb::FragmentedBuffer::Action::operator=(Action &&)
```

Source: `core/platform/d2_fragmented_buffer.hpp:185`
