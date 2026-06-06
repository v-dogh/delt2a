#pragma once

#include <d2_locale.hpp>
#include <d2_module.hpp>
#include <mt/pool.hpp>

#include <vector>

namespace d2::sys
{
    class SystemClipboard : public AbstractModule<SystemClipboard, "Clipboard", Access::TUnsafe>
    {
    public:
        using AbstractModule::AbstractModule;

        virtual void clear() = 0;
        virtual void copy(const d2::string& value) = 0;
        virtual d2::string paste() = 0;
        virtual bool empty() = 0;
    };
    class SystemNotifications
        : public AbstractModule<SystemNotifications, "Notifications", Access::TSafe>
    {
    public:
        enum Targets : unsigned char
        {
            Internal = 1 << 0,
            External = 1 << 1,
        };
    protected:
        unsigned char _targets{Targets::Internal};
    public:
        using AbstractModule::AbstractModule;

        void set_targets(unsigned char targets);

        virtual void notify(
            const std::string& title,
            const std::string& content,
            const std::vector<unsigned char> icon = {}
        ) = 0;
        virtual void remind(
            std::chrono::system_clock::time_point when,
            const std::string& title,
            const std::string& content,
            const std::vector<unsigned char> icon = {}
        ) = 0;
    };
} // namespace d2::sys
namespace d2::sys
{
    using clipboard = SystemClipboard;
    using notifications = SystemNotifications;
} // namespace d2::sys
