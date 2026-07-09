#include "d2_fragmented_buffer.hpp"

#include <algorithm>

namespace d2::fb
{
    namespace
    {
        std::string flatten(const std::list<Shard>& shards)
        {
            std::size_t size = 0;
            for (const auto& it : shards)
                size += it.value().size();

            std::string out;
            out.reserve(size);
            for (const auto& it : shards)
                out.append(it.value());
            return out;
        }
    } // namespace

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
            _off = std::min(off, _src.size());
        }
        void Byte::seek_last()
        {
            _off = _src.empty() ? 0 : _src.size() - 1;
        }
        void Byte::seek(int pos)
        {
            const auto off = static_cast<std::ptrdiff_t>(_off) + pos;
            _off = static_cast<std::size_t>(std::max<std::ptrdiff_t>(0, off));
            _off = std::min(_off, _src.size());
        }
        Locale::Value Byte::value()
        {
            if (is_end())
                return {{}, 1, 1};
            return {_src.substr(_off, 1), 1, 1};
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
            return !is_end() && _src[_off] == '\n';
        }
        void Byte::iterate(const std::function<bool(Value)>& callback) const
        {
            for (std::size_t i = 0; i < _src.size(); i++)
                if (!callback(Value(_src.substr(i, 1), 1, 1)))
                    break;
        }
    } // namespace loc

    // Shard

    Shard::Shard(std::string_view view) : _data(view) {}
    Shard::Shard(std::string str) : _data(std::move(str)) {}
    Shard::Shard(std::size_t block) : _data(std::string())
    {
        std::get<std::string>(_data).reserve(block);
    }

    bool Shard::is_view() const
    {
        return std::holds_alternative<std::string_view>(_data);
    }

    std::string_view Shard::value() const
    {
        return std::holds_alternative<std::string>(_data) ? std::get<std::string>(_data)
                                                          : std::get<std::string_view>(_data);
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
        bool (*callback)(FragmentedBuffer*, void*, void*, Type), void* data
    ) : _data(data), _callback(callback)
    {
    }
    FragmentedBuffer::Action::Action(Action&& copy) : _data(copy._data), _callback(copy._callback)
    {
        copy._data = nullptr;
        copy._callback = nullptr;
    }
    FragmentedBuffer::Action::~Action()
    {
        if (_data && _callback)
            _callback(nullptr, _data, nullptr, Type::Delete);
    }

    FragmentedBuffer::Action& FragmentedBuffer::Action::operator=(Action&& copy)
    {
        if (this == &copy)
            return *this;

        if (_data && _callback)
            _callback(nullptr, _data, nullptr, Type::Delete);

        _data = copy._data;
        _callback = copy._callback;
        copy._data = nullptr;
        copy._callback = nullptr;
        return *this;
    }

    void FragmentedBuffer::Action::apply(FragmentedBuffer* buf)
    {
        if (_callback)
            _callback(buf, _data, nullptr, Type::Apply);
    }
    void FragmentedBuffer::Action::revert(FragmentedBuffer* buf)
    {
        if (_callback)
            _callback(buf, _data, nullptr, Type::Revert);
    }

    bool FragmentedBuffer::Action::merge(Action& action)
    {
        if (_callback && _callback(nullptr, _data, &action, Type::Merge))
        {
            action._callback = nullptr;
            action._data = nullptr;
            return true;
        }
        return false;
    }

    void
    FragmentedBuffer::_iterate(std::function<It(Locale::Value)> callback, std::size_t pos) const
    {
        std::size_t off = 0;
        bool break_line = false;
        for (const auto& it : _shards)
        {
            _locale->source(it.value());
            _locale->iterate(
                [&](Locale::Value value)
                {
                    if (off++ < pos)
                        return true;
                    if (break_line)
                        return false;

                    switch (callback(value))
                    {
                    case It::Break:
                        return false;
                    case It::BreakLine:
                        break_line = true;
                        return value.value() != "\n";
                    case It::Continue:
                        return true;
                    }
                    return true;
                }
            );
            if (break_line)
                break;
        }
    }
    void FragmentedBuffer::_iterate_ptr(std::function<It(Locale::Value)> callback) const
    {
        _iterate(callback, _ptr.abs);
    }
    void FragmentedBuffer::_iterate_raw(std::function<bool(std::string_view)> callback) const
    {
        for (const auto& it : _shards)
            if (!callback(it.value()))
                break;
    }

    // Fragmented Buffer

    // Pointer management

    void FragmentedBuffer::_move_left(std::size_t off)
    {
        _jump_char(_ptr.abs > off ? _ptr.abs - off : 0);
    }
    void FragmentedBuffer::_move_right(std::size_t off)
    {
        _jump_char(_ptr.abs + off);
    }
    void FragmentedBuffer::_move_up(std::size_t off)
    {
        const auto src = flatten(_shards);
        while (off--)
        {
            if (_ptr.abs == 0)
                break;

            const auto line_end = _ptr.abs - _ptr.line_off;
            if (line_end == 0)
            {
                _jump_char(0);
                break;
            }

            std::size_t prev_end = line_end - 1;
            std::size_t prev_start = prev_end;
            while (prev_start != 0 && src[prev_start - 1] != '\n')
                --prev_start;

            const auto prev_len = prev_end - prev_start;
            _jump_char(prev_start + std::min(_ptr.line_off, prev_len));
        }
    }
    void FragmentedBuffer::_move_down(std::size_t off)
    {
        const auto src = flatten(_shards);
        while (off--)
        {
            std::size_t line_start = _ptr.abs - _ptr.line_off;
            std::size_t line_end = line_start;
            while (line_end < src.size() && src[line_end] != '\n')
                ++line_end;

            if (line_end >= src.size())
            {
                _jump_char(src.size());
                break;
            }

            const std::size_t next_start = line_end + 1;
            std::size_t next_end = next_start;
            while (next_end < src.size() && src[next_end] != '\n')
                ++next_end;

            _jump_char(next_start + std::min(_ptr.line_off, next_end - next_start));
        }
    }
    void FragmentedBuffer::_jump_char(std::size_t pos)
    {
        if (_shards.empty())
        {
            _shards.push_back(Shard(_block_size));
            _shard = _shards.begin();
        }

        const auto total = flatten(_shards).size();
        pos = std::min(pos, total);

        _ptr = {};
        _ptr.abs = pos;

        std::size_t off = 0;
        for (auto it = _shards.begin(); it != _shards.end(); ++it)
        {
            const auto size = it->value().size();
            if (pos <= off + size)
            {
                _shard = it;
                _ptr.rel = pos - off;
                break;
            }
            off += size;
        }

        if (_shard == _shards.end())
        {
            _shard = std::prev(_shards.end());
            _ptr.rel = _shard->value().size();
        }

        std::size_t line_off = 0;
        std::size_t prev_line_off = 0;
        std::size_t traversed = 0;
        for (const auto& it : _shards)
        {
            const auto value = it.value();
            for (std::size_t i = 0; i < value.size() && traversed < pos; i++, traversed++)
            {
                if (value[i] == '\n')
                {
                    prev_line_off = line_off;
                    line_off = 0;
                }
                else
                    line_off++;
            }
            if (traversed >= pos)
                break;
        }

        _ptr.line_off = line_off;
        _ptr.prev_line_off = prev_line_off;
    }

    // History

    void FragmentedBuffer::_push_action(Action action)
    {
        if (_history_offset)
        {
            _history.erase(
                _history.end() - static_cast<std::ptrdiff_t>(_history_offset), _history.end()
            );
            _history_offset = 0;
        }

        action.apply(this);
        if (_history.empty() || !_history.back().merge(action))
            _history.push_back(std::move(action));
    }
    void FragmentedBuffer::_undo_action()
    {
        if (_history_offset >= _history.size())
            return;

        _history[_history.size() - _history_offset - 1].revert(this);
        _history_offset++;
    }
    void FragmentedBuffer::_redo_action()
    {
        if (_history_offset == 0)
            return;

        _history[_history.size() - _history_offset].apply(this);
        _history_offset--;
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
        _history_offset = 0;
        _shards.clear();
        _shards.push_back(value);
        _shard = _shards.begin();
        _ptr = {};
    }

    // Actions

    void FragmentedBuffer::_split_shard()
    {
        const auto s = _shard->value();
        if (_ptr.rel == 0)
        {
            _shard = _shards.insert(_shard, Shard(_block_size));
        }
        else if (_ptr.rel >= s.size())
        {
            _shard = _shards.insert(std::next(_shard), Shard(_block_size));
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
        const auto pos = _ptr.abs;

        if (_shard->is_view())
            _split_shard();
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
            const auto i = data.size() - rem;
            const auto block = std::min<std::size_t>(rem, _block_size - _shard->value().size());

            if (block)
            {
                _shard->buffer().insert(_ptr.rel, data.substr(i, block));
                _ptr.rel += block;
            }
            if ((rem -= block) != 0)
            {
                _shard = _shards.insert(std::next(_shard), Shard(_block_size));
                _ptr.rel = 0;
            }
            else
                break;
        }

        _jump_char(pos + data.size());
    }

    void FragmentedBuffer::_action_insert(std::string_view data)
    {
        struct State
        {
            std::size_t abs{0};
            std::size_t len{0};
        };
        if (!_enabled_history)
        {
            _insert_data(data);
            return;
        }

        _push_action(
            {[](FragmentedBuffer* buf, void* data, void*, Action::Type type) -> bool
             {
                 const auto [state, raw] = Action::get_state<State>(data);
                 const auto str = std::string_view(reinterpret_cast<const char*>(raw), state->len);
                 switch (type)
                 {
                 case Action::Type::Apply:
                     buf->_jump_char(state->abs);
                     buf->_insert_data(str);
                     return true;
                 case Action::Type::Revert:
                     buf->_jump_char(state->abs);
                     buf->_enabled_history = false;
                     buf->_action_erase(state->len);
                     buf->_enabled_history = true;
                     buf->_jump_char(state->abs);
                     return true;
                 case Action::Type::Delete:
                     Action::delete_state<State>(data);
                     return true;
                 case Action::Type::Merge:
                     return false;
                 }
                 return false;
             },
             Action::alloc_state<State>(
                 {.abs = _ptr.abs, .len = data.size()}, std::span(data.begin(), data.end())
             )}
        );
    }
    void FragmentedBuffer::_action_erase(std::size_t len)
    {
        auto src = flatten(_shards);
        if (_ptr.abs >= src.size())
            return;

        len = std::min(len, src.size() - _ptr.abs);
        if (len == 0)
            return;

        struct State
        {
            std::size_t abs{0};
            std::size_t len{0};
        };

        const auto data = src.substr(_ptr.abs, len);

        if (!_enabled_history)
        {
            src.erase(_ptr.abs, len);
            _shards.clear();
            if (src.empty())
                _shards.push_back(Shard(_block_size));
            else
                _shards.push_back(Shard(std::move(src)));
            _shard = _shards.begin();
            _jump_char(_ptr.abs);
            return;
        }

        _push_action(
            {[](FragmentedBuffer* buf, void* data, void*, Action::Type type) -> bool
             {
                 const auto [state, raw] = Action::get_state<State>(data);
                 const auto str = std::string_view(reinterpret_cast<const char*>(raw), state->len);
                 switch (type)
                 {
                 case Action::Type::Apply:
                 {
                     auto src = flatten(buf->_shards);
                     if (state->abs < src.size())
                     {
                         const auto len = std::min(state->len, src.size() - state->abs);
                         src.erase(state->abs, len);
                     }

                     buf->_shards.clear();
                     if (src.empty())
                         buf->_shards.push_back(Shard(FragmentedBuffer::_block_size));
                     else
                         buf->_shards.push_back(Shard(std::move(src)));
                     buf->_shard = buf->_shards.begin();
                     buf->_jump_char(state->abs);
                     return true;
                 }
                 case Action::Type::Revert:
                     buf->_jump_char(state->abs);
                     buf->_insert_data(str);
                     buf->_jump_char(state->abs);
                     return true;
                 case Action::Type::Delete:
                     Action::delete_state<State>(data);
                     return true;
                 case Action::Type::Merge:
                     return false;
                 }
                 return false;
             },
             Action::alloc_state<State>(
                 {.abs = _ptr.abs, .len = len}, std::span(data.begin(), data.end())
             )}
        );
    }
} // namespace d2::fb
