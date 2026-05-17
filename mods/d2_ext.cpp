#include "d2_ext.hpp"

namespace d2::sys::ext
{
    void SystemNotifications::set_targets(unsigned char targets)
    {
        _targets = targets;
    }

    LocalSystemClipboard::Status LocalSystemClipboard::_load_impl()
    {
        return Status::Ok;
    }
    LocalSystemClipboard::Status LocalSystemClipboard::_unload_impl()
    {
        _value.clear();
        _value.shrink_to_fit();
        return Status::Ok;
    }

    void LocalSystemClipboard::clear()
    {
        _value.clear();
    }
    void LocalSystemClipboard::copy(const string& value)
    {
        _value = value;
    }
    string LocalSystemClipboard::paste()
    {
        return _value;
    }
    bool LocalSystemClipboard::empty()
    {
        return _value.empty();
    }
} // namespace d2::sys::ext
