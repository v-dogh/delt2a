#include <d2_std.hpp>
#include <elements/d2_box.hpp>
#include <elements/d2_image.hpp>
#include <elements/d2_text.hpp>
#include <iostream>
#include <os/image/kitty/d2_kitty_images.hpp>
#include <os/image/sixel/d2_sixel_images.hpp>
#ifdef D2_EXAMPLE_PNG
#include <os/image/png/d2_png_loader.hpp>
#endif

using namespace d2;
sys::SystemImages::Config image_config;
image::Raster::ptr gradient, checker;
std::string image_path;
mt::co_future<void> load_image(std::shared_ptr<dx::Image> image)
{
    try { co_await image->load(image_path); }
    catch (const std::exception& error)
    { if (auto logs = rs::context::get()) logs->log(rs::Severity::Error, "images", error.what()); }
}

struct ImageState : TreeState
{
    using TreeState::TreeState;
    Signals::Handle keys{};
};
using demo = Tree<ImageState, dx::Box,
    [](TreeCtx<dx::Box, ImageState> ctx)
    {
        using namespace dx;
        if (auto images = ctx.context()->sys_if<sys::SystemImages>(); images)
            images->configure(image_config);
        ctx.elem([](TreeCtx<Text> ctx)
        {
            dstyle(Value, "Images: Ctrl+R moves; Ctrl+D removes; Ctrl+L unloads; Ctrl+Q exits.");
            dstyle(Width, Unit(75));
            dstyle(TextAlignment, Text::Alignment::Left);
            dstyle(ForegroundColor, px::foreground(255,255,255));
        });
        auto back = ctx.elem([](TreeCtx<Image> ctx)
        {
            dstyle(Source, gradient);
            dstyle(X, Unit(2)); dstyle(Y, Unit(3));
            dstyle(Width, Unit(24)); dstyle(Height, Unit(9));
        });
        auto front = ctx.elem([](TreeCtx<Image> ctx)
        {
            dstyle(Source, checker);
            dstyle(X, Unit(14)); dstyle(Y, Unit(6));
            dstyle(Width, Unit(22)); dstyle(Height, Unit(9));
            dstyle(ZIndex, 1);
        });
        if (!image_path.empty()) load_image(front);
        ctx.elem([](TreeCtx<Text> ctx)
        {
            dstyle(Value, " Text clips images at cell boundaries ");
            dstyle(X, Unit(4)); dstyle(Y, Unit(8));
            dstyle(Width, Unit(38)); dstyle(Height, Unit(1));
            dstyle(ZIndex, 2);
            dstyle(TextAlignment, Text::Alignment::Left);
            dstyle(ForegroundColor, px::foreground(255,255,255));
            dstyle(BackgroundColor, px::background(24,24,24));
        });
        ctx.state()->keys = ctx.context()->listen<sys::screen::Event::KeyInput>(
            [weak = std::weak_ptr<IOContext>(ctx.context()),
             back = std::weak_ptr<Image>(back), front = std::weak_ptr<Image>(front), moved = false]
            (in::InputFrame& frame) mutable
            {
                auto context = weak.lock();
                if (!context || !frame.map().current[std::size_t(in::special::LeftControl)]) return;
                if (frame.active(in::key('q'), in::mode::Press)) context->stop();
                else if (frame.active(in::key('r'), in::mode::Press))
                {
                    moved = !moved;
                    if (auto image = front.lock()) image->set<Image::X>(Unit(moved ? -4 : 14));
                }
                else if (frame.active(in::key('d'), in::mode::Press))
                {
                    if (auto image = front.lock()) image->setstate(Element::Display, false);
                    if (auto image = back.lock()) image->setstate(Element::Display, false);
                }
                else if (frame.active(in::key('l'), in::mode::Press))
                    if (context->sys_if<sys::SystemImages>()) context->unload<sys::SystemImages>();
            });
    }>;

int main(int argc, char** argv)
{
    const auto mode = argc > 1 ? std::string(argv[1]) : "plain";
    if (mode != "plain" && mode != "kitty" && mode != "sixel")
    {
        std::cerr << "Usage: d2_images [plain|kitty|sixel] [cell-width cell-height [image.png]]\n";
        return 1;
    }
    if (argc > 3) image_config.cell_pixels = {std::stoi(argv[2]), std::stoi(argv[3])};
    if (argc > 4) image_path = argv[4];
#ifndef D2_EXAMPLE_PNG
    if (!image_path.empty()) { std::cerr << "This build has no PNG loader\n"; return 1; }
#endif
    std::vector<image::RGBA> first(64 * 64), second(64 * 64);
    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 64; ++x)
        {
            first[y * 64 + x] = {static_cast<unsigned char>(x * 4), static_cast<unsigned char>(y * 4), 80, 255};
            second[y * 64 + x] = {(x / 8 + y / 8) % 2 ? (unsigned char)255 : (unsigned char)20, 80, 240, 170};
        }
    gradient = image::Raster::rgba(64,64,std::move(first));
    checker = image::Raster::rgba(64,64,std::move(second));
#ifdef D2_EXAMPLE_PNG
    using Loader = sys::SystemPngLoader;
#else
    using Loader = void;
#endif
    if (mode == "plain")
        IOContext::make<sys::input, sys::output, sys::screen, Loader>()->run<NamedTree<"Images",demo>>();
    else if (mode == "kitty")
        IOContext::make<sys::input, sys::output, sys::screen, Loader, sys::SystemImagesKitty>()->run<NamedTree<"Images",demo>>();
    else
        IOContext::make<sys::input, sys::output, sys::screen, Loader, sys::SystemImagesSixel>()->run<NamedTree<"Images",demo>>();
}
