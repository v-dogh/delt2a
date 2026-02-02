#include "d2_module.hpp"
#include "d2_io_handler.hpp"

namespace d2::sys
{
    void SystemComponent::_mark_safe(std::thread::id id)
    {
        if (_status == Status::Ok)
        {
            D2_TAG_MODULE_RUNTIME(name())
            D2_THRW("Cannot add safe threads after initialization");
        }
        _safe_threads.insert(id);
    }
    void SystemComponent::_stat(Status status)
    {
        _status = status;
    }

    SystemComponent::Status SystemComponent::status()
    {
        return _status;
    }
    void SystemComponent::load()
    {
        D2_TAG_MODULE_RUNTIME(name())
        const auto result = D2_SAFE_BLOCK_BEGIN
            D2_TLOG(Module, "Loading module")
            _status = _load_impl();
        if (_status == Status::Offline)
        {
            D2_TLOG(Warning, "Module degraded")
            _status = Status::Degraded;
        }
        else
            D2_TLOG(Module, "Loaded module")
        D2_SAFE_BLOCK_END
            if (!result)
        {
            D2_TLOG(Module, "Failed to load module")
            _status = Status::Failure;
        }
    }
    void SystemComponent::unload()
    {
        D2_TAG_MODULE_RUNTIME(name())
        const auto result = D2_SAFE_BLOCK_BEGIN
            D2_TLOG(Module, "Unloading module")
            _status = _unload_impl();
        if (_status == Status::Ok)
            _status = Status::Offline;
        D2_TLOG(Module, "Unloaded module")
        D2_SAFE_BLOCK_END
            if (!result)
        {
            D2_TLOG(Module, "Failed to unload module")
            _status = Status::Failure;
        }
    }
    bool SystemComponent::accessible() const
    {
        if (!_is_thread_safe && _safe_threads.empty())
            return _ctx.lock()->is_synced();
        return
            _is_thread_safe ||
            _safe_threads.contains(std::this_thread::get_id());
    }
    std::string SystemComponent::name() const
    {
        return _name;
    }

    namespace ext
    {
        SystemAudio::FilterPipeline SystemAudio::FilterPipeline::operator|(std::function<bool(Stream)> callback) const
        {
            FilterPipeline pipeline;
            pipeline.transforms.reserve(4);
            pipeline.transforms.push_back({ std::nullopt, std::move(callback) });
            return pipeline;
        }
        SystemAudio::FilterPipeline SystemAudio::FilterPipeline::operator|(const std::string& name) const
        {
            FilterPipeline pipeline;
            pipeline.transforms.reserve(4);
            pipeline.transforms.push_back({ name, nullptr });
            return pipeline;
        }
        SystemAudio::FilterPipeline& SystemAudio::FilterPipeline::operator&&(std::function<bool(Stream)> callback)
        {
            if (transforms.empty() ||
                !transforms.back().first.has_value() ||
                transforms.back().second != nullptr)
                return operator|(std::move(callback));
            transforms.back().second = std::move(callback);
            return *this;
        }
        SystemAudio::FilterPipeline& SystemAudio::FilterPipeline::operator|(const std::string& name)
        {
            transforms.push_back({ name, nullptr });
            return *this;
        }
        SystemAudio::FilterPipeline& SystemAudio::FilterPipeline::operator|(std::function<bool(Stream)> callback)
        {
            transforms.push_back({ std::nullopt, std::move(callback) });
            return *this;
        }

        SystemAudio::Status SystemAudio::_load_impl()
        {
            _sig_generic = context()->connect<Device, const DeviceName&, Event, module<SystemAudio>>();
            _sig_in = context()->connect<Event, const DeviceName&, module<SystemAudio>>(Signals::id(Device::Input));
            _sig_out = context()->connect<Event, const DeviceName&, module<SystemAudio>>(Signals::id(Device::Output));
            return Status::Ok;
        }
        SystemAudio::Status SystemAudio::_unload_impl()
        {
            _sig_generic.disconnect();
            _sig_in.disconnect();
            _sig_out.disconnect();
            return Status::Ok;
        }

        void SystemAudio::Stream::push(std::span<float> in)
        {
            for (std::size_t i = 0; i < std::min(in.size(), _data.size()); i++)
                _data[i] = in[i];
        }
        void SystemAudio::Stream::push_expand(std::span<float> in)
        {
            for (std::size_t i = 0; i < std::min(in.size(), _data.size() / _format->channels); i++)
                for (std::size_t j = 0; j < _format->channels; j++)
                    _data[i + j] = in[i];
        }

        void SystemAudio::Stream::push(std::span<float> in, float mul, float shift)
        {
            for (std::size_t i = 0; i < std::min(in.size(), _data.size()); i++)
                _data[i] = in[i] * mul + shift;
        }
        void SystemAudio::Stream::push_expand(std::span<float> in, float mul, float shift)
        {
            for (std::size_t i = 0; i < std::min(in.size(), _data.size() / _format->channels); i++)
                for (std::size_t j = 0; j < _format->channels; j++)
                    _data[i + j] = in[i] * mul + shift;
        }

