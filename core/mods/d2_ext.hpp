#pragma once

#include <core/io/d2_module.hpp>
#include <core/platform/d2_locale.hpp>
#include <core/tree/d2_tree_element.hpp>
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

    // Handles generic user-facing notifications
    // Exposes message levels and output types (whether in-app or system-level)
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

    // Module responsible for providing an add-on generic text selection frontend
    // This allows cross-element text content selection
    class SystemSelector : public Module<
                               SystemSelector,
                               "Selector",
                               Access::TUnsafe,
                               Load::Immediate,
                               SystemClipboard,
                               SystemScreen
                           >
    {
    public:
        enum class Event
        {
            Selected,
        };
        using EventManifest = SignalManifest<
            // Content|StartPos|EndPos
            EvDefault<Event, d2::string_view, Position, Position>
        >;
    private:
        Signals::Signal _sig{};
        Signals::Handle _mouse_ev{};

        virtual Status _load_impl() override;
        virtual Status _unload_impl() override;
    public:
        using Module::Module;
    };

    using clipboard = SystemClipboard;
    using notify = SystemNotify;
    using selector = SystemSelector;
} // namespace d2::sys

namespace d2
{
    template<> struct SignalRegistry<sys::selector::Event> : sys::selector::EventManifest
    {
    };
} // namespace d2
