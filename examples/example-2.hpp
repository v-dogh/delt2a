#ifndef EXAMPLE_2_HPP
#define EXAMPLE_2_HPP

#include "delt2a/elements/d2_std.hpp"
#include "delt2a/d2_std.hpp"
#include "delt2a/templates/d2_debug_box.hpp"

// d2::Element is the base class for all elements
// d2::ParentElement is a superset which represents an object that manages sub-elements
// Each element has a few key components
// The most important of these will be described below
class CustomElement :
	public d2::dx::Box,
	// UAI (universal access interface) provides an interface for accessing styles (properties) of objects
	// What provides the stylesheet is an interface
	// Custom interfaces may also be written, for reference look at d2_styles.hpp and it's implementation of the standard interfaces
	// Interfaces define getters and id's for each property and UAI handles call resolution etc.
	public d2::style::UAIC<
		d2::dx::Box::data,
		CustomElement,
		d2::style::ILayout,
		d2::style::IColors
	>
{
public:
	using data = d2::style::UAIC<
		d2::dx::Box::data,
		CustomElement,
		d2::style::ILayout,
		d2::style::IColors
	>;
	// If we derive from an object that itself derives from UAI we must chain it
	// This ensures that the base interfaces are also visible to our UAI instance
	D2_UAI_CHAIN(d2::dx::Box);
public:
	using Box::Box;
};

void example2()
{
// 	using namespace d2::dx;
// 	d2::Screen::make<test>(
// 		d2::IOContext::make<
// 			d2::os::input,
// 			d2::os::output,
// 			d2::os::clipboard
// 		>()
// 	)->start_blocking(
// 		d2::Screen::fps(44),
// 		d2::Screen::Profile::Stable
// 	);
}

#endif // EXAMPLE_2_HPP
