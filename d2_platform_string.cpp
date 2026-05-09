#include "d2_platform_string.hpp"

namespace d2
{
#   if D2_LOCALE_MODE == ASCII
    std::size_t StringIterator::distance(const StringIterator& first, const StringIterator& second)
    {
        return std::distance(first._it, second._it);
    }

    StringIterator StringIterator::begin() const
    {
        return StringIterator(_str, _str.begin());
    }
    StringIterator StringIterator::end() const
    {
        return StringIterator(_str, _str.end());
    }

    bool StringIterator::increment()
    {
        return ++_it != _str.end();
    }
    string_view StringIterator::current() const
    {
        return { &*_it, &*_it + 1 };
    }

    bool StringIterator::is_end() const
    {
        return _it == _str.end();
    }
    bool StringIterator::is_space() const
    {
        return std::isspace(*_it);
    }
    bool StringIterator::is_endline() const
    {
        return *_it == '\n';
    }

    bool StringIterator::operator==(const StringIterator& copy) const
    {
        return _it == copy._it;
    }
    string_view operator*() const
    {
        return current();
    }
    StringIterator& StringIterator::operator++()
    {
        ++_it;
        return *this;
    }
    StringIterator StringIterator::operator++(int)
    {
        auto cpy = _it;
        ++_it;
        return StringIterator(cpy);
    }
#   elif D2_LOCALE_MODE == UNICODE
    std::size_t StringIterator::distance(const StringIterator& first, const StringIterator& second)
    {
        std::size_t i = 0;
        iterator fit;
        iterator sit;
        if ((*first._it).data() < (*second._it).data())
        {
            fit = first._it;
            sit = second._it;
        }
        else
        {
            fit = second._it;
            sit = first._it;
        }
        while (fit != first._view.end() && fit != sit)
            i++;
        return i;
    }

    StringIterator StringIterator::begin() const
    {
        return StringIterator(_view, _view.begin());
    }
    StringIterator StringIterator::end() const
    {
        return StringIterator(_view, _view.end());
    }

    bool StringIterator::increment()
    {
        return ++_it != _view.end();
    }
    string_view StringIterator::current() const
    {
        return *_it;
    }

    bool StringIterator::is_end() const
    {
        return _it == _view.end();
    }
    bool StringIterator::is_space() const
    {
        for (auto ch : *_it)
        {
            if (!una::codepoint::is_whitespace(ch))
                return false;
        }
        return true;
    }
    bool StringIterator::is_endline() const
    {
        return current().size() == 1 && current()[0] == '\n';
    }

    StringIterator& StringIterator::operator=(const StringIterator& copy)
    {
        _view = una::ranges::grapheme::utf8_view<string_view>(copy._view);
        _it = copy._it;
        return *this;
    }

    bool StringIterator::operator==(const StringIterator& copy) const
    {
        return _it == copy._it;
    }
    string_view StringIterator::operator*() const
    {
        return current();
    }
    StringIterator& StringIterator::operator++()
    {
        ++_it;
        return *this;
    }
    StringIterator StringIterator::operator++(int)
    {
        auto cpy = _it;
        ++_it;
        return StringIterator(_view, cpy);
    }
#   endif
}
