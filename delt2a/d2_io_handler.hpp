#ifndef D2_IO_HANDLER_HPP
#define D2_IO_HANDLER_HPP

#include <shared_mutex>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>
#include <span>

#include "d2_exceptions.hpp"
#include "d2_screen_frwd.hpp"
#include "d2_pixel.hpp"
#include "mt/thread_pool.hpp"
#include "utils/cmptime.hpp"

namespace d2
{
	class IOContext;
	namespace sys
	{
		namespace impl
		{
			inline std::atomic<std::size_t> component_uid_track = 0;
			inline std::size_t component_uidgen() noexcept
			{
				return component_uid_track++;
			}

			struct ComponentUIDGeneratorBase
			{
				virtual std::size_t uid() const noexcept = 0;
				std::size_t uid() noexcept
				{
					return const_cast<const ComponentUIDGeneratorBase*>(this)->uid();
				}
			};
		}

		class SystemComponent : public impl::ComponentUIDGeneratorBase
		{
		private:

		protected:
			const std::weak_ptr<IOContext> context_{};
		public:
			template<typename Component>
			static std::unique_ptr<Component> make(util::mt::AsynchronousScheduler scheduler)
			{
				return std::make_unique<Component>(std::move(scheduler));
			}

			SystemComponent(std::weak_ptr<IOContext> ptr)
				: context_(ptr) {}
			virtual ~SystemComponent() = default;

			template<typename Type>
			Type* as()
			{
				auto* p = dynamic_cast<Type*>(this);
				if (p == nullptr)
					throw std::logic_error{ "Attempt to access SystemComponent through an invalid object" };
				return p;
			}
		};

		namespace impl
		{
			template<typename Type>
			struct ComponentUIDGenerator : public SystemComponent
			{
				using SystemComponent::SystemComponent;

				static inline const std::size_t uidc = impl::component_uidgen();
				virtual std::size_t uid() const noexcept override
				{
					return uidc;
				}
			};
		}

		// Additional system wrappers (Optional ones)
        namespace ext
        {
            class SystemClipboard : public impl::ComponentUIDGenerator<SystemClipboard>
            {
            public:
                using ComponentUIDGenerator::ComponentUIDGenerator;

                virtual void clear() = 0;
                virtual void copy(const string& value) = 0;
                virtual string paste() = 0;
                virtual bool empty() = 0;
            };
            class SystemAudio : public impl::ComponentUIDGenerator<SystemAudio>
            {
            public:
                enum class Event
                {
                    Activate,
                    Deactivate,
                    Create,
                    Destroy
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

                    void apply(auto&& func, std::size_t channel, float start = 0.0f, float end = 1.f)
                    {
                        const auto channel_size = _data.size() / _format->channels;
                        const auto astart = std::size_t(start * channel_size);
                        const auto aend = std::size_t(end * channel_size);
                        for (std::size_t i = astart; i < aend; i += _format->channels)
                            _data[i + channel] = func(_data[i + channel], float(i + _offset) / _total);
                    }
                    void apply(auto&& func, float start = 0.0f, float end = 1.f)
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

                    const FormatInfo& info() const noexcept
                    {
                        return *_format;
                    }
                    std::size_t size() const noexcept
                    {
                        return _total / _format->channels;
                    }
                    std::size_t chunk() const noexcept
                    {
                        return _data.size() / _format->channels;
                    }

                    float& at(std::size_t idx, std::size_t channel) const noexcept
                    {
                        return _data[(idx * _format->channels) + channel];
                    }
                    float& at(float idx, std::size_t channel) const noexcept
                    {
                        return _data[(idx * _format->channels) + channel];
                    }

                    Stream& operator=(const Stream&) = default;
                    Stream& operator=(Stream&&) = default;
                };
                struct FilterPipeline
                {
                    std::vector<std::function<void(Stream)>> transforms;

                    FilterPipeline() = default;
                    FilterPipeline(std::function<void(Stream)> callback)
                    {
                        transforms.push_back(std::move(callback));
                    }

