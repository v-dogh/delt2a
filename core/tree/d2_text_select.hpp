#pragma once

#include <core/platform/d2_locale.hpp>
#include <core/types/d2_vtypes.hpp>

namespace d2::style
{
    struct TextNode
    {
        std::string text{};
        BoundingBox bounds{};
    };

    class IVSelectable
    {
    protected:
        bool enable_selection{true};
    public:
        bool selectable() const noexcept;

        // virtual std::vector<TextNode> nodes() const noexcept = 0;
        // virtual void highlight() = 0;
    };
} // namespace d2::style
