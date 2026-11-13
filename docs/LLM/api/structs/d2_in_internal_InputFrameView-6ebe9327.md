# `d2::in::internal::InputFrameView`

**Kind:** struct
**Declared in:** `core/io/d2_input_base.hpp:207`

## Functions

### `from`

```cpp
static auto d2::in::internal::InputFrameView::from(InputFrame *ptr)
```

Source: `core/io/d2_input_base.hpp:214`

### `InputFrameView`

```cpp
d2::in::internal::InputFrameView::InputFrameView(const InputFrameView &)=default
```

Source: `core/io/d2_input_base.hpp:219`

### `InputFrameView`

```cpp
d2::in::internal::InputFrameView::InputFrameView(InputFrameView &&)=default
```

Source: `core/io/d2_input_base.hpp:220`

### `swap`

```cpp
void d2::in::internal::InputFrameView::swap()
```

Source: `core/io/d2_input_base.hpp:222`

### `set`

```cpp
void d2::in::internal::InputFrameView::set(Mouse mouse, bool value=true)
```

Source: `core/io/d2_input_base.hpp:224`

### `set`

```cpp
void d2::in::internal::InputFrameView::set(Special mod, bool value=true)
```

Source: `core/io/d2_input_base.hpp:225`

### `set`

```cpp
void d2::in::internal::InputFrameView::set(keytype key, bool value=true)
```

Source: `core/io/d2_input_base.hpp:226`

### `pulse`

```cpp
void d2::in::internal::InputFrameView::pulse(Mouse mouse)
```

Source: `core/io/d2_input_base.hpp:228`

### `pulse`

```cpp
void d2::in::internal::InputFrameView::pulse(Special mod)
```

Source: `core/io/d2_input_base.hpp:229`

### `pulse`

```cpp
void d2::in::internal::InputFrameView::pulse(keytype key)
```

Source: `core/io/d2_input_base.hpp:230`

### `set_scroll_delta`

```cpp
void d2::in::internal::InputFrameView::set_scroll_delta(Position pos)
```

Source: `core/io/d2_input_base.hpp:232`

### `set_mouse_position`

```cpp
void d2::in::internal::InputFrameView::set_mouse_position(Position pos)
```

Source: `core/io/d2_input_base.hpp:233`

### `set_screen_size`

```cpp
void d2::in::internal::InputFrameView::set_screen_size(BoundingBox size)
```

Source: `core/io/d2_input_base.hpp:234`

### `set_screen_capacity`

```cpp
void d2::in::internal::InputFrameView::set_screen_capacity(BoundingBox size)
```

Source: `core/io/d2_input_base.hpp:235`

### `set_text`

```cpp
void d2::in::internal::InputFrameView::set_text(std::string in)
```

Source: `core/io/d2_input_base.hpp:236`

### `get_consume`

```cpp
std::pair< bool, std::thread::id > d2::in::internal::InputFrameView::get_consume()
```

Source: `core/io/d2_input_base.hpp:238`

### `restore_consume`

```cpp
void d2::in::internal::InputFrameView::restore_consume(std::pair< bool, std::thread::id > data)
```

Source: `core/io/d2_input_base.hpp:239`

### `set_consume`

```cpp
void d2::in::internal::InputFrameView::set_consume(bool value, std::thread::id ctx)
```

Source: `core/io/d2_input_base.hpp:241`

### `apply_consume`

```cpp
void d2::in::internal::InputFrameView::apply_consume()
```

Source: `core/io/d2_input_base.hpp:242`

### `reset_consume`

```cpp
void d2::in::internal::InputFrameView::reset_consume()
```

Source: `core/io/d2_input_base.hpp:243`

### `mask_key_consume`

```cpp
InputFrame::keymap d2::in::internal::InputFrameView::mask_key_consume(InputFrame::keymap mask)
```

Source: `core/io/d2_input_base.hpp:245`

### `operator=`

```cpp
InputFrameView & d2::in::internal::InputFrameView::operator=(const InputFrameView &)=default
```

Source: `core/io/d2_input_base.hpp:247`

### `operator=`

```cpp
InputFrameView & d2::in::internal::InputFrameView::operator=(InputFrameView &&)=default
```

Source: `core/io/d2_input_base.hpp:248`