                    FilterPipeline operator|(std::function<void(Stream)> callback) const noexcept
                    {
                        FilterPipeline pipeline;
                        pipeline.transforms.reserve(4);
                        pipeline.transforms.push_back(std::move(callback));
                        return pipeline;
                    }
                    FilterPipeline& operator|(std::function<void(Stream)> callback) noexcept
                    {
                        transforms.push_back(std::move(callback));
                        return *this;
                    }
                } static inline const signal;
            private:
                struct DeviceData
                {
                    FilterPipeline filter;
                    std::function<void(const DeviceName&, Event)> listener;
                };
                std::array<DeviceData, 2> _device_data{};
            private:
                std::shared_mutex _device_mtx{};
            protected:
                std::unique_lock<std::shared_mutex> _lock_device_write();
                std::shared_lock<std::shared_mutex> _lock_device_read();

                std::size_t _stream_normalize_required(std::span<const unsigned char> data, FormatInfo format);
                void _stream_normalize(std::span<float> out, std::span<const unsigned char> data, FormatInfo format);

                void _run_device_filter(Device dev, Stream stream);
                void _trigger_device_event(Device dev, Event ev, const DeviceName&);
            public:         
                static constexpr std::size_t sample_size(Format format) noexcept
                {
                    switch (format)
                    {
                    case SystemAudio::Format::PCM16: return 16 / 8;
                    case SystemAudio::Format::PCM32: return 32 / 8;
                    case SystemAudio::Format::Float32: return 32 / 8;
                    case SystemAudio::Format::Float64: return 64 / 8;
                    }
                }

                using ComponentUIDGenerator::ComponentUIDGenerator;

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

                void filter(Device dev, std::function<void(Stream)> callback) { filter(dev, FilterPipeline(callback)); }
                void filter(Device dev, FilterPipeline filter);
                void watch(Device dev, std::function<void(const DeviceName&, Event)> callback);
            };
            class SystemNotifications : public impl::ComponentUIDGenerator<SystemNotifications>
            {
            public:
                using ComponentUIDGenerator::ComponentUIDGenerator;

                virtual void notify(const std::string& title, const std::string& content, const std::vector<unsigned char> icon = {}) = 0;
                virtual void remind(std::chrono::system_clock::time_point when, const std::string& title, const std::string& content, const std::vector<unsigned char> icon = {}) = 0;
            };

            class LocalSystemClipboard : public SystemClipboard
            {
            private:
                std::shared_mutex mtx_{};
                string value_{};
            public:
                using SystemClipboard::SystemClipboard;

                virtual void clear() override
                {
                    std::unique_lock lock(mtx_);
                    value_.clear();
                }
                virtual void copy(const string& value) override
                {
                    std::unique_lock lock(mtx_);
                    value_ = value;
                }
                virtual string paste() override
                {
                    std::shared_lock lock(mtx_);
                    return value_;
                }
                virtual bool empty() override
                {
                    std::shared_lock lock(mtx_);
                    return value_.empty();
                }
            };
        }

		// Generic base for system input
        class SystemInput : public impl::ComponentUIDGenerator<SystemInput>
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
            static constexpr char key(value_type ch)
			{
				return _resolve_key(ch);
			}

			using ComponentUIDGenerator::ComponentUIDGenerator;

			void begincycle() noexcept
			{
				_begincycle_impl();
			}
			void endcycle() noexcept
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
		// Stuff regarding encoding (images):
		// Images are encoded by storing a hash (std::size_t) inside the pixels and using an escape sequence to indicate an image
		// As such, an image has to occupy at least (sizeof(std::size_t) / sizeof(PixelBase) + 1) pixels (contiguous)
		// The escape value for images is 0x10
		// Subsequent lines of the image should use 0x11 and also store the hash so they can be skipped as well
        class SystemOutput : public impl::ComponentUIDGenerator<SystemOutput>
		{
		public:
			// Manages an RGBA image resource with 8-bit components
			// This object does not guarantee that the image data will be stored in it
			// I.e. the implementation may convert the image to it's internal representation and remove the raw image data
			class ImageInstance
			{
			public:
				struct Pixel
				{
					unsigned char r{};
					unsigned char g{};
					unsigned char b{};
					unsigned char a{};
				};
				using ptr = std::shared_ptr<ImageInstance>;
				using wptr = std::weak_ptr<ImageInstance>;
				using data = std::vector<Pixel>;
				using id = std::size_t;
			private:
				std::size_t width_{ 0 };
				std::size_t height_{ 0 };
				data data_{};
			public:
				ImageInstance(data data, std::size_t width, std::size_t height) :
					data_(data),
					width_(width),
					height_(height)
				{}
				~ImageInstance() = default;

