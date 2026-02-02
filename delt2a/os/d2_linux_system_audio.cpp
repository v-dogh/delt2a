#include "d2_linux_system_audio.hpp"

namespace d2::sys::ext
{
    PipewireSystemAudio::Status PipewireSystemAudio::_load_impl()
    {
        if (const auto status = SystemAudio::_load_impl();
            status != Status::Ok)
            return status;

        std::atomic_flag flag = false;
        flag.test_and_set();
        _thread = std::jthread([this, &flag] {
            pw_init(nullptr, nullptr);
            _main_thread = std::this_thread::get_id();
            _loop = pw_main_loop_new(nullptr);
            _context = pw_context_new(pw_main_loop_get_loop(_loop), nullptr, 0);
            _core = pw_context_connect(_context, nullptr, 0);

            _registry = pw_core_get_registry(_core, PW_VERSION_REGISTRY, 0);
            const pw_registry_events reg_events = {
                .version = PW_VERSION_REGISTRY_EVENTS,
                .global = &_event_global,
                .global_remove = &_event_global_remove,
            };
            pw_registry_add_listener(_registry, &_reg_listener, &reg_events, this);
            flag.clear();
            pw_main_loop_run(_loop);
        });
        while (flag.test_and_set())
            std::this_thread::yield();
        return Status::Ok;
    }
    PipewireSystemAudio::Status PipewireSystemAudio::_unload_impl()
    {
        _stop_stream(Device::Input);
        _stop_stream(Device::Output);

        // if (_registry) pw_registry_destroy(_registry);
        if (_core) pw_core_disconnect(_core);
        if (_context) pw_context_destroy(_context);
        if (_loop)
        {
            pw_main_loop_quit(_loop);
            _thread.join();
            pw_main_loop_destroy(_loop);
        }
        pw_deinit();

        _voices.clear();
        _nodes.clear();
        _inputs.clear();
        _outputs.clear();
        _active_output = nullptr;
        _active_input = nullptr;
        _input_push = nullptr;

        return SystemAudio::_unload_impl();
    }

