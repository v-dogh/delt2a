#ifndef D2_SLIDER_HPP
#define D2_SLIDER_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2
{
    namespace style
    {
        struct Slider
        {
            float min{ 0.f };
            float max{ 0.f };
            float step{ 1.f };
            PixelForeground slider_background_color{ .v = charset::slider_horizontal };
            Pixel slider_color{ .r = 255, .g = 255, .b = 255, .v = ' ' };
            HDUnit slider_width{ 1 };
            std::function<void(TreeIter, float, float)> on_change{};

            template<uai_property Property>
            auto at_style()
            {
                D2_UAI_SETUP(Style)
                D2_UAI_VAR_START
                D2_UAI_GET_VAR_A(0, max)
                D2_UAI_GET_VAR_A(1, min)
                D2_UAI_GET_VAR(2, step, None)
                D2_UAI_GET_VAR_A(3, slider_background_color)
                D2_UAI_GET_VAR_A(4, slider_color)
                D2_UAI_GET_VAR_A(5, slider_width)
                D2_UAI_GET_VAR(6, on_change, None)
                D2_UAI_VAR_END;
            }
        };

        template<std::size_t PropBase>
        struct ISlider : Slider, InterfaceHelper<ISlider, PropBase, 7>
        {
            using data = Slider;
            enum Property : uai_property
            {
                Max = PropBase,
                Min,
                Step,
                BackgroundColor,
                SliderColor,
                SliderWidth,
                OnChange,
            };
        };
        using IZSlider = ISlider<0>;
    }
    namespace dx
    {
        class Slider : public style::UAI<Slider, style::ILayout, style::ISlider, style::IKeyboardNav>
        {
        public:
            friend class UnitUpdateHelper;
            using data = style::UAI<Slider, style::ILayout, style::ISlider, style::IKeyboardNav>;
        protected:
            int _slider_spos{ 0 };
            float _slider_pos{ 0 };

            void _submit();

            virtual bool _provides_input_impl() const noexcept override;
            virtual void _state_change_impl(State state, bool value) override;
            virtual void _event_impl(IOContext::Event ev) override;

            virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;

            virtual int _aligned_mouse() noexcept;
            virtual int _aligned_width() noexcept;

            int _relative_to_absolute(float rel) noexcept;
            float _absolute_to_relative(int abs) noexcept;
        public:
            Slider(
                const std::string& name,
                TreeState::ptr state,
                pptr parent
            );

            void reset_absolute(int value = 0) noexcept;
            void reset_relative(float value = 0) noexcept;

            float absolute_value() noexcept;
            float relative_value() noexcept;
        };
        class VerticalSlider : public Slider
        {
        protected:
            virtual int _aligned_mouse() noexcept override;
            virtual int _aligned_width() noexcept override;
            virtual void _frame_impl(PixelBuffer::View buffer) noexcept override;
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