				std::span<const Pixel> read() const noexcept
				{
					return { data_.begin(), data_.end() };
				}

				id code() const noexcept
				{
					return std::hash<const ImageInstance*>()(this);
				}
				id code() noexcept
				{
					return std::hash<ImageInstance*>()(this);
				}
			};
		public:
			using ComponentUIDGenerator::ComponentUIDGenerator;

			virtual ImageInstance::ptr make_image(ImageInstance::data data, std::size_t width, std::size_t height) = 0;
			virtual void release_image(ImageInstance::ptr ptr) = 0;

			virtual void write(std::span<const Pixel> buffer, std::size_t width, std::size_t height) = 0;
		};

		// Aliases for components

		using input = SystemInput;
		using output = SystemOutput;

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

	class IOContext : public std::enable_shared_from_this<IOContext>
	{
	public:
		template<typename Type>
		using future = util::mt::future<Type>;
		using ptr = std::shared_ptr<IOContext>;
		using wptr = std::weak_ptr<IOContext>;
		enum class Event
		{
			Invalid = -1,
			// Parameters: Screen::ptr
			Resize,
			// Parameters: Screen::ptr
			PreRedraw,
			// Parameters: Screen::ptr
			PostRedraw,
			// Parameters: Screen::ptr
			KeyInput,
			// Parameters: Screen::ptr
			KeySequenceInput,
			// Parameters: Screen::ptr
			MouseInput,
			// Parameters: Screen::ptr
			Update,
		};
	private:
		class EventListenerState : public std::enable_shared_from_this<EventListenerState>
		{
		public:
			using ptr = std::shared_ptr<EventListenerState>;
			using wptr = std::weak_ptr<EventListenerState>;
			enum class State
			{
				Active,
				Muted,
			};
		protected:
			IOContext::wptr _obj{};
		private:
			std::size_t _idx{ ~0ull };
			Event _event{};
			State _state{ State::Active };
		public:
			EventListenerState() = default;
			EventListenerState(EventListenerState&&) = default;
			EventListenerState(const EventListenerState&) = default;
			EventListenerState(IOContext::ptr ptr, std::size_t idx, Event ev) :
				_obj(ptr), _idx(idx), _event(ev)
			{}

			State state() const noexcept
			{
				return _state;
			}
			Event event() const noexcept
			{
				return _event;
			}
			std::size_t index() const noexcept
			{
				return _idx;
			}

			void setstate(State state) noexcept
			{
				_state = state;
			}

			void unmute() noexcept
			{
				auto ptr = _obj.lock();
				if (ptr)
				{
					ptr->_unmute_listener(shared_from_this());
				}
			}
			void mute() noexcept
			{
				auto ptr = _obj.lock();
				if (ptr)
				{
					ptr->_mute_listener(shared_from_this());
				}
			}
			void destroy() noexcept
			{
				auto ptr = _obj.lock();
				if (ptr)
				{
					ptr->_destroy_listener(shared_from_this());
					_idx = ~0ull;
				}
			}

			operator bool() const noexcept
			{
				return !_obj.expired() && _idx != ~0ull;
			}

			bool operator==(std::nullptr_t) const noexcept
			{
				return !bool();
			}
			bool operator!=(std::nullptr_t) const noexcept
			{
				return bool();
			}

			EventListenerState& operator=(EventListenerState&&) = default;
			EventListenerState& operator=(const EventListenerState&) = default;
		};
	public:
		class EventListener
		{
		private:
			EventListenerState::wptr _ptr{};
		public:
			EventListener() = default;
			EventListener(EventListener&&) = default;
			EventListener(const EventListener&) = default;
			EventListener(EventListenerState::ptr ptr)
				: _ptr(ptr) {}

