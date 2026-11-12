#include "check.hpp"
#include <d2_std.hpp>
#include <elements/d2_box.hpp>
#include <elements/d2_image.hpp>
#include <os/io/unix/d2_unix_system_io.hpp>
#include <semaphore>

using namespace d2;

struct Input : sys::SystemInput, sys::ModuleImpl<Input, sys::Load::Immediate>
{
    using SystemInput::SystemInput;
    in::InputFrame::ptr frame_value{in::InputFrame::make()};
    Status _load_impl() override { return Status::Ok; }
    Status _unload_impl() override { return Status::Ok; }
    in::InputFrame::ptr _frame_impl() override { return frame_value; }
    void _begincycle_impl() override { context()->deadline(std::chrono::milliseconds(1)); }
    void _endcycle_impl() override {}
    MainWorker::ptr _worker_impl() override { return mt::Worker::make<sys::UnixWorker>(); }
};
struct Output : sys::SystemOutput, sys::ModuleImpl<Output, sys::Load::Immediate>
{
    using SystemOutput::SystemOutput;
    Status _load_impl() override { return Status::Ok; }
    Status _unload_impl() override { return Status::Ok; }
    std::size_t delta_size() override { return 0; }
    std::size_t swapframe_size() override { return 0; }
    void write(std::span<const pixel>, std::size_t, std::size_t) override {}
};
struct Loader : sys::SystemImageLoader, sys::ModuleImpl<Loader, sys::Load::Immediate>
{
    using SystemImageLoader::SystemImageLoader;
    mutable std::binary_semaphore gate{0};
    Status _load_impl() override { return Status::Ok; }
    Status _unload_impl() override { return Status::Ok; }
    image::Raster::ptr decode(std::span<const unsigned char>, image::LoadOptions) const override { return {}; }
    image::Raster::ptr load(const std::filesystem::path& path, image::LoadOptions) const override
    {
        CHECK(!context()->is_synced());
        if (path == "slow") gate.acquire();
        if (path == "fail") throw std::runtime_error("decode failed");
        if (path == "null") return {};
        return image::Raster::rgba(1, 1, {{static_cast<unsigned char>(path == "slow" ? 10 : 20), 0, 0, 255}});
    }
};

struct State : TreeState
{
    using TreeState::TreeState;
    Signals::Handle update;
    std::shared_ptr<dx::Image> image;
    mt::co_future<bool> first, second;
    int step{0};
    bool done{false};
    std::exception_ptr error;
    void tick()
    {
        auto ctx = context();
        auto loader = ctx->sys<sys::SystemImageLoader>()->ptr<Loader>();
        try
        {
            CHECK(ctx->is_synced());
            if (step && !first.block()->is_ready()) return;
            switch (step++)
            {
            case 0:
                image = std::make_shared<dx::Image>("test", shared_from_this());
                second = image->load("slow");
                first = image->load("fast");
                loader->gate.release();
                break;
            case 1:
                CHECK(first.value());
                CHECK(image->get<dx::Image::Source>()->pixels()[0].r == 20);
                first = second;
                break;
            case 2:
                CHECK(!first.value());
                CHECK(image->get<dx::Image::Source>()->pixels()[0].r == 20);
                first = image->load("slow");
                image->cancel_load();
                loader->gate.release();
                break;
            case 3:
                CHECK(!first.value());
                first = image->load("slow");
                image->set<dx::Image::Source>(image::Raster::rgba(1, 1, {{30, 0, 0, 255}}));
                loader->gate.release();
                break;
            case 4:
                CHECK(!first.value());
                CHECK(image->get<dx::Image::Source>()->pixels()[0].r == 30);
                first = image->load("fail");
                break;
            case 5:
                throws<std::runtime_error>([&] { first.value(); });
                CHECK(image->get<dx::Image::Source>()->pixels()[0].r == 30);
                first = image->load("null");
                break;
            case 6:
                throws<std::runtime_error>([&] { first.value(); });
                first = image->load("slow");
                image.reset();
                loader->gate.release();
                break;
            case 7:
                CHECK(!first.value());
                image = std::make_shared<dx::Image>("test", shared_from_this());
                ctx->load<Loader>("other");
                first = image->load("fast", {}, "other");
                break;
            case 8:
                CHECK(first.value());
                CHECK(image->get<dx::Image::Source>()->width() == 1);
                done = true;
                image.reset();
                ctx->stop();
                break;
            }
        }
        catch (...)
        {
            error = std::current_exception();
            loader->gate.release();
            image.reset();
            ctx->stop();
        }
    }
};
std::shared_ptr<State> state;
using TestTree = Tree<State, dx::Box, [](TreeCtx<dx::Box, State> tree)
{
    state = tree.state();
    state->update = tree.context()->listen<sys::screen::Event::Update>([weak = std::weak_ptr<State>(state)]
    { if (auto state = weak.lock()) state->tick(); });
}>;
int main()
{
    auto detached = std::make_shared<dx::Image>("detached", nullptr);
    throws<std::logic_error>([&] { detached->load("any"); });
    detached->cancel_load();
    auto ctx = IOContext::make<Input, Output, sys::screen, Loader>();
    ctx->run<NamedTree<"test", TestTree>>();
    if (state->error) std::rethrow_exception(state->error);
    CHECK(state->done);
    state.reset();
}
