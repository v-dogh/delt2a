#pragma once

#include <d2_module.hpp>
#include <mods/d2_ext.hpp>

namespace d2::sys
{
    class LocalSystemClipboard : public SystemClipboard,
                                 public ModuleImpl<LocalSystemClipboard, Load::Lazy>
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
} // namespace d2::sys
