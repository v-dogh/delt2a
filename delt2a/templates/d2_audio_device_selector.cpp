#include "d2_audio_device_selector.hpp"
#include "d2_widget_theme_base.hpp"
#include "../elements/d2_switch.hpp"

namespace d2::ctm
{
    D2_TREE_DEFINE(audio_device_selector)
        D2_STYLE(Width, 72.0_px)
        D2_STYLE(Height, 6.0_px)
        D2_STYLE(ContainerBorder, true)
        D2_STYLES_APPLY(border_color)
        // Panel selector
        D2_ELEM(Switch)
            D2_STYLE(Width, 1.0_pc)
            D2_STYLE(Height, 1.0_px)
            D2_STYLE(Options, Switch::opts{ "Input", "Output" })
            D2_STYLE(OnChangeValues,
                [](d2::TypedTreeIter<Switch> ptr, d2::string, d2::string n) {
                    auto p = ptr->parent();
                    if (p->exists("in") && p->exists("out"))
                    {
                        auto in = p->at("in");
                        auto out = p->at("out");
                        in->setstate(
                            d2::Element::Display,
                            n == "Output"
                        );
                        out->setstate(
                            d2::Element::Display,
                            n == "Input"
                        );
                    }
                }
            )
            D2_STYLES_APPLY(switch_color)
            ptr->reset("Input");
        D2_ELEM_END
        // Input/Output device selectors
        auto create_device_list = [&](const d2::string& name, d2::sys::audio::Device dev)
        {
            D2_ELEM_STR(VerticalSwitch, name)
                D2_STATIC(ids, std::unordered_map<std::size_t, std::string>)
                D2_STATIC(names, std::unordered_map<std::string, std::size_t>)
                D2_STATIC(watch, d2::sys::audio::EventListener)
                D2_STYLE(ZIndex, Box::overlap)
                D2_STYLE(Width, 1.0_pc)
                D2_STYLE(Height, 3.0_pxi)
                D2_STYLE(Y, 2.0_px)
                D2_STYLE(ContainerOptions,
                    Switch::EnableBorder |
                    Switch::DisableBorderBottom |
                    Switch::DisableBorderTop
                )
                D2_STYLE(OnChange, [=](d2::TypedTreeIter<VerticalSwitch> ptr, int n, int o) {
                    auto audio = ptr->state()->context()->sys<d2::sys::audio>();
                    audio->flush(dev);
                    audio->deactivate(dev);
                    audio->activate(dev, (*ids)[n]);
                })
                D2_STYLES_APPLY(border_color)
                D2_STYLES_APPLY(selector_color)
                D2_LISTEN_EXPR(Created, false, watch->destroy())
                *watch = ptr->state()->context()->sys<d2::sys::audio>()->watch(
                    dev, [=](const d2::sys::audio::DeviceName& name, d2::sys::audio::Event ev, d2::sys::audio::EventListener) {
                        switch (ev)
                        {
                        case d2::sys::ext::SystemAudio::Event::Activate: break;
                        case d2::sys::ext::SystemAudio::Event::Deactivate: break;
                        case d2::sys::ext::SystemAudio::Event::Create:
                            ptr->apply_set<VerticalSwitch::Options>([&](VerticalSwitch::opts& opts) {
                                (*ids)[opts.size()] = name.id;
                                (*names)[name.id] = opts.size();
                                opts.push_back(name.name);
                            });
                            break;
                        case d2::sys::ext::SystemAudio::Event::Destroy:
                            ptr->apply_set<VerticalSwitch::Options>([&](VerticalSwitch::opts& opts) {
                                const auto idx = (*names)[name.id];
                                ids->erase(idx);
                                opts.erase(opts.begin() + idx);
                            });
                            break;
                        }
                    }
                );
            D2_ELEM_END
        };
        create_device_list("in", d2::sys::audio::Device::Input);
        create_device_list("out", d2::sys::audio::Device::Output);
        ptr->at("out")->setstate(d2::Element::State::Display, false);
    D2_TREE_DEFINITION_END
}
