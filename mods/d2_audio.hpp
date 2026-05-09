#ifndef D2_MODS_AUDIO_HPP
#define D2_MODS_AUDIO_HPP

#include <d2_io_handler.hpp>
#include <d2_module.hpp>

namespace d2::sys::ext
{
    namespace impl
    {
        enum class Format
        {
            PCM16,
            PCM32,
            Float32,
            Float64,
        };

        // Compiler fucking complaining or whatever
        struct FormatInfo
        {
            Format format{Format::Float32};
            std::size_t channels{2};
            std::size_t sample_rate{48'000};
        };
        struct DeviceConfig
        {
            Format format{Format::Float32};
            std::size_t channels{2};
            std::size_t sample_rate{48'000};
            std::chrono::milliseconds latency{24};
            bool force_latency{true};
        };
    } // namespace impl

    class SystemAudio : public AbstractModule<"Audio">
    {
    public:
        using Format = impl::Format;
        using FormatInfo = impl::FormatInfo;
        using DeviceConfig = impl::DeviceConfig;

        enum class Event
        {
            Activate,
            Deactivate,
            Create,
            Destroy,
            DefaultChange,
        };
        enum class Device
        {
            Input,
            Output,
        };

        struct DeviceName
        {
            std::string id{""};
            std::string name{"Unknown"};
        };
        class Stream
        {
        private:
            std::span<float> _data{};
            std::size_t _total{0};
            std::size_t _offset{0};
            const FormatInfo* _format{nullptr};
            bool _stop{false};
        public:
            static float mix(float a, float b, float prog = 0.5f);

            Stream(
                std::span<float> data,
                std::size_t offset,
                std::size_t total,
                const FormatInfo* format
            ) : _data(data), _format(format), _offset(offset), _total(total)
            {
            }
            Stream(const Stream&) = default;
            Stream(Stream&&) = default;

            void apply(auto&& func, std::size_t channel, float start = 0.f, float end = 1.f)
            {
                const auto channel_size = _data.size() / _format->channels;
                const auto astart = std::size_t(start * channel_size);
                const auto aend = std::size_t(end * channel_size);
                for (std::size_t i = astart; i < aend; i += _format->channels)
                    _data[i + channel] =
                        func(float(_data[i + channel]), float(i + _offset) / _total);
            }
            void apply(auto&& func, float start = 0.f, float end = 1.f)
            {
                const auto astart = std::size_t(start * _data.size());
                const auto aend = std::size_t(end * _data.size());
                const auto dist = aend - astart;
                if (_format->channels == 2)
                {
                    for (std::size_t i = astart; i < aend; i += 2)
                    {
                        _data[i] = func(float(_data[i]), float(i + _offset) / _total);
                        _data[i + 1] = func(float(_data[i + 1]), float(i + 1 + _offset) / _total);
                    }
                }
                else if (_format->channels == 4)
                {
                    for (std::size_t i = astart; i < aend; i += 4)
                    {
                        _data[i] = func(float(_data[i]), float(i + _offset) / _total);
                        _data[i + 1] = func(float(_data[i + 1]), float(i + 1 + _offset) / _total);
                        _data[i + 2] = func(float(_data[i + 2]), float(i + 2 + _offset) / _total);
                        _data[i + 3] = func(float(_data[i + 3]), float(i + 3 + _offset) / _total);
                    }
                }
                else
                {
                    for (std::size_t i = astart; i < aend; i++)
                        _data[i] = func(float(_data[i]), float(i + _offset) / _total);
                }
            }

            void push(std::span<float> in);
            void push_expand(std::span<float> in);

            void push(std::span<float> in, float mul, float shift = 0.f);
            void push_expand(std::span<float> in, float mul, float shift = 0.f);

            void stop();
            bool is_stopped();

            const FormatInfo& info() const;
            std::size_t size() const;
            std::size_t chunk() const;

            float& at(std::size_t idx, std::size_t channel) const;
            float& at(float idx, std::size_t channel) const;

            Stream& operator=(const Stream&) = default;
            Stream& operator=(Stream&&) = default;
        };
        struct FilterPipeline
        {
            std::vector<std::pair<std::optional<std::string>, std::function<bool(Stream)>>>
                transforms;

