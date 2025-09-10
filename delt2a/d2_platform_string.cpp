#include "d2_platform_string.hpp"

namespace d2
{
#   if D2_LOCALE_MODE == ASCII
    std::size_t StringIterator::distance(const StringIterator& first, const StringIterator& second) noexcept
    {
        return std::distance(first._it, second._it);
    }

    StringIterator StringIterator::begin() const noexcept
    {
        return StringIterator(_str, _str.begin());
    }
    StringIterator StringIterator::end() const noexcept
    {
        return StringIterator(_str, _str.end());
    }

    bool StringIterator::increment() noexcept
    {
        return ++_it != _str.end();
    }
    string_view StringIterator::current() const noexcept
    {
        return { &*_it, &*_it + 1 };
    }

    bool StringIterator::is_end() const noexcept
    {
        return _it == _str.end();
    }
    bool StringIterator::is_space() const noexcept
    {
        return std::isspace(*_it);
    }
    bool StringIterator::is_endline() const noexcept
    {
        return *_it == '\n';
    }

    bool StringIterator::operator==(const StringIterator& copy) const noexcept
    {
        return _it == copy._it;
    }
    string_view operator*() const noexcept
    {
        return current();
    }
    StringIterator& StringIterator::operator++() noexcept
    {
        ++_it;
        return *this;
    }
    StringIterator StringIterator::operator++(int) noexcept
    {
        auto cpy = _it;
        ++_it;
        return StringIterator(cpy);
    }
#   elif D2_LOCALE_MODE == UTF8
    std::size_t StringIterator::distance(const StringIterator& first, const StringIterator& second) noexcept
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

    StringIterator StringIterator::begin() const noexcept
    {
        return StringIterator(_view, _view.begin());
    }
    StringIterator StringIterator::end() const noexcept
    {
        return StringIterator(_view, _view.end());
    }

    bool StringIterator::increment() noexcept
    {
        return ++_it != _view.end();
    }
    string_view StringIterator::current() const noexcept
    {
        return *_it;
    }

    bool StringIterator::is_end() const noexcept
    {
        return _it == _view.end();
    }
    bool StringIterator::is_space() const noexcept
    {
        for (auto ch : *_it)
        {
            if (!una::codepoint::is_whitespace(ch))
                return false;
        }
        return true;
    }
    bool StringIterator::is_endline() const noexcept
    {
        return current().size() == 1 && current()[0] == '\n';
    }

    StringIterator& StringIterator::operator=(const StringIterator& copy)
    {
        _view = una::ranges::grapheme::utf8_view<string_view>(copy._view);
        _it = copy._it;
        return *this;
    }

    bool StringIterator::operator==(const StringIterator& copy) const noexcept
    {
        return _it == copy._it;
    }
    string_view StringIterator::operator*() const noexcept
    {
        return current();
    }
    StringIterator& StringIterator::operator++() noexcept
    {
        ++_it;
        return *this;
    }
    StringIterator StringIterator::operator++(int) noexcept
    {
        auto cpy = _it;
        ++_it;
        return StringIterator(_view, cpy);
    }
#   endif
}
