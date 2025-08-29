#ifndef D2_BOX_HPP
#define D2_BOX_HPP

#include "../d2_tree_element.hpp"
#include "../d2_tree_parent.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2::dx
{
    class Box :
        public style::UAIE<VecParentElement, Box, style::ILayout, style::IContainer, style::IColors>,
        public impl::ContainerHelper<Box>
    {
    public:
        static constexpr auto overlap = 6;
        friend class UnitUpdateHelper;
        friend class ContainerHelper;
        using data = style::UAIE<VecParentElement, Box, style::ILayout, style::IContainer, style::IColors>;
        using data::data;
    protected:
        int _perfect_width() const noexcept;
        int _perfect_height() const noexcept;

        virtual int _get_border_impl(BorderType type, cptr elem) const noexcept override;
        virtual Unit _layout_impl(Element::Layout type) const noexcept override;

        virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;

        virtual void _render_start() {}
        virtual void _object_render(PixelBuffer::View buffer, ptr obj);
    };
}

#endif // D2_BOX_HPP
