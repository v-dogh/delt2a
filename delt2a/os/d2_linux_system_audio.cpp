#include "d2_linux_system_audio.hpp"

namespace d2::sys::ext
{
    PipewireSystemAudio::PipewireSystemAudio(std::weak_ptr<IOContext> ptr) : SystemAudio(ptr)
    {
        std::atomic_flag flag = false;
        flag.test_and_set();
        _thread = std::jthread([this, &flag] {
            pw_init(nullptr, nullptr);
            _loop = pw_main_loop_new(nullptr);
            _context = pw_context_new(pw_main_loop_get_loop(_loop), nullptr, 0);
            _core = pw_context_connect(_context, nullptr, 0);

            _registry = pw_core_get_registry(_core, PW_VERSION_REGISTRY, 0);
            const pw_registry_events reg_events = {
                .version = PW_VERSION_REGISTRY_EVENTS,
                .global = &_event_global,
                .global_remove = &_event_global_remove
            };
            pw_registry_add_listener(_registry, &_reg_listener, &reg_events, this);
            flag.clear();
            pw_main_loop_run(_loop);
        });
        while (!flag.test_and_set());
    }
    PipewireSystemAudio::~PipewireSystemAudio()
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
    }

    void PipewireSystemAudio::_process_callback_input(void* data)
    {
        auto* s = static_cast<PipewireSystemAudio*>(data);
        if (s->_recursive_check)
            return;
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
        if (s->_recursive_check)
            return;
        const auto lock = s->_lock_device_read();
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
                if (!voice.samples.empty())
                {
                    for (std::size_t i = 0; i < sample_count; i++)
                        chunk_data[i] += voice.samples[sample_offset + i];
                }
                else
                {
                    Stream stream{
                        { chunk_data, sample_count },
                        offset / sizeof(float),
                        total / sizeof(float),
                        &ex_fmt
                    };
                    if (!voice.filter.transforms.empty())
                    {
                        for (decltype(auto) it : voice.filter.transforms)
                            it(stream);
                    }
                    s->_run_device_filter(Device::Output, stream);
                }

                offset += chunk_size;
                voice.progress = float(offset) / total;

                if (voice.progress >= 1.f)
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
            auto lock = _lock_device_write();
            DeviceName* device_ptr;
            if (media == "Audio/Source") { device_ptr = &_add_device(_inputs, nid, name); }
            else if (media == "Audio/Sink") { device_ptr = &_add_device(_outputs, nid, name); }
            else return;
            _nodes[id] = { media, device_ptr };
        }
        if (media == "Audio/Source") _trigger_device_event(Device::Input, Event::Create, device);
        if (media == "Audio/Sink") _trigger_device_event(Device::Output, Event::Create, device);
    }
    void PipewireSystemAudio::_event_global_remove_impl(uint32_t id)
    {
        std::optional<std::pair<std::string, DeviceName*>> gone;
        {
            auto lock = _lock_device_write();
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

    SystemAudio::DeviceName*& PipewireSystemAudio::_active_for(Device dev) noexcept
    {
        return dev == Device::Input ? _active_input : _active_output;
    }
    void PipewireSystemAudio::_start_stream(Device dev, std::string name, std::size_t sample_rate, std::size_t channels)
    {
        if (_recursive_check)
            return;
        auto lock = _lock_device_write();
        if (dev == Device::Input)
        {
            if (_in_stream)
            {
                _recursive_check = true;
                pw_stream_destroy(_in_stream);
                _recursive_check = false;
            }
            pw_properties* props = pw_properties_new(
                PW_KEY_MEDIA_TYPE, "Audio",
                PW_KEY_MEDIA_CATEGORY, "Capture",
                PW_KEY_MEDIA_ROLE, "Music",
                PW_KEY_NODE_DESCRIPTION, "D2A System Audio Input",
                nullptr
            );
            if (!name.empty())
                pw_properties_set(props, PW_KEY_TARGET_OBJECT, name.c_str());

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

            spa_audio_info_raw info{};
            info.format = SPA_AUDIO_FORMAT_F32;
            info.rate = sample_rate;
            info.channels = channels;
            info.position[0] = SPA_AUDIO_CHANNEL_FL;
            info.position[1] = SPA_AUDIO_CHANNEL_FR;
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
            _in_info = info;
            _in_frame_size = info.channels * sizeof(float);
        }
        else
        {
            if (_out_stream)
            {
                _recursive_check = true;
                pw_stream_destroy(_out_stream);
                _recursive_check = false;
            }
            pw_properties* props = pw_properties_new(
                PW_KEY_MEDIA_TYPE, "Audio",
                PW_KEY_MEDIA_CATEGORY, "Playback",
                PW_KEY_MEDIA_ROLE, "Music",
                PW_KEY_NODE_DESCRIPTION, "D2A System Audio Output",
                nullptr
                );
            if (!name.empty())
                pw_properties_set(props, PW_KEY_TARGET_OBJECT, name.c_str());

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

            spa_audio_info_raw info{};
            info.format = SPA_AUDIO_FORMAT_F32;
            info.rate = sample_rate;
            info.channels = channels;
            info.position[0] = SPA_AUDIO_CHANNEL_FL;
            info.position[1] = SPA_AUDIO_CHANNEL_FR;
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
            _out_info = info;
            _out_frame_size = info.channels * sizeof(float);
        }
    }
    void PipewireSystemAudio::_stop_stream(Device dev)
    {
        if (_recursive_check)
            return;
        auto lock = _lock_device_write();
        if (dev == Device::Input)
        {
            if (_in_stream)
            {
                _recursive_check = true;
                pw_stream_destroy(_in_stream);
                _recursive_check = false;
                _in_stream = nullptr;
            }
        }
        else
        {
            if (_out_stream)
            {
                _recursive_check = true;
                pw_stream_destroy(_out_stream);
                _out_stream = nullptr;
                _recursive_check = false;
            }
        }
    }
    void PipewireSystemAudio::_push_stream(std::chrono::seconds time, std::span<const unsigned char> data, std::vector<unsigned char> vec, FormatInfo fmt, FilterPipeline filter)
    {
        if (_recursive_check)
            return;
        auto lock = _lock_device_read();
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
                it(stream);
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
        auto lock = _lock_device_write();
        _input_push = std::move(stream);
    }
    void PipewireSystemAudio::transmit(std::span<const unsigned char> data, FormatInfo fmt, FilterPipeline filter)
    {
        auto lock = _lock_device_read();
        if (!_out_stream)
            return;
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
        auto lock = _lock_device_read();
        if (!_out_stream)
            return;
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
        auto lock = _lock_device_read();
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
        auto lock = _lock_device_write();
        auto& active = _active_for(dev);
        if (active)
        {
            const auto cpy = std::move(*active);
            active = nullptr;
            _run_this_async(&PipewireSystemAudio::_stop_stream, dev);
            lock.unlock();
            _trigger_device_event(dev, Event::Deactivate, cpy);
        }
    }
    void PipewireSystemAudio::activate(Device dev, const std::string& id, std::size_t sample_rate, std::size_t channels)
    {
        auto lock = _lock_device_write();
        if (const auto name = _find_device(dev == Device::Input ? _inputs : _outputs, id);
            name != nullptr)
        {
            _active_for(dev) = name;
            _run_this_async(&PipewireSystemAudio::_start_stream, dev, id, sample_rate, channels);
            lock.unlock();
            _trigger_device_event(dev, Event::Activate, *name);
        }
    }
    SystemAudio::DeviceName PipewireSystemAudio::active(Device dev)
    {
        auto lock = _lock_device_read();
        auto active = _active_for(dev);
        return active == nullptr ? DeviceName() : *active;
    }
    SystemAudio::FormatInfo PipewireSystemAudio::format(Device dev)
    {
        auto lock = _lock_device_read();
        return {
            .format = Format::Float32,
            .channels = _out_info.channels,
            .sample_rate = _out_info.rate
        };
    }
    std::vector<SystemAudio::DeviceName> PipewireSystemAudio::enumerate(Device dev)
    {
        auto lock = _lock_device_read();
        return (dev == Device::Input) ? _inputs : _outputs;
    }
}
