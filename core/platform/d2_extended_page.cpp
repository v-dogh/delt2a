#include "core/platform/d2_extended_page.hpp"
#include <stdexcept>

namespace d2
{
        AutoValueType::AutoValueType(const std::string& ext) :
            _value(global_extended_code_page.write(ext)) {}
        AutoValueType::AutoValueType(const char* ext) :
            AutoValueType(std::string(ext)) {}

    std::size_t ExtendedCodePage::_value_to_index(standard_value_type value) const
    {
        const auto byte = static_cast<unsigned char>(value);
        if (byte < 9)
            return byte;
        if (byte < 32 && byte != 9)
            return byte - 1;
        if (byte >= 128)
            return byte - 97;
        return extended_code_range;
    }
    standard_value_type ExtendedCodePage::_index_to_value(std::size_t idx) const
    {
        if (idx < 9)
            return static_cast<standard_value_type>(idx);
        if (idx < 31)
            return static_cast<standard_value_type>(idx + 1);
        return static_cast<standard_value_type>(idx + 97);
    }

    void ExtendedCodePage::activate_thread()
    {
#       if D2_LOCALE_MODE == UNICODE
            global_extended_code_page.activate();
#       endif
    }
    void ExtendedCodePage::deactivate_thread()
    {
#       if D2_LOCALE_MODE == UNICODE
            global_extended_code_page.deactivate();
#       endif
    }

    bool ExtendedCodePage::is_activated() const
    {
        return _activated;
    }
    bool ExtendedCodePage::is_extended(standard_value_type value) const
    {
        return _value_to_index(value) < extended_code_range;
    }
    std::optional<standard_value_type> ExtendedCodePage::try_write(std::string_view value)
    {
        if (value.empty())
            return ' ';
        if (value.size() == 1 && (value[0] == '\t' || (value[0] >= 32 && value[0] <= 126)))
            return value[0];
#if D2_LOCALE_MODE == ASCII
        return '?';
#else
        auto str = std::string(value);
        if (const auto f = _map.find(str); f != _map.end())
            return _index_to_value(f->second);
        if (_ctr == extended_code_range)
            return std::nullopt;
        _page[_ctr] = str;
        _map.emplace(std::move(str), _ctr);
        return _index_to_value(_ctr++);
#endif
    }
    standard_value_type ExtendedCodePage::write(std::string_view value)
    {
        if (auto token = try_write(value))
            return *token;
        throw std::overflow_error("Extended code page is full");
    }
    std::string_view ExtendedCodePage::read(standard_value_type value) const
    {
        const auto idx = _value_to_index(value);
        return idx < _ctr ? std::string_view(_page[idx]) : std::string_view("?");
    }
    void ExtendedCodePage::clear()
    {
        _map.clear();
        for (std::size_t i = 0; i < _ctr; ++i)
            _page[i].clear();
        _ctr = 0;
    }
    void ExtendedCodePage::activate()
    {
        _activated = true;
    }
    void ExtendedCodePage::deactivate()
    {
        _activated = false;
    }
}
