#include <delt2a/d2_std.hpp>
#include <delt2a/elements/d2_std.hpp>
#include <delt2a/templates/d2_file_explorer.hpp>
#include <delt2a/programs/d2_debug_box.hpp>
#include <delt2a/templates/d2_standard_widget_theme.hpp>
#include <delt2a/templates/d2_theme_selector.hpp>

// Simple terminal (though the terminal functionality is debatable)
// Mainly as a showcase of most of the features like interpolators, themes, popups, stylesheets, the tree structure
// This example is both unicode and ASCII compatible (without any issues) as it relies on default values from the charset
// You can open tabs either by pressing the indicator (empty button next to the tab list) or typing $tabopen in the input box
// To close a tab either type exit or $tabclose
// After you try opening a tab you will be able to type a name for as long as the button is in focus
// Be careful though with input as it's not fully functional and some input sanitation is omitted

D2_STATELESS_TREE(terminal)
    // On unix we can mask interrupts so CTR+C works (as an example)
    state->context()->sys<d2::os::input>()->mask_interrupts();
    // D2_ELEM creates an element in the tree
    // Each D2_ELEM must be matched by a D2_ELEM_END
    // calls to it may be nested, in which case the element will be added to the parent element in the scope
    // the outermost scope (so here, right now) refers to the root element
    // This particular element is a type of a box
    // A FlowBox implements an automatic layout system with relative units (more on these below)
    // Units are divided into
    // _px - pixels
    // _pc - percentage of parent
    // _pv - percentage of viewport
    // _center - center in relation to parent
    // _relative - let's the implementation provide offsets (i.e. depends on the parent type, some ignore it, others do not)
    // _auto - let parent decide the units (default)
    // Additionally units may be inverted, i.e. the origin will be opposite to the standard,
    // e.g. for horizontal, position units the standard is origin to the left, inverted units will count from the right
    D2_ELEM(FlowBox)
        // The main method of referring to specific elements is through names
        // But this method requires explicit traversal of the tree (there are no global ids)
        // To make access to elements easier one may anchor an element
        // Now, in the scope where the anchor is setup, elements may access the anchored element by symbol
        D2_ANCHOR(Button, indicator)
        D2_ANCHOR(Switch, tab)
        D2_ANCHOR(ScrollBox, out)
        D2_ANCHOR(Input, in)
        // Most macros in the DSL are convenience macros
        // D2_STYLE is simply an alias for ptr->set<Property>(...)
        // which automatically scopes the passed property to the element type and grabs the pointer created by D2_ELEM
        // Each property is an enum value scoped to the element type, while properties will not have the same values for all types (even if they are the same properties),
        // generic access through a property may be done through the ptr->set/get/getref_for<Interface, Property>(...), but where the Property is instead of a specific element's property
        // a property of the Interface instantiated with a zero (special case) (these are aliased as IZ<interface name>, e.g. IZLayout stores properties for layout stuff like width, height, x, y)
        D2_STYLE(Width, 1.0_pc)
        D2_STYLE(Height, 1.0_pc)
        // D2_STYLES_APPLY applies a stylesheet
        // d2::ctm:: stores some stylesheets for the standard widget theme (more on themes later)
        // So that the colors/properties of the widgets match up with developer code
        D2_STYLES_APPLY(d2::ctm::border_color)
        // D2_ELEM_ANCHOR is just an equivalent of D2_ELEM, but simply sets up the anchor
        // it is terminated the same way as D2_ELEM
        D2_ELEM_ANCHOR(tab)
            D2_STYLE(Width, 12.0_pxi)
            D2_STYLE(Height, 3.0_px)
            D2_STYLE(X, 0.0_relative)
            D2_STYLE(Options, Switch::opts{ "default" })
            D2_STYLE(OnChangeValues, [=](d2::TreeIter ptr, d2::string opt, d2::string old) {
                if (out->exists(opt))
                {
                    out->at(old)->setstate(d2::Element::Display, false);
                    out->at(opt)->setstate(d2::Element::Display, true);
                }
            })
            D2_STYLES_APPLY(d2::ctm::border_color)
            D2_STYLES_APPLY(d2::ctm::switch_color)
        D2_ELEM_END
        D2_ELEM_ANCHOR(indicator)
            // D2_STATIC ensures that instances of a tree will end up with unique and consistent values for these variables
            D2_STATIC(bool, is_input, false)
            D2_STATIC(d2::IOContext::EventListener, input_ev)
            D2_STATIC(d2::IOContext::EventListener, control_ev)
            D2_STYLE(Width, 5.0_px)
            D2_STYLE(Height, 3.0_px)
            D2_STYLE(X, 0.0_relative)
            D2_STYLES_APPLY(d2::ctm::border_color)
            D2_STYLES_APPLY(d2::ctm::focus_color)
            auto commit = [=]() {
                if (is_input)
                {
                    is_input = false;
                    input_ev.mute();
                    control_ev.mute();
                    (*out->override<Text>(tab->getref<Switch::Options>()->back()))
                    .set<Text::TextOptions>(Text::Paragraph)
                    .set<Text::TextAlignment>(Text::Alignment::Left);
                }
            };
            D2_STYLE(OnSubmit, [=](d2::TreeIter) {
                tab->apply_set<Switch::Options>([](Switch::opts& opts) { opts.emplace_back(); });
                if (!is_input)
                {
                    is_input = true;
                    if (input_ev == nullptr)
                    {
                        input_ev = state->context()->listen<d2::IOContext::Event::KeySequenceInput>([=](auto, auto state) {
                            tab->apply_set<Switch::Options>([&](Switch::opts& opts) {
                                opts.back() += state->context()->input()->key_sequence();
                            });
                        });
                        control_ev = state->context()->listen<d2::IOContext::Event::KeyInput>([=](auto, auto state) {
                            if (state->context()->input()->is_pressed(d2::sys::SystemInput::Tab))
                                commit();
                        });
                    }
                    else if (input_ev.is_muted())
                    {
                        input_ev.unmute();
                        control_ev.unmute();
                    }
                }
            })
            // Event listener functions such as D2_LISTEN have both block variants
            // i.e. begin block with D2_LISTEN, end block with D2_LISTEN_END
            // and compact variants with the _EXPR postfix
            // the _EXPR variants do add a semicolon at the end so for nesting them in each other you would use _EEXPR inside (which does not include the semicolon)
            D2_LISTEN_EXPR(Focused, false, commit())
        D2_ELEM_END
        D2_ELEM(Button)
            D2_STYLE(Width, 5.0_px)
            D2_STYLE(Height, 3.0_px)
            D2_STYLE(X, 0.0_relative)
            D2_STYLES_APPLY(d2::ctm::border_color)
            D2_STYLES_APPLY(d2::ctm::focus_color)
            D2_STYLE(OnSubmit, [=](d2::TreeIter) {
                // Here we open the theme selector widget when the button is clicked, which allows the user to setup accents and select themes for the program
                // we handle it when the theme data is submitted to us
                using d2::ctm::ThemeSelector;
                (*ptr->screen()->root()->override<ThemeSelector>("__theme_select__").as<ThemeSelector>())
                .set<ThemeSelector::OnSubmit>(
                    [](d2::TreeIter ptr, const ThemeSelector::theme& theme) {
                        if (std::holds_alternative<ThemeSelector::accent_vec>(theme))
                        {
                            auto& accents = std::get<ThemeSelector::accent_vec>(theme);
                            if (!accents.empty())
                            {
                                d2::ctm::StandardTheme::accent_dynamic(
                                    &ptr->state()->screen()->theme<d2::ctm::StandardTheme>(),
                                    accents[0]
                                );
                            }
                        }
                    });
            })
        D2_ELEM_END
        D2_ELEM_ANCHOR(out)
            D2_STYLE(Width, 4.0_pxi)
            D2_STYLE(Height, 8.0_pxi)
            D2_STYLE(X, 0.0_center)
            D2_STYLE(Y, 0.0_relative)
            D2_ELEM(Text, default)
                D2_STYLE(TextOptions, Text::Paragraph)
                D2_STYLE(TextAlignment, Text::Alignment::Left)
            D2_ELEM_END
        D2_ELEM_END
        D2_ELEM(Box)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 3.0_px)
            D2_STYLE(Y, 0.0_relative)
            D2_STYLES_APPLY(d2::ctm::border_color)
            D2_ELEM_ANCHOR(in)
                ptr->setcache(d2::Element::CachePolicyVolatile);
                auto load = D2_INTERPOLATE_GEN(1000, Sequential, indicator, Value, std::vector<d2::string>{ ".", "o", "O", "o" });
                load->mute();
                D2_STYLE(Width, 1.0_pc)
                D2_STYLE(Pre, "> ")
                D2_STYLE(OnSubmit, [=](d2::TreeIter, d2::string value) {
                    const auto buf = out->at(tab->choice()).as<Text>();
                    if (value == "$exit")
                    {
                        state->screen()->stop_blocking();
                    }
                    else if (value == "$tabclose" || value == "exit")
                    {
                        Switch::opts opts = tab->get<Switch::Options>();
                        if (opts.size() == 1)
                        {
                            state->screen()->stop_blocking();
                        }
                        else
                        {
                            opts.erase(std::remove_if(opts.begin(), opts.end(), [&](auto v) {
                                return v == tab->choice();
                            }), opts.end());
                            out->remove(tab->choice());
                            tab->set<Switch::Options>(std::move(opts));
                            tab->reset(0);
                        }
                    }
                    else if (value == "$tabopen")
                    {
                        indicator->on_submit(indicator);
                    }
                    else if (value == "clear")
                    {
                        buf->set<Text::Value>("");
                    }
                    else
                    {
                        load->unmute();
                        D2_ASYNC_TASK
                            D2_STATIC(std::atomic<int>, ref, 0)
                            ++ref;
                            std::array<char, 256> buffer{};
                            d2::string result;

                            FILE* pipe = popen(value.c_str(), "r");
                            if (pipe == nullptr)
                            {
                                buf->apply_set<Text::Value>([](d2::string& value) {
                                    value += "Error opening pipe";
                                });
                            }
                            else
                            {
                            result += std::format("> {}\n", value);
                            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
                                result += buffer.data();
                            result += '\n';
                            buf->apply_set<Text::Value>([&](d2::string& value) {
                                value += result;
                            });
                            }
                            if (pipe)
                                pclose(pipe);
                            if (--ref == 0)
                                // Any code that runs in a different thread than the main thread must be synchronized
                                // only set/get/getref and their _for variants are automatically synchronized for convenience
                                D2_SYNC_BLOCK
                                    load->mute();
                                    indicator->set<Button::Value>("");
                                D2_SYNC_BLOCK_END
                        D2_ASYNC_END
                    }
                })
                D2_STYLES_APPLY(d2::ctm::input_color)
            D2_ELEM_END
        D2_ELEM_END
    D2_ELEM_END
D2_TREE_END

int main()
{
    d2::Screen::make<terminal>(
        // Here we pass modules to the IOContext object
        // modules implement OS specific functionality through a common interface
        d2::IOContext::make<
            d2::os::input,
            d2::os::output,
            d2::os::clipboard
        >(),
        // Here we pass a theme to be created
        // This is the standard widget theme which is used by all widgets
        // You can create your own themes as well (and if you want them to be compatible with widgets, just derive from it and link it's variables)
        // (but more on themes in another example)
        d2::style::Theme::make<d2::ctm::StandardTheme>(
            d2::ctm::StandardTheme::accent_default
        )
    )->start_blocking(
        d2::Screen::fps(60),
        // Adaptive (as opposed to Stable) will adjust the FPS based on activity of the user
        // and rendering activity, in this case the fps passed are treated as the maximum
        d2::Screen::Profile::Adaptive
    );
}
