# `d2::sys::SystemAudio`

**Kind:** class
**Declared in:** `core/mods/d2_audio.hpp:35`

**Bases:** `d2::sys::ModuleDecl< SystemAudio, "Audio", Access::TSafe >`

## Enums

### `Event`

```cpp
enum Event
```

Source: `core/mods/d2_audio.hpp:298`

Values: `Activate`, `Deactivate`, `Create`, `Destroy`, `DefaultChange`

### `Device`

```cpp
enum Device
```

Source: `core/mods/d2_audio.hpp:306`

Values: `Input`, `Output`


## Functions

### `_stream_normalize_required`

```cpp
std::size_t d2::sys::SystemAudio::_stream_normalize_required(std::span< const unsigned char > data, FormatInfo format)
```

Source: `core/mods/d2_audio.hpp:331`

### `_stream_normalize`

```cpp
void d2::sys::SystemAudio::_stream_normalize(std::span< float > out, std::span< const unsigned char > data, FormatInfo format)
```

Source: `core/mods/d2_audio.hpp:332`

### `_set_device_status`

```cpp
void d2::sys::SystemAudio::_set_device_status(Device dev, DeviceData::Status status)
```

Source: `core/mods/d2_audio.hpp:335`

### `_get_device_status`

```cpp
SystemAudio::DeviceData::Status d2::sys::SystemAudio::_get_device_status(Device dev)
```

Source: `core/mods/d2_audio.hpp:336`

### `_run_device_filter`

```cpp
void d2::sys::SystemAudio::_run_device_filter(Device dev, Stream &stream)
```

Source: `core/mods/d2_audio.hpp:337`

### `_run_filter`

```cpp
void d2::sys::SystemAudio::_run_filter(Stream &stream, FilterPipeline &pipeline)
```

Source: `core/mods/d2_audio.hpp:338`

### `_trigger_device_event`

```cpp
void d2::sys::SystemAudio::_trigger_device_event(Device dev, Event ev, const DeviceName &)
```

Source: `core/mods/d2_audio.hpp:339`

### `_filter`

```cpp
void d2::sys::SystemAudio::_filter(Device dev, FilterPipeline filter)
```

Source: `core/mods/d2_audio.hpp:345`

### `_filter_push`

```cpp
void d2::sys::SystemAudio::_filter_push(Device dev, const std::string &after, FilterPipeline filter)
```

Source: `core/mods/d2_audio.hpp:346`

### `_filter_push`

```cpp
void d2::sys::SystemAudio::_filter_push(Device dev, FilterPipeline filter)
```

Source: `core/mods/d2_audio.hpp:347`

### `_filter_override`

```cpp
void d2::sys::SystemAudio::_filter_override(Device dev, const std::string &name, std::function< bool(Stream)> filter)
```

Source: `core/mods/d2_audio.hpp:349`

### `_filter_remove`

```cpp
void d2::sys::SystemAudio::_filter_remove(Device dev, const std::string &name)
```

Source: `core/mods/d2_audio.hpp:350`

### `_filter_clear`

```cpp
void d2::sys::SystemAudio::_filter_clear(Device dev)
```

Source: `core/mods/d2_audio.hpp:351`

### `_load_impl`

```cpp
SystemAudio::Status d2::sys::SystemAudio::_load_impl() override
```

Source: `core/mods/d2_audio.hpp:353`

### `_unload_impl`

```cpp
SystemAudio::Status d2::sys::SystemAudio::_unload_impl() override
```

Source: `core/mods/d2_audio.hpp:354`

### `sample_size`

```cpp
static constexpr std::size_t d2::sys::SystemAudio::sample_size(Format format)
```

Source: `core/mods/d2_audio.hpp:356`

### `flush`

```cpp
virtual void d2::sys::SystemAudio::flush(Device dev)=0
```

Source: `core/mods/d2_audio.hpp:373`

### `record`

```cpp
virtual void d2::sys::SystemAudio::record(std::function< void(std::span< const unsigned char >)> stream)=0
```

Source: `core/mods/d2_audio.hpp:374`

### `transmit`

```cpp
virtual void d2::sys::SystemAudio::transmit(std::span< const unsigned char > stream, FormatInfo format, FilterPipeline filter={})=0
```

Source: `core/mods/d2_audio.hpp:375`

### `transmit`

