#ifndef D2_MODS_CORE_HPP
#define D2_MODS_CORE_HPP

#include <d2_input_base.hpp>
#include <d2_main_worker.hpp>
#include <d2_module.hpp>
#include <d2_pixel.hpp>
#include <mt/pool.hpp>

namespace d2::sys
{
    // Generic base for system input
    class SystemInput : public AbstractModule<"Input">
    {
    private:
        struct FrameLock
        {
        private:
            SystemInput* _ptr;
            std::pair<bool, std::thread::id> _consume_state{};
            bool _mask{};
        public:
            FrameLock(FrameLock&& copy);
            FrameLock(const FrameLock&) = delete;
            FrameLock(SystemInput* ptr, bool consume_mask = false);
            ~FrameLock();

            FrameLock& operator=(FrameLock&& copy);
            FrameLock& operator=(const FrameLock&) = delete;
        };
    public:
        struct FramePtr
        {
        private:
            in::InputFrame::ptr _ptr{nullptr};
            FrameLock _lock{nullptr};
        public:
            FramePtr(in::InputFrame::ptr ptr, FrameLock lock);
            FramePtr(FramePtr&&) = default;
            FramePtr(const FramePtr&) = delete;

            in::InputFrame* get() const;

            in::InputFrame& operator*() const;
            in::InputFrame* operator->() const;

            FramePtr& operator=(FramePtr&&) = default;
            FramePtr& operator=(const FramePtr&) = delete;
        };
    private:
        std::atomic<bool> _enabled{true};
    protected:
        virtual void _lock_frame_impl() {}
        virtual void _unlock_frame_impl() {}
        virtual in::InputFrame::ptr _frame_impl() = 0;
        virtual void _begincycle_impl() = 0;
        virtual void _endcycle_impl() = 0;
        virtual MainWorker::ptr _worker_impl() = 0;
    public:
        using AbstractModule::AbstractModule;

        FramePtr frame();
        MainWorker::ptr worker();

        void begincycle();
        void endcycle();

        void enable();
        void disable();

        bool had_event(in::Event ev);

        in::mouse_position mouse_position();
        in::screen_size screen_capacity();
        in::screen_size screen_size();

        bool active(in::Mouse mouse, in::Mode mode);
        bool active(in::Special mod, in::Mode mode);
        bool active(in::keytype key, in::Mode mode);

        std::string_view sequence();
    };

    // Generic base for system output
    // Provides interfaces used for rendering to the console and other OS agnostic interfaces
    class SystemOutput : public AbstractModule<"Output">
    {
    public:
        static constexpr auto image_constant =
            px::combined{.v = std::numeric_limits<standard_value_type>::max()};
        class ImageInstance
        {
        public:
            using data = std::vector<unsigned char>;
        private:
            unsigned char format_{0x00};
            std::size_t width_{0};
            std::size_t height_{0};
            data data_{};
        public:
            ImageInstance(
                data data, std::size_t width, std::size_t height, unsigned char format = 0x00
            ) : data_(data), width_(width), height_(height), format_(format)
            {
            }
            ~ImageInstance() = default;

            std::span<const unsigned char> read() const
            {
                return {data_.begin(), data_.end()};
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
        using AbstractModule::AbstractModule;

        virtual void load_image(const std::string& path, ImageInstance img) = 0;
        virtual void release_image(const std::string& path) = 0;
        virtual ImageInstance image_info(const std::string& path) = 0;
        virtual image image_id(const std::string& path) = 0;

        virtual std::size_t delta_size() = 0;
        virtual std::size_t swapframe_size() = 0;

        virtual void
        write(std::span<const Pixel> buffer, std::size_t width, std::size_t height) = 0;
    };

    // Generic base class for the main screen
    // Handles trees, themes, the main loop, and a bunch of other stuff
    class SystemScreen;

    // Aliases for components

    using input = SystemInput;
    using output = SystemOutput;
    using screen = SystemScreen;
} // namespace d2::sys

#endif
