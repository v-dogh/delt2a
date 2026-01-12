#ifndef D2_IO_HANDLER_HPP
#define D2_IO_HANDLER_HPP

#include <shared_mutex>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>
#include <span>
#include <tuple>

#include "d2_signal_handler.hpp"
#include "d2_screen_frwd.hpp"
#include "d2_io_handler_frwd.hpp"
#include "d2_pixel.hpp"
#include "mt/thread_pool.hpp"
#include "d2_meta.hpp"

namespace d2
{
	class IOContext;
	namespace sys
	{
		namespace impl
		{
			inline std::atomic<std::size_t> component_uid_track = 0;
			inline std::size_t component_uidgen()
			{
				return component_uid_track++;
			}

			struct ComponentUIDGeneratorBase
			{
				virtual std::size_t uid() const = 0;
				std::size_t uid()
				{
					return const_cast<const ComponentUIDGeneratorBase*>(this)->uid();
				}
			};
		}

		class SystemComponent : public impl::ComponentUIDGeneratorBase
		{
        public:
            enum class Status
            {
                Ok,
                Degraded,
                Failure,
                Offline,
            };
        private:
            const std::weak_ptr<IOContext> _ctx{};
            const std::string _name{ "<Unknown>" };
            const bool _is_thread_safe{ false };
            Status _status{ Status::Offline };
        protected:
            virtual Status _load_impl() { return Status::Ok; }
            virtual Status _unload_impl() { return Status::Ok; }
		public:
			template<typename Component>
            static std::unique_ptr<Component> make(mt::ThreadPool::ptr scheduler)
			{
				return std::make_unique<Component>(std::move(scheduler));
			}

            SystemComponent(std::weak_ptr<IOContext> ptr, const std::string& name, bool is_thread_safe)
                : _ctx(ptr), _name(name), _is_thread_safe(is_thread_safe) {}
            virtual ~SystemComponent() = default;

            std::shared_ptr<IOContext> context() const noexcept
            {
                return _ctx.lock();
            }

            Status status();
            void load();
            void unload();
            bool thread_safe() const
            {
                return _is_thread_safe;
            }
            std::string name() const
            {
                return _name;
            }

			template<typename Type>
			Type* as()
			{
				auto* p = dynamic_cast<Type*>(this);
				if (p == nullptr)
					throw std::logic_error{ "Attempt to access SystemComponent through an invalid object" };
				return p;
			}
		};

        template<meta::ConstString Name, typename Type>
        struct SystemComponentBase : public SystemComponent
        {
            static inline constexpr auto name = Name.view();
            static inline const auto uidc = impl::component_uidgen();

            using SystemComponent::SystemComponent;

            virtual std::size_t uid() const override
            {
                return uidc;
            }
        };
        template<bool ThreadSafe>
        struct SystemComponentCfg
        {
            static inline constexpr auto tsafe = ThreadSafe;
        };

		// Additional system wrappers (Optional ones)
        namespace ext
        {
            class SystemClipboard : public SystemComponentBase<"Clipboard", SystemClipboard>
            {
            public:
                using SystemComponentBase::SystemComponentBase;

                virtual void clear() = 0;
                virtual void copy(const string& value) = 0;
                virtual string paste() = 0;
                virtual bool empty() = 0;
            };
            class SystemAudio : public SystemComponentBase<"Audio", SystemAudio>
            {
            public:
                enum class Event
                {
                    Activate,
                    Deactivate,
                    Create,
                    Destroy,
                };
                enum class Device
                {
                    Input,
                    Output,
                };
                enum class Format
                {
                    PCM16,
                    PCM32,
                    Float32,
                    Float64,
                };
                struct FormatInfo
                {
                    Format format{ Format::Float32 };
                    std::size_t channels{ 1 };
                    std::size_t sample_rate{ 48'000 };
                };
                struct DeviceName
                {
                    std::string id{ "" };
                    std::string name{ "Unknown" };
                };
                class Stream
                {
                private:
                    std::span<float> _data{};
                    std::size_t _total{ 0 };
                    std::size_t _offset{ 0 };
                    const FormatInfo* _format{ nullptr };
                public:
                    Stream(std::span<float> data, std::size_t offset, std::size_t total, const FormatInfo* format) :
                        _data(data), _format(format), _offset(offset), _total(total) {}
                    Stream(const Stream&) = default;
                    Stream(Stream&&) = default;

