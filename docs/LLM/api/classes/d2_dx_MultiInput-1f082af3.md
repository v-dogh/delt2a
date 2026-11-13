# `d2::dx::MultiInput`

**Kind:** class
**Declared in:** `elements/d2_multiline_input.hpp:40`

**Bases:** `d2::style::UniversalAccessInterfaceProto< Elem, Base, Interface, Interfaces... >`

## Functions

### `_write_ptr`

```cpp
void d2::dx::MultiInput::_write_ptr(pixel &px)
```

Source: `elements/d2_multiline_input.hpp:100`

### `_doc_dirty`

```cpp
void d2::dx::MultiInput::_doc_dirty() const
```

Source: `elements/d2_multiline_input.hpp:102`

### `_doc_model`

```cpp
const MultiInput::DocModel & d2::dx::MultiInput::_doc_model() const
```

Source: `elements/d2_multiline_input.hpp:103`

### `_line_count`

```cpp
std::size_t d2::dx::MultiInput::_line_count(const DocModel &doc) const
```

Source: `elements/d2_multiline_input.hpp:104`

### `_line_length`

```cpp
std::size_t d2::dx::MultiInput::_line_length(const DocModel &doc, std::size_t line) const
```

Source: `elements/d2_multiline_input.hpp:105`

### `_line_col_from_abs`

```cpp
std::pair< std::size_t, std::size_t > d2::dx::MultiInput::_line_col_from_abs(const DocModel &doc, std::size_t abs) const
```

Source: `elements/d2_multiline_input.hpp:107`

### `_abs_from_line_col`

```cpp
std::size_t d2::dx::MultiInput::_abs_from_line_col(const DocModel &doc, std::size_t line, std::size_t col) const
```

Source: `elements/d2_multiline_input.hpp:109`

### `_range_value`

```cpp
string d2::dx::MultiInput::_range_value(std::size_t pos, std::size_t len) const
```

Source: `elements/d2_multiline_input.hpp:110`

### `_replace`

```cpp
void d2::dx::MultiInput::_replace(std::size_t begin, std::size_t end, std::string_view value, std::optional< std::size_t > cursor_after={})
```

Source: `elements/d2_multiline_input.hpp:111`

### `_ptr_overlapped`

```cpp
bool d2::dx::MultiInput::_ptr_overlapped() const
```

Source: `elements/d2_multiline_input.hpp:114`

### `_draw_ptr`

```cpp
bool d2::dx::MultiInput::_draw_ptr() const
```

Source: `elements/d2_multiline_input.hpp:115`

### `_render_max_width`

```cpp
std::size_t d2::dx::MultiInput::_render_max_width(const DocModel &doc) const
```

Source: `elements/d2_multiline_input.hpp:116`

### `_fixed_width`

```cpp
bool d2::dx::MultiInput::_fixed_width() const
```

Source: `elements/d2_multiline_input.hpp:118`

### `_fixed_height`

```cpp
bool d2::dx::MultiInput::_fixed_height() const
```

Source: `elements/d2_multiline_input.hpp:119`

### `_vscroll_width`

```cpp
int d2::dx::MultiInput::_vscroll_width() const
```

Source: `elements/d2_multiline_input.hpp:120`

### `_hscroll_height`

```cpp
int d2::dx::MultiInput::_hscroll_height() const
```

Source: `elements/d2_multiline_input.hpp:121`

### `_viewport`

```cpp
MultiInput::Viewport d2::dx::MultiInput::_viewport(const DocModel &doc) const
```

Source: `elements/d2_multiline_input.hpp:122`

### `_sync_scrollbars`

```cpp
void d2::dx::MultiInput::_sync_scrollbars()
```

Source: `elements/d2_multiline_input.hpp:124`

### `_scroll_x_to`

```cpp
void d2::dx::MultiInput::_scroll_x_to(std::size_t off)
```

Source: `elements/d2_multiline_input.hpp:125`

### `_scroll_y_to`

```cpp
void d2::dx::MultiInput::_scroll_y_to(std::size_t off)
```

Source: `elements/d2_multiline_input.hpp:126`

### `_clamp_ptr`

```cpp
void d2::dx::MultiInput::_clamp_ptr()
```

Source: `elements/d2_multiline_input.hpp:128`

### `_sync_ptr`

```cpp
void d2::dx::MultiInput::_sync_ptr()
```

Source: `elements/d2_multiline_input.hpp:129`

### `_ensure_ptr_visible`

```cpp
void d2::dx::MultiInput::_ensure_ptr_visible()
```

Source: `elements/d2_multiline_input.hpp:130`

### `_is_highlighted`

```cpp
bool d2::dx::MultiInput::_is_highlighted() const
```

Source: `elements/d2_multiline_input.hpp:132`

### `_remove_highlighted`

```cpp
void d2::dx::MultiInput::_remove_highlighted()
```

Source: `elements/d2_multiline_input.hpp:133`

### `_begin_highlight_if`

