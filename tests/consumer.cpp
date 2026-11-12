#include <d2_std.hpp>
#include <d2_platform.hpp>
#include <elements/d2_text.hpp>
#include <core/platform/d2_text_layout.hpp>
#ifdef D2_PACKAGE_INFERENCE
#include <core/mods/inference/d2_std.hpp>
#endif
#ifdef D2_PACKAGE_IMAGES
#include <os/image/kitty/d2_kitty_images.hpp>
#endif
#ifdef D2_PACKAGE_OPENAI
#include <os/inference/openai/d2_openai.hpp>
#endif
#ifdef D2_PACKAGE_EDITORS
#include <elements/d2_input.hpp>
#include <elements/d2_multiline_input.hpp>
#endif

#ifdef D2_PACKAGE_PNG
#include <os/image/png/d2_png_loader.hpp>
#endif

int main()
{
#ifdef D2_PACKAGE_OPENAI
    auto provider = d2::sys::SystemModule::make<d2::sys::SystemOpenAI>(std::shared_ptr<d2::IOContext>{});
    provider->configure({});
    if (provider->model_info("unknown").context_window) return 1;
#endif
#ifdef D2_PACKAGE_PNG
    auto loader = d2::sys::SystemModule::make<d2::sys::SystemPngLoader>(std::shared_ptr<d2::IOContext>{});
    try { loader->decode({}); return 1; } catch (const std::invalid_argument&) {}
#endif
    d2::PixelBuffer buffer(1, 1);
    buffer.at(0, 0).v = 'X';
    d2::text::Layout text("package\ncheck");
    d2::TextSelection selection;
#ifdef D2_PACKAGE_INFERENCE
    d2::ai::ToolSet tools;
    d2::ai::Request request;
    request.model = {"local", "model"};
    request.messages.push_back(d2::ai::Message::text(d2::ai::Role::User, "package check"));
    request.tools = tools.definitions();
    d2::ai::validate_request(request, {});
    auto conversations = std::shared_ptr<d2::sys::SystemConversations>(
        d2::sys::SystemModule::make<d2::sys::SystemConversations>(std::shared_ptr<d2::IOContext>{}));
    auto chat = conversations->create("package", {request.model});
    chat->append(request.messages.front());
    if (chat->snapshot().context.size() != 1) return 1;
#endif
#ifdef D2_PACKAGE_IMAGES
    auto images = d2::sys::SystemModule::make<d2::sys::SystemImagesKitty>(std::shared_ptr<d2::IOContext>{});
    images->configure({true,{1,1}});
#endif
    auto raster = d2::image::Raster::rgba(1,1,{{40,80,120,255}});
    d2::PixelBuffer::View view(&buffer,0,0,1,1);
    view.image(raster,{0,0,1,1});
    if (d2::image::compose(view.image_scene(),{1,1},{2,2}).pixels.size() != 4) return 1;
#ifdef D2_PACKAGE_EDITORS
    auto editor = std::make_shared<d2::dx::MultiInput>("editor", nullptr);
    editor->value("package");
    editor->sync();
    editor->insert("!");
    editor->undo();
    if (editor->value() != "package" || editor->cursor() != 7)
        return 1;
    auto single = std::make_shared<d2::dx::Input>("single", nullptr);
    single->sync();
    if (single->cursor() != 0)
        return 1;
#endif
    return buffer.data().size() != 1 || text.size() != 13 || text.bounds().height != 2;
}
