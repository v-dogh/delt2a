#ifndef D2_PLATFORM_STRING_HPP
#define D2_PLATFORM_STRING_HPP

#if D2_LOCALE_MODE == UTF8
#include <uni_algo/ranges_grapheme.h>
#include <uni_algo/ranges.h>
#include <uni_algo/prop.h>
#endif
#include "d2_locale.hpp"

namespace d2
{
#   if D2_LOCALE_MODE == ASCII
    class StringIterator
    {
    private:
        string_view _str{};
        string_view::iterator _it{};
    public:
        static std::size_t distance(const StringIterator& first, const StringIterator& second) noexcept;

        StringIterator(StringIterator&) = default;
        StringIterator(const StringIterator&) = default;
        StringIterator(string_view str) : _str(str) {}
        StringIterator(string_view str, string_view::iterator it) : _str(str), _it(it) {}

        StringIterator begin() const noexcept;
        StringIterator end() const noexcept;

        void increment() noexcept;
        string_view current() const noexcept;

        bool is_end() const noexcept;
        bool is_space() const noexcept;
        bool is_endline() const noexcept;

        StringIterator& operator=(const StringIterator&) = default;
        StringIterator& operator=(StringIterator&&) = default;

        bool operator==(const StringIterator& copy) const noexcept;
        StringIterator& operator++() noexcept;
        StringIterator operator++(int) noexcept;
        string_view operator*() const noexcept;
    };
#   elif D2_LOCALE_MODE == UTF8
    class StringIterator
    {
    private:
        using iterator = decltype(una::ranges::grapheme::utf8_view<string_view>().begin());
    private:
        mutable una::ranges::grapheme::utf8_view<string_view> _view{};
        iterator _it{};
    public:
        static std::size_t distance(const StringIterator& first, const StringIterator& second) noexcept;

        StringIterator(StringIterator&) = default;
        StringIterator(const StringIterator&) = default;
        StringIterator(string_view str) : _view(str), _it(_view.begin()) {}
        StringIterator(una::ranges::grapheme::utf8_view<string_view> view, iterator it) : _view(view), _it(it) {}

        StringIterator begin() const noexcept;
        StringIterator end() const noexcept;

        bool increment() noexcept;
        string_view current() const noexcept;

        bool is_end() const noexcept;
        bool is_space() const noexcept;
        bool is_endline() const noexcept;

        StringIterator& operator=(const StringIterator& copy);
        StringIterator& operator=(StringIterator&&) = default;

        bool operator==(const StringIterator& copy) const noexcept;
        StringIterator& operator++() noexcept;
        StringIterator operator++(int) noexcept;
        string_view operator*() const noexcept;
    };
#   endif
}

#endif // D2_PLATFORM_STRING_HPP
