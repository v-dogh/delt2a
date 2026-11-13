# `d2::dx::graph::CyclicVerticalBar`

**Kind:** class
**Declared in:** `elements/d2_graph.hpp:56`

**Bases:** `style:: UAI< CyclicVerticalBar, style::ILayout, style::IContainer, style::IColors, style::IRangeGraph >`, `d2::dx::impl::ContainerHelper< CyclicVerticalBar >`

## Friends

### `ContainerHelper< CyclicVerticalBar >`

```cpp
friend class ContainerHelper< CyclicVerticalBar >
```

Source: `elements/d2_graph.hpp:46`


## Functions

### `_frame_impl`

```cpp
void d2::dx::graph::CyclicVerticalBar::_frame_impl(PixelBuffer::View buffer) override
```

Source: `elements/d2_graph.hpp:77`

### `_resolution`

```cpp
std::pair< int, int > d2::dx::graph::CyclicVerticalBar::_resolution() const
```

Source: `elements/d2_graph.hpp:79`

### `clear`

```cpp
void d2::dx::graph::CyclicVerticalBar::clear()
```

Source: `elements/d2_graph.hpp:81`

### `push`

```cpp
void d2::dx::graph::CyclicVerticalBar::push(float data_point)
```

Source: `elements/d2_graph.hpp:82`

### `rescale`

```cpp
void d2::dx::graph::CyclicVerticalBar::rescale(float scale)
```

Source: `elements/d2_graph.hpp:83`

### `rescale`

```cpp
void d2::dx::graph::CyclicVerticalBar::rescale(float scale, float trans)
```

Source: `elements/d2_graph.hpp:84`


## Typedefs

### `data`

```cpp
using d2::dx::graph::CyclicVerticalBar::data = style:: UAI<CyclicVerticalBar, style::ILayout, style::IContainer, style::IColors, style::IRangeGraph>
```

Source: `elements/d2_graph.hpp:66`


## Variables

### `_data`

```cpp
std::vector<float> d2::dx::graph::CyclicVerticalBar::_data
```

Source: `elements/d2_graph.hpp:74`

### `_start_offset`

```cpp
int d2::dx::graph::CyclicVerticalBar::_start_offset
```

Source: `elements/d2_graph.hpp:75`