    void PipewireSystemAudio::_process_callback_input(void* data)
    {
        auto* s = static_cast<PipewireSystemAudio*>(data);
        pw_buffer* buf = pw_stream_dequeue_buffer(s->_in_stream);
        if (!buf)
            return;
        auto* b = buf->buffer;
        if (b->n_datas == 0 || !b->datas[0].data)
        {
            pw_stream_queue_buffer(s->_in_stream, buf);
            return;
        }

        const std::uint8_t* ptr = static_cast<const std::uint8_t*>(b->datas[0].data);
        std::size_t bytes = b->datas[0].chunk->size ? b->datas[0].chunk->size : b->datas[0].maxsize;

        if (s->_input_push)
        {
            if (s->_in_info.format == SPA_AUDIO_FORMAT_F32)
            {
                FormatInfo fi{SystemAudio::Format::Float32, s->_in_info.channels, s->_in_info.rate};
                std::span<float> f{reinterpret_cast<float*>(b->datas[0].data), bytes / sizeof(float)};
                Stream stream(f, 0, f.size(), &fi);
                s->_run_device_filter(Device::Input, stream);
            }
            std::span<const unsigned char> block{ptr, bytes};
            s->_input_push(block);
        }

        pw_stream_queue_buffer(s->_in_stream, buf);
    }
    void PipewireSystemAudio::_process_callback_output(void* data)
    {
        auto* s = static_cast<PipewireSystemAudio*>(data);
        if (s->_out_info.format != SPA_AUDIO_FORMAT_F32 || s->_voices.empty()) return;
        if (auto* buf = pw_stream_dequeue_buffer(s->_out_stream); buf && buf->buffer)
        {
            std::memset(
                buf->buffer->datas[0].data, 0,
                buf->buffer->datas[0].maxsize
            );

            for (auto it = s->_voices.begin(); it != s->_voices.end();)
            {
                auto& voice = *it;
                if (voice.progress >= 1.f)
                    continue;

                auto& buffer = buf->buffer->datas[0];
                const auto block_sample = s->_out_info.channels * sizeof(float);
                const auto block = s->_out_info.rate * block_sample;
                const auto total = voice.samples.empty() ? voice.time.count() * block : voice.samples.size() * sizeof(float);
                const auto ex_fmt = FormatInfo{
                    .format = Format::Float32,
                    .channels = s->_out_info.channels,
                    .sample_rate = s-> _out_info.rate
                };

                std::size_t offset = total * voice.progress;
                if (!buffer.data)
                    break;

                const auto chunk_size = (std::min<std::size_t>(
                    total - offset,
                    (buffer.maxsize / (block_sample)) * block_sample
                ));
                auto* chunk_data = static_cast<float*>(buffer.data);
                auto* chunk_metadata = buffer.chunk;

                chunk_metadata->offset = 0;
                chunk_metadata->size = std::max<std::size_t>(chunk_metadata->size, chunk_size);
                chunk_metadata->stride = s->_out_frame_size;

                const auto sample_offset = offset / sizeof(float);
                const auto sample_count = chunk_size / sizeof(float);
                Stream stream{
                    { chunk_data, sample_count },
                    offset / sizeof(float),
                    total / sizeof(float),
                    &ex_fmt
                };

                if (!voice.samples.empty())
                {
                    for (std::size_t i = 0; i < sample_count; i++)
                        chunk_data[i] += voice.samples[sample_offset + i];
                }
                else
                {
                    if (!voice.filter.transforms.empty())
                        s->_run_filter(stream, voice.filter);
                    if (!stream.is_stopped())
                        s->_run_device_filter(Device::Output, stream);
                }

                offset += chunk_size;
                voice.progress = float(offset) / total;

                if (voice.progress >= 1.f || stream.is_stopped())
                {
                    auto cpy = it++;
                    s->_voices.erase(cpy);
                }
                else
                    ++it;
            }
            pw_stream_queue_buffer(s->_out_stream, buf);
        }
    }

    void PipewireSystemAudio::_event_global(void* data, uint32_t id, uint32_t perms, const char* type, uint32_t ver, const struct spa_dict* props)
    {
        static_cast<PipewireSystemAudio*>(data)->_event_global_impl(id, type, props);
    }
    void PipewireSystemAudio::_event_global_remove(void* data, uint32_t id)
    {
        static_cast<PipewireSystemAudio*>(data)->_event_global_remove_impl(id);
    }
    void PipewireSystemAudio::_event_global_impl(uint32_t id, const char* type, const spa_dict* props)
    {
        if (std::string_view(type) == PW_TYPE_INTERFACE_Metadata)
        {
            if (_metadata)
                return;
            std::string_view name = spa_dict_lookup(props, PW_KEY_METADATA_NAME);
            if (name == "default")
            {
                _metadata = static_cast<pw_metadata*>(
                    pw_registry_bind(_registry, id, PW_TYPE_INTERFACE_Metadata, PW_VERSION_METADATA, 0)
                    );
                _metadata_events = pw_metadata_events{
                    .version = PW_VERSION_METADATA_EVENTS,
                    .property = &_event_metadata
                };
                pw_metadata_add_listener(_metadata, &_metadata_listener, &_metadata_events, this);
            }
            return;
        }
        if (std::string_view(type) != PW_TYPE_INTERFACE_Node)
            return;

        const auto* media_ptr = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
        const auto* nid_ptr = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        const auto* name_ptr = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
        if (!media_ptr || !nid_ptr || !name_ptr)
            return;
        const auto media = std::string(spa_dict_lookup(props, PW_KEY_MEDIA_CLASS));
        const auto nid = std::string(spa_dict_lookup(props, PW_KEY_NODE_NAME));
        const auto name = std::string(spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION));

