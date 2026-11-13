# `d2::dx::graph::NodeWaveGraph::NodePair`

**Kind:** struct
**Declared in:** `elements/d2_graph.hpp:99`

## Variables

### `id`

```cpp
std::size_t d2::dx::graph::NodeWaveGraph::NodePair::id
```

Source: `elements/d2_graph.hpp:101`

### `start`

```cpp
float d2::dx::graph::NodeWaveGraph::NodePair::start
```

Source: `elements/d2_graph.hpp:102`

### `end`

```cpp
float d2::dx::graph::NodeWaveGraph::NodePair::end
```

Source: `elements/d2_graph.hpp:103`

### `phase`

```cpp
float d2::dx::graph::NodeWaveGraph::NodePair::phase
```

Source: `elements/d2_graph.hpp:104`

### `frequency`

```cpp
float d2::dx::graph::NodeWaveGraph::NodePair::frequency
```

Source: `elements/d2_graph.hpp:105`

### `parameters`

```cpp
std::vector<float> d2::dx::graph::NodeWaveGraph::NodePair::parameters
```

Source: `elements/d2_graph.hpp:106`

### `interpolator`

```cpp
std::function<float(float, float, float, std::span<const float>)> d2::dx::graph::NodeWaveGraph::NodePair::interpolator
```

Source: `elements/d2_graph.hpp:107`
