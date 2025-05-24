#ifndef D2_QUEDITS_H
#define D2_QUEDITS_H

#include "../elements/d2_box.hpp"
#include "../elements/d2_text.hpp"
#include "../d2_colors.hpp"
#include "../d2_dsl.hpp"

namespace d2::prog
{
	using namespace d2::dx;
	D2_STATELESS_TREE(poland)
		D2_ELEM_NESTED_UNNAMED(Box)
			D2_STYLE(Width, 32.0_px)
			D2_STYLE(Height, 12.0_px)
		D2_UELEM_NESTED_BODY
			D2_ELEM_UNNAMED(Text)
				D2_STYLE(X, 0.0_center)
				D2_STYLE(Y, 0.0_center)
				D2_STYLE(ForegroundColor, d2::colors::b::slate_blue)
				D2_STYLE(Value, R"(Quedits:
Written and Directed: Magnarius Cosmo
Design and Implementation: Magnarius Cosmo
Idea: Magnarius Cosmo
Version: sigma.tau.1)")
			D2_UELEM_END
			D2_ELEM_UNNAMED(Box)
				D2_STYLE(Width, 1.0_pc)
				D2_STYLE(Height, 0.5_pc)
				D2_STYLE(BackgroundColor, d2::colors::w::white)
				D2_STYLE(Y, 0.0_relative)
			D2_UELEM_END
			D2_ELEM_UNNAMED(Box)
				D2_STYLE(Width, 1.0_pc)
				D2_STYLE(Height, 0.5_pc)
				D2_STYLE(BackgroundColor, d2::colors::r::red)
				D2_STYLE(Y, 0.0_relative)
			D2_UELEM_END
		D2_UELEM_NESTED_END
	D2_TREE_END(poland)
}

#endif // D2_QUEDITS_H
