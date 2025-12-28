#ifndef D2_COLOR_PICKER_HPP
#define D2_COLOR_PICKER_HPP

#include "../elements/d2_std.hpp"

namespace d2::ctm
{
    class ColorPicker : public style::UAIC<
            dx::VirtualBox,
            ColorPicker,
            style::IResponsive<ColorPicker, px::background>::type
        >
    {
    public:
        using data = style::UAIC<
            VirtualBox,
            ColorPicker,
            style::IResponsive<ColorPicker, px::background>::type
        >;
        D2_UAI_CHAIN(VirtualBox)
    protected:
        static eptr<ColorPicker> _core(TreeState::ptr state);

        virtual void _state_change_impl(State state, bool value) override;

        px::background _get_color() const;
    public:
        ColorPicker(
            const std::string& name,
            TreeState::ptr state,
            pptr parent,
            std::function<void(TypedTreeIter<ColorPicker>, px::background)>
        );

        void submit();
    };
}

#endif // D2_COLOR_PICKER_HPP