```cpp
void d2::dx::MultiInput::_begin_highlight_if()
```

Source: `elements/d2_multiline_input.hpp:134`

### `_reset_highlight`

```cpp
void d2::dx::MultiInput::_reset_highlight()
```

Source: `elements/d2_multiline_input.hpp:135`

### `_highlighted_value`

```cpp
string d2::dx::MultiInput::_highlighted_value() const
```

Source: `elements/d2_multiline_input.hpp:136`

### `_start_blink`

```cpp
void d2::dx::MultiInput::_start_blink()
```

Source: `elements/d2_multiline_input.hpp:138`

### `_stop_blink`

```cpp
void d2::dx::MultiInput::_stop_blink()
```

Source: `elements/d2_multiline_input.hpp:139`

### `_freeze_blink`

```cpp
void d2::dx::MultiInput::_freeze_blink()
```

Source: `elements/d2_multiline_input.hpp:140`

### `_unfreeze_blink`

```cpp
void d2::dx::MultiInput::_unfreeze_blink()
```

Source: `elements/d2_multiline_input.hpp:141`

### `_mouse_abs_pos`

```cpp
std::size_t d2::dx::MultiInput::_mouse_abs_pos() const
```

Source: `elements/d2_multiline_input.hpp:143`

### `_move_to`

```cpp
void d2::dx::MultiInput::_move_to(std::size_t pos, bool keep_highlight=false)
```

Source: `elements/d2_multiline_input.hpp:145`

### `_move_right`

```cpp
void d2::dx::MultiInput::_move_right(std::size_t cnt=1)
```

Source: `elements/d2_multiline_input.hpp:146`

### `_move_left`

```cpp
void d2::dx::MultiInput::_move_left(std::size_t cnt=1)
```

Source: `elements/d2_multiline_input.hpp:147`

### `_move_up`

```cpp
void d2::dx::MultiInput::_move_up(std::size_t cnt=1, bool select=false)
```

Source: `elements/d2_multiline_input.hpp:148`

### `_move_down`

```cpp
void d2::dx::MultiInput::_move_down(std::size_t cnt=1, bool select=false)
```

Source: `elements/d2_multiline_input.hpp:149`

### `_insert`

```cpp
void d2::dx::MultiInput::_insert(std::string_view value)
```

Source: `elements/d2_multiline_input.hpp:151`

### `_erase_before`

```cpp
void d2::dx::MultiInput::_erase_before(std::size_t len=1)
```

Source: `elements/d2_multiline_input.hpp:152`

### `_erase_after`

```cpp
void d2::dx::MultiInput::_erase_after(std::size_t len=1)
```

Source: `elements/d2_multiline_input.hpp:153`

### `_signal_write_impl`

```cpp
void d2::dx::MultiInput::_signal_write_impl(write_flag type, unsigned int prop, ptr element) override
```

Source: `elements/d2_multiline_input.hpp:156`

### `_layout_impl`

```cpp
Unit d2::dx::MultiInput::_layout_impl(Element::Layout type) const override
```

Source: `elements/d2_multiline_input.hpp:157`

### `_provides_input_impl`

```cpp
bool d2::dx::MultiInput::_provides_input_impl() const override
```

Source: `elements/d2_multiline_input.hpp:158`

### `_state_change_impl`

```cpp
void d2::dx::MultiInput::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_multiline_input.hpp:159`

### `_event_impl`

```cpp
void d2::dx::MultiInput::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_multiline_input.hpp:160`

### `_frame_impl`

```cpp
void d2::dx::MultiInput::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_multiline_input.hpp:161`

### `_update_layout_impl`

```cpp
void d2::dx::MultiInput::_update_layout_impl() override
```

Source: `elements/d2_multiline_input.hpp:162`

### `foreach_internal`

```cpp
void d2::dx::MultiInput::foreach_internal(foreach_internal_callback callback) const override
```

Source: `elements/d2_multiline_input.hpp:163`

### `~MultiInput`

```cpp
d2::dx::MultiInput::~MultiInput() override
```

Source: `elements/d2_multiline_input.hpp:165`

### `value`

```cpp
string d2::dx::MultiInput::value() const
```

Source: `elements/d2_multiline_input.hpp:166`

### `value`

```cpp
void d2::dx::MultiInput::value(std::string text)
```

Source: `elements/d2_multiline_input.hpp:167`

### `clear`

```cpp
void d2::dx::MultiInput::clear()
```

Source: `elements/d2_multiline_input.hpp:169`

### `undo`

```cpp
void d2::dx::MultiInput::undo()
```

Source: `elements/d2_multiline_input.hpp:171`

### `redo`

```cpp
void d2::dx::MultiInput::redo()
```

Source: `elements/d2_multiline_input.hpp:172`

### `is_synced`

```cpp
bool d2::dx::MultiInput::is_synced() const
```

Source: `elements/d2_multiline_input.hpp:174`

### `sync`

```cpp
void d2::dx::MultiInput::sync()
```

