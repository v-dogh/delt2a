#ifndef D2_ELEMENT_TEST_HPP
#define D2_ELEMENT_TEST_HPP

#include "../elements/d2_std.hpp"

namespace d2::prog
{
	// Slider
	D2_ELEM(Text, slider-out)
		D2_STYLE(Value, "Value: 0")
		D2_STYLE(X, 1.0_px)
		D2_STYLE(Y, 0.0_px)
	D2_ELEM_END(slider-out),
	D2_ELEM(Slider, slider)
		D2_STYLE(Width, 10.0_px)
		D2_STYLE(Min, 1)
		D2_STYLE(Max, 10)
		D2_STYLE(X, 1.0_px)
		D2_STYLE(Y, 1.0_px)
		D2_STYLE(OnChange, [](d2::Element::TraversalWrapper ptr, float rel, float abs) {
			(*ptr->screen()/"slider-out")
				.as<Text>()
				->set<Text::Value>(std::format("Value: {}", std::round(abs)));
		});
	D2_ELEM_END(slider),
	// Input
	D2_ELEM(Input, input)
		D2_STYLE(X, 1.0_px)
		D2_STYLE(Y, 3.0_px)
		D2_STYLE(Width, 32.0_px)
		D2_STYLE(Value, "we got input too??? Nah bruv fr fr")
		D2_STYLE(Pre, ">")
	D2_ELEM_END(input),
	// Dropdown
	D2_ELEM(Dropdown, drippy)
		D2_STYLE(Width, 15.0_px)
		D2_STYLE(X, 36.0_px)
		D2_STYLE(Y, 0.0_px)
		D2_STYLE(ContainerBorder, true)
		D2_STYLE(BorderHorizontalColor, d2::px::background{ .v = '+' })
		D2_STYLE(BorderVerticalColor, d2::px::background{ .v = '+' })
		D2_STYLE(DefaultValue, "Select")
		D2_STYLE(Options, Dropdown::opts{ "A", "B", "C" })
	D2_ELEM_END(drippy),
	D2_ELEM(Button, clicky)
		D2_STYLE(Value, "Click Me")
		D2_STYLE(Width, 16.0_px)
		D2_STYLE(X, 60.0_px)
		D2_STYLE(Y, 0.0_px)
		D2_STYLE(ContainerBorder, true)
		D2_STYLE(BorderHorizontalColor, d2::px::background{ .a = 0, .v = '+' })
		D2_STYLE(BorderVerticalColor, d2::px::background{ .a = 0, .v = '+' })

