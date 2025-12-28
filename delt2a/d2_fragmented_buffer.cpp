#include "d2_fragmented_buffer.hpp"

namespace d2::fb
{
    // Locale

    std::string_view Locale::Value::value() const
    {
        return _data;
    }

    std::size_t Locale::Value::width() const
    {
        return _width;
    }
    std::size_t Locale::Value::height() const
    {
        return _height;
    }

    namespace loc
    {
        void Byte::source(std::string_view str)
        {
            _src = str;
            _off = 0;
        }
        void Byte::jump(std::size_t off)
        {
            _off = off;
        }
        void Byte::seek_last()
        {
            _off = _src.size() - 1;
        }
        void Byte::seek(int pos)
        {
            _off += pos;
        }
        Locale::Value Byte::value()
        {
            return { _src.substr(_off, 1), 1, 1 };
        }
        std::size_t Byte::offset() const
        {
            return _off;
        }
        bool Byte::is_end() const
        {
            return _off >= _src.size();
        }
        bool Byte::is_endline() const
        {
            return _src[_off] == '\n';
        }
        void Byte::iterate(const std::function<bool(Value)>& callback) const
        {
            for (decltype(auto) it : _src)
                if (!callback(Value(std::string_view(&it, 1), 1, 1)))
                    break;
        }
    }

    // Shard

    Shard::Shard(std::string_view view) : _data(view) {}
    Shard::Shard(std::string str) : _data(str) {}
    Shard::Shard(std::size_t block)
        : _data(std::string())
    { std::get<std::string>(_data).reserve(block); }

    bool Shard::is_view() const
    {
        return std::holds_alternative<std::string_view>(_data);
    }

    std::string_view Shard::value() const
    {
        return std::holds_alternative<std::string>(_data) ?
                   std::get<std::string>(_data) : std::get<std::string_view>(_data);
    }
    const std::string& Shard::buffer() const
    {
        return std::get<std::string>(_data);
    }
    std::string& Shard::buffer()
    {
        return std::get<std::string>(_data);
    }

    bool Shard::operator==(std::nullptr_t) const
    {
        return std::holds_alternative<std::nullopt_t>(_data);
    }

    // Action

    FragmentedBuffer::Action::Action(
        bool(*callback)(FragmentedBuffer*, void*, void*, Type),
        void* data) :
        _callback(callback),
        _data(data)
    {}
    FragmentedBuffer::Action::Action(Action&& copy) :
        _data(copy._data),
        _callback(copy._callback)
    {
        copy._data = nullptr;
    }
    FragmentedBuffer::Action::~Action()
    {
        if (_data)
            _callback(nullptr, _data, nullptr, Type::Delete);
    }

    void FragmentedBuffer::Action::apply(FragmentedBuffer* buf)
    {
        _callback(buf, _data, nullptr, Type::Apply);
    }
    void FragmentedBuffer::Action::revert(FragmentedBuffer* buf)
    {
        _callback(buf, _data, nullptr, Type::Revert);
    }

    bool FragmentedBuffer::Action::merge(Action& action)
    {
        if (_callback(nullptr, _data, &action, Type::Merge))
        {
            _callback(nullptr, _data, nullptr, Type::Delete);
            _data = nullptr;
            return true;
        }
        return false;
    }

    void FragmentedBuffer::_iterate(const std::function<It(Locale::Value)>& callback, std::size_t pos) const
    {
        bool end = false;
        for (decltype(auto) it : _shards)
        {
            _locale->source(it.value());
            _locale->iterate([&](Locale::Value value) {
                callback(value);
                return true;
            });
        }
    }
    void FragmentedBuffer::_iterate_ptr(const std::function<It(Locale::Value)>& callback) const
    {
        if (_shards.empty())
            return;

        auto it = _shard;
        _locale->source(it->value().substr(_ptr.rel));
        _locale->iterate([&](Locale::Value value) {
            callback(value);
            return true;
        });
        ++it;

        if (it != _shards.end())
            for (; it != _shards.end(); ++it)
            {
                _locale->source(it->value());
                _locale->iterate([&](Locale::Value value) {
                    callback(value);
                    return true;
                });
            }
    }

    // Fragmented Buffer

    // Pointer management

