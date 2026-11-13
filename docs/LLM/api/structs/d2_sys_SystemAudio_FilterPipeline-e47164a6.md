# `d2::sys::SystemAudio::FilterPipeline`

**Kind:** struct
**Declared in:** `core/mods/d2_audio.hpp:127`

## Functions

### `FilterPipeline`

```cpp
d2::sys::SystemAudio::FilterPipeline::FilterPipeline()=default
```

Source: `core/mods/d2_audio.hpp:132`

### `operator|`

```cpp
SystemAudio::FilterPipeline d2::sys::SystemAudio::FilterPipeline::operator|(std::function< bool(Stream)> callback) const
```

Source: `core/mods/d2_audio.hpp:134`

### `operator|`

```cpp
SystemAudio::FilterPipeline d2::sys::SystemAudio::FilterPipeline::operator|(const std::string &name) const
```

Source: `core/mods/d2_audio.hpp:135`

### `operator&&`

```cpp
SystemAudio::FilterPipeline & d2::sys::SystemAudio::FilterPipeline::operator&&(std::function< bool(Stream)> callback)
```

Source: `core/mods/d2_audio.hpp:136`

### `operator|`

```cpp
SystemAudio::FilterPipeline & d2::sys::SystemAudio::FilterPipeline::operator|(const std::string &name)
```

Source: `core/mods/d2_audio.hpp:137`

### `operator|`

```cpp
SystemAudio::FilterPipeline & d2::sys::SystemAudio::FilterPipeline::operator|(std::function< bool(Stream)> callback)
```

Source: `core/mods/d2_audio.hpp:138`


## Variables

### `transforms`

```cpp
std::vector<std::pair<std::optional<std::string>, std::function<bool(Stream)> > > d2::sys::SystemAudio::FilterPipeline::transforms
```

Source: `core/mods/d2_audio.hpp:130`