        void SystemAudio::Stream::stop()
        {
            _stop = true;
        }
        bool SystemAudio::Stream::is_stopped()
        {
            return _stop;
        }

        const SystemAudio::FormatInfo& SystemAudio::Stream::info() const
        {
            return *_format;
        }
        std::size_t SystemAudio::Stream::size() const
        {
            return _total / _format->channels;
        }
        std::size_t SystemAudio::Stream::chunk() const
        {
            return _data.size() / _format->channels;
        }

        float& SystemAudio::Stream::at(std::size_t idx, std::size_t channel) const
        {
            return _data[(idx * _format->channels) + channel];
        }
        float& SystemAudio::Stream::at(float idx, std::size_t channel) const
        {
            return _data[(idx * _format->channels) + channel];
        }

        float SystemAudio::Stream::mix(float a, float b, float prog)
        {
            return a + prog * (b - a);
        }

        template<SystemAudio::Format>
        inline float _convert_sample(const unsigned char* sample);

        template<> inline float _convert_sample<SystemAudio::Format::PCM16>(const unsigned char* sample)
        {
            return static_cast<float>(*reinterpret_cast<const unsigned short*>(sample)) / 32768.0f;
        }
        template<> inline float _convert_sample<SystemAudio::Format::PCM32>(const unsigned char* sample)
        {
            return static_cast<float>(*reinterpret_cast<const unsigned int*>(sample)) / 2147483648.0f;
        }
        template<> inline float _convert_sample<SystemAudio::Format::Float32>(const unsigned char* sample)
        {
            return *reinterpret_cast<const float*>(sample);
        }
        template<> inline float _convert_sample<SystemAudio::Format::Float64>(const unsigned char* sample)
        {
            return static_cast<float>(*reinterpret_cast<const double*>(sample));
        }

        template<SystemAudio::Format Fmt>
        void _convert_from(std::span<float> out, std::span<const unsigned char> data, SystemAudio::FormatInfo format, SystemAudio::FormatInfo expected)
        {
            constexpr auto sample_size = SystemAudio::sample_size(Fmt);

            if (format.channels == expected.channels &&
                format.sample_rate == expected.sample_rate &&
                format.format == expected.format)
            {
                std::memcpy(out.data(), data.data(), data.size());
                return;
            }

            const auto sample_ratio = expected.sample_rate / format.sample_rate;
            if (format.channels == expected.channels)
            {
                if (format.sample_rate > expected.sample_rate)
                {
                    for (std::size_t i = 0; i < data.size();)
                    {
                        for (std::size_t j = 0; j < expected.sample_rate; j += sample_size)
                        {
                            const auto base = float(j) / expected.sample_rate;
                            const auto pos = static_cast<std::size_t>(base * format.sample_rate);
                            const auto a = _convert_sample<Fmt>(&data[i + pos]);
                            const auto b = _convert_sample<Fmt>(&data[i + pos + 1]);
                            out[j++] += SystemAudio::Stream::mix(
                                a, b,
                                base - float(std::size_t(base))
                                );
                        }
                        i += format.sample_rate;
                    }
                }
                else if (format.sample_rate < expected.sample_rate)
                {
                    for (std::size_t i = 0; i < data.size();)
                    {
                        const auto a = _convert_sample<Fmt>(&data[i]);
                        const auto b = _convert_sample<Fmt>(&data[i + 1]);
                        for (std::size_t j = 0; j < expected.sample_rate; j += sample_size)
                        {
                            out[j++] += SystemAudio::Stream::mix(
                                a, b,
                                float(j) / expected.sample_rate
                                );
                        }
                        i += format.sample_rate;
                    }
                }
                else
                {
                    for (std::size_t i = 0, j = 0; j < data.size(); j += sample_size)
                        out[i++] += _convert_sample<Fmt>(&data[j]);
                }
            }
            else
            {
                const auto in_samples = data.size() / sample_size / format.channels;
                // Combine channels
                if (format.channels > expected.channels)
                {
                    if (format.sample_rate > expected.sample_rate)
                    {

                    }
                    else if (format.sample_rate < expected.sample_rate)
                    {

                    }
                    else
                    {
                        for (std::size_t oi = 0, ii = 0; ii < in_samples;)
                        {
                            double sum = 0.f;
                            for (std::size_t j = 0; j < format.channels; j++, ii++)
                                sum += _convert_sample<Fmt>(&data[ii * sample_size]);
                            for (std::size_t j = 0; j < expected.channels; j++, oi++)
                                out[oi * sizeof(float)] += sum;
                        }
                    }
                }
                // Propagate to additional channels
                else
                {
                    if (format.sample_rate > expected.sample_rate)
                    {

                    }
                    else if (format.sample_rate < expected.sample_rate)
                    {

                    }
                    else
                    {
                        for (std::size_t oi = 0, ii = 0; ii < in_samples;)
                        {
                            double sum = 0.f;
                            for (std::size_t j = 0; j < format.channels; j++, ii++)
                                sum += _convert_sample<Fmt>(&data[ii * sample_size]);
                            sum /= format.channels;
                            for (std::size_t j = 0; j < expected.channels; j++, oi++)
                                out[oi * sizeof(float)] += sum;
                        }
                    }
                }
            }
        }

