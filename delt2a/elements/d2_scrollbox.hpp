#ifndef D2_SCROLLBOX_HPP
#define D2_SCROLLBOX_HPP

#include "d2_box.hpp"
#include "d2_slider.hpp"

namespace d2::dx
{
    class ScrollBox : public Box
    {
    protected:
        std::shared_ptr<VerticalSlider> scrollbar_{ nullptr };
        mutable int offset_{ 0 };
        mutable int computed_height_{ 0 };

        void _recompute_height() const;

        virtual bool _provides_input_impl() const override;
        virtual void _state_change_impl(State state, bool value) override;
        virtual void _event_impl(Screen::Event ev) override;

        virtual int _get_border_impl(BorderType type, Element::cptr elem) const override;
        virtual void _layout_for_impl(enum Element::Layout type, cptr ptr) const override;
        virtual void _update_layout_impl() override;
    public:
        using Box::Box;

        TreeIter<VerticalSlider> scrollbar() const;
        void scroll_to(int xoffset);

        virtual void foreach_internal(foreach_internal_callback callback) const override;
    };
}

#endif // D2_SCROLLBOX_HPP
