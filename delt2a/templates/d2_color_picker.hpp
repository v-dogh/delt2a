#ifndef D2_COLOR_PICKER_HPP
#define D2_COLOR_PICKER_HPP

#include "../elements/d2_std.hpp"

namespace d2::ctm
{
    namespace impl
    {
        class ColorPickerBase
        {
        private:
            px::background _color{};
        public:
            virtual ~ColorPickerBase() = default;
            virtual void submit() = 0;
            void set_color(px::background color);
            px::background get_color();
        };
    }

    class ColorPicker :
        public style::UAIC<
            dx::FlowBox,
            ColorPicker,
            style::IResponsive<ColorPicker, px::background>::type
        >,
        public impl::ColorPickerBase
    {
    public:
        using data = style::UAIC<
            FlowBox,
            ColorPicker,
            style::IResponsive<ColorPicker, px::background>::type
        >;
        D2_UAI_CHAIN(FlowBox)
    protected:
        virtual void _state_change_impl(State state, bool value) override;
    public:
        virtual void submit() override;
    };
    class ColorPickerWindow :
        public style::UAIC<
            dx::VirtualFlowBox,
            ColorPickerWindow,
            style::IResponsive<ColorPickerWindow, px::background>::type
        >,
        public impl::ColorPickerBase
    {
    public:
        using data = style::UAIC<
            VirtualFlowBox,
            ColorPickerWindow,
            style::IResponsive<ColorPickerWindow, px::background>::type
        >;
        D2_UAI_CHAIN(VirtualFlowBox)
    protected:
        virtual void _state_change_impl(State state, bool value) override;
    public:
        virtual void submit() override;
    };
}

#endif // D2_COLOR_PICKER_HPP
