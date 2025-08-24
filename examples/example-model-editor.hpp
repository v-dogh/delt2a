#ifndef D2_MODEL_EDITOR_HPP
#define D2_MODEL_EDITOR_HPP

#include "../delt2a/d2_std.hpp"
#include "../delt2a/elements/d2_std.hpp"
#include "../delt2a/templates/d2_exit_prompt.hpp"
#include "../delt2a/templates/d2_color_picker.hpp"
#include "../delt2a/templates/d2_file_explorer.hpp"
#include "../delt2a/templates/d2_theme_selector.hpp"
#include "../delt2a/templates/d2_popup_theme_base.hpp"
#include "../delt2a/programs/d2_debug_box.hpp"

// To run this example call editor::run()
namespace editor
{
	using namespace d2::dx;

	class ModelEditorState : public d2::TreeState
	{
	public:
		struct Theme : d2::style::Theme, d2::ctm::PopupTheme
		{
			D2_DEPENDENCY(d2::px::background, primary_background)
			D2_DEPENDENCY(d2::px::background, secondary_background)
			D2_DEPENDENCY(d2::px::combined, border_vertical)
			D2_DEPENDENCY(d2::px::combined, border_horizontal)
			D2_DEPENDENCY(d2::px::foreground, text)
			D2_DEPENDENCY(d2::px::combined, text_ptr)
			D2_DEPENDENCY(d2::px::background, button)
			D2_DEPENDENCY(d2::px::foreground, button_foreground)
			D2_DEPENDENCY(d2::px::foreground, button_react_foreground)
			D2_DEPENDENCY(d2::px::background, button_hover)
			D2_DEPENDENCY(d2::px::background, button_press)

			D2_DEPENDENCY_LINK(pt_bg_primary, primary_background)
			D2_DEPENDENCY_LINK(pt_bg_secondary, secondary_background)
			D2_DEPENDENCY_LINK(pt_border_vertical, border_vertical)
			D2_DEPENDENCY_LINK(pt_border_horizontal, border_horizontal)

			D2_DEPENDENCY_LINK(pt_text, text)
			D2_DEPENDENCY_LINK(pt_text_ptr, text_ptr)

			D2_DEPENDENCY_LINK(pt_bg_button, button)
			D2_DEPENDENCY_LINK(pt_fg_button, button_foreground)
			D2_DEPENDENCY_LINK(pt_pbg_button, button_press)
			D2_DEPENDENCY_LINK(pt_pfg_button, button_react_foreground)
			D2_DEPENDENCY_LINK(pt_hbg_button, button_hover)

			void accents(
				d2::px::background bgp,
				d2::px::background bgs,
				d2::px::combined bdh,
				d2::px::combined bdv,
				d2::px::foreground txt,
				d2::px::foreground crs,
				d2::px::background bt,
				d2::px::background bth,
				d2::px::background btp,
				d2::px::foreground btf,
				d2::px::foreground btrf
			)
			{
				primary_background = bgp;
				secondary_background = bgs;
				border_horizontal = bdh;
				border_vertical = bdv;
				text = txt;
				text_ptr = crs;
				button = bt;
				button_hover = bth;
				button_press = btp;
				button_foreground = btf;
				button_react_foreground = btrf;
			}

			static void accent_dynamic(Theme* theme, d2::px::background tint)
			{
				auto from_tint_manual = [](float r, float g, float b, d2::px::background tint)
				{
					return d2::px::background{
						.r = static_cast<std::uint8_t>(tint.r * r),
						.g = static_cast<std::uint8_t>(tint.g * g),
						.b = static_cast<std::uint8_t>(tint.b * b),
					};
				};
				auto from_tint = [&tint, &from_tint_manual](float r, float g, float b)
				{
					return from_tint_manual(r, g, b, tint);
				};

				theme->accents(
					from_tint(0.25f, 0.3f, 0.35f),
					from_tint(0.18f, 0.2f, 0.25f),
					from_tint(0.95f, 0.95f, 0.95f).extend('_'),
					from_tint(0.95f, 0.95f, 0.95f).extend('|'),
					from_tint(0.95f, 0.95f, 0.95f),
					from_tint(0.95f, 0.95f, 0.95f).extend('_'),
					from_tint(0.5f, 0.5f, 0.5f),
					from_tint(0.75f, 0.75f, 0.75f),
					from_tint(0.95f, 0.95f, 0.95f),
					from_tint(0.95f, 0.95f, 0.95f),
					from_tint(0.25f, 0.25f, 0.25f)
				);
			}
			static void accent_default(Theme* theme)
			{
				accent_dynamic(theme, d2::colors::b::slate_blue);
			}
		};
	protected:
		d2::IOContext::EventListener setpixel_ev_{};
		d2::px::combined brush_{ d2::px::combined(d2::colors::w::white) };
		d2::px::combined brush_secondary_{ d2::px::combined(d2::colors::w::black) };
	public:
		using TreeState::TreeState;

