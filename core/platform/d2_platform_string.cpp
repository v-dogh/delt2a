#include "core/platform/d2_platform_string.hpp"
#include <stdexcept>
#include <cctype>

namespace d2
{
#   if D2_LOCALE_MODE == ASCII
    std::size_t StringIterator::distance(const StringIterator& first, const StringIterator& second)
    {
        if (first._str.data() != second._str.data() || first._str.size() != second._str.size())
            throw std::invalid_argument("String iterators have different ranges");
        if (first._it == second._it)
            return 0;
        return first._it < second._it ? second._it - first._it : first._it - second._it;
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
        if (!is_end())
            ++_it;
        return !is_end();
    }
    string_view StringIterator::current() const
    {
        return is_end() ? string_view{} : string_view(&*_it, 1);
    }

    bool StringIterator::is_end() const
    {
        return _it == _str.end();
    }
    bool StringIterator::is_space() const
    {
        return !is_end() && std::isspace(static_cast<unsigned char>(*_it));
    }
    bool StringIterator::is_endline() const
    {
        return !is_end() && *_it == '\n';
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
        increment();
        return *this;
    }
    StringIterator StringIterator::operator++(int)
    {
        auto cpy = _it;
        increment();
        return StringIterator(_str, cpy);
    }
#   elif D2_LOCALE_MODE == UNICODE
    std::size_t StringIterator::distance(const StringIterator& first, const StringIterator& second)
    {
        if (first._view.begin() != second._view.begin() || first._view.end() != second._view.end())
            throw std::invalid_argument("String iterators have different ranges");
        std::size_t i = 0;
        auto fit = first._it;
        while (fit != first._view.end() && fit != second._it)
        {
            ++fit;
            ++i;
        }
        if (fit == second._it)
            return i;
        i = 0;
        fit = second._it;
        while (fit != first._it)
        {
            ++fit;
            ++i;
        }
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
        if (!is_end())
            ++_it;
        return !is_end();
    }
    string_view StringIterator::current() const
    {
        return is_end() ? string_view{} : *_it;
    }

    bool StringIterator::is_end() const
    {
        return _it == _view.end();
    }
    bool StringIterator::is_space() const
    {
        if (is_end())
            return false;
        for (auto ch : current() | una::views::utf8)
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
        increment();
        return *this;
    }
    StringIterator StringIterator::operator++(int)
    {
        auto cpy = _it;
        increment();
        return StringIterator(_view, cpy);
    }
#   endif
}
