#pragma once

#include <d2_locale.hpp>
#include <d2_module.hpp>
#include <d2_tree_element.hpp>
#include <mt/pool.hpp>

namespace d2::sys
{
    class SystemClipboard : public ModuleDecl<SystemClipboard, "Clipboard", Access::TUnsafe>
    {
    public:
        using ModuleDecl::ModuleDecl;

        virtual void clear() = 0;
        virtual void copy(const d2::string& value) = 0;
        virtual d2::string paste() = 0;
        virtual bool empty() = 0;
    };
    class SystemNotify : public ModuleDecl<SystemNotify, "Notifications", Access::TSafe>
    {
    public:
        enum Mode : unsigned char
        {
            // First two describe the level
            // Local is the default

            Local = 1 << 0,
            System = 1 << 1,

            // This one describes delivery
            // System only supports Passive

            Passive = 1 << 2,
            Prompt = 1 << 3,
            SoftPrompt = 1 << 4,
        };
        enum class Level : unsigned char
        {
            Info,
            Warning,
            Error,
        };
    public:
        using ModuleDecl::ModuleDecl;

        virtual bool supports(unsigned char mode) = 0;

        virtual void notify(
            Level level,
            std::string_view title,
            std::string_view content,
            unsigned char targets = Local | Passive
        ) = 0;
        virtual void notify(
            Level level,
            std::string_view title,
            std::string_view content,
            Element::ptr view,
            unsigned char targets = Local | Passive
        ) = 0;

        virtual void clear() = 0;
    };

    using clipboard = SystemClipboard;
    using notify = SystemNotify;
} // namespace d2::sys