```cpp
virtual void d2::sys::SystemAudio::transmit(std::vector< unsigned char > stream, FormatInfo format, FilterPipeline filter={})=0
```

Source: `core/mods/d2_audio.hpp:378`

### `transmit`

```cpp
virtual void d2::sys::SystemAudio::transmit(std::chrono::seconds time, FilterPipeline filter={})=0
```

Source: `core/mods/d2_audio.hpp:381`

### `deactivate`

```cpp
virtual void d2::sys::SystemAudio::deactivate(Device dev)=0
```

Source: `core/mods/d2_audio.hpp:382`

### `activate`

```cpp
virtual void d2::sys::SystemAudio::activate(Device dev, const std::string &name="", DeviceConfig cfg={})=0
```

Source: `core/mods/d2_audio.hpp:383`

### `reactivate`

```cpp
virtual void d2::sys::SystemAudio::reactivate(Device dev, DeviceConfig cfg={})=0
```

Source: `core/mods/d2_audio.hpp:384`

### `active`

```cpp
virtual DeviceName d2::sys::SystemAudio::active(Device dev)=0
```

Source: `core/mods/d2_audio.hpp:385`

### `default_device`

```cpp
virtual DeviceName d2::sys::SystemAudio::default_device(Device dev)=0
```

Source: `core/mods/d2_audio.hpp:386`

### `format`

```cpp
virtual FormatInfo d2::sys::SystemAudio::format(Device dev)=0
```

Source: `core/mods/d2_audio.hpp:387`

### `enumerate`

```cpp
virtual std::vector< DeviceName > d2::sys::SystemAudio::enumerate(Device dev)=0
```

Source: `core/mods/d2_audio.hpp:388`

### `filter`

```cpp
virtual void d2::sys::SystemAudio::filter(Device dev, FilterPipeline filter)=0
```

Source: `core/mods/d2_audio.hpp:390`

### `filter_push`

```cpp
virtual void d2::sys::SystemAudio::filter_push(Device dev, const std::string &after, FilterPipeline filter)=0
```

Source: `core/mods/d2_audio.hpp:391`

### `filter_push`

```cpp
virtual void d2::sys::SystemAudio::filter_push(Device dev, FilterPipeline filter)=0
```

Source: `core/mods/d2_audio.hpp:392`

### `filter_override`

```cpp
virtual void d2::sys::SystemAudio::filter_override(Device dev, const std::string &name, std::function< bool(Stream)> filter)=0
```

Source: `core/mods/d2_audio.hpp:393`

### `filter_remove`

```cpp
virtual void d2::sys::SystemAudio::filter_remove(Device dev, const std::string &name)=0
```

Source: `core/mods/d2_audio.hpp:396`

### `filter_clear`

```cpp
virtual void d2::sys::SystemAudio::filter_clear(Device dev)=0
```

Source: `core/mods/d2_audio.hpp:397`

### `watch`

```cpp
Signals::Handle d2::sys::SystemAudio::watch(Device dev, Event ev, std::function< void(const DeviceName &)> callback)
```

Source: `core/mods/d2_audio.hpp:400`

### `watch`

```cpp
Signals::Handle d2::sys::SystemAudio::watch(Device dev, std::function< void(const DeviceName &, Event)> callback)
```

Source: `core/mods/d2_audio.hpp:401`


## Typedefs

### `Format`

```cpp
using d2::sys::SystemAudio::Format = impl::Format
```

Source: `core/mods/d2_audio.hpp:38`

### `FormatInfo`

```cpp
using d2::sys::SystemAudio::FormatInfo = impl::FormatInfo
```

Source: `core/mods/d2_audio.hpp:39`

### `DeviceConfig`

```cpp
using d2::sys::SystemAudio::DeviceConfig = impl::DeviceConfig
```

Source: `core/mods/d2_audio.hpp:40`

### `EventManifest`

```cpp
using d2::sys::SystemAudio::EventManifest = SignalManifest<EvDefault<Event, Device, const DeviceName&>>
```

Source: `core/mods/d2_audio.hpp:312`


## Variables

### `signal`

```cpp
struct d2::sys::SystemAudio::FilterPipeline d2::sys::SystemAudio::signal
```

Source: `core/mods/d2_audio.hpp:139`

### `_device_data`

```cpp
std::array<DeviceData, 2> d2::sys::SystemAudio::_device_data
```

Source: `core/mods/d2_audio.hpp:328`
