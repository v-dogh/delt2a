# `d2::in::InputFrame`

**Kind:** class
**Declared in:** `core/io/d2_input_base.hpp:109`

## Friends

### `internal::InputFrameView`

```cpp
friend class internal::InputFrameView
```

Source: `core/io/d2_input_base.hpp:175`


## Functions

### `make`

```cpp
static auto d2::in::InputFrame::make()
```

Source: `core/io/d2_input_base.hpp:170`

### `InputFrame`

```cpp
d2::in::InputFrame::InputFrame()=default
```

Source: `core/io/d2_input_base.hpp:177`

### `InputFrame`

```cpp
d2::in::InputFrame::InputFrame(const InputFrame &)=default
```

Source: `core/io/d2_input_base.hpp:178`

### `InputFrame`

```cpp
d2::in::InputFrame::InputFrame(InputFrame &&)=default
```

Source: `core/io/d2_input_base.hpp:179`

### `had_event`

```cpp
bool d2::in::InputFrame::had_event(Event ev) const
```

Source: `core/io/d2_input_base.hpp:181`

### `had_pulse`

```cpp
bool d2::in::InputFrame::had_pulse() const
```

Source: `core/io/d2_input_base.hpp:182`

### `scroll_delta`

```cpp
Position d2::in::InputFrame::scroll_delta()
```

Source: `core/io/d2_input_base.hpp:184`

### `mouse_position`

```cpp
Position d2::in::InputFrame::mouse_position()
```

Source: `core/io/d2_input_base.hpp:185`

### `screen_capacity`

```cpp
BoundingBox d2::in::InputFrame::screen_capacity()
```

Source: `core/io/d2_input_base.hpp:186`

### `screen_size`

```cpp
BoundingBox d2::in::InputFrame::screen_size()
```

Source: `core/io/d2_input_base.hpp:187`

### `active_list`

```cpp
absl::InlinedVector< std::pair< keytype, mode >, 8 > d2::in::InputFrame::active_list() const
```

Source: `core/io/d2_input_base.hpp:189`

### `active`

```cpp
bool d2::in::InputFrame::active(Mouse mouse, Mode mode)
```

Source: `core/io/d2_input_base.hpp:191`

### `active`

```cpp
bool d2::in::InputFrame::active(Special mod, Mode mode)
```

Source: `core/io/d2_input_base.hpp:192`

### `active`

```cpp
bool d2::in::InputFrame::active(keytype key, Mode mode)
```

Source: `core/io/d2_input_base.hpp:193`

### `map`

```cpp
InputFrame::ExportKeymap< InputFrame::keymap > d2::in::InputFrame::map() const noexcept
```

Source: `core/io/d2_input_base.hpp:195`

### `sequence`

```cpp
std::string_view d2::in::InputFrame::sequence()
```

Source: `core/io/d2_input_base.hpp:197`

### `mask`

```cpp
void d2::in::InputFrame::mask()
```

Source: `core/io/d2_input_base.hpp:199`

### `operator=`

```cpp
InputFrame & d2::in::InputFrame::operator=(const InputFrame &)=default
```

Source: `core/io/d2_input_base.hpp:201`

### `operator=`

```cpp
InputFrame & d2::in::InputFrame::operator=(InputFrame &&)=default
```

Source: `core/io/d2_input_base.hpp:202`


## Typedefs

### `keymap`

```cpp
using d2::in::InputFrame::keymap = std::bitset<std::size_t(Mouse::MouseKeyMax)>
```

Source: `core/io/d2_input_base.hpp:112`

### `ptr`

```cpp
using d2::in::InputFrame::ptr = std::shared_ptr<InputFrame>
```

Source: `core/io/d2_input_base.hpp:113`