                    void apply(auto&& func, std::size_t channel, float start = 0.0f, float end = 1.f);
                    void apply(auto&& func, float start = 0.0f, float end = 1.f);
                    void push(std::span<float> in);
                    void push_expand(std::span<float> in);

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
                    std::vector<
                        std::pair<
                            std::optional<std::string>,
                            std::function<bool(Stream) >
                        >
                    > transforms;

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
                            stream.apply([gain](float x, float) -> float {
                                return x * gain;
                            });
                        };
                    }
                    static auto fade(auto func)
                    {
                        return [func](Stream stream)
                        {
                            stream.apply([&](float x, float p) -> float {
                                return x * func(p);
                            });
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
                            stream.apply([levels, steps](float x, float p) -> float {
                                const auto q = int(steps * p);
                                static float last = 0.f;
                                if (q % steps == 0) {
                                    last = std::round(x * levels) / levels;
                                }
                                return last;
                            });
                        };
                    }
                    static auto tremolo(float frequency, float depth = 0.5f)
                    {
                        return [frequency, depth](Stream stream)
                        {
                            stream.apply([frequency, depth](float x, float p) -> float {
                                const auto lfo = 0.5f * (1.f + std::sin(2.f * 3.1415926f * frequency * p));
                                const auto gain = 1.f - depth + depth * lfo;
                                return x * gain;
                            });
                        };
                    }

                    static auto wave(auto func, float gain = 1.f)
                    {
                        return [func, gain](Stream stream) mutable
                        {
                            stream.apply([&](float old, float p) -> float {
                                return old + func(p) * gain;
                            });
                        };
                    }
                    static auto wave_am(auto func)
                    {
                        return [func](Stream stream) mutable
                        {
                            stream.apply([&](float old, float p) -> float {
                                return old * func(p);
                            });
                        };
                    }