    void FragmentedBuffer::_move_left(std::size_t off)
    {
        while (off--)
        {
            if (_ptr.rel == 0)
            {
                if (_shard == _shards.begin()) return;
                --_shard;
                _ptr.rel = _shard->value().size() - 1;
            }
            else
            {
                _locale->source(_shard->value());
                _locale->jump(_ptr.rel);
                _locale->seek(-1);
                _ptr.abs -= _ptr.rel - _locale->offset();
                _ptr.rel = _locale->offset();
            }

            if (_locale->is_endline())
            {
                _ptr.line_off = 0;
            }
            else
                _ptr.line_off--;
        }
    }
    void FragmentedBuffer::_move_right(std::size_t off)
    {
        if (_shard->value().empty()) return;
        while (off--)
        {
            if (_ptr.rel < _shard->value().size())
            {
                _locale->source(_shard->value());
                _locale->jump(_ptr.rel);
                if (_locale->is_endline())
                {
                    _ptr.line_off = 0;
                }
                else
                    _ptr.line_off++;

                _locale->seek(1);
                _ptr.abs += _locale->offset() - _ptr.rel;
                _ptr.rel = _locale->offset();
            }
            if (_ptr.rel >= _shard->value().size())
            {
                _ptr.rel = 0;
                if (++_shard == _shards.end())
                {
                    _shard = _shards.insert(_shard, Shard(_block_size));
                    return;
                }
            }
        }
    }
    void FragmentedBuffer::_move_up(std::size_t off)
    {
        while (off--)
        {
            if (_shard->value()[_ptr.line_off - 1] != '\n')
                _ptr.prev_line_off = _ptr.line_off;

            _locale->source(_shard->value());
            _locale->seek(_ptr.rel);

            bool end = false;
            do
            {
                while (_locale->offset())
                {
                    _locale->seek(-1);
                    if (_locale->is_endline())
                    {
                        end = true;
                        _ptr.line_off = 0;
                        break;
                    }
                    _ptr.line_off--;
                }

                if (_shard != _shards.begin())
                {
                    _shard = std::prev(_shard);
                    _locale->source(_shard->value());
                    _locale->seek_last();
                }
                _ptr.rel = _locale->offset();
            } while (!end && _shard != _shards.begin());


        }
    }
    void FragmentedBuffer::_move_down(std::size_t off)
    {
        while (off--)
        {

        }
    }
    void FragmentedBuffer::_jump_char(std::size_t pos)
    {

    }

    // History

    void FragmentedBuffer::_push_action(Action action)
    {
        action.apply(this);
        if (_history.empty() ||
            !_history.back().merge(action))
        {
            _history.push_back(std::move(action));
        }
    }
    void FragmentedBuffer::_undo_action()
    {

    }
    void FragmentedBuffer::_redo_action()
    {

    }

    // Setup

    void FragmentedBuffer::_set_locale(std::unique_ptr<Locale> locale)
    {
        _locale = std::move(locale);
    }
    void FragmentedBuffer::_setup_buffer(std::string value)
    {
        _source = std::move(value);
        _setup_view(_source);
    }
    void FragmentedBuffer::_setup_view(std::string_view value)
    {
        _history.clear();
        _shards.clear();
        _shards.push_back(value);
        _shard = _shards.begin();
    }

    // Actions

    void FragmentedBuffer::_split_shard()
    {
        const auto s = _shard->value();
        if (_ptr.rel == 0)
        {
            _shard = _shards.insert(
                _shard,
                Shard(_block_size)
                );
        }
        else if (_ptr.rel == s.size() - 1)
        {
            _shard = _shards.insert(
                std::next(_shard),
                Shard(_block_size)
                );
            _ptr.rel = 0;
        }
        else
        {
            *_shard = Shard(_block_size);
            _shards.insert(_shard, s.substr(0, _ptr.rel));
            _shards.insert(std::next(_shard), s.substr(_ptr.rel));
            _ptr.rel = 0;
        }
    }
    void FragmentedBuffer::_insert_data(std::string_view data)
    {
        if (_shard->is_view()) _split_shard();
        if (_ptr.rel == 0)
        {
            if (_shard != _shards.begin())
            {
                const auto p = std::prev(_shard);
                if (p->value().size() < _block_size && !p->is_view())
                {
                    _shard = std::prev(_shard);
                    _ptr.rel = _shard->value().size();
                }
            }
        }

        // Create new shards and write data until end
        std::size_t rem = data.size();
        while (true)
        {
            const auto pos = data.size() - rem;
            const auto block = std::min<std::size_t>(
                rem,
                _block_size - _shard->value().size()
                );

            if (block)
            {
                _shard->buffer().insert(
                    _ptr.rel,
                    data.substr(pos, block)
                    );
                _ptr.rel += block;
            }
            if ((rem -= block) != 0)
            {
                _shards.insert(
                    std::next(_shard),
                    Shard(_block_size)
                    );
                _ptr.rel = 0;
            }
            else
                break;
        }
        if (_ptr.rel == _shard->value().size())
        {
            _shard = std::next(_shard);
            _ptr.rel = 0;
        }

        // Count the endlines

        std::size_t poff = _ptr.line_off;
        std::size_t ppoff = 0;
        bool ledl = false;
        for (std::size_t i = 0; i < data.size(); i++)
        {
            if (data[i] == '\n')
            {
                ppoff = poff;
                poff = 0;
                ledl = true;
            }
            else
                ledl = false;
        }

        _ptr.line_off = ledl ? ppoff : data.size() - poff - 1;
        _ptr.abs += data.size();
    }

    void FragmentedBuffer::_action_insert(std::string_view data)
    {
        struct State
        {
            std::size_t abs{ 0 };
            std::size_t len{ 0 };
        };
        _push_action({
            [](FragmentedBuffer* buf, void* data, void*, Action::Type type) -> bool {
                const auto [ state, raw ] = Action::get_state<State>(data);
                const auto str = std::string_view(
                    reinterpret_cast<const char*>(raw),
                    state->len
                    );
                switch (type)
                {
                case Action::Type::Apply:
                    buf->_jump_char(state->abs);
                    buf->_insert_data(str);
                    return true;
                case Action::Type::Revert:
                    return true;
                case Action::Type::Delete:
                    Action::delete_state<State>(data);
                    return true;
                case Action::Type::Merge:
                    return false;
                }
            },
            Action::alloc_state<State>({
                                           .abs = _ptr.abs,
                                           .len = data.size()
                                       }, std::span(data.begin(), data.end()))
        });
    }
    void FragmentedBuffer::_action_erase(std::size_t len)
    {

    }
}
