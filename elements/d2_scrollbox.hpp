#ifndef D2_SCROLLBOX_HPP
#define D2_SCROLLBOX_HPP

#include <elements/d2_box.hpp>
#include <elements/d2_slider.hpp>

namespace d2::dx
{
    class ScrollBox : public Box
    {
    protected:
        std::shared_ptr<VerticalSlider> _scrollbar{nullptr};
        mutable int offset_{0};
        mutable int computed_height_{0};
        mutable bool needs_recompute_{false};

        void _recompute_height() const;

        virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) override;
        virtual bool _provides_input_impl() const override;
        virtual void _state_change_impl(State state, bool value) override;
        virtual void _event_impl(in::InputFrame& frame) override;

        virtual int _get_border_impl(BorderType type, Element::cptr elem) const override;
        virtual void _layout_for_impl(enum Element::Layout type, cptr ptr) const override;
        virtual void _update_layout_impl() override;
    public:
        using Box::Box;

        TreeIter<VerticalSlider> scrollbar() const;
        int internal_height() const;
        int window_height() const;
        int scroll_offset() const noexcept;
        void scroll_to(int xoffset);
        void sync();

        virtual void foreach_internal(foreach_internal_callback callback) const override;
    };
} // namespace d2::dx

#endif // D2_SCROLLBOX_HPP