Source: `elements/d2_multiline_input.hpp:175`

### `jump`

```cpp
void d2::dx::MultiInput::jump(std::size_t pos)
```

Source: `elements/d2_multiline_input.hpp:177`

### `insert`

```cpp
void d2::dx::MultiInput::insert(std::string_view text)
```

Source: `elements/d2_multiline_input.hpp:178`

### `append`

```cpp
void d2::dx::MultiInput::append(std::string_view text)
```

Source: `elements/d2_multiline_input.hpp:179`

### `erase`

```cpp
void d2::dx::MultiInput::erase(std::size_t len=1)
```

Source: `elements/d2_multiline_input.hpp:180`

### `erase_at`

```cpp
void d2::dx::MultiInput::erase_at(std::size_t pos, std::size_t len=1)
```

Source: `elements/d2_multiline_input.hpp:181`

### `disable_history`

```cpp
void d2::dx::MultiInput::disable_history()
```

Source: `elements/d2_multiline_input.hpp:183`

### `enable_history`

```cpp
void d2::dx::MultiInput::enable_history()
```

Source: `elements/d2_multiline_input.hpp:184`

### `cursor`

```cpp
std::size_t d2::dx::MultiInput::cursor() const
```

Source: `elements/d2_multiline_input.hpp:186`

### `vertical_scrollbar`

```cpp
d2::TreeIter< VerticalSlider > d2::dx::MultiInput::vertical_scrollbar() const noexcept
```

Source: `elements/d2_multiline_input.hpp:188`

### `vertical_scollbar`

```cpp
d2::TreeIter< VerticalSlider > d2::dx::MultiInput::vertical_scollbar() const noexcept
```

Source: `elements/d2_multiline_input.hpp:189`

### `horizontal_scrollbar`

```cpp
d2::TreeIter< Slider > d2::dx::MultiInput::horizontal_scrollbar() const noexcept
```

Source: `elements/d2_multiline_input.hpp:190`

### `buffer`

```cpp
const fb::FragmentedBuffer & d2::dx::MultiInput::buffer() const noexcept
```

Source: `elements/d2_multiline_input.hpp:192`


## Typedefs

### `data`

```cpp
using d2::dx::MultiInput::data = style::UAIE< MetaParentElement, MultiInput, style::ILayout, style::IColors, style::IMultiInput, style::IInput, style::ISelectable, style::IResponsive<Input, string>::type >
```

Source: `elements/d2_multiline_input.hpp:52`


## Variables

### `_btask`

```cpp
IOContext::future<void> d2::dx::MultiInput::_btask
```

Source: `elements/d2_multiline_input.hpp:78`

### `_buffer`

```cpp
fb::FragmentedBuffer d2::dx::MultiInput::_buffer
```

Source: `elements/d2_multiline_input.hpp:79`

### `_dm_cache`

```cpp
DocModel d2::dx::MultiInput::_dm_cache
```

Source: `elements/d2_multiline_input.hpp:80`

### `_vscrollbar`

```cpp
std::shared_ptr<VerticalSlider> d2::dx::MultiInput::_vscrollbar
```

Source: `elements/d2_multiline_input.hpp:82`

### `_hscrollbar`

```cpp
std::shared_ptr<Slider> d2::dx::MultiInput::_hscrollbar
```

Source: `elements/d2_multiline_input.hpp:83`

### `_syncing_vscroll_`

```cpp
bool d2::dx::MultiInput::_syncing_vscroll_
```

Source: `elements/d2_multiline_input.hpp:85`

### `_syncing_hscroll_`

```cpp
bool d2::dx::MultiInput::_syncing_hscroll_
```

Source: `elements/d2_multiline_input.hpp:86`

### `_ptr_abs`

```cpp
std::size_t d2::dx::MultiInput::_ptr_abs
```

Source: `elements/d2_multiline_input.hpp:88`

### `_preferred_x`

```cpp
std::size_t d2::dx::MultiInput::_preferred_x
```

Source: `elements/d2_multiline_input.hpp:89`

### `_view_x`

```cpp
std::size_t d2::dx::MultiInput::_view_x
```

Source: `elements/d2_multiline_input.hpp:91`

### `_view_y`

```cpp
std::size_t d2::dx::MultiInput::_view_y
```

Source: `elements/d2_multiline_input.hpp:92`

### `_highlight_start`

```cpp
std::size_t d2::dx::MultiInput::_highlight_start
```

Source: `elements/d2_multiline_input.hpp:94`

### `_highlight_end`

```cpp
std::size_t d2::dx::MultiInput::_highlight_end
```

Source: `elements/d2_multiline_input.hpp:95`

### `_ptr_blink`

```cpp
bool d2::dx::MultiInput::_ptr_blink
```

Source: `elements/d2_multiline_input.hpp:97`

### `_dirty_dm`

```cpp
bool d2::dx::MultiInput::_dirty_dm
```

Source: `elements/d2_multiline_input.hpp:98`