        DeviceName device{ nid, name };
        {
            DeviceName* device_ptr = nullptr;
            if (media == "Audio/Source") device_ptr = &_add_device(_inputs, nid, name);
            else if (media == "Audio/Sink") device_ptr = &_add_device(_outputs, nid, name);
            else return;
            _nodes[id] = { media, device_ptr };
        }
        if (media == "Audio/Source")
        {
            _trigger_device_event(Device::Input, Event::Create, device);
            if (device.id == _default_input_id)
                _trigger_device_event(Device::Input, Event::DefaultChange, device);
        }
        if (media == "Audio/Sink")
        {
            _trigger_device_event(Device::Output, Event::Create, device);
            if (device.id == _default_output_id)
                _trigger_device_event(Device::Output, Event::DefaultChange, device);
        }
    }
    void PipewireSystemAudio::_event_global_remove_impl(uint32_t id)
    {
        std::optional<std::pair<std::string, DeviceName*>> gone;
        {
            auto it = _nodes.find(id);
            if (it == _nodes.end())
                return;
            gone = it->second;
            _nodes.erase(it);
            if (gone->first == "Audio/Source") { _remove_device(_inputs, gone->second->id); }
            else if (gone->first == "Audio/Sink") { _remove_device(_outputs, gone->second->id); }
            else return;
        }
        if (!gone) return;
        if (gone->first == "Audio/Source") _trigger_device_event(Device::Input, Event::Destroy, *gone->second);
        if (gone->first == "Audio/Sink") _trigger_device_event(Device::Output, Event::Destroy, *gone->second);
    }
    int PipewireSystemAudio::_event_metadata(void *data, uint32_t id, const char* key, const char* type, const char* value)
    {
        static_cast<PipewireSystemAudio*>(data)->_event_metadata_impl(id, key, type, value);
        return 0;
    }
    void PipewireSystemAudio::_event_metadata_impl(uint32_t id, const char* key, const char* type, const char* value)
    {
        static auto extract_id = [](std::string_view js) -> std::string_view {
            // Trim it first
            while (!js.empty() &&
                   (js.front()==' ' ||
                    js.front()=='\t' ||
                    js.front()=='\n' ||
                    js.front()=='\r'))
                js.remove_prefix(1);

            // No json, just id
            if (js.empty() || js.front() != '{')
                return js;

            const auto key_value = std::string_view("\"name\"");
            const auto key = js.find(key_value);
            if (key == std::string_view::npos)
                return "";

            const auto colon = js.find(':', key + key_value.size());
            if (colon == std::string_view::npos)
                return "";

            auto quote_start = js.find('"', colon + 1);
            if (quote_start == std::string_view::npos)
                return "";

            std::size_t quote_end = quote_start + 1;
            for (; quote_end < js.size(); quote_end++)
            {
                if (js[quote_end] == '"' && js[quote_end - 1] != '\\')
                    break;
            }
            if (quote_end >= js.size())
                return "";

            return js.substr(
                quote_start + 1,
                quote_end - (quote_start + 1)
            );
        };

        if (key == nullptr || value == nullptr)
            return;

        if (!std::strcmp(key, "default.audio.sink"))
        {
            if (const auto id = std::string(extract_id(value)); !id.empty())
            {
                const auto* dev = _find_device(_outputs, id);
                _default_output_id = id;
                if (dev) _trigger_device_event(Device::Output, Event::DefaultChange, *dev);
            }
        }
        else if (!std::strcmp(key, "default.audio.source"))
        {
            if (const auto id = std::string(extract_id(value)); !id.empty())
            {
                const auto* dev = _find_device(_inputs, id);
                _default_input_id = id;
                if (dev) _trigger_device_event(Device::Input, Event::DefaultChange, *dev);
            }
        }
    }

    SystemAudio::DeviceName* PipewireSystemAudio::_find_device(std::vector<DeviceName>& v, const std::string& id)
    {
        auto it = std::find_if(v.begin(), v.end(), [&](const auto& v) {
            return v.id == id;
        });
        if (it == v.end())
            return nullptr;
        return &*it;
    }
    SystemAudio::DeviceName& PipewireSystemAudio::_add_device(std::vector<DeviceName>& v, const std::string& id, const std::string name)
    {
        auto it = std::find_if(v.begin(), v.end(), [&](const auto& v) {
            return v.id == id;
        });
        if (it == v.end())
        {
            v.push_back({ id, name });
            return v.back();
        }
        else
        {
            it->name = name;
            return *it;
        }
    }
    void PipewireSystemAudio::_remove_device(std::vector<DeviceName>& v, const std::string& id)
    {
        auto it = std::find_if(v.begin(), v.end(), [&](const auto& v) {
            return v.id == id;
        });
        if (it != v.end())
            v.erase(it);
    }

    SystemAudio::DeviceName*& PipewireSystemAudio::_active_for(Device dev)
    {
        return dev == Device::Input ? _active_input : _active_output;
    }
    void PipewireSystemAudio::_start_stream(Device dev, std::string name, DeviceConfig cfg)
    {
        using stat = DeviceData::Status;
        if (dev == Device::Input)
        {
            if (_get_device_status(dev) == stat::Opened)
            {
                _set_device_status(dev, stat::Opening);
                pw_stream_destroy(_in_stream);
                if (_get_device_status(dev) != stat::Opening)
                    return;
            }
            _set_device_status(dev, stat::Opening);
            // Properties
            pw_properties* props = nullptr;
            {
                props = pw_properties_new(
                    PW_KEY_MEDIA_TYPE, "Audio",
                    PW_KEY_MEDIA_CATEGORY, "Capture",
                    PW_KEY_MEDIA_ROLE, "Music",
                    PW_KEY_NODE_DESCRIPTION, "D2A System Audio Input",
                    nullptr
                );
                if (!name.empty())
                    pw_properties_set(props, PW_KEY_TARGET_OBJECT, name.c_str());

                auto frames = static_cast<std::size_t>((cfg.latency.count() / 1000.0) * cfg.sample_rate);
                std::size_t pw2 = 1;
                while (pw2 < frames)
                    pw2 <<= 1;
                frames = pw2;
                const auto latency_str = std::to_string(frames) + "/" + std::to_string(cfg.sample_rate);
                pw_properties_set(props, "node.latency", latency_str.c_str());
                if (cfg.force_latency)
                    pw_properties_set(props, "node.lock-quantum", "true");
            }
            // Stream
            {
                _in_events = pw_stream_events{
                    .version = PW_VERSION_STREAM_EVENTS,
                    .process = _process_callback_input
                };
                _in_stream = pw_stream_new_simple(
                    pw_main_loop_get_loop(_loop), "sysaudio-in",
                    props,
                    &_in_events,
                    this
                    );
                if (_get_device_status(dev) != stat::Opening)
                    return;

                spa_audio_info_raw info{};
                switch (cfg.format)
                {
                case Format::Float32: info.format = SPA_AUDIO_FORMAT_F32; break;
                case Format::Float64: info.format = SPA_AUDIO_FORMAT_F64; break;
                case Format::PCM16: info.format = SPA_AUDIO_FORMAT_S16; break;
                case Format::PCM32: info.format = SPA_AUDIO_FORMAT_S32; break;
                default: throw std::logic_error{ "Unuspported format" };
                }
                info.rate = cfg.sample_rate;
                info.channels = cfg.channels;
                if (info.channels == 1)
                {
                    info.position[0] = SPA_AUDIO_CHANNEL_MONO;
                }
                else
                {
                    info.position[0] = SPA_AUDIO_CHANNEL_FL;
                    info.position[1] = SPA_AUDIO_CHANNEL_FR;
                }
                spa_pod_builder pod;
                uint8_t buf[1024];
                spa_pod_builder_init(&pod, buf, sizeof(buf));
                const spa_pod* params[1];
                params[0] = spa_format_audio_raw_build(&pod, SPA_PARAM_EnumFormat, &info);

                pw_stream_connect(
                    _in_stream,
                    PW_DIRECTION_INPUT,
                    PW_ID_ANY,
                    pw_stream_flags(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS),
                    params, 1
                    );
                if (_get_device_status(dev) != stat::Opening)
                    return;

                _in_info = info;
                _in_frame_size = info.channels * sizeof(float);
            }
            _set_device_status(dev, stat::Opened);
        }
        else
        {
            if (_get_device_status(dev) == stat::Opened)
            {
                _set_device_status(dev, stat::Opening);
                pw_stream_destroy(_out_stream);
                if (_get_device_status(dev) != stat::Opening)
                    return;
            }
            _set_device_status(dev, stat::Opening);
            // Properties
            pw_properties* props = nullptr;
            {
                props = pw_properties_new(
                    PW_KEY_MEDIA_TYPE, "Audio",
                    PW_KEY_MEDIA_CATEGORY, "Playback",
                    PW_KEY_MEDIA_ROLE, "Music",
                    PW_KEY_NODE_DESCRIPTION, "D2A System Audio Output",
                    nullptr
                    );
                if (!name.empty())
                    pw_properties_set(props, PW_KEY_TARGET_OBJECT, name.c_str());

                auto frames = static_cast<std::size_t>((cfg.latency.count() / 1000.0) * cfg.sample_rate);
                std::size_t pw2 = 1;
                while (pw2 < frames)
                    pw2 <<= 1;
                frames = pw2;
                const auto latency_str = std::to_string(frames) + "/" + std::to_string(cfg.sample_rate);
                pw_properties_set(props, "node.latency", latency_str.c_str());
                if (cfg.force_latency)
                    pw_properties_set(props, "node.lock-quantum", "true");
            }
            // Stream
            {
                _out_events = {
                    .version = PW_VERSION_STREAM_EVENTS,
                    .process = _process_callback_output
                };
                _out_stream = pw_stream_new_simple(
                    pw_main_loop_get_loop(_loop), "sysaudio-out",
                    props,
                    &_out_events,
                    this
                    );
                if (_get_device_status(dev) != stat::Opening)
                    return;

                spa_audio_info_raw info{};
                switch (cfg.format)
                {
                case Format::Float32: info.format = SPA_AUDIO_FORMAT_F32; break;
                case Format::Float64: info.format = SPA_AUDIO_FORMAT_F64; break;
                case Format::PCM16: info.format = SPA_AUDIO_FORMAT_S16; break;
                case Format::PCM32: info.format = SPA_AUDIO_FORMAT_S32; break;
                default: throw std::logic_error{ "Unuspported format" };
                }
                info.rate = cfg.sample_rate;
                info.channels = cfg.channels;
                if (info.channels == 1)
                {
                    info.position[0] = SPA_AUDIO_CHANNEL_MONO;
                }
                else
                {
                    info.position[0] = SPA_AUDIO_CHANNEL_FL;
                    info.position[1] = SPA_AUDIO_CHANNEL_FR;
                }
                uint8_t buf[1024];
                spa_pod_builder b;
                spa_pod_builder_init(&b, buf, sizeof(buf));
                const spa_pod* params[1];
                params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);

                pw_stream_connect(
                    _out_stream,
                    PW_DIRECTION_OUTPUT,
                    PW_ID_ANY,
                    pw_stream_flags(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS),
                    params, 1
                    );
                if (_get_device_status(dev) != stat::Opening)
                    return;

                _out_info = info;
                _out_frame_size = info.channels * sizeof(float);
            }
            _set_device_status(dev, stat::Opened);
        }
    }
    void PipewireSystemAudio::_stop_stream(Device dev)
    {
        using stat = DeviceData::Status;
        if (dev == Device::Input)
        {
            if (_get_device_status(dev) == stat::Opened)
            {
                _set_device_status(dev, stat::Closing);
                pw_stream_destroy(_in_stream);
                if (_get_device_status(dev) != stat::Closing)
                    return;
                _in_stream = nullptr;
            }
            _set_device_status(dev, stat::Closed);
        }
        else
        {
            if (_get_device_status(dev) == stat::Opened)
            {
                _set_device_status(dev, stat::Closing);
                pw_stream_destroy(_out_stream);
                if (_get_device_status(dev) != stat::Closing)
                    return;
                _out_stream = nullptr;
            }
            _set_device_status(dev, stat::Closed);
        }
    }
    void PipewireSystemAudio::_push_stream(std::chrono::seconds time, std::span<const unsigned char> data, std::vector<unsigned char> vec, FormatInfo fmt, FilterPipeline filter)
    {
        const auto block_sample = _out_info.channels * sizeof(float);
        const auto block = _out_info.rate * block_sample;
        const auto total = data.empty() ? time.count() * block : _stream_normalize_required(data, fmt);
        const auto ex_fmt = FormatInfo{
            .format = Format::Float32,
            .channels = _out_info.channels,
            .sample_rate = _out_info.rate
        };

        if (!data.empty())
        {
            auto& voice = _voices.emplace_back(Voice{ .samples = std::vector<float>(total) });

            _stream_normalize(
                std::span(voice.samples),
                data,
                fmt
            );

            Stream stream{
                std::span(voice.samples),
                0,
                voice.samples.size(),
                &ex_fmt
            };
            _run_device_filter(Device::Output, stream);
            for (decltype(auto) it : filter.transforms)
                it.second(stream);
        }
        else
        {
            _voices.emplace_back(Voice{ .time = time, .filter = std::move(filter) });
        }
    }
    void PipewireSystemAudio::_flush_voices()
    {
        _voices.clear();
    }

    void PipewireSystemAudio::flush(Device dev)
    {
        if (dev == Device::Output)
            _run_this_async(&PipewireSystemAudio::_flush_voices);
    }
    void PipewireSystemAudio::record(std::function<void(std::span<const unsigned char>)> stream)
    {
        _run_this([&](SystemAudio*) {
           _input_push = std::move(stream);
        });
    }
    void PipewireSystemAudio::transmit(std::span<const unsigned char> data, FormatInfo fmt, FilterPipeline filter)
    {
        _run_this_async(
            &PipewireSystemAudio::_push_stream,
            std::chrono::seconds(0),
            data,
            std::vector<unsigned char>(),
            fmt,
            std::move(filter)
        );
    }
    void PipewireSystemAudio::transmit(std::vector<unsigned char> data, FormatInfo fmt, FilterPipeline filter)
    {
        _run_this_async(
            &PipewireSystemAudio::_push_stream,
            std::chrono::seconds(0),
            std::span<const unsigned char>(data),
            std::move(data),
            fmt,
            std::move(filter)
        );
    }
    void PipewireSystemAudio::transmit(std::chrono::seconds time, FilterPipeline filter)
    {
        if (!_out_stream)
            return;
        _run_this_async(
            &PipewireSystemAudio::_push_stream,
            time,
            std::span<const unsigned char>(),
            std::vector<unsigned char>(),
            FormatInfo(),
            std::move(filter)
        );
    }

    void PipewireSystemAudio::deactivate(Device dev)
    {
        _run_this_async([](PipewireSystemAudio* ptr, Device dev) {
            auto& active = ptr->_active_for(dev);
            if (active)
            {
                const auto tmp = *active;
                active = nullptr;
                ptr->_stop_stream(dev);
                ptr->_trigger_device_event(dev, Event::Deactivate, tmp);
            }
        }, dev);
    }
    void PipewireSystemAudio::activate(Device dev, const std::string& id, DeviceConfig cfg)
    {
        _run_this_async(
            [](PipewireSystemAudio* ptr, Device dev, std::string id, DeviceConfig cfg) {
                if (const auto active = ptr->_active_for(dev); active && active->id == id) return;
                if (const auto name = _find_device(dev == Device::Input ? ptr->_inputs : ptr->_outputs, id);
                    name != nullptr)
                {
                    ptr->_active_for(dev) = name;
                    ptr->_start_stream(dev, id, cfg);
                    ptr->_trigger_device_event(dev, Event::Activate, *name);
                }
            },
            dev, id, cfg
        );
    }
    void PipewireSystemAudio::reactivate(Device dev, DeviceConfig cfg)
    {
        _run_this_async([](PipewireSystemAudio* ptr, Device dev, DeviceConfig cfg) {
            auto& active = ptr->_active_for(dev);
            if (active)
            {
                ptr->_stop_stream(dev);
                ptr->_trigger_device_event(dev, Event::Deactivate, *active);
                if (const auto name = _find_device(dev == Device::Input ? ptr->_inputs : ptr->_outputs, active->id);
                    name != nullptr)
                {
                    ptr->_active_for(dev) = name;
                    ptr->_start_stream(dev, active->id, cfg);
                    ptr->_trigger_device_event(dev, Event::Activate, *name);
                }
            }
        }, dev, cfg);
    }
    SystemAudio::DeviceName PipewireSystemAudio::active(Device dev)
    {
        SystemAudio::DeviceName name;
        _run_this([&](SystemAudio*) {
            auto active = _active_for(dev);
            name = active == nullptr ? DeviceName() : *active;
        });
        return name;
    }
    SystemAudio::DeviceName PipewireSystemAudio::default_device(Device dev)
    {
        DeviceName name;
        _run_this([&](SystemAudio*) {

        });
        return name;
    }
    SystemAudio::FormatInfo PipewireSystemAudio::format(Device dev)
    {
        FormatInfo info;
        _run_this([&](SystemAudio*) {
            info = {
                .format = Format::Float32,
                .channels = _out_info.channels,
                .sample_rate = _out_info.rate
            };
        });
        return info;
    }
    std::vector<SystemAudio::DeviceName> PipewireSystemAudio::enumerate(Device dev)
    {
        std::vector<SystemAudio::DeviceName> vec;
        _run_this([&](SystemAudio*) {
            vec = (dev == Device::Input) ? _inputs : _outputs;
        });
        return vec;
    }

    void PipewireSystemAudio::filter(Device dev, FilterPipeline filter)
    {
        _run_this_async([=, this, filter = std::move(filter)](SystemAudio*) {
            _filter(dev, std::move(filter));
        });
    }
    void PipewireSystemAudio::filter_push(Device dev, const std::string& after, FilterPipeline filter)
    {
        _run_this_async([=, this, filter = std::move(filter)](SystemAudio*) {
            _filter_push(dev, after, std::move(filter));
        });
    }
    void PipewireSystemAudio::filter_push(Device dev, FilterPipeline filter)
    {
        _run_this_async([=, this, filter = std::move(filter)](SystemAudio*) {
            _filter_push(dev, std::move(filter));
        });
    }
    void PipewireSystemAudio::filter_override(Device dev, const std::string& name, std::function<bool(Stream)> filter)
    {
        _run_this_async([=, this, filter = std::move(filter)](SystemAudio*) {
            _filter_override(dev, name, std::move(filter));
        });
    }
    void PipewireSystemAudio::filter_remove(Device dev, const std::string& name)
    {
        _run_this_async([=, this](SystemAudio*) {
            _filter_remove(dev, name);
        });
    }
    void PipewireSystemAudio::filter_clear(Device dev)
    {
        _run_this_async([=, this](SystemAudio*) {
            _filter_clear(dev);
        });
    }
}