                    class wav
                    {
                    public:
                        static auto sine(float frequency = 440.f, float sample_rate = 48'000.f)
                        {
                            auto phase = 0.f;
                            const auto phase_inc = frequency / sample_rate;
                            return [phase, phase_inc](float) mutable {
                                const auto out = std::sin(phase * 2.f * 3.1415926535f);
                                phase += phase_inc;
                                if (phase >= 1.f) phase -= 1.f;
                                return out;
                            };
                        }
                        static auto square(float frequency = 440.f, float sample_rate = 48'000.f)
                        {
                            auto phase = 0.f;
                            const auto phase_inc = frequency / sample_rate;
                            return [phase, phase_inc](float) mutable {
                                const auto out = (phase < 0.5f) ? 1.f : -1.f;
                                phase += phase_inc;
                                if (phase >= 1.f) phase -= 1.f;
                                return out;
                            };
                        }
                        static auto saw(float frequency = 440.f, float sample_rate = 48'000.f)
                        {
                            auto phase = 0.f;
                            const auto phase_inc = frequency / sample_rate;
                            return [phase, phase_inc](float) mutable {
                                const auto out = 2.f * phase - 1.f;
                                phase += phase_inc;
                                if (phase >= 1.f) phase -= 1.f;
                                return out;
                            };
                        }
                        static auto triangle(float frequency = 440.f, float sample_rate = 48'000.f)
                        {
                            auto phase = 0.f;
                            const auto phase_inc = frequency / sample_rate;
                            return [phase, phase_inc](float) mutable {
                                float out = 0.f;
                                if (phase < 0.5f)
                                    out = 4.f * phase - 1.f;
                                else
                                    out = 3.f - 4.f * phase;

                                phase += phase_inc;
                                if (phase >= 1.f) phase -= 1.f;
                                return out;
                            };
                        }
                        static auto noise()
                        {
                            return [](float p) {
                                return (float(std::rand()) / (static_cast<float>(RAND_MAX) / 2.f)) - 1.f;
                            };
                        }
                    };
                };
            private:
                Signals::Signal _sig_generic{ nullptr };
                Signals::Signal _sig_in{ nullptr };
                Signals::Signal _sig_out{ nullptr };
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
                    Status status{ Status::Closed };
                };
                std::array<DeviceData, 2> _device_data{};
            protected:
                std::size_t _stream_normalize_required(std::span<const unsigned char> data, FormatInfo format);
                void _stream_normalize(std::span<float> out, std::span<const unsigned char> data, FormatInfo format);
                void _set_device_status(Device dev, DeviceData::Status status);
                DeviceData::Status _get_device_status(Device dev);
                void _run_device_filter(Device dev, Stream stream);
                void _trigger_device_event(Device dev, Event ev, const DeviceName&);

                // Helpers for the implementation
                // We do not implement these publicly because we cannot assume the concurrency model of the implementation

                void _filter(Device dev, FilterPipeline filter);
                void _filter_push(Device dev, const std::string& after, FilterPipeline filter);
                void _filter_push(Device dev, FilterPipeline filter);
                void _filter_override(Device dev, const std::string& name, std::function<bool(Stream)> filter);
                void _filter_remove(Device dev, const std::string& name);
                void _filter_clear(Device dev);

                virtual Status _load_impl() override;
                virtual Status _unload_impl() override;
            public:         
                static constexpr std::size_t sample_size(Format format)
                {
                    switch (format)
                    {
                    case SystemAudio::Format::PCM16: return 16 / 8;
                    case SystemAudio::Format::PCM32: return 32 / 8;
                    case SystemAudio::Format::Float32: return 32 / 8;
                    case SystemAudio::Format::Float64: return 64 / 8;
                    }
                }

                using SystemComponentBase::SystemComponentBase;

                virtual void flush(Device dev) = 0;
                virtual void record(std::function<void(std::span<const unsigned char>)> stream) = 0;
                virtual void transmit(std::span<const unsigned char> stream, FormatInfo format, FilterPipeline filter = {}) = 0;
                virtual void transmit(std::vector<unsigned char> stream, FormatInfo format, FilterPipeline filter = {}) = 0;
                virtual void transmit(std::chrono::seconds time, FilterPipeline filter = {}) = 0;
                virtual void deactivate(Device dev) = 0;
                virtual void activate(Device dev, const std::string& name = "", std::size_t sample_rate = 48'000, std::size_t channels = 2) = 0;
                virtual DeviceName active(Device dev) = 0;
                virtual FormatInfo format(Device dev) = 0;
                virtual std::vector<DeviceName> enumerate(Device dev) = 0;

                virtual void filter(Device dev, FilterPipeline filter) = 0;
                virtual void filter_push(Device dev, const std::string& after, FilterPipeline filter) = 0;
                virtual void filter_push(Device dev, FilterPipeline filter) = 0;
                virtual void filter_override(Device dev, const std::string& name, std::function<bool(Stream)> filter) = 0;
                virtual void filter_remove(Device dev, const std::string& name) = 0;
                virtual void filter_clear(Device dev) = 0;

                Signals::Handle watch(Device dev, Event ev, std::function<void(const DeviceName&, SystemAudio&)> callback);
                Signals::Handle watch(Device dev, std::function<void(const DeviceName&, Event, SystemAudio&)> callback);
            };
            class SystemNotifications : public SystemComponentBase<"Notifications", SystemNotifications>
            {
            public:
                using SystemComponentBase::SystemComponentBase;

                virtual void notify(const std::string& title, const std::string& content, const std::vector<unsigned char> icon = {}) = 0;
                virtual void remind(std::chrono::system_clock::time_point when, const std::string& title, const std::string& content, const std::vector<unsigned char> icon = {}) = 0;
            };
            class SystemPlugins : public SystemComponentBase<"Plugins", SystemPlugins>
            {
            public:
                enum class LoadStatus
                {
                    VersionMismatch,
                    InvalidResponse,
                    Ok
                };
            private:
                struct Environment
                {
                    std::shared_ptr<IOContext> ctx;
                    std::vector<unsigned char> params;
                };
                struct Pipe
                {

                };
            public:
                using SystemComponentBase::SystemComponentBase;

                template<typename... Components, typename... Argv>
                Environment sandbox(std::tuple<Argv...>&& params, mt::ThreadPool::ptr pool = nullptr)
                {
                    return Environment();
                }
                template<typename... Components, typename... Argv>
                Environment sandbox(mt::ThreadPool::ptr pool = nullptr)
                {
                    return Environment();
                }

                template<typename Ret, typename... Argv>
                mt::future<Ret> send(const std::string& name, const std::string& endpoint, Argv&&... args)
                {

                }
                template<typename Ret>
                mt::future<Ret> receive(const std::string& name, const std::string& endpoint)
                {

                }

                virtual LoadStatus load(const std::filesystem::path& path, const std::string& name, Environment ctx = Environment()) = 0;
                virtual void unload(const std::string& name) = 0;
            };
            class SystemIO : public SystemComponentBase<"IO", SystemIO>
            {

            };

            class LocalSystemClipboard :
                public SystemClipboard,
                public SystemComponentCfg<false>
            {
            private:
                string value_{};
            protected:
                virtual Status _load_impl() override
                {
                    return Status::Ok;
                }
                virtual Status _unload_impl() override
                {
                    value_.clear();
                    value_.shrink_to_fit();
                    return Status::Ok;
                }
            public:
                using SystemClipboard::SystemClipboard;

                virtual void clear() override
                {
                    value_.clear();
                }
                virtual void copy(const string& value) override
                {
                    value_ = value;
                }
                virtual string paste() override
                {
                    return value_;
                }
                virtual bool empty() override
                {
                    return value_.empty();
                }
            };
        }

