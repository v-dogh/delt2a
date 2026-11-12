#pragma once

#include <core/mods/d2_image_loader.hpp>

namespace d2::sys
{
    class SystemPngLoader : public SystemImageLoader, public ModuleImpl<SystemPngLoader, Load::Lazy>
    {
    protected:
        Status _load_impl() override { return Status::Ok; }
        Status _unload_impl() override { return Status::Ok; }
    public:
        using SystemImageLoader::SystemImageLoader;
        image::Raster::ptr decode(std::span<const unsigned char> bytes, image::LoadOptions options = {}) const override;
    };

    namespace img 
    {
        using png = SystemPngLoader;
    }
}
