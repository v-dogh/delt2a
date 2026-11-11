#include "check.hpp"
#include <os/image/terminal/d2_terminal_images.hpp>
#include <os/io/unix/d2_unix_system_io.hpp>
#include <core/io/d2_context.hpp>
#include <absl/strings/escaping.h>
#include <cstdio>
#include <unistd.h>

using namespace d2;
using Protocol = sys::SystemImages::Protocol;

std::string capture(auto action)
{
    auto file = std::tmpfile();
    CHECK(file);
    const auto saved = dup(STDOUT_FILENO);
    CHECK(saved >= 0 && dup2(fileno(file), STDOUT_FILENO) >= 0);
    try { action(); }
    catch (...)
    {
        dup2(saved, STDOUT_FILENO); close(saved); std::fclose(file); throw;
    }
    CHECK(dup2(saved, STDOUT_FILENO) >= 0);
    close(saved); std::rewind(file);
    std::string result;
    std::array<char, 4096> bytes;
    while (auto count = std::fread(bytes.data(), 1, bytes.size(), file)) result.append(bytes.data(), count);
    std::fclose(file);
    return result;
}
std::vector<int> sixel_pixels(std::string_view bytes, int width, int height)
{
    std::vector<int> result(std::size_t(width) * height, -1);
    auto pos = bytes.find("q\"1;1;");
    CHECK(pos != std::string_view::npos);
    pos += 2;
    auto number = [&]()
    {
        int value = 0;
        CHECK(pos < bytes.size() && bytes[pos] >= '0' && bytes[pos] <= '9');
        while (pos < bytes.size() && bytes[pos] >= '0' && bytes[pos] <= '9')
            value = value * 10 + bytes[pos++] - '0';
        return value;
    };
    for (int i = 0; i < 4; ++i) { number(); if (i < 3) CHECK(bytes[pos++] == ';'); }
    int x = 0, y = 0, color = -1;
    while (pos < bytes.size() && bytes[pos] != '\x1b')
    {
        const auto token = bytes[pos++];
        if (token == '#')
        {
            color = number();
            while (bytes[pos] == ';') { ++pos; number(); }
        }
        else if (token == '$') x = 0;
        else if (token == '-') { x = 0; y += 6; }
        else
        {
            int count = 1, bits = token - '?';
            if (token == '!') { count = number(); bits = bytes[pos++] - '?'; }
            CHECK(bits >= 0 && bits < 64 && x + count <= width && color >= 0);
            for (int i = 0; i < count; ++i, ++x)
                for (int bit = 0; bit < 6; ++bit)
                    if (bits & (1 << bit))
                    {
                        CHECK(y + bit < height);
                        result[std::size_t(y + bit) * width + x] = color;
                    }
        }
    }
    return result;
}
struct Output : sys::UnixTerminalOutput
{
    Output(std::weak_ptr<IOContext> ctx, ModPreset preset, std::size_t size) :
        UnixTerminalOutput(ctx, preset, size) { _mark_safe(); }
};
struct Images : sys::TerminalImages
{
    Images(std::weak_ptr<IOContext> ctx, ModPreset preset, std::size_t size) :
        TerminalImages(ctx, preset, size) { _mark_safe(); }
};

int main()
{
    auto raster = image::Raster::rgba(1,1,{{255,0,0,255}});
    image::Scene scene;
    scene.place(raster,{1,1,10,4},{0,0,12,6});
    const auto frame = image::compose(scene,{12,6},{8,16});
    const auto kitty = image::encode_kitty(frame, 42);
    CHECK(kitty.find("s=80,v=64,i=42,q=2,m=1") != std::string::npos);
    CHECK(kitty.find("p=1,c=10,r=4,z=0,C=1,q=2") != std::string::npos);
    std::string payload;
    std::size_t begin = 0, chunks = 0;
    while ((begin = kitty.find("\x1b_G", begin)) != std::string::npos)
    {
        const auto end = kitty.find("\x1b\\", begin);
        const auto semi = kitty.find(';',begin);
        CHECK(semi < end);
        if (semi + 1 < end)
        {
            CHECK(end - semi - 1 <= 4096);
            payload += kitty.substr(semi + 1, end - semi - 1);
            ++chunks;
        }
        begin = end + 2;
    }
    std::string decoded;
    CHECK(absl::Base64Unescape(payload, &decoded));
    CHECK(decoded.size() == frame.pixels.size() * 4 && chunks > 1);
    CHECK(std::memcmp(decoded.data(),frame.pixels.data(),decoded.size()) == 0);

    std::vector<image::RGBA> samples(7 * 8);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 7; ++x)
            samples[y * 7 + x] = y == 7 ? image::RGBA{0,255,0,255} :
                image::RGBA{255,0,0,static_cast<unsigned char>(x < 3 ? 255 : x == 3 ? 0 : 128)};
    image::Scene bands;
    bands.place(image::Raster::rgba(7,8,std::move(samples)),{0,0,7,8},{0,0,7,8});
    std::vector<pixel> backgrounds(56, pixel{.b=255, .a=255});
    const auto sixel = image::encode_sixel(image::compose(bands,{7,8},{1,1}),backgrounds,7);
    const auto decoded_sixel = sixel_pixels(sixel,7,8);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 7; ++x)
            CHECK(decoded_sixel[y * 7 + x] == (y == 7 ? 30 : x < 3 ? 180 : x == 3 ? -1 : 110));

    auto renderer = sys::SystemModule::make<sys::TerminalImages>(std::shared_ptr<IOContext>{});
    renderer->configure({Protocol::Kitty, {8,16}});
    std::vector<pixel> cells(72);
    auto plan = renderer->prepare(cells,{12,6},scene,{});
    CHECK(!plan.after.empty() && plan.before.empty() && !plan.repaint);
    plan = renderer->prepare(cells,{12,6},scene,{});
    CHECK(plan.after.empty() && plan.before.empty() && !plan.repaint);
    renderer->invalidate();
    plan = renderer->prepare(cells,{12,6},scene,{});
    CHECK(!plan.after.empty() && plan.before.find("a=d,d=I") != std::string::npos);
    renderer->configure({Protocol::Sixel,{1,1}});
    plan = renderer->prepare(cells,{12,6},scene,{});
    CHECK(plan.repaint && plan.before.find("a=d,d=I") != std::string::npos);
    CHECK(plan.after.find("\x1b[?80l") != std::string::npos);
    CHECK(plan.before.find("\x1b[2J") != std::string::npos);
    CHECK(plan.after.find("\x1bP0;1;0q\"1;1;10;4") != std::string::npos);
    CHECK(plan.after.find("#180;2;100;0;0") != std::string::npos);
    CHECK(plan.after.find("!10N") != std::string::npos);
    CHECK(renderer->prepare(cells,{12,6},scene,{}).after.empty());
    cells[0].v = 'T';
    CHECK(renderer->prepare(cells,{12,6},scene,{}).repaint);
    plan = renderer->prepare(cells,{12,6},{},{});
    CHECK(plan.repaint && plan.after.empty());
    renderer->configure({Protocol::Sixel,{}});
    throws<std::invalid_argument>([&] { renderer->prepare(cells,{12,6},scene,{}); });
    renderer->configure({Protocol::Off,{}});
    CHECK(renderer->prepare(cells,{12,6},scene,{}).after.empty());
    throws<std::invalid_argument>([&] { renderer->configure({Protocol::Kitty,{0,4}}); });
