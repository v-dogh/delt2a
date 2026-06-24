#pragma once

#include <d2_module.hpp>
#include <d2_screen.hpp>
#include <d2_tree_construct.hpp>
#include <mods/d2_ext.hpp>

namespace d2::sys
{
    class LocalSystemNotify : public SystemNotify,
                              public ModuleImpl<LocalSystemNotify, Load::Lazy, SystemScreen>
    {
    private:
        d2::TreeIter<ParentElement> _display{};

        virtual Status _unload_impl() override;

        void _create_display_if();
        void _handle_prompt(
            Level level, std::string title, std::string content, Element::ptr view, bool soft
        );
        void
        _handle_message(Level level, std::string title, std::string content, Element::ptr view);
    public:
        using SystemNotify::SystemNotify;

        virtual bool supports(unsigned char mode) override;

        virtual void notify(
            Level level,
            std::string_view title,
            std::string_view content,
            unsigned char targets = Local | Passive
        ) override;
        virtual void notify(
            Level level,
            std::string_view title,
            std::string_view content,
            Element::ptr view,
            unsigned char targets = Local | Passive
        ) override;

        virtual void clear() override;
    };
} // namespace d2::sys
