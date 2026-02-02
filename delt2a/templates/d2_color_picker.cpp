#include "d2_color_picker.hpp"
#include "d2_widget_theme_base.hpp"

namespace d2::ctm
{
    class ColorPickerDiamond : public style::UAI<
            ColorPickerDiamond,
            style::ILayout,
            style::IResponsive<ColorPickerDiamond, px::background>::type
        >
    {
    public:
        using data = style::UAI<
            ColorPickerDiamond,
            style::ILayout,
            style::IColors,
            style::IResponsive<ColorPickerDiamond, px::background>::type
        >;
    protected:
        virtual void _state_change_impl(State state, bool value) override;
    };

    static ColorPicker::eptr<impl::ColorPickerBase> _core(TreeState::ptr state)
    {
        return state->core()->traverse().as<impl::ColorPickerBase>();
    }
    static ColorPicker::eptr<ColorPickerWindow> _wcore(TreeState::ptr state)
    {
        return state->core()->traverse().as<ColorPickerWindow>();
    }

    D2_STYLESHEET_BEGIN(color_preset, px::background color)
        D2_STYLE(Value, " ")
        D2_STYLE(X, 1.0_relative)
        D2_STYLE(BackgroundColor, D2_VAR(WidgetTheme, wg_bg_secondary()))
        D2_STYLE(ForegroundColor, color)
        D2_ON_EXPR(Focused,
            ptr->template set<type::Value>(charset::icon::middle_square),
            _core(ptr->state())->set_color(color)
        )
        D2_OFF_EXPR(Focused,
            ptr->template set<type::Value>(" ")
        )
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(color_preset_nline, px::background color)
        D2_STYLES_APPLY(color_preset, color)
        D2_STYLE(Y, 0.0_relative)
        D2_STYLE(X, 1.0_relative)
    D2_STYLESHEET_END
    D2_STYLESHEET_BEGIN(rgba_picker)
        D2_STYLE(X, 0.0_relative)
        D2_STYLE(Width, 5.0_pxi)
        D2_STYLE(Min, 0)
        D2_STYLE(Max, 255)
        D2_STYLE(Step, 1)
        D2_STYLES_APPLY(slider_color)
    D2_STYLESHEET_END

    D2_STATELESS_TREE(color_picker, bool window)
        // Controls
        if (window) D2_ELEM(Button, exit)
            D2_STYLE(Value, "<X>")
            D2_STYLE(X, 1.0_pxi)
            D2_STYLE(OnSubmit, [](d2::TreeIter<Button> ptr) {
                _wcore(ptr->state())->close();
            })
            D2_STYLES_APPLY(button_react)
        D2_ELEM_END
        if (window) D2_ELEM(Button, submit)
            D2_STYLE(Value, "<Ok>")
            D2_STYLE(X, 5.0_pxi)
            D2_STYLE(OnSubmit, [](d2::TreeIter<Button> ptr) {
                _core(ptr->state())->submit();
            })
            D2_STYLES_APPLY(button_react)
        D2_ELEM_END
        D2_ELEM(FlowBox, presets)
            D2_STYLE(X, 0.0_center)
            D2_STYLE(Height, 2.0_px)
            constexpr std::array samples = {
                std::array{
                    colors::w::black,
                    colors::w::dark_gray,
                    colors::w::gray,
                    colors::w::silver,
                    colors::w::light_gray,
                    colors::w::white
                },
                std::array{
                    colors::r::coral,
                    colors::r::yellow,
                    colors::g::olive,
                    colors::g::forest_green,
                    colors::b::dark_slate_blue,
                    colors::b::indigo
                }
            };
            for (std::size_t i = 0; i < 2; i++)
            {
                D2_ELEM(Button)
                    D2_STYLES_APPLY(color_preset_nline, samples[i][0])
                D2_ELEM_END
                for (std::size_t j = 0; j < 5; j++)
                {
                    D2_ELEM(Button)
                        D2_STYLES_APPLY(color_preset, samples[i][j + 1])
                    D2_ELEM_END
                }
            }
        D2_ELEM_END
        // // Value
        // D2_ELEM(FlowBox, value)
        //     D2_STYLE(X, 0.0_pxi)
        //     D2_STYLE(Y, 1.0_px)
        //     D2_STYLE(Width, 0.5_pc)
        //     D2_STYLE(Height, 1.0_pxi)
        //     D2_ELEM(Text)
        //         D2_STYLE(Value, "Value: #")
        //         D2_STYLE(X, 1.0_relative)
        //         D2_STYLES_APPLY(text_color)
        //     D2_ELEM_END
        //     D2_ELEM(Input, hex)
        //         D2_STYLE(Hint, "00000000")
        //         D2_STYLE(X, 0.0_relative)
        //         D2_STYLE(OnSubmit, [](TreeIter<Input> ptr, string value) {

