#ifndef D2_MODS_EXT_HPP
#define D2_MODS_EXT_HPP

#include <d2_locale.hpp>
#include <d2_module.hpp>
#include <mt/pool.hpp>

#include <tuple>
#include <vector>

namespace d2::sys::ext
{
    class SystemClipboard : public AbstractModule<"Clipboard">
    {
    public:
        using AbstractModule::AbstractModule;

        virtual void clear() = 0;
        virtual void copy(const d2::string& value) = 0;
        virtual d2::string paste() = 0;
        virtual bool empty() = 0;
    };
    class SystemNotifications : public AbstractModule<"Notifications">
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
    class SystemPlugins : public AbstractModule<"Plugins">
    {
    public:
        enum class LoadStatus
        {
            VersionMismatch,
            InvalidResponse,
            Ok
        };
    private:
        struct Environment
        {
            std::shared_ptr<IOContext> ctx;
            std::vector<unsigned char> params;
        };
        struct Pipe
        {
        };
    public:
        using AbstractModule::AbstractModule;

        template<typename... Components, typename... Argv>
        Environment sandbox(std::tuple<Argv...>&& params, mt::ConcurrentPool::ptr pool = nullptr)
        {
            return Environment();
        }
        template<typename... Components, typename... Argv>
        Environment sandbox(mt::ConcurrentPool::ptr pool = nullptr)
        {
            return Environment();
        }

        template<typename Ret, typename... Argv>
        mt::future<Ret> send(const std::string& name, const std::string& endpoint, Argv&&... args)
        {
        }
        template<typename Ret>
        mt::future<Ret> receive(const std::string& name, const std::string& endpoint)
        {
        }

        virtual LoadStatus load(
            const std::filesystem::path& path,
            const std::string& name,
            Environment ctx = Environment()
        ) = 0;
        virtual void unload(const std::string& name) = 0;
    };
    class SystemIO : public AbstractModule<"IO">
    {
    };

    class LocalSystemClipboard
        : public SystemClipboard,
          public ConcreteModule<LocalSystemClipboard, Access::TUnsafe, Load::Lazy>
    {
    private:
        string value_{};
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
    using plugins = ext::SystemPlugins;
    using clipboard = ext::SystemClipboard;
    using notifications = ext::SystemNotifications;
} // namespace d2::sys

#endif