			Event event() const noexcept
			{
				if (_ptr.expired())
					return Event::Invalid;
				return _ptr.lock()->event();
			}
			std::size_t index() const noexcept
			{
				if (_ptr.expired())
					return ~0ull;
				return _ptr.lock()->index();
			}

			void unmute() noexcept
			{
				if (!_ptr.expired())
				{
					_ptr.lock()->unmute();
				}
			}
			void mute() noexcept
			{
				if (!_ptr.expired())
				{
					_ptr.lock()->mute();
				}
			}
			void destroy() noexcept
			{
				if (!_ptr.expired())
				{
					_ptr.lock()->destroy();
				}
			}
            bool is_muted() const noexcept
            {
                return !_ptr.expired() && _ptr.lock()->state() == EventListenerState::State::Muted;
            }

			operator bool() const noexcept
			{
				return !_ptr.expired();
			}

			bool operator==(std::nullptr_t) const noexcept
			{
                return _ptr.expired();
			}
			bool operator!=(std::nullptr_t) const noexcept
			{
                return !_ptr.expired();
			}

			EventListener& operator=(EventListener&&) = default;
			EventListener& operator=(const EventListener&) = default;
		};
	private:
		template<typename... Argv>
		class ConcreteEventListenerState : public EventListenerState
		{
		public:
            using callback = std::function<void(EventListener, Argv...)>;
		private:
			callback _func{ nullptr };
		public:
			template<typename Func>
			ConcreteEventListenerState(IOContext::ptr ptr, std::size_t idx, Event ev, Func callback)
				: EventListenerState(ptr, idx, ev), _func(std::forward<Func>(callback)) {}

			template<typename... Argvv>
			void invoke(Argvv&&... args)
			{
				if (_func != nullptr && state() == State::Active)
                    _func(EventListener(shared_from_this()), std::forward<Argvv>(args)...);
			}
		};
	private:
		template<Event Ev, typename... Argv>
		using ev_state = std::vector<std::shared_ptr<ConcreteEventListenerState<Argv...>>>;
		using event_state_map = util::PackInfo<
			ev_state<Event::Resize, TreeState::ptr>,
			ev_state<Event::PreRedraw, TreeState::ptr>,
			ev_state<Event::PostRedraw, TreeState::ptr>,
			ev_state<Event::KeyInput, TreeState::ptr>,
			ev_state<Event::KeySequenceInput, TreeState::ptr>,
			ev_state<Event::MouseInput, TreeState::ptr>,
			ev_state<Event::Update, TreeState::ptr>
		>;
		template<Event Ev>
		using event_state = event_state_map::At<Ev>::value_type::element_type;
		template<Event Ev>
		using event_callback = event_state<Ev>::callback;
	private:
		mutable std::recursive_mutex mtx_{};
		std::vector<std::unique_ptr<sys::SystemComponent>> components_{};
		util::mt::AsynchronousScheduler scheduler_{ nullptr };
		util::EnumStateMap<Event, event_state_map> event_states_{};

		void _unmute_listener(EventListenerState::ptr listener) noexcept
		{
			std::lock_guard lock(mtx_);
			event_states_.apply(listener->event(), [&](auto& state) {
				D2_ASSERT(listener->index() < state.size());
				state[listener->index()]->setstate(EventListenerState::State::Active);
			});
		}
		void _mute_listener(EventListenerState::ptr listener) noexcept
		{
			std::lock_guard lock(mtx_);
			event_states_.apply(listener->event(), [&](auto& state) {
				D2_ASSERT(listener->index() < state.size());
				state[listener->index()]->setstate(EventListenerState::State::Muted);
			});
		}
		void _destroy_listener(EventListenerState::ptr listener) noexcept
		{
			std::lock_guard lock(mtx_);
			event_states_.apply(listener->event(), [&](auto& state) {
				D2_ASSERT(listener->index() < state.size());
				state[listener->index()] = nullptr;
				for (auto it = state.begin(); it != state.end();)
				{
					const auto beg = it;
					for (; it != state.end(); ++it)
					{
						if (*it != nullptr)
							break;
					}
					if (it == state.end())
					{
						state.erase(
							beg, state.end()
						);
						return;
					}
					else ++it;
				}
			});
		}

