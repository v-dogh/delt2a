#include "core/mods/d2_ext.hpp"

#include <core/screen/d2_screen.hpp>

namespace d2::sys
{
    SystemSelector::Status SystemSelector::_load_impl()
    {
        _sig = IOContext::get()->connect<EventManifest>();
        _mouse_ev = IOContext::get()->listen<screen::Event::MouseInput>([](in::InputFrame& in) {
            
        });
        return Status::Ok;
    }
    SystemSelector::Status SystemSelector::_unload_impl()
    {
        _sig.disconnect();
        _mouse_ev.close();
        return Status::Ok;
    }
} // namespace d2::sys
