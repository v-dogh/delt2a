#include "d2_color_picker.hpp"
#include "d2_widget_theme_base.hpp"
#include <d2_pixel.hpp>
#include <d2_tree_element_frwd.hpp>

namespace d2::ex
{
    class ColorPickerDiamond : public style::
                                   UAI<ColorPickerDiamond,
                                       style::ILayout,
                                       style::IResponsive<ColorPickerDiamond, px::background>::type>
    {
    public:
        using data = style::
            UAI<ColorPickerDiamond,
                style::ILayout,
                style::IColors,
                style::IResponsive<ColorPickerDiamond, px::background>::type>;
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

    // clang-format off
    D2_STYLESHEET_BEGIN(color_preset, px::background color)
        D2_STYLE(Value, " ")
        D2_STYLE(X, 1.0_relative)
        D2_STYLE(BackgroundColor, color)
        D2_STYLE(ForegroundColor, color)
        D2_ON(Focused)
            D2_STYLE(Value, charset::icon::middle_square)
            D2_STYLE(BackgroundColor, color.mask(d2::colors::w::black, 0.2f))
            _core(ptr->state())->set_color(color);
        D2_ON_END
        D2_OFF(Focused)
            D2_STYLE(Value, " ")
            D2_STYLE(BackgroundColor, color)
        D2_OFF_END
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
            D2_STYLE(Y, 1.0_relative)
            D2_STYLE(Height, 3.0_px)
            constexpr std::array samples = {
                std::array{
                    colors::w::dark_gray,
                    colors::w::silver,
                    colors::w::white,
                    colors::r::red,
                    colors::g::green,
                    colors::b::blue,
                    colors::w::transparent,
                },
                std::array{
                    colors::g::sea_green,
                    colors::g::lime,
                    colors::r::yellow,
                    colors::r::chrome_yellow,
                    colors::r::coral,
                    colors::r::magenta,
                    colors::r::firebrick,
                },
                std::array{
                    colors::g::olive,
                    colors::g::forest_green,
                    colors::b::turquoise,
                    colors::b::cyan,
                    colors::b::indigo,
                    colors::b::dark_slate_blue,
                    colors::w::transparent,
                }
            };
            for (std::size_t i = 0; i < samples.size(); i++)
            {
                D2_ELEM(Button)
                    D2_STYLES_APPLY(color_preset, samples[i][0])
                    if (i) D2_STYLE(Y, 0.0_relative)
                    if (i % 2 == 0) D2_STYLE(X, 1.0_relative)
                    else D2_STYLE(X, 0.0_relative)
                D2_ELEM_END
                for (std::size_t j = 1; j < samples[i].size(); j++)
                {
                    if (samples[i][j].a != 0)
                    {
                        D2_ELEM(Button)
                            D2_STYLES_APPLY(color_preset, samples[i][j])
                        D2_ELEM_END
                    }
                }
            }
        D2_ELEM_END
        // Value
        D2_ELEM(FlowBox, value)
            D2_STYLE(Y, 1.0_relative)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 0.0_relative)
            D2_ANCHOR(Input, value)
            D2_ANCHOR(Slider, r)
            D2_ANCHOR(Slider, g)
            D2_ANCHOR(Slider, b)
            D2_ANCHOR(Slider, a)
            static auto hex_to_rgba = [](const string& hex) -> std::optional<px::background>
            {
                auto htob = [&](std::size_t pos) -> px::background::component {
                    return static_cast<px::background::component>(std::stoi(hex.substr(pos, 2), nullptr, 16));
                };
                if (hex.length() != 8)
                    return std::nullopt;
                return px::background{
                    htob(0),
                    htob(2),
                    htob(4),
                    htob(6)
                };
            };
            static auto rgba_to_hex = [](px::background color) -> string 
            {
                return std::format("#{:02X}{:02X}{:02X}{:02X}", color.r, color.g, color.b, color.a);
            };
            auto update = [=]() 
            {
                px::background color{
                    .r = static_cast<px::background::component>(r->absolute_value()),
                    .g = static_cast<px::background::component>(g->absolute_value()),
                    .b = static_cast<px::background::component>(b->absolute_value()),
                    .a = static_cast<px::background::component>(a->absolute_value())
                };
                value->set<Input::Value>(rgba_to_hex(color));
                value->set<Input::ForegroundColor>(color);
                _core(ptr->state())->set_color(color);
            };
            D2_ELEM(FlowBox)
                D2_STYLE(X, 0.0_center)
                D2_STYLE(Y, 0.0_relative)
                D2_STYLE(Height, 1.0_px)
                D2_ELEM_ANCHOR(value)
                    D2_STYLE(Pre, "Value->0x")
                    D2_STYLE(Hint, "00000000")
                    D2_STYLE(X, 0.0_relative)
                    D2_STYLE(OnSubmit, [=](d2::TreeIter<Input> ptr, string v) {
                        const auto color = hex_to_rgba(v);
                        if (!color.has_value())
                            return;
                        r->reset_absolute(color->r);
                        g->reset_absolute(color->g);
                        b->reset_absolute(color->b);
                        a->reset_absolute(color->a);
                        _core(ptr->state())->set_color(*color);
                        value->set<Input::ForegroundColor>(*color);
                        ptr->set<Input::Value>(std::move(v));
                    })
                    D2_STYLES_APPLY(input_color)
                D2_ELEM_END
                D2_ELEM(Switch)
                    D2_STYLE(Options, Switch::opts{ "Diamond", "Sliders" })
                    D2_STYLE(X, 1.0_relative)
                    D2_STYLES_APPLY(switch_color)
                    ptr->reset(1);
                D2_ELEM_END
            D2_ELEM_END
            D2_ELEM(FlowBox, manual)
                D2_STYLE(Width, 1.0_pc)
                D2_STYLE(Y, 0.0_relative)
                D2_ELEM(Text)
                    D2_STYLE(Value, "R: ")
                    D2_STYLE(X, 1.0_relative)
                    D2_STYLES_APPLY(text_color)
                D2_ELEM_END
                D2_ELEM_ANCHOR(r)
                    D2_STYLES_APPLY(rgba_picker)
                    D2_STYLE(OnSubmit, [=](auto&&...) { update(); })
                D2_ELEM_END
                D2_ELEM(Text)
                    D2_STYLE(Value, "G: ")
                    D2_STYLE(X, 1.0_relative)
                    D2_STYLE(Y, 0.0_relative)
                    D2_STYLES_APPLY(text_color)
                D2_ELEM_END
                D2_ELEM_ANCHOR(g)
                    D2_STYLES_APPLY(rgba_picker)
                    D2_STYLE(OnSubmit, [=](auto&&...) { update(); })
                D2_ELEM_END
                D2_ELEM(Text)
                    D2_STYLE(Value, "B: ")
                    D2_STYLE(X, 1.0_relative)
                    D2_STYLE(Y, 0.0_relative)
                    D2_STYLES_APPLY(text_color)
                D2_ELEM_END
                D2_ELEM_ANCHOR(b)
                    D2_STYLES_APPLY(rgba_picker)
                    D2_STYLE(OnSubmit, [=](auto&&...) { update(); })
                D2_ELEM_END
                D2_ELEM(Text)
                    D2_STYLE(Value, "A: ")
                    D2_STYLE(X, 1.0_relative)
                    D2_STYLE(Y, 0.0_relative)
                    D2_STYLES_APPLY(text_color)
                D2_ELEM_END
                D2_ELEM_ANCHOR(a)
                    D2_STYLES_APPLY(rgba_picker)
                    D2_STYLE(OnSubmit, [=](auto&&...) { update(); })
                D2_ELEM_END
                r->reset_absolute(255);
                g->reset_absolute(255);
                b->reset_absolute(255);
                a->reset_absolute(255);
            D2_ELEM_END
        D2_ELEM_END
    D2_TREE_END
    // clang-format on

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
        if (state == State::Created && value)
        {
            data::width = 46.0_px;
            data::height = 10.0_px;
            data::zindex = 120;
            data::container_options |= ContainerOptions::EnableBorder;
            data::title = "<Pick Color>";

            if (screen()->is_keynav())
            {
                const auto [swidth, sheight] = context()->input()->screen_size();
                const auto [width, height] = box();
                data::x = ((swidth - width) / 2);
                data::y = ((sheight - height) / 2);
            }
            else
            {
                const auto [x, y] = context()->input()->mouse_position();
                data::x = x - (box().width / 2);
                data::y = y;
            }

            auto& src = this->state()->screen()->theme<WidgetTheme>();
            data::set<data::BorderHorizontalColor>(src.wg_border_horizontal());
            data::set<data::BorderVerticalColor>(src.wg_border_vertical());
            data::set<data::FocusedColor>(src.wg_hbg_button());
            data::set<data::BarColor>(
                style::dynavar<[](const auto& value) { return value.extend('.'); }>(
                    src.wg_border_horizontal()
                )
            );

            color_picker::create_at(
                traverse(), TreeState::make<TreeState>(nullptr, traverse().asp(), context()), true
            );

            VirtualFlowBox::_state_change_impl(state, value);
        }
        else
            VirtualFlowBox::_state_change_impl(state, value);
    }
    void ColorPicker::_state_change_impl(State state, bool value)
    {
        if (state == State::Created && value)
        {
            auto& src = this->state()->screen()->theme<WidgetTheme>();
            data::set<data::ContainerBorder>(true);
            data::set<data::BorderHorizontalColor>(src.wg_border_horizontal());
            data::set<data::BorderVerticalColor>(src.wg_border_vertical());

            color_picker::create_at(
                traverse(), TreeState::make<TreeState>(nullptr, traverse().asp(), context()), false
            );

            FlowBox::_state_change_impl(state, value);
        }
        else
            FlowBox::_state_change_impl(state, value);
    }

    void ColorPickerWindow::submit()
    {
        if (data::on_submit != nullptr)
            data::on_submit(
                std::static_pointer_cast<ColorPickerWindow>(shared_from_this()), get_color()
            );
        close();
    }
    void ColorPicker::submit()
    {
        if (data::on_submit != nullptr)
            data::on_submit(std::static_pointer_cast<ColorPicker>(shared_from_this()), get_color());
    }
} // namespace d2::ex
