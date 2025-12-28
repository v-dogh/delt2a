#ifndef D2_FRAGMENTED_BUFFER_HPP
#define D2_FRAGMENTED_BUFFER_HPP

#include <functional>
#include <optional>
#include <variant>
#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <span>

namespace d2::fb
{
    struct Locale
    {
        struct Value
        {
        private:
            std::string_view _data{ "" };
            std::size_t _width{ 1 };
            std::size_t _height{ 1 };
        public:
            Value() = default;
            Value(std::string_view str, std::size_t width, std::size_t height) :
                _data(str), _width(width), _height(height) {}
            Value(const Value&) = default;
            Value(Value&&) = default;

            std::string_view value() const;

            std::size_t width() const;
            std::size_t height() const;

            Value& operator=(const Value&) = default;
            Value& operator=(Value&&) = default;
        };

        virtual ~Locale() = default;
        virtual void source(std::string_view str) = 0;
        virtual void jump(std::size_t off) = 0;
        virtual void seek_last() = 0;
        virtual void seek(int pos) = 0;
        virtual Value value() = 0;
        virtual std::size_t offset() const = 0;
        virtual bool is_end() const = 0;
        virtual bool is_endline() const = 0;
        virtual void iterate(const std::function<bool(Value)>& callback) const = 0;
    };
    namespace loc
    {
        struct Byte : public Locale
        {
        private:
            std::string_view _src{};
            std::size_t _off{ 0 };
        public:
            virtual void source(std::string_view str) override;
            virtual void jump(std::size_t off) override;
            virtual void seek_last() override;
            virtual void seek(int pos) override;
            virtual Locale::Value value() override;
            virtual std::size_t offset() const override;
            virtual bool is_end() const override;
            virtual bool is_endline() const override;
            virtual void iterate(const std::function<bool(Value)>& callback) const override;
        };
    }

    class Shard
    {
    private:
        std::variant<std::string, std::string_view, std::nullopt_t> _data{ std::nullopt };
    public:
        Shard(std::string_view view);
        Shard(std::string str);
        Shard(std::size_t block);
        Shard(const Shard&) = default;
        Shard(Shard&&) = default;

        bool is_view() const;

        std::string_view value() const;
        const std::string& buffer() const;
        std::string& buffer();

        bool operator==(std::nullptr_t) const;

        Shard& operator=(const Shard&) = default;
        Shard& operator=(Shard&&) = default;
    };
    class FragmentedBuffer
    {
    public:
        enum class It
        {
            Break,
            BreakLine,
            Continue
        };
    private:
        struct Namespace
        {
        protected:
            FragmentedBuffer* _buf{ nullptr };
        public:
            Namespace(FragmentedBuffer* buf) : _buf(buf) {}
        };
        struct Action
        {
        public:
            enum class Type
            {
                Apply,
                Revert,
                Delete,
                Merge,
            };
        private:
            void* _data{ nullptr };
            bool(*_callback)(FragmentedBuffer*, void*, void*, Type){ nullptr };
        public:
            template<typename... Argv>
            static auto from(bool(*callback)(FragmentedBuffer*, void*, void*, Type),
                             Argv&&... args)
            {
                return Action(
                    callback,
                    new std::tuple<Argv...>(std::forward<Argv>(args)...)
                    );
            }
            template<typename Type>
            static auto from(bool(*callback)(FragmentedBuffer*, void*, void*, Type),
                             Type* ptr)
            {
                return Action(
                    callback,
                    ptr
                    );
            }

            template<typename State>
            static auto* alloc_state(State value, std::size_t len = 0)
            {
                auto* buffer = ::operator new[](len, std::align_val_t(alignof(State)));
                new (buffer) State{ std::move(value) };
                return buffer;
            }
            template<typename State, typename Type>
            static auto* alloc_state(State value, std::span<Type> data)
                requires std::is_trivial_v<Type>
            {
                auto* buffer = alloc_state<State>(std::move(value), data.size() * sizeof(Type));
                std::memcpy(
                    static_cast<unsigned char*>(buffer) + sizeof(State),
                    data.data(),
                    data.size()
                    );
                return buffer;
            }

            template<typename State, typename Buffer = unsigned char>
            static std::pair<State*, Buffer*> get_state(void* ptr)
            {
                return {
                    static_cast<State*>(ptr),
                    reinterpret_cast<Buffer*>(static_cast<unsigned char*>(ptr) + sizeof(State))
                };
            }
            template<typename State>
            static void delete_state(void* ptr)
            {
                static_cast<State*>(ptr)->~State();
                ::operator delete[](ptr);
            }

