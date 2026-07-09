#pragma once

#include <d2_locale.hpp>

namespace d2::style
{
    class IVSelectable
    {
    protected:
        bool enable_selection{true};
    public:
        virtual void select(std::size_t begin, std::size_t end) = 0;
        virtual string selected() const = 0;
    };
} // namespace d2::style
