# `d2::dx::ScrollBox`

**Kind:** class
**Declared in:** `elements/d2_scrollbox.hpp:8`

**Bases:** `d2::dx::Box`

## Functions

### `_recompute_height`

```cpp
void d2::dx::ScrollBox::_recompute_height() const
```

Source: `elements/d2_scrollbox.hpp:16`

### `_signal_write_impl`

```cpp
void d2::dx::ScrollBox::_signal_write_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_scrollbox.hpp:18`

### `_provides_input_impl`

```cpp
bool d2::dx::ScrollBox::_provides_input_impl() const override
```

Source: `elements/d2_scrollbox.hpp:19`

### `_state_change_impl`

```cpp
void d2::dx::ScrollBox::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_scrollbox.hpp:20`

### `_event_impl`

```cpp
void d2::dx::ScrollBox::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_scrollbox.hpp:21`

### `_get_border_impl`

```cpp
int d2::dx::ScrollBox::_get_border_impl(BorderType type, Element::cptr elem) const override
```

Source: `elements/d2_scrollbox.hpp:23`

### `_layout_for_impl`

```cpp
void d2::dx::ScrollBox::_layout_for_impl(enum Element::Layout type, cptr ptr) const override
```

Source: `elements/d2_scrollbox.hpp:24`

### `_update_layout_impl`

```cpp
void d2::dx::ScrollBox::_update_layout_impl() override
```

Source: `elements/d2_scrollbox.hpp:25`

### `scrollbar`

```cpp
TreeIter< VerticalSlider > d2::dx::ScrollBox::scrollbar() const
```

Source: `elements/d2_scrollbox.hpp:29`

### `internal_height`

```cpp
int d2::dx::ScrollBox::internal_height() const
```

Source: `elements/d2_scrollbox.hpp:30`

### `window_height`

```cpp
int d2::dx::ScrollBox::window_height() const
```

Source: `elements/d2_scrollbox.hpp:31`

### `scroll_offset`

```cpp
int d2::dx::ScrollBox::scroll_offset() const noexcept
```

Source: `elements/d2_scrollbox.hpp:32`

### `scroll_to`

```cpp
void d2::dx::ScrollBox::scroll_to(int xoffset)
```

Source: `elements/d2_scrollbox.hpp:33`

### `sync`

```cpp
void d2::dx::ScrollBox::sync()
```

Source: `elements/d2_scrollbox.hpp:34`

### `foreach_internal`

```cpp
void d2::dx::ScrollBox::foreach_internal(foreach_internal_callback callback) const override
```

Source: `elements/d2_scrollbox.hpp:36`


## Variables

### `_scrollbar`

```cpp
std::shared_ptr<VerticalSlider> d2::dx::ScrollBox::_scrollbar
```

Source: `elements/d2_scrollbox.hpp:11`

### `offset_`

```cpp
int d2::dx::ScrollBox::offset_
```

Source: `elements/d2_scrollbox.hpp:12`

### `computed_height_`

```cpp
int d2::dx::ScrollBox::computed_height_
```

Source: `elements/d2_scrollbox.hpp:13`

### `needs_recompute_`

```cpp
bool d2::dx::ScrollBox::needs_recompute_
```

Source: `elements/d2_scrollbox.hpp:14`
