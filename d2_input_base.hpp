#ifndef D2_INPUT_BASE_H
#define D2_INPUT_BASE_H

#include <array>
#include <bitset>
#include <memory>
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
    enum class Mouse : keytype
    {
        Left,
        Right,
        Middle,
        SideTop,
        SideBottom,
        MouseKeyMax
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
    enum class Event : unsigned char
    {
        Reserved = 1 << 0,
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
        using keyboard_keymap = std::bitset<std::size_t(Special::SpecialKeyMax) + 1>;
        using mouse_keymap = std::bitset<std::size_t(Special::SpecialKeyMax) + 1>;
        using ptr = std::shared_ptr<InputFrame>;

        template<typename Type> struct ExportKeymap
        {
            Type current{};
            Type previous{};
        };
    private:
        struct ConsumeState
        {
            keyboard_keymap keys_consumed{};
            mouse_keymap mouse_consumed{};
            bool consumed_scroll{false};
            bool consumed_sequence{false};
        };
    private:
        // Consumed
        // mouse press (not hold)
        // key press (not hold)
        // scroll delta

        std::thread::id _consume_ctx{};
        bool _consume{false};

        ConsumeState _active_consume{};
        ConsumeState _consume_swap{};

        unsigned char _event_state{0x00};

        screen_size _screen_capacity{-1, -1};
        screen_size _screen_size{-1, -1};
        mouse_position _mouse_pos{0, 0};
        mouse_position _scroll_delta{0, 0};

        // c - current, p - previous
        // c for hold
        // c and !p for press
        // !c and p for release

        std::array<keyboard_keymap, 2> _keys_poll{};
        std::array<mouse_keymap, 2> _mouse_poll{};

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

        bool had_event(Event ev);

        mouse_position scroll_delta();
        mouse_position mouse_position();
        screen_size screen_capacity();
        screen_size screen_size();

        bool active(Mouse mouse, Mode mode);
        bool active(Special mod, Mode mode);
        bool active(keytype key, Mode mode);

        ExportKeymap<keyboard_keymap> keyboard_map() const noexcept;
        ExportKeymap<mouse_keymap> mouse_map() const noexcept;

        std::string_view sequence();

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
            void swap(const InputFrame* ptr);

            void set(Mouse mouse, bool value = true);
            void set(Special mod, bool value = true);
            void set(keytype key, bool value = true);

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

            InputFrame::keyboard_keymap mask_keyboard_consume(InputFrame::keyboard_keymap mask);
            InputFrame::mouse_keymap mask_mouse_consume(InputFrame::mouse_keymap mask);

            InputFrameView& operator=(const InputFrameView&) = default;
            InputFrameView& operator=(InputFrameView&&) = default;
        };
    } // namespace internal
} // namespace d2::in

#endif // D2_INPUT_BASE_H
