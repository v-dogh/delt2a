# `d2::dx::Slider`

**Kind:** class
**Declared in:** `elements/d2_slider.hpp:27`

**Bases:** `style:: UAI< Slider, style::ILayout, style::ISlider, style::IColors, style::IKeyboardNav, style::IResponsive< Slider, float, float >::type >`

## Functions

### `_submit`

```cpp
void d2::dx::Slider::_submit()
```

Source: `elements/d2_slider.hpp:47`

### `_provides_input_impl`

```cpp
bool d2::dx::Slider::_provides_input_impl() const override
```

Source: `elements/d2_slider.hpp:49`

### `_state_change_impl`

```cpp
void d2::dx::Slider::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_slider.hpp:50`

### `_event_impl`

```cpp
void d2::dx::Slider::_event_impl(in::InputFrame &frame) override
```

Source: `elements/d2_slider.hpp:51`

### `_frame_impl`

```cpp
void d2::dx::Slider::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_slider.hpp:53`

### `_aligned_mouse`

```cpp
int d2::dx::Slider::_aligned_mouse()
```

Source: `elements/d2_slider.hpp:55`

### `_aligned_width`

```cpp
int d2::dx::Slider::_aligned_width()
```

Source: `elements/d2_slider.hpp:56`

### `_relative_to_absolute`

```cpp
int d2::dx::Slider::_relative_to_absolute(float rel)
```

Source: `elements/d2_slider.hpp:58`

### `_absolute_to_relative`

```cpp
float d2::dx::Slider::_absolute_to_relative(int abs)
```

Source: `elements/d2_slider.hpp:59`

### `Slider`

```cpp
d2::dx::Slider::Slider(const std::string &name, TreeState::ptr state)
```

Source: `elements/d2_slider.hpp:61`

### `reset_absolute`

```cpp
void d2::dx::Slider::reset_absolute(float value=0)
```

Source: `elements/d2_slider.hpp:63`

### `reset_relative`

```cpp
void d2::dx::Slider::reset_relative(float value=0)
```

Source: `elements/d2_slider.hpp:64`

### `absolute_value`

```cpp
float d2::dx::Slider::absolute_value()
```

Source: `elements/d2_slider.hpp:66`

### `relative_value`

```cpp
float d2::dx::Slider::relative_value()
```

Source: `elements/d2_slider.hpp:67`


## Typedefs

### `data`

```cpp
using d2::dx::Slider::data = style:: UAI<Slider, style::ILayout, style::ISlider, style::IColors, style::IKeyboardNav, style::IResponsive<Slider, float, float>::type>
```

Source: `elements/d2_slider.hpp:36`


## Variables

### `_slider_spos`

```cpp
int d2::dx::Slider::_slider_spos
```

Source: `elements/d2_slider.hpp:44`

### `_slider_pos`

```cpp
float d2::dx::Slider::_slider_pos
```

Source: `elements/d2_slider.hpp:45`
