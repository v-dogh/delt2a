#include "check.hpp"
#include <core/types/d2_image.hpp>
#include <core/types/d2_pixel.hpp>
#include <core/platform/d2_text_layout.hpp>
#include <climits>
#ifdef D2_IMAGE_ELEMENTS
#include <elements/d2_image.hpp>
#include <elements/d2_box.hpp>
#endif

using namespace d2;
using namespace d2::image;

int main()
{
    static_assert(sizeof(pixel) == 10);
    auto red = Raster::rgba(1, 1, {{255, 0, 0, 255}});
    auto blue = Raster::rgba(1, 1, {{0, 0, 255, 128}});
    auto strip = Raster::rgba(2, 1, {{255, 0, 0, 255}, {0, 255, 0, 255}});
    throws<std::invalid_argument>([] { Raster::rgba(0, 1, {}); });
    throws<std::invalid_argument>([] { Raster::rgba(INT_MAX, INT_MAX, {}); });
    throws<std::invalid_argument>([] { Raster::rgb(1, 1, {}); });
    Scene scene;
    scene.place(red, {0, 0, 3, 2}, {0, 0, 3, 2});
    scene.place(blue, {1, 0, 2, 1}, {0, 0, 3, 2});
    auto frame = compose(scene, {3, 2}, {1, 1});
    CHECK((frame.pixels[0] == RGBA{255, 0, 0, 255}));
    CHECK((frame.pixels[1] == RGBA{127, 0, 128, 255}));
    scene.erase({1, 0, 1, 1});
    frame = compose(scene, {3, 2}, {1, 1});
    CHECK(frame.pixels[1].a == 0 && frame.occupied[1] == 0);
    CHECK(frame.pixels[2].a == 255);
    scene.clear();
    scene.place(strip, {-2, 0, 4, 1}, {0, 0, 2, 1});
    frame = compose(scene, {2, 1}, {2, 3});
    CHECK(frame.width() == 4 && frame.height() == 3);
    for (auto pixel : frame.pixels) CHECK((pixel == RGBA{0, 255, 0, 255}));
    throws<std::out_of_range>([&] { scene.place(red, {0,0,1,1}, {0,0,1,1}, {0,0,2,1}); });
    throws<std::invalid_argument>([&] { scene.place({}, {}, {}); });
    throws<std::length_error>([&] { compose(scene, {2,1}, {INT_MAX,INT_MAX}); });

    PixelBuffer source(4, 2), target(3, 2);
    source.fill(pixel{.a=0, .af=0});
    target.fill(pixel{.a=0, .af=0});
    PixelBuffer::View view(&source, 0, 0, 4, 2);
    view.image(strip, {0,0,4,2});
    source.compress();
    target.inscribe(-1, 0, PixelBuffer::View(&source, 1,0,3,2));
    frame = compose(target.image_scene(), {3,2}, {1,1});
    CHECK(frame.cells.width == 2 && frame.cells.height == 2);
    for (auto pixel : frame.pixels) CHECK((pixel == RGBA{0,255,0,255}));
    CHECK(source.image_scene().placements().size() == 1);
    const auto before = target.image_scene();
    target.inscribe(0,0, PixelBuffer::View(&target,0,0,3,2));
    CHECK(target.image_scene() == before);
    target.at(0,0) = pixel{.v='X'};
    frame = compose(target.image_scene(), {3,2}, {1,1});
    CHECK(frame.pixels[0].a == 0 && frame.pixels[1].a == 255);
    PixelBuffer overlay(1,1);
    overlay.fill(pixel{.a=0, .af=0});
    target.inscribe(1,0,PixelBuffer::View(&overlay,0,0,1,1));
    CHECK(compose(target.image_scene(),{3,2},{1,1}).pixels[1].a == 255);
    overlay.at(0,0) = pixel{.a=0, .v='T'};
    target.inscribe(1,0,PixelBuffer::View(&overlay,0,0,1,1));
    CHECK(compose(target.image_scene(),{3,2},{1,1}).pixels[1].a == 0);
    target.fill(pixel{}, 0,1,3,1);
    CHECK(compose(target.image_scene(),{3,2},{1,1}).empty());

    PixelBuffer paint(3,1);
    PixelBuffer::View painter(&paint,0,0,3,1);
    painter.image(red,{0,0,3,1});
    text::Layout("x").draw(painter,pixel{}, {1,0});
    frame = compose(paint.image_scene(),{3,1},{1,1});
    CHECK(frame.pixels[0].a == 255 && frame.pixels[1].a == 0 && frame.pixels[2].a == 255);
    painter.image(blue,{1,0,1,1});
    CHECK(compose(paint.image_scene(),{3,1},{1,1}).pixels[1].a == 128);
    paint.set_size(3,1);
    CHECK(paint.image_scene().empty());
#ifdef D2_IMAGE_ELEMENTS
    struct Root : dx::Box
    {
        Root() : Box("images", nullptr)
        {
            data::width = Unit(4); data::height = Unit(2);
            override_dimensions(4,2); setcache(CachePolicy::Static);
        }
        void attach(const Element::ptr& child)
        { _elements.push_back(child); _setparent_of(child); }
    };
    struct Picture : dx::Image
    {
        Picture(image::Raster::ptr source, int z = 0) : Image("picture", nullptr)
        {
            data::width = Unit(4); data::height = Unit(2);
            data::source = std::move(source); data::zindex = z;
            override_dimensions(4,2);
        }
        void move(int x)
        {
            data::x = Unit(x); override_position(x,0);
            _signal_write(WriteType::Offset, X);
        }
        void tiny()
        {
            data::width = Unit(1); data::height = Unit(1);
            override_dimensions(1,1);
            data::container_options = EnableBorder; data::border_width = HDUnit(5);
        }
    };
    for (auto policy : {Element::CachePolicy::Static, Element::CachePolicy::Dynamic, Element::CachePolicy::Volatile})
    {
        auto root = std::make_shared<Root>();
        auto lower = std::make_shared<Picture>(red);
        auto upper = std::make_shared<Picture>(blue,1);
        for (auto item : {lower, upper}) { root->attach(item); item->setcache(policy); }
        for (int i = 0; i < 3; ++i)
        {
            upper->move(i == 1 ? -1 : 0);
            const auto result = compose(root->frame().buffer().image_scene(),{4,2},{1,1});
            CHECK(!result.empty());
            CHECK((result.pixels[0] == RGBA{127,0,128,255}));
        }
    }
    auto tiny = std::make_shared<Picture>(red);
    tiny->tiny();
    CHECK(tiny->frame().buffer().image_scene().empty());
#endif
    Scene transparent;
    transparent.place(Raster::rgba(1,1,{{255,0,0,0}}), {0,0,1,1}, {0,0,1,1});
    CHECK(compose(transparent,{1,1},{1,1}).empty());
}