#ifdef D2_IMAGE_PNG
    const unsigned char png[] = {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,2,0,0,0,1,8,6,0,0,0,244,34,127,138,0,0,0,17,73,68,65,84,120,156,99,248,207,192,208,192,192,240,255,63,0,13,126,3,126,234,30,23,33,0,0,0,0,73,69,78,68,174,66,96,130};
    const auto png_image = renderer->decode_png(png);
    CHECK(png_image->width() == 2 && png_image->height() == 1);
    CHECK((png_image->pixels()[0] == image::RGBA{255,0,0,128}));
    CHECK((png_image->pixels()[1] == image::RGBA{0,0,255,255}));
    throws<std::invalid_argument>([&] { renderer->decode_png({png, 12}); });
#else
    throws<std::logic_error>([&] { renderer->decode_png({}); });
#endif

    auto ctx = std::make_shared<IOContext>(mt::ConcurrentPool::make<>(), nullptr);
    ctx->load<Output, Images>();
    auto output = ctx->sys<sys::SystemOutput>();
    auto images = ctx->sys<sys::SystemImages>();
    images->configure({Protocol::Kitty,{1,1}});
    PixelBuffer buffer(3,2);
    buffer.at(0,0).v = 'a';
    PixelBuffer::View view(&buffer,0,0,3,2);
    view.image(raster,{0,0,2,1});
    CHECK(capture([&] { output->write_frame(view); }).find("\x1b_Ga=t") != std::string::npos);
    CHECK(capture([&] { output->write_frame(view); }).empty());
    buffer.compress();
    CHECK(capture([&] { output->write_frame(view); }).empty());
    throws<std::exception>([&] { ctx->unload<sys::SystemOutput>(); });
    CHECK(capture([&] { ctx->unload<sys::SystemImages>(); }).find("a=d,d=I") != std::string::npos);
    CHECK(capture([&] { output->write_frame(view); }).find("\x1b_G") == std::string::npos);
    ctx->load<Images>();
    images = ctx->sys<sys::SystemImages>();
    images->configure({Protocol::Kitty,{1,1}});
    auto full = open("/dev/full", O_WRONLY);
    CHECK(full >= 0);
    const auto saved = dup(STDOUT_FILENO);
    CHECK(saved >= 0 && dup2(full,STDOUT_FILENO) >= 0);
    bool failed = false;
    try { output->write_frame(view); } catch (const std::system_error&) { failed = true; }
    CHECK(dup2(saved,STDOUT_FILENO) >= 0); close(saved); close(full);
    CHECK(failed);
    const auto retry = capture([&] { output->write_frame(view); });
    CHECK(retry.find("a=d,d=I") != std::string::npos && retry.find("a=t,t=d") != std::string::npos);
    full = open("/dev/full", O_WRONLY);
    const auto saved_clear = dup(STDOUT_FILENO);
    CHECK(full >= 0 && saved_clear >= 0 && dup2(full,STDOUT_FILENO) >= 0);
    failed = false;
    try { output->clear_images(); } catch (const std::system_error&) { failed = true; }
    CHECK(dup2(saved_clear,STDOUT_FILENO) >= 0); close(saved_clear); close(full);
    CHECK(failed);
    CHECK(capture([&] { output->clear_images(); }).find("a=d,d=I") != std::string::npos);
    CHECK(capture([&] { output->write_frame(view); }).find("a=t,t=d") != std::string::npos);
    CHECK(capture([&] { ctx.reset(); }).find("a=d,d=I") != std::string::npos);
}