		virtual void swap_in() override
		{
			if (!setpixel_ev_)
			{
				(core()->traverse()/"editor"/"model")
					.as<Matrix>()
					->set<Matrix::Model>(d2::MatrixModel::make());

				setpixel_ev_ = context()->listen<d2::IOContext::Event::MouseInput>([](auto, auto, TreeState::ptr state) {
					auto model =
						(state->core()->traverse()/"editor"/"model")
						.as<Matrix>();
					if (model->getstate(d2::Element::Hovered))
					{
						const auto [ x, y ] = model->mouse_object_space();
						model->apply_set<Matrix::Model>([&](d2::MatrixModel::ptr& model) {
							if (x >= 0 && x < model->width() &&
								y >= 0 && y < model->height())
							{
								if (state->context()->input()->is_pressed_mouse(d2::sys::SystemInput::Left))
								{
									model->at(x, y) =
										state->as<ModelEditorState>()->brush_;
								}
								else if (state->context()->input()->is_pressed_mouse(d2::sys::SystemInput::Right))
								{
									model->at(x, y) =
										state->as<ModelEditorState>()->brush_secondary_;
								}
							}
						});
					}
				});
			}
		}

		void save(std::filesystem::path path)
		{
			auto r = core()->traverse();
			const auto opt =
				(r/"sidebar"/"project"/"model-type")
				.as<Dropdown>()->index();
			try
			{
				(r/"editor"/"model")
					.as<Matrix>()
					->get<Matrix::Model>()
					->save_model(
						path.string(),
						opt == 0 ? d2::Screen::ModelType::Visual : d2::Screen::ModelType::Raw
					);
			}
			catch (...) {}
		}
		void open(std::filesystem::path path)
		{
			auto r = core()->traverse();
			const auto opt =
				(r/"sidebar"/"project"/"model-type")
				.as<Dropdown>()->index();
			try
			{
				(r/"editor"/"model")
					.as<Matrix>()
					->get<Matrix::Model>()
					->load_model(
						path.string(),
						opt == 0 ? d2::Screen::ModelType::Visual : d2::Screen::ModelType::Raw
					);
				(r/"editor"/"topbar"/"title")
					.as<Text>()
					->set<Text::Value>(std::format("<{}>", path.string()));
			}
			catch (...) {}
		}
		void set_color(d2::px::background color)
		{
			auto opts = core()->traverse()/"sidebar"/"style"/"options";
			auto mode = (opts/"brush-choice").as<Button>();
			auto layer = (opts/"color-choice").as<Button>();

			const auto fg = layer->apply_get<Button::Value>([](const auto& mode) { return mode[0] == 'F'; });
			const auto pr = mode->apply_get<Button::Value>([&, this](const auto& mode) { return mode[0] == 'L'; });

			// lol
			if (pr)
			{
				if (fg)
				{
					brush_.rf = color.r;
					brush_.gf = color.g;
					brush_.bf = color.b;
					brush_.af = color.a;
				}
				else
				{
					brush_.r = color.r;
					brush_.g = color.g;
					brush_.b = color.b;
					brush_.a = color.a;
				}
			}
			else
			{
				if (fg)
				{
					brush_secondary_.rf = color.r;
					brush_secondary_.gf = color.g;
					brush_secondary_.bf = color.b;
					brush_secondary_.af = color.a;
				}
				else
				{
					brush_secondary_.r = color.r;
					brush_secondary_.g = color.g;
					brush_secondary_.b = color.b;
					brush_secondary_.a = color.a;
				}
			}
		}
		void set_brush(d2::value_type value)
		{
			auto mode = (core()->traverse()
				/"sidebar"/"style"/"options"/"brush-choice").as<Button>();
			mode->apply_get<Button::Value>([&, this](const auto& mode) {
				if (mode[0] == 'L')
				{
					brush_.v = value;
				}
				else
				{
					brush_secondary_.v = value;
				}
			});
		}
		void resize()
		{
			try
			{
				auto r = core()->traverse();
				auto dims = r/"sidebar"/"style";
				const auto width =
					std::abs(std::stoi((dims/"width")
					.as<Input>()
					->get<Input::Value>()));
				const auto height =
					std::abs(std::stoi((dims/"height")
					.as<Input>()
					->get<Input::Value>()));

				auto model = (r/"editor"/"model").as<Matrix>();
				model->apply_set<Matrix::Model>([&](d2::MatrixModel::ptr& model) {
					model->set_size(width, height);
					model->fill({ .v = ' ' });
				});
			}
			catch (...) {}
		}
	};
	using MDTheme = ModelEditorState::Theme;

