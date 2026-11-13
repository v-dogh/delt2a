# `d2::dx::graph::NodeWaveGraph`

**Kind:** class
**Declared in:** `elements/d2_graph.hpp:87`

**Bases:** `d2::style:: UAI< NodeWaveGraph, d2::style::ILayout, d2::style::IColors, d2::style::INodeWaveGraph >`

## Functions

### `_state_change_impl`

```cpp
void d2::dx::graph::NodeWaveGraph::_state_change_impl(State state, bool value) override
```

Source: `elements/d2_graph.hpp:121`

### `_frame_impl`

```cpp
void d2::dx::graph::NodeWaveGraph::_frame_impl(d2::PixelBuffer::View buffer) override
```

Source: `elements/d2_graph.hpp:122`

### `push`

```cpp
std::size_t d2::dx::graph::NodeWaveGraph::push()
```

Source: `elements/d2_graph.hpp:126`

### `push`

```cpp
std::size_t d2::dx::graph::NodeWaveGraph::push(float start, float end)
```

Source: `elements/d2_graph.hpp:127`

### `pop`

```cpp
void d2::dx::graph::NodeWaveGraph::pop(std::size_t id)
```

Source: `elements/d2_graph.hpp:128`

### `set_parameters`

```cpp
void d2::dx::graph::NodeWaveGraph::set_parameters(std::size_t id, const NodePair &node)
```

Source: `elements/d2_graph.hpp:129`


## Typedefs

### `data`

```cpp
using d2::dx::graph::NodeWaveGraph::data = d2::style:: UAI<NodeWaveGraph, d2::style::ILayout, d2::style::IColors, d2::style::INodeWaveGraph>
```

Source: `elements/d2_graph.hpp:94`
