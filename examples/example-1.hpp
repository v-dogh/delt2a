#ifndef EXAMPLE_1_HPP
#define EXAMPLE_1_HPP

// Global macros
// These have to be set in the cmake configuration
// Possible values: ASCII or UTF8
// Defines the value types for pixels
// UTF8 mode will use more memory
// #define D2_LOCALE_MODE ASCII
// If STRICT, lack of a component will result in a compilation error
// Else compilation will proceed and the component will be null
// #define D2_COMPATIBILITY_MODE STRICT
// Includes standard elements
#include "../delt2a/elements/d2_std.hpp"
// Includes core headers
#include "../delt2a/d2_std.hpp"
// Includes the debug box tree
#include "../delt2a/programs/d2_debug_box.hpp"
// ^ Quedits tree
#include "../delt2a/programs/d2_quedits.hpp"
// Popup theme
#include "../delt2a/templates/d2_popup_theme_base.hpp"

void example1()
{
	// Namespace with all of the standard elements
	using namespace d2::dx;

	// This part will be about theming so you can return to it after you look a bit at the tree
	// Themes are handled through dependencies
	// A dependency will track any subscribers and automatically update their values if its value changes
	struct Theme :
		d2::style::Theme,
		// If you want the popups to also be affected by your theme you will have to derive from the popup theme and link it (below)
		d2::ctm::PopupTheme
	{
		// This will create a dependency with a type of a background color
		// Any writes to this dependency will reflect in all dependent variables
		Dependency<d2::px::background> color;

		// You can also use macros for that
		D2_DEPENDENCY(d2::px::background, other_color)
		// To link a dependency (we have to do that for popup theme)
		// Now any changes to color will also change pt_bg_primary
		// We have to do that for all popup dependencies (you can look into the header for them)
		D2_DEPENDENCY_LINK(pt_bg_primary, color)

		// This function will be passed to d2::style::Theme::make to initialize our default theme
		// You can also make the initialization function take arguments and pass them to d2::style::Theme::make
		static void default_accents(Theme* theme)
		{
			theme->color = d2::colors::g::olive;
		}
	};

	// Defines a tree without any custom state
	// The parameter is the symbol name which refers to the tree
	// The macro DSL aliases complex/repetetive expressions
	D2_STATELESS_TREE(test1)
		// D2_U<...> variants are shorthands for unnamed elements
		// The closing clauses' labels are usually optional
		// I.e. for D2_ELEM_END(label) the label can be treated as a comment
		D2_ELEM_UNNAMED(Button)
			// Most nested macros (that establish a block) create a context
			// The context contains __ptr, __state, __object_type values
			// __ptr stores the current object (Element::TraversalWrapper)
			// __state, the tree state pointer d2::TreeState::ptr
			// __object_type is the type of the object that __ptr points to
			// D2_CONTEXT or D2_CONTEXT_RED may be used to establish those variables
			// In this specific case __ptr and __state are aliased without the leading underscores

			// The first argument is the property of the element
			// The second one is the value which is set to the property
			D2_STYLE(Value, "Click Me")
			// Here we set the foreground color to the color Dependency from Theme
			// For more about themes go to the beginning
			D2_STYLE(ForegroundColor, D2_VAR(Theme, color))
			// Dynavars allow you to process a dynamic variable before it is propagated
			// Here we adjust the value's alpha channel
			D2_STYLE(BackgroundColor, D2_DYNAVAR(Theme, color, value.alpha(0.3f)))

			// Here we set the dependency to blue after 3 seconds which should become visible immediately
			// After that time passes
			// This is also how you normally refer to (without the dsl) the themes and dependencies
			// You just pass them by reference to element.set(...)
			// If you want to set a dynavar you use d2::style::dynavar<[](const auto& value) { ... }>(var)
			D2_DEFER_EXPR(3000, state->screen()->theme<Theme>().color = d2::colors::b::blue)

			// Units include
			// _px (pixels)
			// _pc (parent percentage)
			// _pv (viewport percentage)
			// _center (center relative to parent)
			// _relative (up to parent to align)
			// Every unit here constructs a d2::Unit instance
			// For relative units Box implements it's automatic alignment protocol

			D2_STYLE(X, 0.0_center)
			D2_STYLE(Y, 0.0_center)

			D2_STYLE(OnSubmit, [](d2::Element::TraversalWrapper ptr) {
				// Changes the screen's tree
				ptr
					->screen()
					->set("Test2");
			});

			// Interpolators may be used to animate objects
			// The twoway interpolator automatically animates backwards
			// When the event is set to false
			// So for hovered it will reverse the hover animation after the user moves the cursor outside
			D2_INTERPOLATE_TWOWAY_AUTO(
				// Element::State
				Hovered,
				// Time in milliseconds
				500,
				// Interpolator type
				// Custom interpolators may also be implemented
				// Standard ones also include a Sequential interpolator
				// This one cycles through a list of states
				Linear,
				// Property
				ForegroundColor,
				// Final value
				d2::colors::b::slate_blue
			)
		D2_UELEM_END
	D2_TREE_END(test1)

	D2_STATELESS_TREE(test2)
		// We can inject trees with these macros
		// These will be created in place
		// At runtime we can also use the create_at static method of the tree type
		// E.g. d2::ctm::debug::create_at(...)
		D2_EMBED(d2::prog::debug)
		// Here we inject poland
		D2_EMBED(d2::prog::poland)
		D2_ELEM(Text, example-named-element)
			D2_STYLE(X, 0.0_center)
			D2_STYLE(Y, 0.0_center)

			D2_INTERPOLATE_GEN_AUTO(
				5000,
				Sequential,
				Value,
				// Lol
				// Maybe not the most convenient way
				// But it does work
				std::vector<d2::string>{
					"H", "He", "Hel", "Hell", "Hello",
					"Hello ", "Hello W", "Hello Wo",
					"Hello Wor", "Hello Worl",
					"Hello World", "Hello World!"
				}
			);
		D2_ELEM_END(example-named-element)
		D2_ELEM_UNNAMED(Text)
			D2_STYLE(X, 0.0_center)
			D2_STYLE(Y, -1.0_center)

			// We can also achieve the same goal with cyclic tasks
			// These run on the main thread (so they will block rendering)
			// Same is with deferred tasks (which only differ from cyclics that they are invoked once)
			// If one needs to perform some blocking work he can launch an async task
			// But those require synchronization through synchronized blocks (for object access)
			// Task macros also have EXPR/EEXPR variants
			// These are wrappers for launching simple expressions
			// They may also be nested with the EXPR variants always being on the top of the recursive hierarchy and EEXPR being required for the lower calls
			D2_CYCLIC_TASK(500)
				static constexpr d2::string_view str{ "Hello World!" };
				static constexpr std::array<d2::px::background, 7> colors{
					d2::colors::g::olive,
					d2::colors::g::chartreuse,
					d2::colors::g::sea_green,
					d2::colors::b::slate_blue,
					d2::colors::b::indigo,
					d2::colors::r::magenta,
					d2::colors::r::coral
				};
				// Here we skip the macros and access styles manually
				// Apply set allows us to view a reference to the property (while still handling thread safety and signals)
				// For simpler sets/gets we may use 'set'/'get'
				ptr.as<Text>()->apply_set<Text::Value>([](d2::string& value) {
					if (value.size() == str.size())
					{
						value.clear();
					}
					else
					{
						value.push_back(str[value.size()]);
					}
				});

				// The following is an example of launching an async task and synchronizing it (using the EXPR syntax)
				// Any code that refers to elements needs to be synchronized (here D2_SYNCED_EEXPR)
				// Not really useful in this particular case (just to showcase the asynchronous capabilities)
				D2_ASYNC_EXPR(
					D2_SYNCED_EEXPR(
						ptr.as<Text>()->set<Text::BackgroundColor>(
							colors[ptr.as<Text>()->get<Text::Value>().size() % colors.size()]
						)
					)
				)
			D2_CYCLIC_END
		D2_UELEM_END
	D2_TREE_END(test2)

	// As an example this is what the above tree would look like without the DSL
	// (skipping the interpolation case)
	using test3 = d2::TreeTemplate<
		// These classes accept a build lambda which will construct the tree
		// Here we simply use a default constructor
		d2::tree::SubTree<d2::prog::debug>,
		d2::tree::SubTree<d2::prog::poland>,
		// This type will construct a Text object with the passed name and call the callback on it
		// The callback is where we set styles and perform any initialization
		d2::tree::Elem<Text, "name", [](d2::Element::TraversalWrapper elem, d2::TreeState::ptr state) {
			Text& ptr = *elem.as<Text>();
			ptr
				.set<Text::X>(0.0_center)
				.set<Text::Y>(0.0_center);

			// While 'set/get' are the most convenient way to access properties, one can also use their 'more' type erased variants
			// set/get_for instead operate on interfaces and do not require you to know the exact object type
			// E.g. to se the foreground color
			elem->set_for<
				// We pass the interface
				d2::style::IColors,
				// We pass the property
				// the IZ variant is just an instantiation of IColors<0> so you can access the properties more easily
				d2::style::IZColors::ForegroundColor
			>(d2::colors::b::indigo);

			state->context()->scheduler()->launch_cyclic_task([ptr = elem.as<Text>(), state](auto) {
				static constexpr d2::string_view str{ "Hello World!" };
				static constexpr std::array<d2::px::background, 7> colors{
					d2::colors::g::olive,
					d2::colors::g::chartreuse,
					d2::colors::g::sea_green,
					d2::colors::b::slate_blue,
					d2::colors::b::indigo,
					d2::colors::r::magenta,
					d2::colors::r::coral
				};

				ptr->apply_set<Text::Value>([](d2::string& value) {
					if (value.size() == str.size())
					{
						value.clear();
					}
					else
					{
						value.push_back(str[value.size()]);
					}
				});

				state->context()->scheduler()->launch_task([ptr, state]() {
					state->screen()->sync([ptr]() {
						ptr->set<Text::BackgroundColor>(
							colors[ptr->get<Text::Value>().size() % colors.size()]
						);
					});
				});
			}, std::chrono::milliseconds(500));
		}>
	>;

	// Create a screen with two trees
	d2::Screen::make<d2::tree::make<test1, "Test1">, d2::tree::make<test2, "Test2">>(
		// The IOContext handles global events and components
		// Components can be loaded at runtime and provide OS specific functionality
		// There are two core components input & output
		// Input (self-explanatory)
		// Output provides functions for rendering (meant to be used internally)
		d2::IOContext::make<
			// d2::os::<...> contains aliases for automatically asigned components (per-platform)
			// One may load custom components if they want to (and define your own for any purpose)
			// Components that are inactive (or not present) are void
			d2::os::input,
			d2::os::output,
			d2::os::clipboard
		>(),
		// Here we load the theme into the screen
		// This will make it accessible at d2::Screen::theme<Type>
		// You can also load themes at runtime with d2::Screen::create_theme
		d2::style::Theme::make<Theme>(Theme::default_accents)
	// Starts the application
	)->start_blocking(
		// Sets the target delay between frames
		// fps is just a helper function which translates target fps to target delay
		d2::Screen::fps(44),
		// The delay profile
		// Stable will attempt to maintain constant delay
		// Adaptive may lower or increase the delay depending on activity
		d2::Screen::Profile::Stable
	);
}

#endif // EXAMPLE_1_HPP
