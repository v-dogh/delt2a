#include "d2_extended_page.hpp"

namespace d2
{
#   if D2_LOCALE_MODE == UTF8
        AutoValueType::AutoValueType(const std::string& ext) noexcept :
            _value(global_extended_code_page.write(ext)) {}
        AutoValueType::AutoValueType(const char* ext) noexcept :
            AutoValueType(std::string(ext)) {}
#   endif

    std::size_t ExtendedCodePage::_value_to_index(standard_value_type value) const noexcept
    {
        if (value >= 0 && value < 32) return static_cast<std::size_t>(value);
        else if (value < 0) return static_cast<std::size_t>(value + 128) + 32;
        return 0;
    }
    standard_value_type ExtendedCodePage::_index_to_value(std::size_t idx) const noexcept
    {
        if (idx < 32)
            return static_cast<standard_value_type>(idx);
        return static_cast<standard_value_type>((idx - 32) - 128);
    }

    void ExtendedCodePage::activate_thread() noexcept
    {
#       if D2_LOCALE_MODE == UTF8
            global_extended_code_page.activate();
#       endif
    }
    void ExtendedCodePage::deactivate_thread() noexcept
    {
#       if D2_LOCALE_MODE == UTF8
        global_extended_code_page.deactivate();
#       endif
    }

    bool ExtendedCodePage::is_activated() const noexcept
    {
        return _activated;
    }
    bool ExtendedCodePage::is_extended(standard_value_type value) const noexcept
    {
        return value < 32 || value > 126;
    }
    standard_value_type ExtendedCodePage::write(std::string_view value) noexcept
    {
        auto str = std::string(value);
        if (auto f = _map.find(str);
            f != _map.end())
            return f->second;
        _append_page += value.size();
        std::memcpy(_page.data() + _append_page, value.data(), value.size());
        const auto idx = _append_link++;
        _link[idx] = { _append_page, static_cast<std::uint8_t>(value.size()) };
        _map.emplace(std::move(str), idx);
        return _index_to_value(idx);
    }
    std::string_view ExtendedCodePage::read(standard_value_type value) const noexcept
    {
        const auto idx = _value_to_index(value);
        return std::string_view(
            _page.data() + _link[idx].first,
            _link[idx].second
            );
    }
    void ExtendedCodePage::clear() noexcept
    {
        _map.clear();
        _append_page = 0;
        _append_link = 0;
    }
    void ExtendedCodePage::activate() noexcept
    {
        _activated = true;
    }
    void ExtendedCodePage::deactivate() noexcept
    {
        _activated = false;
    }
}