            FilterPipeline() = default;

            FilterPipeline operator|(std::function<bool(Stream)> callback) const;
            FilterPipeline operator|(const std::string& name) const;
            FilterPipeline& operator&&(std::function<bool(Stream)> callback);
            FilterPipeline& operator|(const std::string& name);
            FilterPipeline& operator|(std::function<bool(Stream)> callback);
        } static inline const signal;
        class preset
        {
        public:
            static auto volume(float gain)
            {
                return [gain](Stream stream)
                {
                    stream.apply([gain](float x, float) -> float { return x * gain; });
                    return true;
                };
            }
            static auto fade(auto func)
            {
                return [func](Stream stream)
                {
                    stream.apply([&](float x, float p) -> float { return x * func(p); });
                    return true;
                };
            }
            static auto fade_in(auto func)
            {
                return fade(func);
            }
            static auto fade_out(auto func)
            {
                return fade([func](float p) { return 1.f - func(p); });
            }
            static auto fade_in()
            {
                return fade_in([](float p) { return p; });
            }
            static auto fade_out()
            {
                return fade_out([](float p) { return p; });
            }

            static auto bitcrusher(int bits = 8, int steps = 32)
            {
                const auto levels = float(1 << bits);
                return [levels, steps](Stream stream)
                {
                    stream.apply(
                        [levels, steps](float x, float p) -> float
                        {
                            const auto q = int(steps * p);
                            static float last = 0.f;
                            if (q % steps == 0)
                            {
                                last = std::round(x * levels) / levels;
                            }
                            return last;
                        }
                    );
                    return true;
                };
            }
            static auto tremolo(float frequency, float depth = 0.5f)
            {
                return [frequency, depth](Stream stream)
                {
                    stream.apply(
                        [frequency, depth](float x, float p) -> float
                        {
                            const auto lfo =
                                0.5f * (1.f + std::sin(2.f * 3.1415926f * frequency * p));
                            const auto gain = 1.f - depth + depth * lfo;
                            return x * gain;
                        }
                    );
                    return true;
                };
            }

            static auto wave(auto func, float gain = 1.f)
            {
                return [func, gain](Stream stream) mutable
                {
                    stream.apply([&](float old, float p) -> float { return old + func(p) * gain; });
                    return true;
                };
            }
            static auto wave_am(auto func)
            {
                return [func](Stream stream) mutable
                {
                    stream.apply([&](float old, float p) -> float { return old * func(p); });
                    return true;
                };
            }

