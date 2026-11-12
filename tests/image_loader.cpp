#include "check.hpp"
#include <os/image/png/d2_png_loader.hpp>
#include <core/io/d2_context.hpp>
#include <fstream>

using namespace d2;

int main()
{
    const unsigned char png[] = {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,2,0,0,0,1,8,6,0,0,0,244,34,127,138,0,0,0,17,73,68,65,84,120,156,99,248,207,192,208,192,192,240,255,63,0,13,126,3,126,234,30,23,33,0,0,0,0,73,69,78,68,174,66,96,130};
    auto pool = mt::ConcurrentPool::make<>();
    pool->start();
    auto context = std::make_shared<IOContext>(pool, nullptr);
    context->load<sys::SystemPngLoader>();
    auto loader = context->sys<sys::SystemImageLoader>();
    auto raster = loader->decode(png);
    CHECK(raster->width() == 2 && raster->height() == 1);
    CHECK((raster->pixels()[0] == image::RGBA{255,0,0,128}));
    CHECK((raster->pixels()[1] == image::RGBA{0,0,255,255}));
    throws<std::invalid_argument>([&] { loader->decode({}); });
    throws<std::invalid_argument>([&] { loader->decode({png, 12}); });
    throws<std::invalid_argument>([&] { loader->decode(png, {.max_bytes = sizeof(png) - 1}); });
    throws<std::length_error>([&] { loader->decode(png, {.max_pixels = 1}); });
    throws<std::invalid_argument>([&] { loader->decode(png, {.max_pixels = 0}); });
    throws<std::invalid_argument>([&] { loader->decode(png, {.max_pixels = image::Raster::max_pixels + 1}); });
    const auto path = std::filesystem::temp_directory_path() / ("d2-image-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    CHECK(std::filesystem::create_directory(path));
    struct Cleanup { std::filesystem::path path; ~Cleanup() { std::filesystem::remove_all(path); } } cleanup{path};
    const auto file = path / "picture.data";
    { std::ofstream out(file, std::ios::binary); out.write(reinterpret_cast<const char*>(png), sizeof(png)); CHECK(out.good()); }
    CHECK(loader->load(file)->width() == 2);
    throws<std::runtime_error>([&] { loader->load(path / "missing"); });
    throws<std::length_error>([&] { loader->load(file, {.max_bytes = 8}); });
    std::vector<mt::future<image::Raster::ptr>> pending;
    for (int i = 0; i < 8; ++i) pending.push_back(loader->load_async(file));
    context->unload<sys::SystemImageLoader>();
    for (auto& future : pending) CHECK(future.value()->pixels()[0].a == 128);
    throws<std::runtime_error>([&] { loader->load_async(path / "missing").value(); });
    pool->stop();
}
