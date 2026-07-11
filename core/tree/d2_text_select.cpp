#include "d2_text_select.hpp"

namespace d2::style
{
    bool IVSelectable::selectable() const noexcept
    {
        return enable_selection;
    }
} // namespace d2::style