            class wav
            {
            public:
                static auto sine(float frequency = 440.f, float sample_rate = 48'000.f)
                {
                    auto phase = 0.f;
                    const auto phase_inc = frequency / sample_rate;
                    return [phase, phase_inc](float) mutable
                    {
                        const auto out = std::sin(phase * 2.f * 3.1415926535f);
                        phase += phase_inc;
                        if (phase >= 1.f)
                            phase -= 1.f;
                        return out;
                    };
                }
                static auto square(float frequency = 440.f, float sample_rate = 48'000.f)
                {
                    auto phase = 0.f;
                    const auto phase_inc = frequency / sample_rate;
                    return [phase, phase_inc](float) mutable
                    {
                        const auto out = (phase < 0.5f) ? 1.f : -1.f;
                        phase += phase_inc;
                        if (phase >= 1.f)
                            phase -= 1.f;
                        return out;
                    };
                }
                static auto saw(float frequency = 440.f, float sample_rate = 48'000.f)
                {
                    auto phase = 0.f;
                    const auto phase_inc = frequency / sample_rate;
                    return [phase, phase_inc](float) mutable
                    {
                        const auto out = 2.f * phase - 1.f;
                        phase += phase_inc;
                        if (phase >= 1.f)
                            phase -= 1.f;
                        return out;
                    };
                }
                static auto triangle(float frequency = 440.f, float sample_rate = 48'000.f)
                {
                    auto phase = 0.f;
                    const auto phase_inc = frequency / sample_rate;
                    return [phase, phase_inc](float) mutable
                    {
                        float out = 0.f;
                        if (phase < 0.5f)
                            out = 4.f * phase - 1.f;
                        else
                            out = 3.f - 4.f * phase;

                        phase += phase_inc;
                        if (phase >= 1.f)
                            phase -= 1.f;
                        return out;
                    };
                }
                static auto noise()
                {
                    return [](float p)
                    { return (float(std::rand()) / (static_cast<float>(RAND_MAX) / 2.f)) - 1.f; };
                }
            };
        };
    private:
        Signals::Signal _sig_generic{nullptr};
        Signals::Signal _sig_in{nullptr};
        Signals::Signal _sig_out{nullptr};
    protected:
        struct DeviceData
        {
            enum class Status
            {
                Opening,
                Opened,
                Closed,
                Closing
            };
            FilterPipeline filter;
            Status status{Status::Closed};
        };
        std::array<DeviceData, 2> _device_data{};
    protected:
        std::size_t
        _stream_normalize_required(std::span<const unsigned char> data, FormatInfo format);
        void _stream_normalize(
            std::span<float> out, std::span<const unsigned char> data, FormatInfo format
        );
        void _set_device_status(Device dev, DeviceData::Status status);
        DeviceData::Status _get_device_status(Device dev);
        void _run_device_filter(Device dev, Stream& stream);
        void _run_filter(Stream& stream, FilterPipeline& pipeline);
        void _trigger_device_event(Device dev, Event ev, const DeviceName&);

        // Helpers for the implementation
        // We do not implement these publicly because we cannot assume the concurrency model
        // of the implementation

        void _filter(Device dev, FilterPipeline filter);
        void _filter_push(Device dev, const std::string& after, FilterPipeline filter);
        void _filter_push(Device dev, FilterPipeline filter);
        void
        _filter_override(Device dev, const std::string& name, std::function<bool(Stream)> filter);
        void _filter_remove(Device dev, const std::string& name);
        void _filter_clear(Device dev);

        virtual Status _load_impl() override;
        virtual Status _unload_impl() override;
    public:
        static constexpr std::size_t sample_size(Format format)
        {
            switch (format)
            {
            case SystemAudio::Format::PCM16:
                return 16 / 8;
            case SystemAudio::Format::PCM32:
                return 32 / 8;
            case SystemAudio::Format::Float32:
                return 32 / 8;
            case SystemAudio::Format::Float64:
                return 64 / 8;
            }
        }

        using AbstractModule::AbstractModule;

        virtual void flush(Device dev) = 0;
        virtual void record(std::function<void(std::span<const unsigned char>)> stream) = 0;
        virtual void transmit(
            std::span<const unsigned char> stream, FormatInfo format, FilterPipeline filter = {}
        ) = 0;
        virtual void transmit(
            std::vector<unsigned char> stream, FormatInfo format, FilterPipeline filter = {}
        ) = 0;
        virtual void transmit(std::chrono::seconds time, FilterPipeline filter = {}) = 0;
        virtual void deactivate(Device dev) = 0;
        virtual void activate(Device dev, const std::string& name = "", DeviceConfig cfg = {}) = 0;
        virtual void reactivate(Device dev, DeviceConfig cfg = {}) = 0;
        virtual DeviceName active(Device dev) = 0;
        virtual DeviceName default_device(Device dev) = 0;
        virtual FormatInfo format(Device dev) = 0;
        virtual std::vector<DeviceName> enumerate(Device dev) = 0;

        virtual void filter(Device dev, FilterPipeline filter) = 0;
        virtual void filter_push(Device dev, const std::string& after, FilterPipeline filter) = 0;
        virtual void filter_push(Device dev, FilterPipeline filter) = 0;
        virtual void filter_override(
            Device dev, const std::string& name, std::function<bool(Stream)> filter
        ) = 0;
        virtual void filter_remove(Device dev, const std::string& name) = 0;
        virtual void filter_clear(Device dev) = 0;

        Signals::Handle watch(
            Device dev,
            Event ev,
            std::function<void(const DeviceName&, module<SystemAudio>)> callback
        );
        Signals::Handle watch(
            Device dev, std::function<void(const DeviceName&, Event, module<SystemAudio>)> callback
        );
    };

} // namespace d2::sys::ext
namespace d2::sys
{
    using audio = ext::SystemAudio;
}

#endif
