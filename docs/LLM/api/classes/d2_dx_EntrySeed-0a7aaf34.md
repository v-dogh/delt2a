# `d2::dx::EntrySeed`

**Kind:** class
**Declared in:** `elements/d2_demand_scrollbox.hpp:11`

## Friends

### `DemandScrollBox`

```cpp
friend class DemandScrollBox
```

Source: `elements/d2_demand_scrollbox.hpp:22`


## Functions

### `EntrySeed`

```cpp
d2::dx::EntrySeed::EntrySeed(const EntrySeed &)=delete
```

Source: `elements/d2_demand_scrollbox.hpp:24`

### `EntrySeed`

```cpp
d2::dx::EntrySeed::EntrySeed(EntrySeed &&)=delete
```

Source: `elements/d2_demand_scrollbox.hpp:25`

### `EntrySeed`

```cpp
d2::dx::EntrySeed::EntrySeed(std::size_t idx, d2::TreeIter< DemandScrollBox > ptr)
```

Source: `elements/d2_demand_scrollbox.hpp:26`

### `create`

```cpp
template<typename Element, typename... Argv>
d2::TreeIter< Element > d2::dx::EntrySeed::create(Argv &&... args)
```

Source: `elements/d2_demand_scrollbox.hpp:30`

### `index`

```cpp
std::size_t d2::dx::EntrySeed::index() const noexcept
```

Source: `elements/d2_demand_scrollbox.hpp:45`

### `parent`

```cpp
d2::TreeIter< DemandScrollBox > d2::dx::EntrySeed::parent() const noexcept
```

Source: `elements/d2_demand_scrollbox.hpp:46`

### `operator=`

```cpp
EntrySeed & d2::dx::EntrySeed::operator=(const EntrySeed &)=delete
```

Source: `elements/d2_demand_scrollbox.hpp:48`

### `operator=`

```cpp
EntrySeed & d2::dx::EntrySeed::operator=(EntrySeed &&)=delete
```

Source: `elements/d2_demand_scrollbox.hpp:49`