		// Generic base for system input
        class SystemInput : public SystemComponentBase<"Input", SystemInput>
		{
		public:
			using keytype = short;
			enum MouseKey : keytype
			{
				Left,
				Right,
				Middle,
				SideTop,
				SideBottom,
				ScrollUp,
				ScrollDown,
				MouseKeyMax
			};
			enum SpecialKey : keytype
			{
				Reserved = (('`' - ' ') + ('~' - '{')) + 1,
				Backspace,
				Enter,
				ArrowUp,
				ArrowDown,
				ArrowLeft,
				ArrowRight,
				LeftAlt,
				RightAlt,
				LeftControl,
				RightControl,
				Tab,
				Shift,
				Home,
				End,
				Insert,
				Delete,
				PgUp,
				PgDown,
				Escape,
				Super,
				// Any further values indicate function keys e.g. (Fn + 1 indicate F1)
				Fn,
				// For reference, the max value a character can take (when resolved)
				SpecialKeyMax = (Fn + 24),
			};
			enum class KeyMode
			{
				Press,
				Release,
				Hold
			};
		private:
			bool enabled_{ true };
		protected:
            static constexpr keytype _resolve_key(char ch)
			{
				if (ch <= keymax() && ch >= keymin())
				{
					if (ch)
					{
						if (ch <= 'z')
							return std::toupper(ch) - keymin();
						return ch - ('z' - 'a');
					}
					else
					{
						return ch - keymin();
					}
				}
				return Reserved;
			}

			virtual bool _is_pressed_impl(keytype ch, KeyMode mode) = 0;
			virtual bool _is_pressed_mouse_impl(MouseKey key, KeyMode mode) = 0;
			virtual const string& _key_sequence_input_impl() = 0;

			virtual std::pair<int, int> _mouse_position_impl() = 0;
			virtual std::pair<std::size_t, std::size_t> _screen_size_impl() = 0;
			virtual std::pair<std::size_t, std::size_t> _screen_capacity_impl() = 0;

			virtual bool _is_key_sequence_input_impl() = 0;
			virtual bool _is_key_input_impl() = 0;
			virtual bool _is_mouse_input_impl() = 0;
			virtual bool _is_screen_resize_impl() = 0;
			virtual bool _has_mouse_impl() = 0;

			virtual void _begincycle_impl() = 0;
			virtual void _endcycle_impl() = 0;
		public:
            static constexpr char keymin()
			{
				return ' ';
			}
            static constexpr char keymax()
			{
				return '~';
			}
            static constexpr char key(char ch)
			{
				return _resolve_key(ch);
			}

            using SystemComponentBase::SystemComponentBase;

			void begincycle()
			{
				_begincycle_impl();
			}
			void endcycle()
			{
				_endcycle_impl();
			}

			void enable()
			{
				enabled_ = true;
			}
			void disable()
			{
				enabled_ = false;
			}

			bool has_mouse()
			{
				return _has_mouse_impl();
			}

			bool is_key_sequence_input()
			{
				return this->_is_key_sequence_input_impl();
			}
			bool is_key_input()
			{
				return this->_is_key_input_impl();
			}
			bool is_mouse_input()
			{
				return this->_is_mouse_input_impl();
			}
			bool is_screen_resize()
			{
				return this->_is_screen_resize_impl();
			}

