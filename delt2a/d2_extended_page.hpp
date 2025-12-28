#ifndef D2_EXTENDED_PAGE_HPP
#define D2_EXTENDED_PAGE_HPP

#include <absl/container/flat_hash_map.h>
#include <string>
#include <array>

namespace d2
{
    using standard_value_type = char;

    class AutoValueType
    {
    private:
        standard_value_type _value{};
    public:
        constexpr AutoValueType() = default;
        constexpr AutoValueType(standard_value_type ch) : _value(ch) {}
        AutoValueType(const std::string& ext);
        AutoValueType(const char* ext);
        AutoValueType(const AutoValueType& other) = default;

        constexpr AutoValueType& operator=(standard_value_type ch) { _value = ch; return *this; }
        constexpr AutoValueType& operator=(const AutoValueType& copy) = default;

        constexpr operator const standard_value_type&() const { return _value; }
        constexpr operator standard_value_type&() { return _value; }

        constexpr bool operator==(const standard_value_type& copy) const { return _value == copy; }
        constexpr bool operator!=(const standard_value_type& copy) const { return _value != copy; }
        constexpr bool operator<(const standard_value_type& copy) const { return _value < copy; }
        constexpr bool operator>(const standard_value_type& copy) const { return _value > copy; }
        constexpr bool operator<=(const standard_value_type& copy) const { return _value <= copy; }
        constexpr bool operator>=(const standard_value_type& copy) const { return _value >= copy; }

        constexpr AutoValueType operator+(const standard_value_type& copy) const { return _value + copy; }
        constexpr AutoValueType operator-(const standard_value_type& copy) const { return _value - copy; }
        constexpr AutoValueType operator*(const standard_value_type& copy) const { return _value * copy; }
        constexpr AutoValueType operator/(const standard_value_type& copy) const { return _value / copy; }
        constexpr AutoValueType operator%(const standard_value_type& copy) const { return _value % copy; }

        constexpr AutoValueType& operator+=(const standard_value_type& copy) { _value += copy; return *this; }
        constexpr AutoValueType& operator-=(const standard_value_type& copy) { _value -= copy; return *this; }
        constexpr AutoValueType& operator*=(const standard_value_type& copy) { _value *= copy; return *this; }
        constexpr AutoValueType& operator/=(const standard_value_type& copy) { _value /= copy; return *this; }
        constexpr AutoValueType& operator%=(const standard_value_type& copy) { _value %= copy; return *this; }

        constexpr AutoValueType& operator++() { ++_value; return *this; }
        constexpr AutoValueType operator++(int) { AutoValueType tmp(*this); ++_value; return tmp; }
        constexpr AutoValueType& operator--() { --_value; return *this; }
        constexpr AutoValueType operator--(int) { AutoValueType tmp(*this); --_value; return tmp; }
    };

    class ExtendedCodePage
    {
    private:
        static constexpr auto extended_code_range = 160;
        static constexpr auto maximum_code_length = 8;
    private:
        absl::flat_hash_map<std::string, standard_value_type> _map{};
        std::array<std::pair<std::uint16_t, std::uint8_t>, extended_code_range> _link{};
        std::array<char, extended_code_range * maximum_code_length> _page{};
        std::size_t _append_link{ 0 };
        std::size_t _append_page{ 0 };
        bool _activated{ false };

        std::size_t _value_to_index(standard_value_type value) const;
        standard_value_type _index_to_value(std::size_t idx) const;
    public:
        static void activate_thread();
        static void deactivate_thread();

        ExtendedCodePage() = default;
        ExtendedCodePage(const ExtendedCodePage&) = delete;
        ExtendedCodePage(ExtendedCodePage&&) = default;

        bool is_activated() const;
        bool is_extended(standard_value_type value) const;
        standard_value_type write(std::string_view value);
        std::string_view read(standard_value_type value) const;
        void clear();
        void activate();
        void deactivate();

        ExtendedCodePage& operator=(const ExtendedCodePage&) = delete;
        ExtendedCodePage& operator=(ExtendedCodePage&&) = default;
    };

#   if D2_LOCALE_MODE == UNICODE
        inline thread_local ExtendedCodePage global_extended_code_page{};
#   endif
}

#endif // D2_EXTENDED_PAGE_HPP
