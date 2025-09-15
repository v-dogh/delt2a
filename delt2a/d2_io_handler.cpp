#include "d2_io_handler.hpp"

namespace d2
{
    namespace sys
    {
        namespace ext
        {
            template<SystemAudio::Format>
            inline float _convert_sample(const unsigned char* sample) noexcept;

            template<> inline float _convert_sample<SystemAudio::Format::PCM16>(const unsigned char* sample) noexcept
            {
                return static_cast<float>(*reinterpret_cast<const unsigned short*>(sample)) / 32768.0f;
            }
            template<> inline float _convert_sample<SystemAudio::Format::PCM32>(const unsigned char* sample) noexcept
            {
                return static_cast<float>(*reinterpret_cast<const unsigned int*>(sample)) / 2147483648.0f;
            }
            template<> inline float _convert_sample<SystemAudio::Format::Float32>(const unsigned char* sample) noexcept
            {
                return *reinterpret_cast<const float*>(sample);
            }
            template<> inline float _convert_sample<SystemAudio::Format::Float64>(const unsigned char* sample) noexcept
            {
                return static_cast<float>(*reinterpret_cast<const double*>(sample));
            }

            inline float _lerp(float t, float a, float b) noexcept
            {
                return a + t * (b - a);
            }

            template<SystemAudio::Format Fmt>
            void _convert_from(std::span<float> out, std::span<const unsigned char> data, SystemAudio::FormatInfo format, SystemAudio::FormatInfo expected) noexcept
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

            std::unique_lock<std::shared_mutex> SystemAudio::_lock_device_write()
            {
                return std::unique_lock<std::shared_mutex>{ _device_mtx };
            }
            std::shared_lock<std::shared_mutex> SystemAudio::_lock_device_read()
            {
                return std::shared_lock<std::shared_mutex>{ _device_mtx };
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

            void SystemAudio::_run_device_filter(Device dev, Stream stream)
            {
                for (decltype(auto) it : _device_data[std::size_t(dev)].filter.transforms)
                    it(stream);
            }
            void SystemAudio::_trigger_device_event(Device dev, Event ev, const DeviceName& name)
            {
                if (auto& cb = _device_data[std::size_t(dev)];
                    cb.listener != nullptr)
                    cb.listener(name, ev);
            }

            void SystemAudio::filter(Device dev, FilterPipeline filter)
            {
                _device_data[std::size_t(dev)].filter
                    = std::move(filter);
            }
            void SystemAudio::watch(Device dev, std::function<void(const DeviceName&, Event)> callback)
            {
                for (decltype(auto) it : enumerate(dev))
                    callback(it, Event::Activate);
                _device_data[std::size_t(dev)].listener
                    = std::move(callback);
            }
        }
    }
}