			bool is_pressed(keytype ch, KeyMode mode = KeyMode::Hold)
			{
				if (enabled_)
				{
					return
						ch >= 0 &&
						ch <= (Fn + 24) &&
						this->_is_pressed_impl(ch, mode);
				}
				return false;
			}
			bool is_pressed_mouse(MouseKey key, KeyMode mode = KeyMode::Hold)
			{
				return enabled_ && this->_is_pressed_mouse_impl(key, mode);
			}
			const string& key_sequence()
			{
				return this->_key_sequence_input_impl();
			}
			std::pair<int, int> mouse_position()
			{
				if (!enabled_)
					return { -1, -1 };
				return this->_mouse_position_impl();
			}
			std::pair<std::size_t, std::size_t> screen_size()
			{
				return this->_screen_size_impl();
			}
			std::pair<std::size_t, std::size_t> screen_capacity()
			{
				return this->_screen_capacity_impl();
			}
		};

		// Generic base for system output
		// Provides interfaces used for rendering to the console and other OS agnostic interfaces
        class SystemOutput : public SystemComponentBase<"Output", SystemOutput>
		{
		public:
            static constexpr auto image_constant = px::combined{ .v = std::numeric_limits<standard_value_type>::max() };
			class ImageInstance
			{
			public:
                using data = std::vector<unsigned char>;
			private:
                unsigned char format_{ 0x00 };
				std::size_t width_{ 0 };
				std::size_t height_{ 0 };
				data data_{};
			public:
                ImageInstance(data data, std::size_t width, std::size_t height, unsigned char format = 0x00) :
					data_(data),
					width_(width),
                    height_(height),
                    format_(format)
				{}
				~ImageInstance() = default;

                std::span<const unsigned char> read() const
                {
                    return { data_.begin(), data_.end() };
                }

                std::size_t width() const
                {
                    return width_;
                }
                std::size_t height() const
                {
                    return height_;
                }

                unsigned int format() const
                {
                    return format_;
                }
            };
            using image = std::uint32_t;
            enum Format : unsigned int
            {
                RGB8,
                RGBA8,
                PNG,
            };
        public:
            using SystemComponentBase::SystemComponentBase;

            virtual void load_image(const std::string& path, ImageInstance img) = 0;
            virtual void release_image(const std::string& path) = 0;
            virtual ImageInstance image_info(const std::string& path) = 0;
            virtual image image_id(const std::string& path) = 0;

            virtual std::size_t delta_size() = 0;
            virtual std::size_t swapframe_size() = 0;

			virtual void write(std::span<const Pixel> buffer, std::size_t width, std::size_t height) = 0;
		};

		// Aliases for components

		using input = SystemInput;
		using output = SystemOutput;

        using plugins = ext::SystemPlugins;
		using clipboard = ext::SystemClipboard;
		using audio = ext::SystemAudio;
		using notifications = ext::SystemNotifications;
	}
    namespace input
    {
        using mouse = sys::SystemInput::MouseKey;
        using special = sys::SystemInput::SpecialKey;
        using mode = sys::SystemInput::KeyMode;
        constexpr auto key = sys::SystemInput::key;
    }

    class IOContext : public Signals
	{
	public:
		template<typename Type>
        using future = mt::future<Type>;
		using ptr = std::shared_ptr<IOContext>;
		using wptr = std::weak_ptr<IOContext>;
	private:
        mutable std::shared_mutex _module_mtx{};
        std::vector<std::unique_ptr<sys::SystemComponent>> _components{};
        mt::ThreadPool::ptr _scheduler{ nullptr };
        mt::ThreadPool::Worker _worker{};
        std::thread::id _main_thread{};

        std::weak_ptr<const IOContext> _weak() const;
        std::shared_ptr<const IOContext> _shared() const;
        std::weak_ptr<IOContext> _weak();
        std::shared_ptr<IOContext> _shared();

