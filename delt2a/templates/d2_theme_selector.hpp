#ifndef D2_THEME_SELECTOR_HPP
#define D2_THEME_SELECTOR_HPP

#include "../elements/d2_std.hpp"
#include "../d2_colors.hpp"
#include "d2_popup_theme_base.hpp"
#include "d2_color_picker.hpp"

namespace d2
{
	namespace style
	{
		struct ThemeSelector
		{
			using accent_vec = std::vector<px::background>;
			using theme = std::variant<accent_vec, string>;
			std::function<void(Element::TraversalWrapper, theme)> on_submit{ nullptr };
			std::vector<string> themes{};
			int accents{ 0 };

			template<uai_property Property>
			auto at_style()
			{
				D2_UAI_SETUP_EMPTY
				D2_UAI_VAR_START
				D2_UAI_GET_VAR_A(0, on_submit)
				D2_UAI_GET_VAR(1, themes, Masked)
				D2_UAI_GET_VAR_A(2, accents)
				D2_UAI_VAR_END;
			}
		};

		template<std::size_t PropBase>
		struct IThemeSelector : ThemeSelector, InterfaceHelper<IThemeSelector, PropBase, 3>
		{
			using data = ThemeSelector;
			enum Property : uai_property
			{
				OnSubmit = PropBase,
				Themes,
				MaxAccents,
			};
		};
		using IZThemeSelector = IThemeSelector<0>;
	}
	namespace templ
	{
		class ThemeSelector : public style::UAIC<dx::VirtualBox, ThemeSelector, style::IThemeSelector>
		{
		public:
			using data = style::UAIC<VirtualBox, ThemeSelector, style::IThemeSelector>;
			using data::data;
			D2_UAI_CHAIN(VirtualBox)
		protected:
			data::accent_vec _accents{};

			static eptr<ThemeSelector> _core(TreeState::ptr state)
			{
				return
					state->core()->traverse().as<ThemeSelector>();
			}

