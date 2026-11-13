# `d2::dx::Input`

**Kind:** class
**Declared in:** `elements/d2_input.hpp:48`

**Bases:** `style:: UAI< Input, style::ILayout, style::IColors, style::IText, style::IInput, style::ISelectable, style::IResponsive< Input, string >::type >`

## Functions

### `_layout`

```cpp
const text::Layout & d2::dx::Input::_layout() const
```

Source: `elements/d2_input.hpp:76`

### `_prefix_width`

```cpp
int d2::dx::Input::_prefix_width() const
```

Source: `elements/d2_input.hpp:77`

### `_ensure_ptr_visible`

```cpp
void d2::dx::Input::_ensure_ptr_visible()
```

Source: `elements/d2_input.hpp:78`

### `_hit`

```cpp
std::size_t d2::dx::Input::_hit(Position position) const
```

Source: `elements/d2_input.hpp:79`

### `_move_to`

```cpp
void d2::dx::Input::_move_to(std::size_t pos, bool select=false)
```

Source: `elements/d2_input.hpp:80`

### `_insert`

```cpp
void d2::dx::Input::_insert(std::string_view value)
```

Source: `elements/d2_input.hpp:81`

### `_erase`

```cpp
void d2::dx::Input::_erase(std::size_t begin, std::size_t end)
```

Source: `elements/d2_input.hpp:82`

### `_write_ptr`

```cpp
void d2::dx::Input::_write_ptr(pixel &px)
```

Source: `elements/d2_input.hpp:83`

### `_is_highlighted`

```cpp
bool d2::dx::Input::_is_highlighted()
```

Source: `elements/d2_input.hpp:84`

### `_remove_highlighted`

```cpp
void d2::dx::Input::_remove_highlighted()
```

Source: `elements/d2_input.hpp:85`

### `_begin_highlight_if`

```cpp
void d2::dx::Input::_begin_highlight_if()
```

Source: `elements/d2_input.hpp:86`

### `_reset_highlight`

```cpp
void d2::dx::Input::_reset_highlight()
```

Source: `elements/d2_input.hpp:87`

### `_reset_ptrs`

```cpp
void d2::dx::Input::_reset_ptrs()
```

Source: `elements/d2_input.hpp:88`

### `_highlighted_value`

```cpp
string d2::dx::Input::_highlighted_value()
```

Source: `elements/d2_input.hpp:89`

### `_start_blink`

```cpp
void d2::dx::Input::_start_blink()
```

Source: `elements/d2_input.hpp:91`

### `_stop_blink`

```cpp
void d2::dx::Input::_stop_blink()
```

Source: `elements/d2_input.hpp:92`

### `_freeze_blink`

```cpp
void d2::dx::Input::_freeze_blink()
```

Source: `elements/d2_input.hpp:93`

### `_unfreeze_blink`

```cpp
void d2::dx::Input::_unfreeze_blink()
```

Source: `elements/d2_input.hpp:94`

### `_move_right`

```cpp
void d2::dx::Input::_move_right(std::size_t cnt=1)
```

Source: `elements/d2_input.hpp:96`

### `_move_left`

```cpp
void d2::dx::Input::_move_left(std::size_t cnt=1)
```

Source: `elements/d2_input.hpp:97`

### `_layout_impl`

```cpp
Unit d2::dx::Input::_layout_impl(Element::Layout type) const override
```

Source: `elements/d2_input.hpp:99`

### `_provides_input_impl`

```cpp
bool d2::dx::Input::_provides_input_impl() const override
```

Source: `elements/d2_input.hpp:100`

### `_state_change_impl`

```cpp
void d2::dx::Input::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_input.hpp:101`

### `_event_impl`

```cpp
void d2::dx::Input::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_input.hpp:102`

### `_frame_impl`

```cpp
void d2::dx::Input::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_input.hpp:103`

### `~Input`

```cpp
d2::dx::Input::~Input()
```

Source: `elements/d2_input.hpp:105`

### `jump`

```cpp
void d2::dx::Input::jump(std::size_t pos)
```

Source: `elements/d2_input.hpp:107`

### `sync`

```cpp
void d2::dx::Input::sync()
```

Source: `elements/d2_input.hpp:108`

### `cursor`

```cpp
std::size_t d2::dx::Input::cursor() const
```

Source: `elements/d2_input.hpp:109`


## Typedefs

### `data`

```cpp
using d2::dx::Input::data = style:: UAI<Input, style::ILayout, style::IColors, style::IText, style::IInput, style::ISelectable, style::IResponsive<Input, string>::type>
```

Source: `elements/d2_input.hpp:58`


## Variables

### `btask_`

```cpp
IOContext::future<void> d2::dx::Input::btask_
```

Source: `elements/d2_input.hpp:68`

### `ptr_`

```cpp
std::size_t d2::dx::Input::ptr_
```

Source: `elements/d2_input.hpp:69`

### `bound_lower_`

```cpp
int d2::dx::Input::bound_lower_
```

Source: `elements/d2_input.hpp:70`

### `highlight_start_`

```cpp
std::size_t d2::dx::Input::highlight_start_
```

Source: `elements/d2_input.hpp:71`

### `highlight_end_`

```cpp
std::size_t d2::dx::Input::highlight_end_
```

Source: `elements/d2_input.hpp:72`

### `ptr_blink_`

```cpp
bool d2::dx::Input::ptr_blink_
```

Source: `elements/d2_input.hpp:73`

### `_text`

```cpp
detail::InputLayout d2::dx::Input::_text
```

Source: `elements/d2_input.hpp:75`
