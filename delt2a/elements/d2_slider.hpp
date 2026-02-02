#ifndef D2_SLIDER_HPP
#define D2_SLIDER_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"

namespace d2
{
    namespace style
    {
        D2_UAI_INTERFACE(Slider,
            D2_UAI_OPTS(),
            D2_UAI_FIELDS(
                float min{ 0.f };
                float max{ 100.f };
                float step{ 1.f };
                Pixel slider_color{ .r = 255, .g = 255, .b = 255, .v = ' ' };
                HDUnit slider_width{ 1 };
            ),
            D2_UAI_PROPS(
                Max,
                Min,
                Step,
                SliderColor,
                SliderWidth
            ),
            D2_UAI_LINK(
                D2_UAI_PROP(Max, max, Style)
                D2_UAI_PROP(Min, min, Style)
                D2_UAI_PROP(Step, step, None)
                D2_UAI_PROP(SliderColor, slider_color, Style)
                D2_UAI_PROP(SliderWidth, slider_width, Style)
            )
        )
    }
    namespace dx
    {
        class Slider : public style::UAI<
            Slider,
            style::ILayout,
            style::ISlider,
            style::IColors,
            style::IKeyboardNav,
            style::IResponsive<Slider, float, float>::type
        >
        {
        public:
            using data = style::UAI<
                Slider,
                style::ILayout,
                style::ISlider,
                style::IColors,
                style::IKeyboardNav,
                style::IResponsive<Slider, float, float>::type
            >;
        protected:
            int _slider_spos{ 0 };
            float _slider_pos{ 0 };

            void _submit();

            virtual bool _provides_input_impl() const override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(sys::screen::Event ev) override;

            virtual void _frame_impl(PixelBuffer::View buffer) override;

            virtual int _aligned_mouse();
            virtual int _aligned_width();

            int _relative_to_absolute(float rel);
            float _absolute_to_relative(int abs);
        public:
            Slider(
                const std::string& name,
                TreeState::ptr state,
                pptr parent
            );

            void reset_absolute(float value = 0);
            void reset_relative(float value = 0);

            float absolute_value();
            float relative_value();
        };
        class VerticalSlider : public Slider
        {
        protected:
            virtual int _aligned_mouse() override;
            virtual int _aligned_width() override;
            virtual void _frame_impl(PixelBuffer::View buffer) override;
        public:
            VerticalSlider(
                const std::string& name,
                TreeState::ptr state,
                pptr parent
            );
        };
    }
}

#endif // D2_SLIDER_HPP
