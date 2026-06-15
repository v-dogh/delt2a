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
            Local = 1 << 0,
            System = 1 << 1,
        };
        enum class Level : unsigned char
        {
            Info,
            Warning,
            Error,
            Prompt,
            SoftPrompt,
        };
    public:
        using AbstractModule::AbstractModule;

        virtual void notify(std::string_view title, std::string_view content) = 0;
        virtual void remind(
            std::chrono::system_clock::time_point when,
            std::string_view title,
            std::string_view content
        ) = 0;
    };
} // namespace d2::sys
namespace d2::sys
{
    using clipboard = SystemClipboard;
    using notifications = SystemNotifications;
} // namespace d2::sys
