#include "d2_io_handler.hpp"

namespace d2
{
    namespace sys
    {
        SystemComponent::Status SystemComponent::status()
        {
            return _status;
        }
        void SystemComponent::load()
        {
            try
            {
                _status
                    = _load_impl();
                if (_status == Status::Offline)
                    _status = Status::Degraded;
            }
            catch (...)
            {
                _status = Status::Failure;
            }
        }
        void SystemComponent::unload()
        {
            try
            {
                _status
                    = _unload_impl();
                if (_status == Status::Ok)
                    _status = Status::Offline;
            }
            catch (...)
            {
                _status = Status::Failure;
            }
        }

        namespace ext
        {
            void SystemAudio::Stream::apply(auto&& func, std::size_t channel, float start, float end)
            {
                const auto channel_size = _data.size() / _format->channels;
                const auto astart = std::size_t(start * channel_size);
                const auto aend = std::size_t(end * channel_size);
                for (std::size_t i = astart; i < aend; i += _format->channels)
                    _data[i + channel] = func(_data[i + channel], float(i + _offset) / _total);
            }
            void SystemAudio::Stream::apply(auto&& func, float start, float end)
            {
                const auto astart = std::size_t(start * _data.size());
                const auto aend = std::size_t(end * _data.size());
                const auto dist = aend - astart;
                if (_format->channels == 2)
                {
                    for (std::size_t i = astart; i < aend; i += 2)
                    {
                        _data[i] = func(_data[i], float(i + _offset) / _total);
                        _data[i + 1] = func(_data[i + 1], float(i + 1 + _offset) / _total);
                    }
                }
                else if (_format->channels == 4)
                {
                    for (std::size_t i = astart; i < aend; i += 4)
                    {
                        _data[i] = func(_data[i], float(i + _offset) / _total);
                        _data[i + 1] = func(_data[i + 1], float(i + 1 + _offset) / _total);
                        _data[i + 2] = func(_data[i + 2], float(i + 2 + _offset) / _total);
                        _data[i + 3] = func(_data[i + 3], float(i + 3 + _offset) / _total);
                    }
                }
                else
                {
                    for (std::size_t i = astart; i < aend; i++)
                        _data[i] = func(_data[i], float(i + _offset) / _total);
                }
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

            inline float _lerp(float t, float a, float b)
            {
                return a + t * (b - a);
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
                                out[j++] += _lerp(
                                    base - float(std::size_t(base)),
                                    a, b
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
                                out[j++] += _lerp(
                                    float(j) / expected.sample_rate,
                                    a, b
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
            void SystemAudio::_run_device_filter(Device dev, Stream stream)
            {
                for (decltype(auto) it : _device_data[std::size_t(dev)].filter.transforms)
                    it(stream);
            }
            void SystemAudio::_trigger_device_event(Device dev, Event ev, const DeviceName& name)
            {
                bool cleanup = false;
                auto& data = _device_data[std::size_t(dev)];
                for (decltype(auto) it : data.listener)
                {
                    auto state = it->state.load();
                    if (state == EventListenerState::State::Active)
                    {
                        it->callback(name, ev, it);
                    }
                    else
                    {
                        cleanup = true;
                    }
                }

                if (cleanup)
                {
                    data.listener.erase(
                        std::remove_if(
                            data.listener.begin(),
                            data.listener.end(),
                            [](auto v) {
                                return v->state == EventListenerState::State::Invalid;
                            }
                        ),
                        data.listener.end()
                    );
                }
            }

            void SystemAudio::filter(Device dev, FilterPipeline filter)
            {
                _device_data[std::size_t(dev)].filter
                    = std::move(filter);
            }
            SystemAudio::EventListener SystemAudio::watch(Device dev, std::function<void(const DeviceName&, Event, EventListener)> callback)
            {
                auto ev = std::make_shared<EventListenerState>(std::move(callback));
                for (decltype(auto) it : enumerate(dev))
                {
                    if (ev->state == EventListenerState::State::Active)
                        ev->callback(it, Event::Create, ev);
                    else
                        break;
                }
                _device_data[std::size_t(dev)].listener.push_back(ev);
                return ev;
            }
        }
    }

    std::weak_ptr<const IOContext> IOContext::_weak() const
    {
        return std::static_pointer_cast<const IOContext>(weak_from_this().lock());
    }
    std::shared_ptr<const IOContext> IOContext::_shared() const
    {
        return std::static_pointer_cast<const IOContext>(shared_from_this());
    }
    std::weak_ptr<IOContext> IOContext::_weak()
    {
        return std::static_pointer_cast<IOContext>(weak_from_this().lock());
    }
    std::shared_ptr<IOContext> IOContext::_shared()
    {
        return std::static_pointer_cast<IOContext>(shared_from_this());
    }

    IOContext::IOContext(mt::ThreadPool::ptr scheduler)
        : _scheduler(scheduler), Signals(scheduler) { }

    void IOContext::initialize()
    {
        using wflags = mt::ThreadPool::Worker::Flags;
        _main_thread = std::this_thread::get_id();
        _scheduler->start();
        _worker = _scheduler->worker(
            wflags::MainWorker |
            wflags::HandleCyclicTask |
            wflags::HandleDeferredTask
        );
        _worker.start();
    }
    void IOContext::deinitialize()
    {
        _worker.stop();
        _scheduler->stop();
    }
    void IOContext::wait(std::chrono::milliseconds ms)
    {
        _worker.wait(ms);
    }

    mt::ThreadPool::ptr IOContext::scheduler()
    {
        return _scheduler;
    }

    std::size_t IOContext::syscnt() const
    {
        std::shared_lock lock(_module_mtx);
        std::size_t cnt = 0;
        for (decltype(auto) it : _components)
            cnt += (it != nullptr);
        return cnt;
    }
    void IOContext::sysenum(std::function<void(sys::SystemComponent*)> callback)
    {
        for (decltype(auto) it : _components)
            callback(it.get());
    }
}