		D2_INTERPOLATE_TWOWAY_AUTO(Hovered, 500, Linear, BackgroundColor, d2::colors::w::silver);
		D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, BackgroundColor, d2::colors::w::white);
		D2_INTERPOLATE_TWOWAY_AUTO(Clicked, 100, Linear, ForegroundColor, d2::colors::w::black);
	D2_ELEM_END(clicky),
	// Scrollbox
	D2_ELEM_NESTED(ScrollBox, scrollbox)
		D2_STYLE(Y, 5.0_px)
		D2_STYLE(Width, 36.0_px)
		D2_STYLE(Height, 12.0_px)
		D2_STYLE(ContainerBorder, true)
		D2_STYLE(BorderHorizontalColor, d2::px::background{ .v = '+' })
		D2_STYLE(BorderVerticalColor, d2::px::background{ .v = '+' })
	D2_ELEM_NESTED_BODY(scrollbox)
		D2_ELEM_UNNAMED(Box)
			D2_STYLE(BackgroundColor, d2::colors::b::slate_blue)
			D2_STYLE(Height, 1.0_pc)
			D2_STYLE(Width, 1.0_pc)
			D2_STYLE(Y, 0.0_relative)
		D2_ELEM_NESTED_BODY()
			D2_ELEM_UNNAMED(Text)
				D2_STYLE(BackgroundColor, d2::colors::w::transparent)
				D2_STYLE(Value, "slate blue!")
			D2_ELEM_END()
		D2_ELEM_NESTED_END(),
		D2_ELEM_NESTED_UNNAMED(Box)
			D2_STYLE(BackgroundColor, d2::colors::b::dark_slate_blue)
			D2_STYLE(Height, 1.0_pc)
			D2_STYLE(Width, 1.0_pc)
			D2_STYLE(Y, 0.0_relative)
		D2_ELEM_NESTED_BODY()
			D2_ELEM_UNNAMED(Text)
				D2_STYLE(X, 0.0_pxi)
				D2_STYLE(Y, 0.0_pxi)
				D2_STYLE(BackgroundColor, d2::colors::w::transparent)
				D2_STYLE(Value, "dark slate blue!!")
			D2_ELEM_END()
		D2_ELEM_NESTED_END(),
		D2_ELEM_UNNAMED(Box)
			D2_STYLE(BackgroundColor, d2::colors::b::indigo)
			D2_STYLE(Height, 1.0_pc)
			D2_STYLE(Width, 1.0_pc)
			D2_STYLE(Y, 0.0_relative)
		D2_ELEM_NESTED_BODY()
			D2_ELEM_UNNAMED(Text)
				D2_STYLE(X, 0.0_center)
				D2_STYLE(Y, 0.0_center)
				D2_STYLE(BackgroundColor, d2::colors::w::transparent)
				D2_STYLE(Value, "indigo!!!")
			D2_ELEM_END()
		D2_ELEM_NESTED_END()
	D2_ELEM_NESTED_END(scrollbox),
	// Draggable box
	D2_ELEM_NESTED(VirtualBox, movable-box)
		D2_STYLE(Title, "<?\?\?>")
		D2_STYLE(ZIndex, 8)
		D2_STYLE(Width, 36.0_px)
		D2_STYLE(Height, 10.0_px)
		D2_STYLE(ContainerBorder, true)
		D2_STYLE(BorderHorizontalColor, d2::px::background{ .v = '_' })
		D2_STYLE(BorderVerticalColor, d2::px::background{ .v = '|' })
	D2_ELEM_NESTED_BODY(movable-box)
		D2_ELEM_UNNAMED(Text)
			D2_STYLE(X, 0.0_center)
			D2_STYLE(Y, 0.0_center)
			D2_STYLE(HandleNewlines, true)
			D2_STYLE(Value,
	R"(Im actually draggable???
	you can even collapse me!!!
	Simply right click my bar)"
	)
		D2_ELEM_END()
	D2_ELEM_NESTED_END(movable-box),
	// Fleshbox test
	D2_ELEM_NESTED_UNNAMED(Box)
		D2_STYLE(Height, 12.0_px)
		D2_STYLE(Width, 36.0_px)
		D2_STYLE(Y, 20.0_px)
		D2_STYLE(BackgroundColor, d2::colors::b::dark_slate_blue)
	D2_ELEM_NESTED_BODY()
		D2_ELEM_UNNAMED(Text)
			D2_STYLE(X, 0.0_center)
			D2_STYLE(Y, 0.0_relative)
			D2_STYLE(Value, "A")
		D2_ELEM_END(),
		D2_ELEM_UNNAMED(Text)
			D2_STYLE(X, 0.0_center)
			D2_STYLE(Y, 0.0_relative)
			D2_STYLE(Value, "B")
		D2_ELEM_END(),
		D2_ELEM_UNNAMED(Text)
			D2_STYLE(X, 0.0_center)
			D2_STYLE(Y, 0.0_relative)
			D2_STYLE(Value, "P")
		D2_ELEM_END(),
		D2_ELEM_UNNAMED(Text)
			D2_STYLE(X, 0.0_center)
			D2_STYLE(Y, 0.0_relative)
			D2_STYLE(Value, "Q")
		D2_ELEM_END()
	D2_ELEM_NESTED_END(),

	D2_STATELESS_TREE(model_editor, "Test")
		D2_ELEM_NESTED(Box, editor)
			D2_STYLE(Width, 26.0_pxi)
			D2_STYLE(Height, 3.0_pxi)
			D2_STYLE(X, 1.0_px)
			D2_STYLE(Y, 1.0_pxi)
			D2_STYLE(ContainerBorder, true)
			D2_STYLE(BorderVerticalColor, d2::px::background{ .v = '+' })
			D2_STYLE(BorderHorizontalColor, d2::px::background{ .v = '+' })
		D2_ELEM_NESTED_BODY(editor)
			D2_ELEM(Text, title)
				D2_STYLE(Value, "<unnamed>")
				D2_STYLE(Y, 0.0_px)
				D2_STYLE(X, 0.0_center)
				D2_STYLE(ZIndex, 60)
			D2_ELEM_END(title),
			D2_ELEM(Matrix, model)
				D2_STYLE(X, 0.0_center)
				D2_STYLE(Y, 0.0_center)
			D2_ELEM_END(model)
		D2_ELEM_NESTED_END(editor),
		D2_ELEM_NESTED(Box, sidebar)
			D2_STYLE(Width, 25.0_px)
			D2_STYLE(Height, 1.0_pc)
			D2_STYLE(X, -1.0_pxi)
			D2_STYLE(ContainerBorder, true)
			D2_STYLE(BorderVerticalColor, d2::px::background{ .v = '|' })
			D2_STYLE(BorderHorizontalColor, d2::px::background{ .v = ' ' })
		D2_ELEM_NESTED_BODY(sidebar)
			D2_ELEM_UNNAMED(Text)
				D2_STYLE(Value, "Project")
				D2_STYLE(X, 0.0_center)
				D2_STYLE(Y, 0.0_relative)
			D2_ELEM_END(),
			D2_ELEM(Button, save)
				D2_STYLE(Value, "Save")
				D2_STYLE(X, 0.0_relative)
				D2_STYLE(Y, 1.0_relative)
			D2_ELEM_END(save),
			D2_ELEM(Button, load)
				D2_STYLE(Value, "Open")
				D2_STYLE(X, 0.0_relative)
			D2_ELEM_END(load),
			D2_ELEM(Button, create)
				D2_STYLE(Value, "Create")
				D2_STYLE(X, 0.0_relative)
			D2_ELEM_END(create),
			D2_ELEM(Dropdown, model-type)
				D2_STYLE(DefaultValue, "Model Type")
				D2_STYLE(Options, Dropdown::opts{ "Visual", "Delta", "DeltaZ" })
				D2_STYLE(X, 0.0_center)
				D2_STYLE(Y, 1.0_relative)
				// D2_STYLE(ContainerBorder, true)
				// D2_STYLE(BorderHorizontalColor, d2::px::background{ .v = '_' })
				// D2_STYLE(BorderVerticalColor, d2::px::background{ .v = '|' })
			D2_ELEM_END(model_type)
		D2_ELEM_NESTED_END(sidebar),
		D2_INJECT_TREE(d2::templ::debug)
	D2_TREE_END(model_editor)

	struct FilesystemExplorerState : d2::TreeState
	{
		std::vector<std::string> forward_buffer{};
		int forward_ptr{ 0 };

		std::filesystem::path base_path{ "/" };
		d2::Unit width{};
		d2::Unit height{};
		int xpos{};
		int ypos{};

		using TreeState::TreeState;
		FilesystemExplorerState(
			d2::Screen::ptr src,
			d2::IOContext::ptr ctx,
			std::shared_ptr<d2::ParentElement> rptr,
			d2::Unit width,
			d2::Unit height
		) :
			TreeState(src, ctx, rptr),
			width(width),
			height(height)
		{
			const auto [ x, y ] = src
				->context()
				->input()
				->mouse_position();
			xpos = x;
			ypos = y;
		}
		FilesystemExplorerState(
			d2::Screen::ptr src,
			d2::IOContext::ptr ctx,
			std::shared_ptr<d2::ParentElement> rptr,
			d2::Unit width,
			d2::Unit height,
			int xpos,
			int ypos
		) :
			TreeState(src, ctx, rptr),
			width(width),
			height(height),
			xpos(xpos),
			ypos(ypos)
		{}

		virtual void swap_in() override
		{
			setpath("/");
		}

		void forwards()
		{
			if (forward_ptr < forward_buffer.size())
			{
				setpath(base_path/forward_buffer[forward_ptr]);
				forward_ptr++;
			}
		}
		void backwards()
		{
			if (forward_ptr)
				forward_ptr--;
			setpath(base_path.parent_path());
		}

		void rselect(const std::string& filename)
		{

		}
		void select(const std::filesystem::path& path)
		{

		}

		void setrpath(const std::string& filename)
		{
			if (!
				forward_buffer.empty() &&
				filename != forward_buffer[forward_ptr] &&
				forward_ptr < forward_buffer.size() - 1
				)
			{
				forward_buffer.clear();
				forward_ptr = 0;
			}
			else
			{
				forward_ptr++;
			}
			forward_buffer.push_back(filename);
			setpath(base_path/filename);
		}
		void setpath(const std::filesystem::path& path)
		{
			const auto dir = (*root()/"__fs__"/"folder").asp();
			const auto out = (*root()/"__fs__"/"path").as<Text>();
			out->set<Text::Value>(path.string());
			dir->clear();
			if (std::filesystem::is_directory(path))
			{
				for (decltype(auto) it : std::filesystem::directory_iterator(path))
				{
					try
					{
						const auto entry = dir->create<Text>().as<Text>();
						entry->set<Text::Y>(0.0_relative);
						if (it.is_regular_file())
						{
							entry->set<Text::Value>(std::format(
								"{:<10} : {:%d.%m.%Y - %H:%M}, {}KiB <file>",
								it.path().filename().string(),
								it.last_write_time(),
								it.file_size()
							));
							entry->listen(d2::Element::State::Clicked, false, [name = it.path().filename()](
								d2::TreeState::ptr state, d2::Element::TraversalWrapper ptr, auto&&...
							) {
								try { state->as<FilesystemExplorerState>()->rselect(name); }
								catch (...) {}
							});
						}
						else
						{
							entry->set<Text::Value>(std::format(
								"{:<10} : {:%d.%m.%Y - %H:%M} <{}>",
								it.path().filename().string(),
								it.last_write_time(),
								it.is_directory() ? "folder" : "other"
							));
							entry->listen(d2::Element::State::Clicked, false, [name = it.path().filename()](
								d2::TreeState::ptr state, d2::Element::TraversalWrapper ptr, auto&&...
							) {
								try { state->as<FilesystemExplorerState>()->setrpath(name); }
								catch (...) {};
							});
						}
					}
					catch (...) {}
				}
				base_path = path;
			}
			else
				base_path = "/";
		}
	};
}

#endif // D2_ELEMENT_TEST_HPP
