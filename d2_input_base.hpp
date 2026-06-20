#pragma once

#include <absl/container/inlined_vector.h>
#include <array>
#include <bitset>
#include <memory>
#include <memory_resource>
#include <string>
#include <thread>

namespace d2::in
{
    using mouse_position = std::pair<int, int>;
    using screen_size = std::pair<std::size_t, std::size_t>;
    using keytype = short;

    enum class Mode
    {
        Press,
        Release,
        Hold
    };
    enum class Special : keytype
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
    enum class Mouse : keytype
    {
        Left = keytype(Special::SpecialKeyMax) + 1,
        Right,
        Middle,
        SideTop,
        SideBottom,
        MouseKeyMax
    };
    enum class Event : unsigned char
    {
        ScreenResize = 1 << 1,
        ScreenCapacityChange = 1 << 2,
        MouseMovement = 1 << 3,
        ScrollWheelMovement = 1 << 4,
        KeyInput = 1 << 5,
        KeyMouseInput = 1 << 6,
        KeySequenceInput = 1 << 7,
    };

    using mode = Mode;
    using mouse = Mouse;
    using special = Special;

    constexpr keytype keymin()
    {
        return ' ';
    }
    constexpr keytype keymax()
    {
        return '~';
    }
    constexpr keytype key(char ch)
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
        return std::size_t(Special::Reserved);
    }
    constexpr keytype fn(std::size_t cnt)
    {
        return keytype(Special::Fn) + cnt;
    }

    namespace internal
    {
        struct InputFrameView;
    }

    class InputFrame
    {
    public:
        using keymap = std::bitset<std::size_t(Mouse::MouseKeyMax)>;
        using ptr = std::shared_ptr<InputFrame>;

        template<typename Type> struct ExportKeymap
        {
            Type current{};
            Type previous{};
        };

        struct ActiveList
        {
        private:
            std::array<std::pair<keytype, mode>, 8> _active_keys_cache_buf_storage{};
            std::pmr::monotonic_buffer_resource _active_keys_cache_buf{
                _active_keys_cache_buf_storage.data(), _active_keys_cache_buf_storage.size()
            };
            std::pmr::vector<std::pair<keytype, mode>> _active_keys_cache{&_active_keys_cache_buf};
        };
    private:
        struct ConsumeState
        {
            keymap keys_consumed{};
            bool consumed_scroll{false};
            bool consumed_sequence{false};
        };
    private:
        // Consumed
        // input press/release/hold
        // scroll delta
        // sequence

        std::thread::id _consume_ctx{};
        bool _consume{false};
        bool _poll_index{false};

        ConsumeState _active_consume{};
        ConsumeState _consume_swap{};

        unsigned char _event_state{0x00};

        screen_size _screen_capacity{-1, -1};
        screen_size _screen_size{-1, -1};
        mouse_position _mouse_pos{0, 0};
        mouse_position _scroll_delta{0, 0};

        // c - current, p - previous
        // c for hold
        // explicit press/release maps for transitions
        // auto release is used for pulse inputs

        std::array<keymap, 2> _keys_poll{};
        keymap _pulse{};

        std::string _sequence{""};

        std::size_t _cur_poll_idx() const;
        std::size_t _prev_poll_idx() const;
    public:
        static auto make()
        {
            return std::make_shared<InputFrame>();
        }

        friend class internal::InputFrameView;

        InputFrame() = default;
        InputFrame(const InputFrame&) = default;
        InputFrame(InputFrame&&) = default;

        bool had_event(Event ev) const;
        bool had_pulse() const;

        mouse_position scroll_delta();
        mouse_position mouse_position();
        screen_size screen_capacity();
        screen_size screen_size();

        absl::InlinedVector<std::pair<keytype, mode>, 8> active_list() const;

        bool active(Mouse mouse, Mode mode);
        bool active(Special mod, Mode mode);
        bool active(keytype key, Mode mode);

        ExportKeymap<keymap> map() const noexcept;

        std::string_view sequence();

        void mask();

        InputFrame& operator=(const InputFrame&) = default;
        InputFrame& operator=(InputFrame&&) = default;
    };

    namespace internal
    {
        struct InputFrameView
        {
        private:
            InputFrame* _ptr{nullptr};

            InputFrameView(InputFrame* ptr) : _ptr(ptr) {}
        public:
            static auto from(InputFrame* ptr)
            {
                return InputFrameView(ptr);
            }

            InputFrameView(const InputFrameView&) = default;
            InputFrameView(InputFrameView&&) = default;

            void swap();

            void set(Mouse mouse, bool value = true);
            void set(Special mod, bool value = true);
            void set(keytype key, bool value = true);

            void pulse(Mouse mouse);
            void pulse(Special mod);
            void pulse(keytype key);

            void set_scroll_delta(mouse_position pos);
            void set_mouse_position(mouse_position pos);
            void set_screen_size(screen_size size);
            void set_screen_capacity(screen_size size);
            void set_text(std::string in);

            std::pair<bool, std::thread::id> get_consume();
            void restore_consume(std::pair<bool, std::thread::id> data);

            void set_consume(bool value, std::thread::id ctx);
            void apply_consume();
            void reset_consume();

            InputFrame::keymap mask_key_consume(InputFrame::keymap mask);

            InputFrameView& operator=(const InputFrameView&) = default;
            InputFrameView& operator=(InputFrameView&&) = default;
        };
    } // namespace internal
} // namespace d2::in