        std::size_t SystemAudio::_stream_normalize_required(std::span<const unsigned char> data, FormatInfo format)
        {
            const auto expected = this->format(Device::Output);
            const auto in_samples = data.size() / sample_size(format.format) / format.channels;
            const auto rate_ratio = double(expected.sample_rate) / double(format.sample_rate);
            const auto channel_ratio = double(expected.channels) / double(format.channels);
            return static_cast<std::size_t>(in_samples * rate_ratio * expected.channels);
        }
        void SystemAudio::_stream_normalize(std::span<float> out, std::span<const unsigned char> data, FormatInfo format)
        {
            switch (format.format)
            {
            case Format::PCM16: _convert_from<Format::PCM16>(out, data, format, this->format(Device::Output)); break;
            case Format::PCM32: _convert_from<Format::PCM32>(out, data, format, this->format(Device::Output)); break;
            case Format::Float32: _convert_from<Format::Float32>(out, data, format, this->format(Device::Output)); break;
            case Format::Float64: _convert_from<Format::Float64>(out, data, format, this->format(Device::Output)); break;
            }
        }

        void SystemAudio::_set_device_status(Device dev, DeviceData::Status status)
        {
            _device_data[std::size_t(dev)].status = status;
        }
        SystemAudio::DeviceData::Status SystemAudio::_get_device_status(Device dev)
        {
            return _device_data[std::size_t(dev)].status;
        }
        void SystemAudio::_run_device_filter(Device dev, Stream& stream)
        {
            auto& device = _device_data[std::size_t(dev)];
            _run_filter(stream, device.filter);
        }
        void SystemAudio::_run_filter(Stream& stream, FilterPipeline& pipeline)
        {
            auto& trans = pipeline.transforms;
            for (std::size_t i = 0; i < trans.size(); i++)
            {
                if (!trans[i].second(stream))
                {
                    trans.erase(trans.begin() + i);
                    i--;
                }
                if (stream.is_stopped())
                    return;
            }
        }
        void SystemAudio::_trigger_device_event(Device dev, Event ev, const DeviceName& name)
        {
            context()->signal(dev, name, ev, module(this));
            context()->signal(ev, Signals::id(dev), name, module(this));
        }

        void SystemAudio::_filter(Device dev, FilterPipeline filter)
        {
            _device_data[std::size_t(dev)].filter
                = std::move(filter);
        }
        void SystemAudio::_filter_push(Device dev, const std::string& after, FilterPipeline filter)
        {
            auto& trans = _device_data[std::size_t(dev)].filter.transforms;
            auto f = std::find_if(trans.begin(), trans.end(), [&](const auto& v) {
                return v.first == after;
            });
            if (f == trans.end())
            {
                trans.insert(
                    trans.end(),
                    std::make_move_iterator(filter.transforms.begin()),
                    std::make_move_iterator(filter.transforms.end())
                    );
            }
            else
            {
                trans.insert(
                    f + 1,
                    std::make_move_iterator(filter.transforms.begin()),
                    std::make_move_iterator(filter.transforms.end())
                    );
            }
        }
        void SystemAudio::_filter_push(Device dev, FilterPipeline filter)
        {
            auto& trans = _device_data[std::size_t(dev)].filter.transforms;
            trans.insert(
                trans.end(),
                std::make_move_iterator(filter.transforms.begin()),
                std::make_move_iterator(filter.transforms.end())
                );
        }
        void SystemAudio::_filter_override(Device dev, const std::string& name, std::function<bool(Stream)> filter)
        {
            auto& trans = _device_data[std::size_t(dev)].filter.transforms;
            auto f = std::find_if(trans.begin(), trans.end(), [&](const auto& v) {
                return v.first == name;
            });
            if (f != trans.end())
                f->second = filter;
        }
        void SystemAudio::_filter_remove(Device dev, const std::string& name)
        {
            auto& trans = _device_data[std::size_t(dev)].filter.transforms;
            auto f = std::find_if(trans.begin(), trans.end(), [&](const auto& v) {
                return v.first == name;
            });
            if (f != trans.end())
                trans.erase(f);
        }
        void SystemAudio::_filter_clear(Device dev)
        {
            _device_data[std::size_t(dev)]
                .filter.transforms.clear();
        }

        Signals::Handle SystemAudio::watch(Device dev, Event ev, std::function<void(const DeviceName&, module<SystemAudio>)> callback)
        {
            return context()->listen(ev, Signals::id(dev), std::move(callback));
        }
        Signals::Handle SystemAudio::watch(Device dev, std::function<void(const DeviceName&, Event, module<SystemAudio>)> callback)
        {
            return context()->listen(dev, std::move(callback));
        }
    }
}
