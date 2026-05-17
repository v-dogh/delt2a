#ifndef D2_MODS_EXT_HPP
#define D2_MODS_EXT_HPP

#include <d2_locale.hpp>
#include <d2_module.hpp>
#include <mt/pool.hpp>

#include <vector>

namespace d2::sys::ext
{
    class SystemClipboard : public AbstractModule<SystemClipboard, "Clipboard">
    {
    public:
        using AbstractModule::AbstractModule;

        virtual void clear() = 0;
        virtual void copy(const d2::string& value) = 0;
        virtual d2::string paste() = 0;
        virtual bool empty() = 0;
    };
    class SystemNotifications : public AbstractModule<SystemNotifications, "Notifications">
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

    class LocalSystemClipboard
        : public SystemClipboard,
          public ConcreteModule<LocalSystemClipboard, Access::TUnsafe, Load::Lazy>
    {
    private:
        string _value{};
    protected:
        virtual Status _load_impl() override;
        virtual Status _unload_impl() override;
    public:
        using SystemClipboard::SystemClipboard;

        virtual void clear() override;
        virtual void copy(const string& value) override;
        virtual string paste() override;
        virtual bool empty() override;
    };

} // namespace d2::sys::ext
namespace d2::sys
{
    using clipboard = ext::SystemClipboard;
    using notifications = ext::SystemNotifications;
} // namespace d2::sys

#endif