			virtual void _state_change_impl(State state, bool value) override
			{
				VirtualBox::_state_change_impl(state, value);
				if (state == State::Swapped && !value && empty())
				{
					VirtualBox::width = 42.0_px;
					VirtualBox::height = 16.0_px;
					VirtualBox::zindex = 120;
					VirtualBox::container_options =
						ContainerOptions::EnableBorder | ContainerOptions::TopFix;
					VirtualBox::title = "<Pick Theme>";

					if (screen()->is_keynav())
					{
						const auto [ swidth, sheight ] = context()->input()->screen_size();
						const auto [ width, height ] = box();
						VirtualBox::x = ((swidth - width) / 2);
						VirtualBox::y = ((sheight - height) / 2);
					}
					else
					{
						const auto [ x, y ] = context()->input()->mouse_position();
						VirtualBox::x = x - (box().width / 2);
						VirtualBox::y = y;
					}

					auto& theme = this->state()->screen()->theme<PopupTheme>();
					data::set<VirtualBox::BorderHorizontalColor>(theme.pt_border_horizontal());
					data::set<VirtualBox::BorderVerticalColor>(theme.pt_border_vertical());
					data::set<VirtualBox::FocusedColor>(theme.pt_hbg_button());
					data::set<VirtualBox::BackgroundColor>(style::dynavar<[](const auto& value) {
						return value.alpha(0.3f);
					}>(theme.pt_bg_secondary()));
					data::set<VirtualBox::BarColor>(style::dynavar<[](const auto& value) {
						return value.extend('.');
					}>(theme.pt_border_horizontal()));

					using namespace dx;
					D2_STATELESS_TREE(theme_picker)
						D2_ELEM(Button, exit)
							D2_STYLE(Value, "<X>")
							D2_STYLE(X, 1.0_pxi)
							D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
							D2_STYLE(FocusedColor, D2_VAR(PopupTheme, pt_hbg_button()))
							D2_STYLE(OnSubmit, [state](d2::Element::TraversalWrapper ptr) {
								_core(state)->close();
							})
						D2_ELEM_END(exit)
						D2_ELEM_UNNAMED(Switch)
							D2_STYLE(Width, 1.0_pc)
							D2_STYLE(EnabledForegroundColor, D2_VAR(PopupTheme, pt_hbg_button()))
							D2_STYLE(EnabledBackgroundColor, d2::colors::w::transparent)
							D2_STYLE(DisabledForegroundColor, D2_VAR(PopupTheme, pt_text()))
							D2_STYLE(DisabledBackgroundColor, d2::colors::w::transparent)
							D2_STYLE(SeparatorColor, D2_VAR(PopupTheme, pt_border_vertical()))
							D2_STYLE(FocusedColor, D2_DYNAVAR(PopupTheme, pt_hbg_button(), value.alpha(0.7f)))
							D2_STYLE(Options, Switch::opts{ "Themes", "Accents" })
							D2_STYLE(OnChange, [](d2::Element::TraversalWrapper ptr, int idx) {
								(ptr->parent()->traverse()/"logic"/"themes")
									->setstate(d2::Element::Display, idx == 0);
								(ptr->parent()->traverse()/"logic"/"accents")
									->setstate(d2::Element::Display, idx == 1);
							})
						D2_UELEM_END
						D2_ELEM_NESTED(Box, logic)
							D2_STYLE(Y, 1.0_px)
							D2_STYLE(Width, 1.0_pc)
							D2_STYLE(Height, 3.0_pxi)
						D2_ELEM_NESTED_BODY(logic)
							D2_ELEM(ScrollBox, themes)
								D2_STYLE(Width, 1.0_pc)
								D2_STYLE(Height, 1.0_pc)
								(*ptr.as<ScrollBox>()->scrollbar().as<VerticalSlider>())
									.set<VerticalSlider::SliderColor>(D2_VAR(PopupTheme, pt_bg_button()))
									.set<VerticalSlider::BackgroundColor>(D2_VAR(PopupTheme, pt_bg_button()));
							D2_ELEM_END(themes)
							D2_ELEM_NESTED(Box, accents)
								D2_STYLE(Width, 1.0_pc)
								D2_STYLE(Height, 1.0_pc)
							D2_ELEM_NESTED_BODY(accents)
								D2_ELEM_NESTED(Box, controls)
									D2_STYLE(X, 0.0_relative)
									D2_STYLE(Width, 0.5_pc)
									D2_STYLE(Height, 1.0_pc)
								D2_ELEM_NESTED_BODY(controls)
									D2_ELEM_UNNAMED(Button)
										D2_STYLE(Value, "Push Accent")
										D2_STYLE(X, 0.0_center)
										D2_STYLE(Y, 2.0_relative)
										D2_STYLE(OnSubmit, [](Element::TraversalWrapper ptr) {
											auto p = ptr->screen()->traverse()
												.asp()
												->override<ColorPicker>("__color_picker__")
												.as<ColorPicker>();
											p->set<ColorPicker::OnSubmit>([local = ptr->parent()->traverse()](Element::TraversalWrapper ptr, px::background color) {
												_core(local->state())->_accents.push_back(color);
												(*(local->parent()->traverse()/"list"/"accents").asp()
													->create<Text>().as<Text>())
													.set<Text::X>(0.0_center)
													.set<Text::Y>(0.0_relative)
													.set<Text::ForegroundColor>(color.alpha(std::max(color.a / 255.f, 0.6f)))
													.set<Text::Value>(std::format("[{} {} {} {}]",
														color.r, color.g, color.b, color.a
													));
											});
										})
										D2_STYLES_APPLY(impl::button_react)
									D2_UELEM_END
									D2_ELEM_UNNAMED(Button)
										D2_STYLE(Value, "Clear Accents")
										D2_STYLE(X, 0.0_center)
										D2_STYLE(Y, 1.0_relative)
										D2_STYLE(OnSubmit, [](Element::TraversalWrapper ptr) {
											(ptr->parent()->traverse()/"list"/"accents").asp()->clear();
											_core(ptr->state())->_accents.clear();
										})
										D2_STYLES_APPLY(impl::button_react)
									D2_UELEM_END
									D2_ELEM_UNNAMED(Button)
										D2_STYLE(Value, "Submit Accents")
										D2_STYLE(X, 0.0_center)
										D2_STYLE(Y, 1.0_relative)
										D2_STYLE(OnSubmit, [](Element::TraversalWrapper ptr) {
											_core(ptr->state())->submit();
										})
										D2_STYLES_APPLY(impl::button_react)
									D2_UELEM_END
								D2_ELEM_NESTED_END(controls)
								D2_ELEM_NESTED(Box, list)
									D2_STYLE(X, 0.0_relative)
									D2_STYLE(Width, 0.5_pc)
									D2_STYLE(Height, 1.0_pc)
								D2_ELEM_NESTED_BODY(list)
									D2_ELEM_UNNAMED(Text)
										D2_STYLE(Value, "Accent List")
										D2_STYLE(X, 0.0_center)
										D2_STYLE(Y, 1.0_relative)
										D2_STYLE(ForegroundColor, D2_VAR(PopupTheme, pt_text()))
									D2_UELEM_END
									D2_ELEM(Box, accents)
										D2_STYLE(Width, 1.0_pc)
										D2_STYLE(Height, 1.0_pxi)
										D2_STYLE(Y, 0.0_relative)
									D2_ELEM_END(accents)
								D2_ELEM_NESTED_END(list)
							D2_ELEM_NESTED_END(accents)
						D2_ELEM_NESTED_END(logic)
					D2_TREE_END(theme_picker)

					theme_picker::create_at(
						traverse(),
						TreeState::make<TreeState>(
							this->state()->screen(),
							this->state()->context(),
							parent()->traverse().asp(),
							traverse().asp()
						)
					);

					(at("logic")/"themes")->setstate(Element::Display, true);
					(at("logic")/"accents")->setstate(Element::Display, false);

					_update_theme_list();
				}
			}
			virtual void _signal_write_impl(write_flag type, unsigned int prop, ptr element) noexcept override
			{
				if (element.get() == this)
				{
					if (prop == data::MaxAccents)
					{

					}
					else if (prop == data::Themes)
					{
						_update_theme_list();
					}
				}
			}

			void _update_theme_list()
			{
				using namespace dx;
				auto& theme = this->state()->screen()->theme<PopupTheme>();
				auto list = (at("logic")/"themes").asp();
				for (decltype(auto) it : data::themes)
				{
					auto elem = list->create<Button>().as<Button>();
					(*elem)
						.set<Button::Value>(it)
						.set<Button::ForegroundColor>(theme.pt_text())
						.set<Button::FocusedColor>(theme.pt_hbg_button())
						.set<Button::BackgroundColor>(d2::colors::w::transparent)
						.set<Button::OnSubmit>([](Element::TraversalWrapper elem) {
							_core(elem->state())->submit(
								elem.as<Button>()->get<Button::Value>()
							);
						});
				}
			}
		public:
			void submit(const std::string& name)
			{
				if (data::on_submit != nullptr)
					data::on_submit(shared_from_this(), name);
				close();
			}
			void submit()
			{
				if (data::on_submit != nullptr)
					data::on_submit(shared_from_this(), _accents);
				close();
			}
		};
	}
}

#endif // D2_THEME_SELECTOR_HPP