		template<typename Type> Type* _get_component()
		{
            const auto uid = Type::uidc;
            [[ unlikely ]] if (_components.size() < uid)
				throw std::logic_error{ "Attempt to query inactive component" };
            auto* ptr = _components[uid].get();
            if (ptr->status() == sys::SystemComponent::Status::Offline)
                ptr->load();
            return static_cast<Type*>(ptr);
		}
		template<typename Type> Type* _get_component_if()
		{
			const auto uid = Type::uidc;
            [[ unlikely ]] if (_components.size() < uid)
				return nullptr;
            auto* ptr = _components[uid].get();
            if (ptr->status() == sys::SystemComponent::Status::Offline)
                ptr->load();
            return static_cast<Type*>(ptr);
		}
		void _insert_component(std::unique_ptr<sys::SystemComponent> ptr)
		{
			const auto idx = ptr->uid();
            _components.resize(std::max(_components.size(), idx + 1));
            _components[idx] = std::move(ptr);
		}
	public:
		template<typename... Components> static ptr make(
            std::size_t threads = 1,
			bool auto_grow = true,
			bool auto_shrink = false
		)
		{
            using flags = mt::ThreadPool::Config::Flags;
            auto scheduler = mt::ThreadPool::make(mt::ThreadPool::Config{
                .flags =
                    static_cast<unsigned char>((flags::AutoGrow * auto_grow) |
                    (flags::AutoShrink * auto_shrink) |
                    flags::MainCyclicWorker),
                .min_threads = threads,
                .max_idle_time = std::chrono::seconds(15),
            });
            auto ptr = std::make_shared<IOContext>(scheduler);
			ptr->load<Components...>();
            return ptr;
		}

        IOContext(mt::ThreadPool::ptr scheduler);
		virtual ~IOContext() = default;

        // State

        void initialize();
        void deinitialize();
        void wait(std::chrono::milliseconds ms);

        // Synchronization

        mt::ThreadPool::ptr scheduler();

        bool is_synced() const
        {
            return (std::this_thread::get_id() == _main_thread) || !_worker.active();
        }
        template<typename Func>
        auto sync(Func&& callback) -> mt::future<decltype(callback())>
        {
            return scheduler()->launch_deferred(std::chrono::milliseconds(0), [callback = std::forward<Func>(callback)]() -> decltype(auto)
            {
                return callback();
            });
        }
        template<typename Func>
        auto sync_if(Func&& callback) -> decltype(callback())
        {
            if (is_synced())
            {
                return callback();
            }
            else
            {
                return scheduler()->launch_deferred(std::chrono::milliseconds(0), [callback = std::forward<Func>(callback)]() -> decltype(auto)
                {
                    return callback();
                }).value();
            }
        }

        // Modules

		template<typename... Components>
		void load()
		{
            std::lock_guard lock(_module_mtx);
			auto insert = [this]<typename Component>()
			{
#				if D2_COMPATIBILITY_MODE == STRICT
					static_assert(!std::is_same_v<Component, void>, "Attempt to load invalid component");
                    _insert_component(std::make_unique<Component>(
                        _weak(),
                        std::string(Component::name),
                        Component::tsafe
                    ));
#				else
					if constexpr (!std::is_same_v<Component, void>)
                        _insert_component(std::make_unique<Component>(
                            _weak(),
                            std::string(Component::name),
                            Component::tsafe
                        ));
#				endif
			};
			(insert.template operator()<Components>(), ...);
		}

        std::size_t syscnt() const;
        void sysenum(std::function<void(sys::SystemComponent*)> callback);

        template<typename Component> void sys_run(
            auto&& callback,
            auto&& fallback = [](sys::SystemComponent::Status) {}
        )
        {
            if (auto* ptr = sys_if<Component>();
                ptr && ptr->status() == sys::SystemComponent::Status::Ok)
                sync_if(callback, ptr);
            else
                fallback(
                    ptr ?
                        ptr->status() :
                        sys::SystemComponent::Status::Offline
                );
        }
		template<typename Component> auto* sys()
		{
            std::shared_lock lock(_module_mtx);
			return _get_component<Component>();
		}
		template<typename Component> auto* sys_if()
		{
            std::shared_lock lock(_module_mtx);
            auto* ptr = _get_component_if<Component>();
            return ptr ?
                (ptr->status() == sys::SystemComponent::Status::Ok ?
                    ptr : nullptr) :
                nullptr;
		}
		auto* input()
		{
            std::shared_lock lock(_module_mtx);
			return _get_component<sys::SystemInput>();
		}
		auto* output()
		{
            std::shared_lock lock(_module_mtx);
            return _get_component<sys::SystemOutput>();
		}
	};
} // d2

#endif // D2_IO_HANDLER_HPP
