#include "d2_extended_page.hpp"

namespace d2
{
#   if D2_LOCALE_MODE == UNICODE
        AutoValueType::AutoValueType(const std::string& ext) :
            _value(global_extended_code_page.write(ext)) {}
        AutoValueType::AutoValueType(const char* ext) :
            AutoValueType(std::string(ext)) {}
#   endif

    std::size_t ExtendedCodePage::_value_to_index(standard_value_type value) const
    {
        if (value >= 0)
            return static_cast<std::size_t>(value);
        return static_cast<std::size_t>(value + 160);
    }
    standard_value_type ExtendedCodePage::_index_to_value(std::size_t idx) const
    {
        if (idx < 32)
            return static_cast<standard_value_type>(idx);
        return static_cast<standard_value_type>(idx - 160);
    }

    void ExtendedCodePage::activate_thread()
    {
#       if D2_LOCALE_MODE == UTF8
            global_extended_code_page.activate();
#       endif
    }
    void ExtendedCodePage::deactivate_thread()
    {
#       if D2_LOCALE_MODE == UTF8
        global_extended_code_page.deactivate();
#       endif
    }

    bool ExtendedCodePage::is_activated() const
    {
        return _activated;
    }
    bool ExtendedCodePage::is_extended(standard_value_type value) const
    {
        return value < 32 || value > 126;
    }
    standard_value_type ExtendedCodePage::write(std::string_view value)
    {
        [[ unlikely ]] if (value.empty())
            return ' ';
        if (value.size() == 1)
            return value[0];
        auto str = std::string(value);
        if (auto f = _map.find(str);
            f != _map.end())
            return _index_to_value(f->second);
        else if (_ctr >= extended_code_range - 1) [[ unlikely ]]
            return '?';
        else [[ likely ]]
            ++_ctr;
        _append_page += value.size();
        std::memcpy(_page.data() + _append_page, value.data(), value.size());
        const auto idx = _append_link++;
        _link[idx] = { _append_page, static_cast<std::uint8_t>(value.size()) };
        _map.emplace(std::move(str), idx);
        return _index_to_value(idx);
    }
    std::string_view ExtendedCodePage::read(standard_value_type value) const
    {
        const auto idx = _value_to_index(value);
        return std::string_view(
            _page.data() + _link[idx].first,
            _link[idx].second
        );
    }
    void ExtendedCodePage::clear()
    {
        _map.clear();
        _append_page = 0;
        _append_link = 0;
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