		template<typename Type> Type* _get_component()
		{
			const auto uid = Type::uidc;
			[[ unlikely ]] if (components_.size() < uid)
				throw std::logic_error{ "Attempt to query inactive component" };
			return static_cast<Type*>(components_[uid].get());
		}
		template<typename Type> Type* _get_component_if()
		{
			const auto uid = Type::uidc;
			[[ unlikely ]] if (components_.size() < uid)
				return nullptr;
			return static_cast<Type*>(components_[uid].get());
		}
		void _insert_component(std::unique_ptr<sys::SystemComponent> ptr)
		{
			const auto idx = ptr->uid();
			components_.resize(std::max(components_.size(), idx + 1));
			components_[idx] = std::move(ptr);
		}
	public:
		template<typename... Components> static ptr make(
			std::size_t threads = 0,
			bool auto_grow = true,
			bool auto_shrink = false
		)
		{
			using s = util::mt::AsynchronousScheduler;
			auto scheduler = util::mt::AsynchronousScheduler::from(
				s::Config{
					.maximum_idle = std::chrono::seconds(15),
					.resize_increment = 1,
					.minimum_threads = threads
				},
				(s::Flags::AutoGrow * auto_grow) |
				(s::Flags::AutoShrink * auto_shrink) |
				s::Flags::ManualCyclicThread
			);
			auto ptr = std::make_shared<IOContext>(scheduler);
			ptr->load<Components...>();
			return ptr;
		}

		template<typename... Components>
		IOContext(util::mt::AsynchronousScheduler scheduler)
			: scheduler_(scheduler) { }
		virtual ~IOContext() = default;

		auto& scheduler() noexcept
		{
			return scheduler_;
		}

		template<typename... Components>
		void load()
		{
			std::lock_guard lock(mtx_);
			auto insert = [this]<typename Component>()
			{
#				if D2_COMPATIBILITY_MODE == STRICT
					static_assert(!std::is_same_v<Component, void>, "Attempt to load invalid component");
					_insert_component(std::make_unique<Component>(weak_from_this()));
#				else
					if constexpr (!std::is_same_v<Component, void>)
						_insert_component(std::make_unique<Component>(weak_from_this()));
#				endif
			};
			(insert.template operator()<Components>(), ...);
		}

		std::size_t syscnt() const noexcept
		{
			std::lock_guard lock(mtx_);
			std::size_t cnt = 0;
			for (decltype(auto) it : components_)
				cnt += (it != nullptr);
			return cnt;
		}

		template<typename Component> auto* sys()
		{
			return _get_component<Component>();
		}
		template<typename Component> auto* sys_if()
		{
			return _get_component_if<Component>();
		}
		auto* input()
		{
			return _get_component<sys::SystemInput>();
		}
		auto* output()
		{
			std::lock_guard lock(mtx_);
			return _get_component<sys::SystemOutput>();
		}

		template<Event Ev, typename... Argv>
		void trigger(Argv&&... args)
		{
			std::lock_guard lock(mtx_);
			auto ptr = shared_from_this();
			auto& v = event_states_.state<Ev>();
			for (std::size_t i = 0; i < v.size(); i++)
			{
				auto c = v[i];
				if (c != nullptr)
				{
					c->invoke(std::forward<Argv>(args)...);
				}
			}
		}

		template<Event Ev>
		EventListener listen(event_callback<Ev> callback)
		{
			std::lock_guard lock(mtx_);
			auto& v = event_states_.state<Ev>();
			v.push_back(
				std::make_shared<event_state<Ev>>(
					shared_from_this(),
					v.size(),
					Ev,
					std::move(callback)
				)
			);
			return EventListener{ v.back() };
		}
	};
} // d2

#endif // D2_IO_HANDLER_HPP