	D2_STYLESHEET_BEGIN(navigable)
		D2_STYLE(FocusedColor, D2_VAR(MDTheme, button_hover))
	D2_STYLESHEET_END(navigable)

	D2_STYLESHEET_BEGIN(align_vertical)
		D2_STYLE(Y, 0.0_relative)
		D2_STYLE(X, 0.0_center)
	D2_STYLESHEET_END(align_vertical)

	D2_STYLESHEET_BEGIN(button_react)
		D2_STYLE(BackgroundColor, D2_VAR(MDTheme, button))
		D2_STYLE(ForegroundColor, D2_VAR(MDTheme, button_foreground))
		D2_INTERPOLATE_TWOWAY_AUTO(Hovered, 500, Linear, BackgroundColor, D2_VAR(MDTheme, button_hover));
		D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, BackgroundColor, D2_VAR(MDTheme, button_press));
		D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, ForegroundColor, D2_VAR(MDTheme, button_react_foreground));
		D2_STYLES_APPLY(navigable)
	D2_STYLESHEET_END(button_react)

	D2_STATEFUL_TREE(model_editor, ModelEditorState)
		// D2_EMBED(d2::prog::debug)
		// The main editor
		D2_ELEM_NESTED(Box, editor)
			D2_STYLE(X, 0.0_relative)
			D2_STYLE(Width, 0.0_relative)
			D2_STYLE(Height, 1.0_pc)
			D2_STYLE(BackgroundColor,
				D2_VAR(MDTheme, secondary_background)
			)
		D2_ELEM_NESTED_BODY
			D2_ELEM_NESTED(Box, topbar)
				D2_STYLE(Width, 1.0_pc)
				D2_STYLE(Height, 3.0_px)
				D2_STYLE(BackgroundColor,
					D2_VAR(MDTheme, primary_background)
				)
			D2_ELEM_NESTED_BODY
				D2_ELEM(Text, title)
					D2_STYLE(X, 0.0_center)
					D2_STYLE(Y, 0.0_center)
					D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
				D2_ELEM_END
			D2_ELEM_NESTED_END
			D2_ELEM(Matrix, model)
				D2_STYLE(X, 0.0_center)
				D2_STYLE(Y, 0.0_center)
			D2_ELEM_END
		D2_ELEM_NESTED_END
		// Sidebar with all of the options
		D2_ELEM_NESTED(Box, sidebar)
			D2_STYLE(ZIndex, 1)
			D2_STYLE(X, 0.0_relative)
			D2_STYLE(Width, 0.2_pc)
			D2_STYLE(Height, 1.0_pc)
			D2_STYLE(ContainerBorder, true)
			D2_STYLE(ContainerDisableRight, true)
			D2_STYLE(BorderHorizontalColor, d2::px::combined{ .a = 0, .af = 0 })
			D2_STYLE(BorderVerticalColor, D2_VAR(MDTheme, border_vertical))
			D2_STYLE(BackgroundColor,
				D2_VAR(MDTheme, primary_background)
			)
			// Resizing logic
			static bool rsize = false;
			D2_LISTEN_EXPR(Clicked, true, rsize = true)
			D2_LISTEN_GLOBAL(MouseInput)
				const auto rel = ptr->context()->input()
					->is_pressed_mouse(d2::sys::SystemInput::Left, d2::sys::SystemInput::KeyMode::Release);
				if (rel) rsize = false;
				else if (rsize)
				{
					const auto [ x, y ] = ptr->mouse_object_space();
					ptr.as<Box>()
						->set<Box::Width>(ptr->box().width - x);
				}
			D2_LISTEN_GLOBAL_END
		D2_ELEM_NESTED_BODY
			D2_ELEM_NESTED(Box, project)
				D2_STYLE(Y, 0.0_relative)
				D2_STYLE(Width, 1.0_pxi)
				D2_STYLE(Height, 0.5_pc)
			D2_ELEM_NESTED_BODY
				D2_ELEM(Text)
					D2_STYLE(Value, "Project")
					D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
					D2_STYLES_APPLY(align_vertical)
				D2_ELEM_END
				D2_ELEM_NESTED(Box)
					D2_STYLES_APPLY(align_vertical)
				D2_UELEM_NESTED_BODY
					D2_ELEM(Button)
						D2_STYLE(Value, "Open")
						D2_STYLE(X, 0.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
						D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
							using d2::ctm::FilesystemExplorer;
							ptr->screen()->traverse().asp()->override<FilesystemExplorer>("__fs_open__")
								.as<FilesystemExplorer>()->set<FilesystemExplorer::OnSubmit>(
									[](d2::Element::TraversalWrapper ptr) {
										ptr->state()
											->as<ModelEditorState>()
											->open(ptr.as<FilesystemExplorer>()->getpath());
									});
						})
						D2_STYLES_APPLY(button_react)
					D2_ELEM_END
					D2_ELEM(Button)
						D2_STYLE(Value, "Save")
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
						D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
							using d2::ctm::FilesystemExplorer;
							ptr->screen()->traverse().asp()->override<FilesystemExplorer>("__fs_save__")
								.as<FilesystemExplorer>()->set<FilesystemExplorer::OnSubmit>(
									[](d2::Element::TraversalWrapper ptr) {
										ptr->state()
											->as<ModelEditorState>()
											->save(ptr.as<FilesystemExplorer>()->getpath());
									});
						})
						D2_STYLES_APPLY(button_react)
					D2_ELEM_END
					D2_ELEM(Button)
						D2_STYLE(Value, "Exit")
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
						D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
							d2::ctm::exit_prompt::create_at(
								ptr->screen()->root(), ptr->state()
							);
						})
						D2_STYLES_APPLY(button_react)
					D2_ELEM_END
					D2_ELEM(Button)
						D2_STYLE(Value, "Theme")
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
						D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
							using d2::ctm::ThemeSelector;
							ptr->screen()->traverse().asp()->override<ThemeSelector>("__theme_select__")
								.as<ThemeSelector>()->set<ThemeSelector::OnSubmit>(
									[](d2::Element::TraversalWrapper ptr, const ThemeSelector::theme& theme) {
										if (std::holds_alternative<ThemeSelector::accent_vec>(theme))
										{
											auto& accents = std::get<ThemeSelector::accent_vec>(theme);
											if (!accents.empty())
											{
												MDTheme::accent_dynamic(
													&ptr->state()->screen()->theme<MDTheme>(),
													accents[0]
												);
											}
										}
									});
						})
						D2_STYLES_APPLY(button_react)
					D2_ELEM_END
				D2_ELEM_NESTED_END
				D2_ELEM(Dropdown, model-type)
					D2_STYLE(DefaultValue, "Model Type")
					D2_STYLE(ContainerBorder, true)
					D2_STYLE(ContainerTopFix, true)
					D2_STYLE(BorderHorizontalColor, D2_VAR(MDTheme, border_horizontal))
					D2_STYLE(BorderVerticalColor, D2_VAR(MDTheme, border_vertical))
					D2_STYLE(Options, Dropdown::opts{ "Visual", "Delta", "DeltaZ" })
					D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
					D2_STYLES_APPLY(navigable)
					D2_STYLES_APPLY(align_vertical)
				D2_ELEM_END
			D2_ELEM_NESTED_END
			D2_ELEM_NESTED(Box, style)
				D2_STYLE(Y, 0.0_relative)
				D2_STYLE(Width, 1.0_pxi)
				D2_STYLE(Height, 0.5_pc)
			D2_ELEM_NESTED_BODY
				D2_ELEM(Text)
					D2_STYLE(Value, "Styles")
					D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
					D2_STYLES_APPLY(align_vertical)
				D2_ELEM_END
				// Options
				D2_ELEM_NESTED(Box, options)
					D2_STYLES_APPLY(align_vertical)
				D2_ELEM_NESTED_BODY
					D2_ELEM(Button)
						D2_STYLE(Value, "Resize")
						D2_STYLE(X, 5.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
						D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
							ptr->state()
								->as<ModelEditorState>()
								->resize();
						})
						D2_STYLES_APPLY(button_react)
					D2_ELEM_END
					D2_ELEM(Button)
						D2_STYLE(Value, "Pick Color")
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
						D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
							using d2::ctm::ColorPicker;
							ptr->screen()->traverse().asp()->override<ColorPicker>("__color_picker__")
								.as<ColorPicker>()->set<ColorPicker::OnSubmit>(
									[](d2::Element::TraversalWrapper ptr, d2::px::background color) {
										ptr->state()
											->as<ModelEditorState>()
											->set_color(color);
									});
						})
						D2_STYLES_APPLY(button_react)
					D2_ELEM_END
					D2_ELEM(Button, brush-mode)
						D2_STYLE(Value, "Value Mode")
						D2_STYLE(Y, 0.0_relative)
						D2_STYLE(X, 0.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
						D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
							if (ptr.as<Button>()->get<Button::Value>()[0] == 'V')
								ptr.as<Button>()->set<Button::Value>("Hex Mode");
							else
								ptr.as<Button>()->set<Button::Value>("Value Mode");
						})
						D2_STYLES_APPLY(button_react)
					D2_ELEM_END
					D2_ELEM(Button, brush-choice)
						D2_STYLE(Value, "L-Brush")
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
						D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
							if (ptr.as<Button>()->get<Button::Value>()[0] == 'L')
								ptr.as<Button>()->set<Button::Value>("R-Brush");
							else
								ptr.as<Button>()->set<Button::Value>("L-Brush");
						})
						D2_STYLES_APPLY(button_react)
					D2_ELEM_END
					D2_ELEM(Button, color-choice)
						D2_STYLE(Value, "Bg-Color")
						D2_STYLE(X, 1.0_relative)
						D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
						D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
							if (ptr.as<Button>()->get<Button::Value>()[0] == 'F')
								ptr.as<Button>()->set<Button::Value>("Bg-Color");
							else
								ptr.as<Button>()->set<Button::Value>("Fg-Color");
						})
						D2_STYLES_APPLY(button_react)
					D2_ELEM_END
				D2_ELEM_NESTED_END
				// Value of the brush
				D2_ELEM(Input)
					D2_STYLE(Pre, "Value: ")
					D2_STYLE(Width, 12.0_pxi)
					D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
					D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr, d2::string value) {
						if (ptr.up().asp()
							->at("options").asp()
							->at("brush-mode").as<Button>()
							->get<Button::Value>()[0] == 'V')
						{
							ptr->state()
								->as<ModelEditorState>()
								->set_brush(value[0]);
						}
						else
						{
							ptr->state()
								->as<ModelEditorState>()
								->set_brush(static_cast<d2::value_type>(std::stoi(value, 0, 16)));
						}
					})
					D2_STYLES_APPLY(align_vertical)
				D2_ELEM_END
				// Size of the model
				D2_ELEM(Input, width)
					D2_STYLE(Pre, "Width: ")
					D2_STYLE(Width, 12.0_pxi)
					D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
					D2_STYLES_APPLY(align_vertical)
				D2_ELEM_END
				D2_ELEM(Input, height)
					D2_STYLE(Pre, "Height: ")
					D2_STYLE(Width, 12.0_pxi)
					D2_STYLE(ForegroundColor, D2_VAR(MDTheme, text))
					D2_STYLES_APPLY(align_vertical)
				D2_ELEM_END
			D2_ELEM_NESTED_END
		D2_ELEM_NESTED_END
	D2_TREE_END

	inline void run()
	{
		d2::Screen::make<d2::tree::make<model_editor, "Test">>(
			d2::IOContext::make<
				d2::os::input,
				d2::os::output,
				d2::os::clipboard
			>(),
			d2::style::Theme::make<MDTheme>(MDTheme::accent_dynamic, d2::colors::w::white)
		)->start_blocking(
			d2::Screen::fps(144),
			d2::Screen::Profile::Stable
		);
	}
}

#endif // D2_MODEL_EDITOR_HPP