        //         })
        //         D2_STYLES_APPLY(input_color)
        //     D2_ELEM_END
        //     D2_ELEM(Text, color)
        //         D2_STYLE(Value, " ")
        //         D2_STYLE(X, 0.0_relative)
        //         D2_STYLE(ForegroundColor, colors::w::white)
        //     D2_ELEM_END
        //     D2_ELEM(Switch)
        //         D2_STYLE(Options, Switch::opts{ "Diamond", "Sliders" })
        //         D2_STYLE(X, 0.0_relative)
        //         D2_STYLES_APPLY(text_color)
        //     D2_ELEM_END
        // D2_ELEM_END
        // D2_ELEM(Box, logic)
        //     D2_STYLE(Y, 0.0_relative)
        //     // Manual Colors
        //     D2_ELEM(FlowBox, manual)
        //         D2_STYLE(Width, 0.5_pc)
        //         D2_STYLE(Height, 12.0_px)
        //         D2_STYLE(X, 1.0_relative)
        //         D2_STYLE(Y, 1.0_px)
        //         D2_ELEM(Text)
        //             D2_STYLE(Value, "R: ")
        //             D2_STYLE(X, 1.0_relative)
        //             D2_STYLES_APPLY(text_color)
        //         D2_ELEM_END
        //         D2_ELEM(Slider, r)
        //             D2_STYLES_APPLY(rgba_picker)
        //         D2_ELEM_END
        //         D2_ELEM(Text)
        //             D2_STYLE(Value, "G: ")
        //             D2_STYLE(X, 1.0_relative)
        //             D2_STYLE(Y, 0.0_relative)
        //             D2_STYLES_APPLY(text_color)
        //         D2_ELEM_END
        //         D2_ELEM(Slider, g)
        //             D2_STYLES_APPLY(rgba_picker)
        //         D2_ELEM_END
        //         D2_ELEM(Text)
        //             D2_STYLE(Value, "B: ")
        //             D2_STYLE(X, 1.0_relative)
        //             D2_STYLE(Y, 0.0_relative)
        //             D2_STYLES_APPLY(text_color)
        //         D2_ELEM_END
        //         D2_ELEM(Slider, b)
        //             D2_STYLES_APPLY(rgba_picker)
        //         D2_ELEM_END
        //         D2_ELEM(Text)
        //             D2_STYLE(Value, "A: ")
        //             D2_STYLE(X, 1.0_relative)
        //             D2_STYLE(Y, 0.0_relative)
        //             D2_STYLES_APPLY(text_color)
        //         D2_ELEM_END
        //         D2_ELEM(Slider, a)
        //             D2_STYLES_APPLY(rgba_picker)
        //         D2_ELEM_END
        //     D2_ELEM_END
        //     // Geometric Colors
        //     D2_ELEM(ColorPickerDiamond, diamond)
        //         D2_STYLE(Width, 1.0_pc)
        //         D2_STYLE(Height, 1.0_pc)
        //         D2_STYLE(OnSubmit, [](TreeIter<ColorPickerDiamond> ptr, px::background color) {
        //             _core(ptr->state())->set_color(color);
        //         })
        //     D2_ELEM_END
        // D2_ELEM_END
    D2_TREE_END

    void impl::ColorPickerBase::set_color(px::background color)
    {
        _color = color;
        submit();
    }
    px::background impl::ColorPickerBase::get_color()
    {
        return _color;
    }

    void ColorPickerWindow::_state_change_impl(State state, bool value)
    {
        VirtualFlowBox::_state_change_impl(state, value);
        if (state == State::Created && value && empty())
        {
            data::width = 46.0_px;
            data::height = 10.0_px;
            data::zindex = 120;
            data::container_options |= ContainerOptions::EnableBorder;
            data::title = "<Pick Color>";

            if (screen()->is_keynav())
            {
                const auto [ swidth, sheight ] = context()->input()->screen_size();
                const auto [ width, height ] = box();
                data::x = ((swidth - width) / 2);
                data::y = ((sheight - height) / 2);
            }
            else
            {
                const auto [ x, y ] = context()->input()->mouse_position();
                data::x = x - (box().width / 2);
                data::y = y;
            }

            auto& src = this->state()->screen()->theme<WidgetTheme>();
            data::set<data::BorderHorizontalColor>(src.wg_border_horizontal());
            data::set<data::BorderVerticalColor>(src.wg_border_vertical());
            data::set<data::FocusedColor>(src.wg_hbg_button());
            data::set<data::BarColor>(style::dynavar<[](const auto& value) {
                return value.extend('.');
            }>(src.wg_border_horizontal()));

            color_picker::create_at(
                traverse(),
                TreeState::make<TreeState>(
                    parent()->traverse().asp(),
                    traverse().asp()
                ),
                true
            );
        }
    }
    void ColorPicker::_state_change_impl(State state, bool value)
    {
        FlowBox::_state_change_impl(state, value);
        if (state == State::Created && value && empty())
        {
            auto& src = this->state()->screen()->theme<WidgetTheme>();
            data::set<data::ContainerBorder>(true);
            data::set<data::BorderHorizontalColor>(src.wg_border_horizontal());
            data::set<data::BorderVerticalColor>(src.wg_border_vertical());

            color_picker::create_at(
                traverse(),
                TreeState::make<TreeState>(
                    parent()->traverse().asp(),
                    traverse().asp()
                ),
                false
            );
        }
    }

    void ColorPickerWindow::submit()
    {
        if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ColorPickerWindow>(shared_from_this()),
                get_color()
            );
        close();
    }
	void ColorPicker::submit()
	{
		if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ColorPicker>(shared_from_this()),
                get_color()
            );
	}
}
