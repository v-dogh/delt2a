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
        value_.clear();
        value_.shrink_to_fit();
        return Status::Ok;
    }

    void LocalSystemClipboard::clear()
    {
        value_.clear();
    }
    void LocalSystemClipboard::copy(const string& value)
    {
        value_ = value;
    }
    string LocalSystemClipboard::paste()
    {
        return value_;
    }
    bool LocalSystemClipboard::empty()
    {
        return value_.empty();
    }
} // namespace d2::sys::ext