            Action(
                bool(*callback)(FragmentedBuffer*, void*, void*, Type),
                void* data = nullptr
            );
            Action(const Action&) = delete;
            Action(Action&& copy);
            ~Action();

            void apply(FragmentedBuffer* buf);
            void revert(FragmentedBuffer* buf);

            bool merge(Action& action);

            Action& operator=(const Action&) = delete;
            Action& operator=(Action&&) = default;
        };
        struct PtrState
        {
            //
            // abs - the absolute offset of the ptr from the zero-eth shard (in bytes)
            // rel - the offset of the ptr in the shard in which it is (in bytes)
            // line_off - the offset of the ptr in its current line (in bytes)
            // line_cnt - the line number since the beginning (starts from 0)
            //

            std::size_t abs{ 0 };
            std::size_t rel{ 0 };
            std::size_t line_off{ 0 };
            std::size_t prev_line_off{ 0 };
        };
        static constexpr auto _block_size{ 128 };
    private:
        std::unique_ptr<Locale> _locale{ nullptr };
        std::vector<Action> _history{};
        std::list<Shard> _shards{};
        std::list<Shard>::iterator _shard{ _shards.end() };
        std::string _source{ "" };
        std::size_t _history_offset{ 0 };
        PtrState _ptr{};

        // Access

        void _iterate(const std::function<It(Locale::Value)>& callback, std::size_t pos) const;
        void _iterate_ptr(const std::function<It(Locale::Value)>& callback) const;

        // Pointer Management

        void _move_left(std::size_t off);
        void _move_right(std::size_t off);
        void _move_up(std::size_t off);
        void _move_down(std::size_t off);
        void _jump_char(std::size_t pos);

        // History

        void _push_action(Action action);
        void _undo_action();
        void _redo_action();

        // Setup

        void _set_locale(std::unique_ptr<Locale> locale);
        void _setup_buffer(std::string value);
        void _setup_view(std::string_view value);

        // Actions

        void _split_shard();
        void _insert_data(std::string_view data);

        void _action_insert(std::string_view data);
        void _action_erase(std::size_t len);
    public:
        FragmentedBuffer() = default;
        FragmentedBuffer(const FragmentedBuffer&) = delete;
        FragmentedBuffer(FragmentedBuffer&&) = default;

        // Fun class-level namespacing???

        struct : Namespace
        {
            template<typename Type>
            void locale() { _buf->_set_locale(std::make_unique<Type>()); }

            void setup_buffer(std::string value) { _buf->_setup_buffer(value); }
            void setup_view(std::string_view value) { _buf->_setup_view(value); }

            void iterate(const std::function<It(Locale::Value)>& callback, std::size_t pos = 0) const
            { _buf->_iterate(callback, pos); }
            void iterate_ptr(const std::function<It(Locale::Value)>& callback) const
            { _buf->_iterate_ptr(callback); }
        } buf{ this };
        struct : Namespace
        {
            void jump(std::size_t pos) { _buf->_jump_char(pos); }
            void left(std::size_t off = 1) { _buf->_move_left(off); }
            void right(std::size_t off = 1) { _buf->_move_right(off); }
            void up(std::size_t off = 1) { _buf->_move_up(off); }
            void down(std::size_t off = 1) { _buf->_move_down(off); }

            std::size_t line_offset() const { return _buf->_ptr.line_off; }
        } ptr{ this };
        struct : Namespace
        {
            void insert(std::string_view data) { _buf->_action_insert(data); }
            void insert(std::size_t pos, std::string_view data)
            {
                const auto ptr = _buf->_ptr;
                _buf->_jump_char(pos);
                _buf->_action_insert(data);
                _buf->_ptr = ptr;
            }
            void erase(std::size_t len) { _buf->_action_erase(len); }
            void erase(std::size_t pos, std::size_t len)
            {
                const auto ptr = _buf->_ptr;
                _buf->_jump_char(pos);
                _buf->_action_erase(len);
                _buf->_ptr = ptr;
            }
        } act{ this };
        struct : Namespace
        {
            void reapply() { _buf->_redo_action(); }
            void revert() { _buf->_undo_action(); }
        } history{ this };

        FragmentedBuffer& operator=(const FragmentedBuffer&) = delete;
        FragmentedBuffer& operator=(FragmentedBuffer&&) = default;
    };
}

#endif // D2_FRAGMENTED_BUFFER_HPP
