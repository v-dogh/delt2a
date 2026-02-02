#ifndef D2_LINUX_SYSTEM_AUDIO_HPP
#define D2_LINUX_SYSTEM_AUDIO_HPP

#include "../d2_io_handler.hpp"

#include <pipewire/pipewire.h>
#include <pipewire/extensions/metadata.h>
#include <spa/param/audio/raw.h>
#include <spa/param/audio/format-utils.h>

#include <thread>
#include <unordered_map>
#include <vector>
#include <string>
#include <span>

namespace d2::sys::ext
{
    class PipewireSystemAudio :
        public SystemAudio,
        public SystemComponentCfg<true, true>
    {
    private:
        struct Voice
        {
            std::vector<float> samples{};
            std::chrono::seconds time{};
            FilterPipeline filter{};
            float progress{ 0.f };
        };
    private:
        std::thread::id _main_thread{};

        std::list<Voice> _voices{};

        pw_main_loop* _loop{};
        pw_context* _context{};
        pw_core* _core{};
        pw_registry* _registry{};
        pw_metadata* _metadata{};
        pw_metadata_events _metadata_events{};
        spa_hook _reg_listener{};        
        spa_hook _metadata_listener{};

        std::jthread _thread{};
        std::unordered_map<std::uint32_t, std::pair<std::string, DeviceName*>> _nodes{};
        std::vector<DeviceName> _inputs{};
        std::vector<DeviceName> _outputs{};
        std::string _default_input_id{};
        std::string _default_output_id{};

        pw_stream_events _in_events{};
        pw_stream_events _out_events{};
        pw_stream* _in_stream{};
        pw_stream* _out_stream{};
        spa_hook _in_listener{};
        spa_hook _out_listener{};
        spa_audio_info_raw _in_info{};
        spa_audio_info_raw _out_info{};
        std::uint32_t _in_frame_size{};
        std::uint32_t _out_frame_size{};

        DeviceName* _active_output{ nullptr };
        DeviceName* _active_input{ nullptr };
        std::function<void(std::span<const unsigned char>)> _input_push{ nullptr };

        static void _event_global(void* data, uint32_t id, uint32_t perms, const char* type, uint32_t ver, const struct spa_dict* props);
        static int _event_metadata(void *data, uint32_t id, const char* key, const char* type, const char* value);
        static void _event_global_remove(void* data, uint32_t id);
        static void _process_callback_input(void* data);
        static void _process_callback_output(void* data);

        static DeviceName* _find_device(std::vector<DeviceName>& v, const std::string& id);
        static DeviceName& _add_device(std::vector<DeviceName>& v, const std::string& id, const std::string name);
        static void _remove_device(std::vector<DeviceName>& v, const std::string& id);

        DeviceName*& _active_for(Device dev);
        void _event_global_impl(uint32_t id, const char* type, const spa_dict* props);
        void _event_global_remove_impl(uint32_t id);
        void _event_metadata_impl(uint32_t id, const char* key, const char* type, const char* value);
        void _start_stream(Device dev, std::string name, DeviceConfig cfg);
        void _stop_stream(Device dev);
        void _push_stream(std::chrono::seconds time, std::span<const unsigned char> data, std::vector<unsigned char> vec, FormatInfo fmt, FilterPipeline filter);
        void _flush_voices();

        template<typename Func, typename... Argv>
        void _run_this(Func&& callback, Argv&&... args)
        {
            if (std::this_thread::get_id() == _main_thread)
            {
                std::invoke(callback, this, std::forward<Argv>(args)...);
            }
            else
            {
                std::tuple block = std::make_tuple(
                    std::forward<Func>(callback),
                    this,
                    std::make_tuple(std::forward<Argv>(args)...)
                );
                const auto* block_ptr = &block;
                pw_loop_invoke(
                    pw_main_loop_get_loop(_loop),
                    +[](spa_loop*, bool, std::uint32_t, const void*, std::size_t, void* data) {
                        auto& arguments = *static_cast<std::remove_cvref_t<decltype(block)>*>(data);
                        std::apply([&](auto&&... args) {
                            std::invoke(std::get<0>(arguments), std::get<1>(arguments), std::forward<Argv>(args)...);
                        }, std::get<2>(arguments));
                        return 0;
                    },
                    0,
                    nullptr,
                    0,
                    true,
                    &block
                );
            }
        }
        template<typename Func, typename... Argv>
        void _run_this_async(Func&& callback, Argv&&... args)
        {
            if (std::this_thread::get_id() == _main_thread)
            {
                std::invoke(callback, this, std::forward<Argv>(args)...);
            }
            else
            {
                auto* block = new std::tuple{
                    std::forward<Func>(callback),
                    this,
                    std::make_tuple(args...)
                };
                while (true)
                {
                    auto result = pw_loop_invoke(
                        pw_main_loop_get_loop(_loop),
                        +[](spa_loop*, bool, std::uint32_t, const void*, std::size_t, void* data) {
                            auto* arguments = static_cast<std::remove_cvref_t<decltype(block)>>(data);
                            std::apply([&](auto&&... args) {
                                std::invoke(std::get<0>(*arguments), std::get<1>(*arguments), std::move(args)...);
                            }, std::get<2>(*arguments));
                            delete arguments;
                            return 0;
                        },
                        0,
                        nullptr,
                        0,
                        false,
                        block
                    );
                    if (result == -EPIPE)
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    else
                        break;
                }
            }
        }
    protected:
        virtual Status _load_impl() override;
        virtual Status _unload_impl() override;
    public:
        using SystemAudio::SystemAudio;

        virtual void flush(Device dev) override;
        virtual void record(std::function<void(std::span<const unsigned char>)> stream) override;
        virtual void transmit(std::span<const unsigned char> data, FormatInfo fmt, FilterPipeline filter = {}) override;
        virtual void transmit(std::vector<unsigned char> data, FormatInfo fmt, FilterPipeline filter = {}) override;
        virtual void transmit(std::chrono::seconds time, FilterPipeline filter = {}) override;
        virtual void deactivate(Device dev) override;
        virtual void activate(Device dev, const std::string& name = "", DeviceConfig cfg = DeviceConfig()) override;
        virtual void reactivate(Device dev, DeviceConfig cfg = DeviceConfig()) override;
        virtual DeviceName active(Device dev) override;
        virtual DeviceName default_device(Device dev) override;
        virtual FormatInfo format(Device dev) override;
        virtual std::vector<DeviceName> enumerate(Device dev) override;

        virtual void filter(Device dev, FilterPipeline filter) override;
        virtual void filter_push(Device dev, const std::string& after, FilterPipeline filter) override;
        virtual void filter_push(Device dev, FilterPipeline filter) override;
        virtual void filter_override(Device dev, const std::string& name, std::function<bool(Stream)> filter) override;
        virtual void filter_remove(Device dev, const std::string& name) override;
        virtual void filter_clear(Device dev) override;
    };
}

#endif // D2_LINUX_SYSTEM_AUDIO_HPP
